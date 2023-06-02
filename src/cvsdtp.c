#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>

#include "cvsdtp.h"

/* Boilerplate Functions */

int CVSDTP_Startup(const char* dstIpAddr, const uint16_t dstPort, const uint16_t localPort, const int device, const int seed) {
	if (!InitializeCriticalSectionAndSpinCount(&gCritSec, 0x00000400))
		return 1;

	if (seed != NULL)
		gSeed = seed;

	if (device == 1) {
		gLocalDevice = PC1;
		gDstDevice = PC2;
	}
	else if (device == 2) {
		gLocalDevice = PC2;
		gDstDevice = PC1;
	}
	else {
		printf("Invalid device number");
		return 1;
	}

	if (initWinSock(dstIpAddr, dstPort, localPort) == SOCKET_ERROR)
		return 1;

	// Create threads and wait
	gReceiverThread = CreateThread(NULL, 0, CVSDTP_StartReceiverThread, NULL, 0, NULL);
	if (gReceiverThread == NULL) {
		printf("Failed to create receiver thread");
		return 1;
	}
	else
		printf("Listening on port %d...\n", localPort);

	gSenderThread = CreateThread(NULL, 0, CVSDTP_StartSenderThread, NULL, 0, NULL);
	if (gSenderThread == NULL) {
		printf("Failed to create sender thread");
		return 1;
	}
	else
		printf("Sending to %s:%d...\n", dstIpAddr, dstPort);

	// Wait for both threads to terminate, at which point initialization should be complete
	WaitForMultipleObjects(2, (const HANDLE[]){gReceiverThread, gSenderThread}, TRUE, INFINITE);

	printf("Initialized with seed %d\n", gSeed);

	printPacket(&gInitPacket);

	return 0;
}

void CVSDTP_Cleanup() {
	closesocket(gLocalSocket);
	DeleteCriticalSection(&gCritSec);
	CloseHandle(gSenderThread);
	CloseHandle(gReceiverThread);
	WSACleanup();
}

/* Boilerplate Helper Functions */

// https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/netds/iphelp/ipconfig/IPConfig.c
void getLocalIPInfo(const uint16_t localPort) {
	//DWORD Err;

	//PFIXED_INFO pFixedInfo;
	//DWORD FixedInfoSize = 0;

	//PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
	//DWORD AdapterInfoSize;
	//PIP_ADDR_STRING pAddrStr;

	//AdapterInfoSize = 0;
	//if ((Err = GetAdaptersInfo(NULL, &AdapterInfoSize)) != 0)
	//{
	//	if (Err != ERROR_BUFFER_OVERFLOW)
	//	{
	//		printf("GetAdaptersInfo sizing failed with error %d\n", Err);
	//		return;
	//	}
	//}

	//// Allocate memory from sizing information
	//if ((pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
	//{
	//	printf("Memory allocation error\n");
	//	return;
	//}

	//// Get actual adapter information
	//if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
	//{
	//	printf("GetAdaptersInfo failed with error %d\n", Err);
	//	return;
	//}

	//pAdapt = pAdapterInfo;
	//pAddrStr = &(pAdapt->IpAddressList);

	//while(pAddrStr)
	//{
	//	printf("\tIP Address. . . . . . . . . : %s\n", pAddrStr->IpAddress.String);
	//	printf("\tSubnet Mask . . . . . . . . : %s\n", pAddrStr->IpMask.String);
	//	pAddrStr = pAddrStr->Next;
	//}

	gLocalAddr.sin_family = AF_INET;
	gLocalAddr.sin_port = htons(localPort);
	inet_pton(AF_INET, "127.0.0.1", &gLocalAddr.sin_addr.s_addr);
	gLocalAddrLen = sizeof(gLocalAddr);
}

int initWinSock(const char* dstIpAddr, const uint16_t dstPort, const uint16_t localPort) {
	getLocalIPInfo(localPort);
	// Initialize WinSock
	if (WSAStartup(MAKEWORD(2, 2), &gWsaData) != NO_ERROR) {
		printf("WinSock failed to initialize; Error: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}

	// Initialize UDP socket
	gLocalSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (gLocalSocket == INVALID_SOCKET) {
		printf("Socket failed to initialize; Error: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}

	// Bind the socket
	if (bind(gLocalSocket, (SOCKADDR*)&gLocalAddr, sizeof(gLocalAddr))) {
		printf("Socket failed to bind; Error: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}

	// Initialize sockaddr_in to hold destination address info
	gDstAddr.sin_family = AF_INET;
	gDstAddr.sin_port = htons(dstPort);
	inet_pton(AF_INET, dstIpAddr, &gDstAddr.sin_addr.s_addr);
	gDstAddrLen = sizeof(gDstAddr);

	return 0;
}

int sum(uint8_t data[]) {
	int sum = 0;
	for (int i = 0; i < 4; i++)
		sum += data[i];
	return sum;
}

bool checksum(uint8_t data[]) {
	uint8_t sum = 0, i = 0;
	for (i = 0; i < 4; i++)
		sum += data[i];
	if (sum == data[i])
		return true;
	return false;
}

int byteArrToInt(uint8_t data[]) {
	int finalInt = 0;
	for (int i = 0; i < 4; i++)
		finalInt |= data[i];
	return finalInt;
}

/* CVSDPT_Packet Transfer Functions */

int CVSDTP_SendPacket(const CVSDTP_Packet* packet) {
	// Copy packet into byte array
	uint8_t dataBuffer[PACKET_SIZE];
	memcpy(dataBuffer, packet, PACKET_SIZE);

	// Send packet
	if (sendto(gLocalSocket, dataBuffer, PACKET_SIZE, 0, &gDstAddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		printf("Failed to send data to dstAddr; Error: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}

	return TRUE;
}

int CVSDTP_RecvPacket(CVSDTP_Packet* receivedPacket) {
	uint8_t dataBuffer[PACKET_SIZE];
	int bytesReceived = SOCKET_ERROR;
	bytesReceived = recvfrom(gLocalSocket, dataBuffer, PACKET_SIZE, 0, &gDstAddr, &gDstAddrLen);

	if (bytesReceived == SOCKET_ERROR) {
		printf("Failed to receive data from dstAddr; Error: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}
	memcpy(receivedPacket, dataBuffer, PACKET_SIZE);

	return TRUE;
}

DWORD WINAPI CVSDTP_StartSenderThread(LPVOID lpParam) {
	const int seedSize = sizeof(int);
	srand(gSeed);
	gSeed = rand();
	uint8_t thisSeedBytes[5];
	memcpy(thisSeedBytes, &gSeed, seedSize);
	// Checksum byte after seed
	thisSeedBytes[4] = sum(thisSeedBytes);

	/* Send initialization packets until the receiver thread
	gets an acknowledgement back from the other instance */
	while (true) {
		// Get ownership of gInitialized and check its state
		EnterCriticalSection(&gCritSec);
		if (!gInitialized) {
			LeaveCriticalSection(&gCritSec);
			// Size of seed is +1 to account for the checksum byte
			sendto(gLocalSocket, (const uint8_t*)thisSeedBytes, 4 + 1, 0, (SOCKADDR*)&gDstAddr, gDstAddrLen);
			Sleep(500);
		}
		else {
			LeaveCriticalSection(&gCritSec);
			break;
		}
	}

	CVSDTP_Packet initState = CVSDTP_CreateRandomState(INITIALIZATION, MAX_TARGETS, MAX_DISTRACTORS, false);
	memcpy(&gInitPacket, &initState, PACKET_SIZE);

	CVSDTP_SendPacket(&gInitPacket);

	return TRUE;
}

DWORD WINAPI CVSDTP_StartReceiverThread(LPVOID lpParam) {
	// Receive seed from other instance
	uint8_t dataBuffer[5];

	int bytesReceived = 0;
	// Wait for the checksum to succeed
	do {
		bytesReceived = recvfrom(gLocalSocket, dataBuffer, 4 + 1, 0, (SOCKADDR*)&gDstAddr, &gDstAddrLen);
	} while (!checksum(dataBuffer));

	int otherSeed = byteArrToInt(dataBuffer);
	printf("Received seed: %d\n", otherSeed);

	if (CVSDTP_RecvPacket(&gInitPacket) == SOCKET_ERROR)
		return SOCKET_ERROR;

	// Get ownership of gInitialized and set its state
	EnterCriticalSection(&gCritSec);
	gInitialized = true;
	LeaveCriticalSection(&gCritSec);

	return TRUE;
}

/* Object Creation Functions */

Target CVSDTP_CreateTarget(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color) {
	return (Target){.coords = coords, .shape = shape, .rotation = rotation, .color = color, .found = false};
}

Distractor CVSDTP_CreateDistractor(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color) {
	return (Distractor){.coords = coords, .shape = shape, .rotation = rotation, .color = color};
}

CVSDTP_Packet CVSDTP_CreateRandomState(CVSDTP_PacketType packetType, int numTargets, int numDistractors, bool sameDistractors) {
	TargetState targetState = { .numTargets = numTargets };
	DistractorState distractorState = { .numDistractors = numDistractors };

	Target target = CVSDTP_CreateTarget(getRandomCoords(),
		getRandomShape(), getRandomRotation(), getRandomColor());

	// Create randomly placed Targets
	for (int i = 0; i < numTargets; i++) {
		target.coords = getRandomCoords();
		targetState.targets[i] = target;
	}

	/* sameDistractors determines whether the same Distractor will
	just be randomly placed, or if completely random Distractors
	will be created */
	if (sameDistractors) {
		Distractor distractor = CVSDTP_CreateDistractor(getRandomCoords(),
			getRandomShape(), getRandomRotation(), getRandomColor());
		// Create randomly placed distractors
		for (int i = 0; i < numDistractors; i++) {
			distractor.coords = getRandomCoords();
			distractorState.distractors[i] = distractor;
		}
	}
	else {
		// Create random Distractors
		for (int i = 0; i < numDistractors; i++) {
			distractorState.distractors[i] = CVSDTP_CreateDistractor(getRandomCoords(),
				getRandomShape(), getRandomRotation(), getRandomColor());
		}
	}

	return createPacket(packetType, &target, &targetState, &distractorState, gLocalDevice, gDstDevice);
}

/* CVSDPT_Packet Creation Helper Functions */

CVSDTP_Packet createPacket(CVSDTP_PacketType packetType, const Target* target, const TargetState* targetState,
						   const DistractorState* distractorState, Device sender, Device receiver) {
	CVSDTP_Packet packet;
	switch (packetType) {
	case INITIALIZATION: writeInitializationPacketData(&packet, targetState, distractorState); break;
	case UPDATE:		 writeUpdatePacketData(&packet, target, targetState);				   break;
	case COMPLETE:
	default:
		break;
	}
	packet.type = packetType;
	packet.sender = sender;
	packet.receiver = receiver;

	return packet;
}

void writeInitializationPacketData(CVSDTP_Packet* packet, const TargetState* targetState,
								   const DistractorState* distractorState) {
	// Copy TargetState to the initialization packet
	memcpy(packet->targetState.targets, targetState->targets, targetState->numTargets);
	packet->targetState.numTargets = targetState->numTargets;

	// Copy DistractorState to the initialization packet
	memcpy(packet->distractorState.distractors, distractorState->distractors, distractorState->numDistractors);
	packet->distractorState.numDistractors = distractorState->numDistractors;
}

void writeUpdatePacketData(CVSDTP_Packet* packet, const Target* target,
						   const TargetState* targetState) {
	// Copy TargetState to the update packet
	memcpy(packet->targetState.targets, targetState->targets, targetState->numTargets);
	packet->targetState.numTargets = targetState->numTargets;
	// Copy Target to the update packet
	packet->target = *target;
}

/* thanks https://stackoverflow.com/a/1202706 */

/* Generate random coordinates in range xBounds, yBounds */
Vec2 getRandomCoords() {
	Vec2 coords;
	coords.x = rand() % (COORD_BOUNDS.y + 1 - COORD_BOUNDS.x) + COORD_BOUNDS.x;
	coords.y = rand() % (COORD_BOUNDS.y + 1 - COORD_BOUNDS.x) + COORD_BOUNDS.x;
	return coords;
}

ObjectShape getRandomShape() {
	return rand() % (SHAPE_BOUNDS.y + 1 - SHAPE_BOUNDS.x) + SHAPE_BOUNDS.x;
}

ObjectRotation getRandomRotation() {
	return rand() % (ROTATION_BOUNDS.y + 1 - ROTATION_BOUNDS.x) + ROTATION_BOUNDS.x;
}

ObjectColor getRandomColor() {
	return rand() % (COLOR_BOUNDS.y + 1 - COLOR_BOUNDS.x) + COLOR_BOUNDS.x;
}

void printPacket(const CVSDTP_Packet* packet) {
	if (packet->type == INITIALIZATION) {
		printf("Type: INITIALIZATION\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(((uint8_t*)packet)[0]));
		printf("numTargets: %d\nnumDistractors: %d\n", packet->targetState.numTargets,
			packet->distractorState.numDistractors);
	}
	else if (packet->type == UPDATE) {
		printf("Type: UPDATE\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(((uint8_t*)packet)[0]));
		printf("target: (%d, %d)\n", packet->target.coords.x, packet->target.coords.y);
		printf("numTargets: %d\n", packet->targetState.numTargets);
	}
	else if (packet->type == INITIALIZED) {
		printf("Type: INITIALIZED\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(((uint8_t*)packet)[0]));
	}
	else {
		printf("Type: COMPLETE\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(((uint8_t*)packet)[0]));
	}
}

int main(int argc, char* argv[]) {
	if (argc == 6) {
		if (CVSDTP_Startup(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5])) == SOCKET_ERROR)
			return 1;
	}
	else if (argc == 5) {
		if (CVSDTP_Startup(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), NULL) == SOCKET_ERROR)
			return 1;
	}
	else {
		printf("usage:\n    <destination IP address> <destination port> <local port> <device id [1|2]> [optional rng seed]");
		return 0;
	}
	CVSDTP_Cleanup();
	return 0;
}