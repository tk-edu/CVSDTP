#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <WinSock2.h>
#include <Ws2tcpip.h>
//#include <iphlpapi.h>
#include <Windows.h>

#include "cvsdtp.h"

/* Global Variables */

static SOCKET gLocalSocket = INVALID_SOCKET;

static struct sockaddr_in gDstAddr;
static int gDstAddrLen;

static struct sockaddr_in gLocalAddr;
static int gLocalAddrLen;

static bool isInitialized = false;
static CVSDTP_Packet gInitPacket;
static uint32_t gSeed;

static WSADATA gWsaData;

static HANDLE gSenderThread;
static HANDLE gReceiverThread;

/* Main API Helper Functions */

// https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/netds/iphelp/ipconfig/IPConfig.c
static void getLocalIPInfo(const uint16_t localPort) {
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

static int initWinSock(const char* dstIpAddr, const uint16_t dstPort) {
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
	//const uint16_t dstPort = 4567; // see wsa_testing_server.c
	gDstAddr.sin_family = AF_INET;
	gDstAddr.sin_port = htons(dstPort);
	inet_pton(AF_INET, dstIpAddr, &gDstAddr.sin_addr.s_addr);
	gDstAddrLen = sizeof(gDstAddr);

	return 0;
}

static CVSDTP_Packet createCVSDTP_State() {
	CVSDTP_Packet packet = {.type = INITIALIZATION};

	// Create random Targets
	for (int i = 0; i < MAX_TARGETS; i++) {
		packet.targetState.targets[i] = createTarget(getRandomCoords(COORD_BOUNDS, COORD_BOUNDS),
			getRandomShape(), getRandomRotation(), getRandomColor());
	}
	packet.targetState.numTargets = MAX_TARGETS;

	// Create random Distractors
	for (int i = 0; i < MAX_DISTRACTORS; i++) {
		packet.distractorState.distractors[i] = createDistractor(getRandomCoords(COORD_BOUNDS, COORD_BOUNDS),
			getRandomShape(), getRandomRotation(), getRandomColor());
	}
	packet.distractorState.numDistractors = MAX_DISTRACTORS;

	return packet;
}

static int sum(uint8_t data[]) {
	int sum = 0;
	for (int i = 0; i < 4; i++)
		sum += data[i];
	return sum;
}

static bool checksum(uint8_t data[]) {
	int sum = 0, i = 0;
	for (i = 0; i < 4; i++) {
		sum += data[i];
		printf("byte %d: %d\n", i, data[i]);
	}
	if (sum == data[i])
		return true;
	return false;
}

/* Main API Functions */

int CVSDTP_Startup(const char* dstIpAddr, const uint16_t dstPort, const uint16_t localPort, uint32_t seed) {
	if (seed != NULL)
		srand(seed);
	CVSDTP_Packet randState = createCVSDTP_State();
	memcpy(&gInitPacket, &randState, PACKET_SIZE);

	getLocalIPInfo(localPort);

	int winSockRes = initWinSock(dstIpAddr, dstPort);
	if (winSockRes == SOCKET_ERROR)
		return 1;

	// Create threads and wait
	gReceiverThread = CreateThread(NULL, 0, CVSDTP_StartReceiverThread, NULL, 0, NULL);
	if (gReceiverThread == NULL) 
	{
		printf("Failed to create receiver thread");
		ExitProcess(3);
	}
	else
		printf("Listening on port %d...\n\n", localPort);

	gSenderThread = CreateThread(NULL, 0, CVSDTP_StartSenderThread, NULL, 0, NULL);
	if (gSenderThread == NULL)
	{
		printf("Failed to create sender thread");
		ExitProcess(3);
	}
	else
		printf("Sending initialization packets to %s:%d...\n", dstIpAddr, dstPort);

	WaitForMultipleObjects(2, (const HANDLE[]){gSenderThread, gReceiverThread}, TRUE, INFINITE);

	printf("Initialized with seed %d", gSeed);

	return winSockRes;
}

void CVSDTP_Cleanup() {
	closesocket(gLocalSocket);
	CloseHandle(gSenderThread);
	CloseHandle(gReceiverThread);
	WSACleanup();
}

DWORD WINAPI CVSDTP_StartSenderThread() {
	uint32_t thisSeed = rand();
	uint8_t thisSeedBytes[5];
	memcpy(thisSeedBytes, &thisSeed, sizeof(uint32_t));
	// Checksum
	thisSeedBytes[4] = sum(thisSeedBytes);
	printf("Press enter to send an initialization packet\n");
	uint8_t dummyBuf[1];

	/* Send initialization packets until the receiver thread
	gets an acknowledgement back from the other instance */
	while (!isInitialized) {
		gets(dummyBuf);
		sendto(gLocalSocket, (const uint8_t*)thisSeedBytes, 4 + 1, 0, (SOCKADDR*)&gDstAddr, gDstAddrLen);
	}

	// Tell other instance that we are initialized
	memset(thisSeedBytes, 0, sizeof(uint32_t));
	sendto(gLocalSocket, thisSeedBytes, sizeof(uint32_t) + 1, 0, (SOCKADDR*)&gDstAddr, gDstAddrLen);

	CVSDTP_Packet initializedPacket = {.type = INITIALIZED};
	uint8_t initPacketBytes[DATA_BUF_LEN];
	memcpy(initPacketBytes, &initializedPacket, PACKET_SIZE);

	if (sendCVSDTP_Packet((const SOCKET*)&gLocalSocket, (const SOCKADDR*)&gDstAddr,
		(const CVSDTP_Packet*)&initializedPacket) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	return 0;
}

void CVSDTP_StopSenderThread() {

}

DWORD WINAPI CVSDTP_StartReceiverThread() {
	// Receive seed from other instance
	uint32_t otherSeed;

	// Wait to receive a packet from the receiver instance
	uint8_t dataBuffer[UDP_PACKET_LEN];
	int bytesReceived = 0;
	// Wait for the checksum to succeed
	do {
		bytesReceived = recvfrom(gLocalSocket, dataBuffer, sizeof(uint32_t) + 1, 0, (SOCKADDR*)&gDstAddr, &gDstAddrLen);
		for (int i = 0; i < 5; i++)
			printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(dataBuffer[i]));
	} while (!checksum(dataBuffer));

	memcpy(&otherSeed, dataBuffer, sizeof(uint32_t));

	if (recvCVSDTP_Packet((const SOCKET*)&gLocalSocket, &dataBuffer,
		(const SOCKADDR*)&gDstAddr, &gDstAddrLen, &gInitPacket) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	/* If they win, set our seed to their seed.
	If we win, set our seed to our seed */
	if (gInitPacket.type == INITIALIZED)
		srand(otherSeed);
	else {
		srand(gSeed);
		CVSDTP_Packet randState = createCVSDTP_State();
		memcpy(&gInitPacket, &randState, PACKET_SIZE);
	}

	isInitialized = true;

	return 0;
}

void CVSDTP_StopReceiverThread() {

}

/* Packet Creation Helper Functions */

static void writeInitializationPacketData(CVSDTP_Packet* packet, const TargetState* targetState,
										  const DistractorState* distractorState) {
	// Copy TargetState to the initialization packet
	memcpy(packet->targetState.targets, targetState->targets, targetState->numTargets);
	packet->targetState.numTargets = targetState->numTargets;

	// Copy DistractorState to the initialization packet
	memcpy(packet->distractorState.distractors, distractorState->distractors, distractorState->numDistractors);
	packet->distractorState.numDistractors = distractorState->numDistractors;
}

static void writeUpdatePacketData(CVSDTP_Packet* packet, const Target* target,
								  const TargetState* targetState) {
	// Copy TargetState to the update packet
	memcpy(packet->targetState.targets, targetState->targets, targetState->numTargets);
	packet->targetState.numTargets = targetState->numTargets;
	// Copy Target to the update packet
	packet->target = *target;
}

/* CVSDTP_Packet Creation Functions */

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

/* CVSDTP_Packet Transfer Functions */

int sendCVSDTP_Packet(const SOCKET* socket, const SOCKADDR* dstAddr, const CVSDTP_Packet* packet) {
	// Copy packet into byte array
	uint8_t dataBuffer[DATA_BUF_LEN];
	memcpy(dataBuffer, &packet, PACKET_SIZE);

	// Send packet
	if (sendto(socket, dataBuffer, PACKET_SIZE, 0, &dstAddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		printf("Failed to send data to theirAddr; Error: %d", WSAGetLastError());
		return SOCKET_ERROR; // should i be returning this?
	}
	return 0;
}

int recvCVSDTP_Packet(const SOCKET* socket, uint8_t dataBuffer[], const SOCKADDR* srcAddr,
								const int* srcAddrLen, const CVSDTP_Packet* receivedPacket) {
	int bytesReceived = SOCKET_ERROR;
	bytesReceived = recvfrom(socket, dataBuffer, UDP_PACKET_LEN - 1, 0, &srcAddr, &srcAddrLen);

	if (bytesReceived == SOCKET_ERROR) {
		//printf("Failed to receive data from theirAddr; Error: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}
	dataBuffer[bytesReceived] = '\0';

	memcpy(receivedPacket, dataBuffer, DATA_BUF_LEN);

	return 0;
}

void printCVSDTP_Packet(const CVSDTP_Packet* packet) {
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
	else {
		printf("Type: COMPLETE\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(((uint8_t*)packet)[0]));
	}
}

/* Object Creation Functions */

Target createTarget(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color) {
	return (Target){.coords = coords, .shape = shape, .rotation = rotation, .color = color, .found = false};
}

Distractor createDistractor(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color) {
	return (Distractor){.coords = coords, .shape = shape, .rotation = rotation, .color = color};
}

/* thanks https://stackoverflow.com/a/1202706 */

/* Generate random coordinates in range xBounds,
yBounds (set seed to NULL to omit seeding) */
Vec2 getRandomCoords(Vec2 xBounds, Vec2 yBounds) {
	Vec2 coords;
	coords.x = rand() % (xBounds.y + 1 - xBounds.x) + xBounds.x;
	coords.y = rand() % (yBounds.y + 1 - yBounds.x) + yBounds.x;

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

int main(int argc, char* argv[]) {
	if (argc == 5) {
		if (CVSDTP_Startup(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4])) == SOCKET_ERROR)
			return 1;
	}
	else if (argc == 4) {
		if (CVSDTP_Startup(argv[1], atoi(argv[2]), atoi(argv[3]), NULL) == SOCKET_ERROR)
			return 1;
	}
	else {
		printf("usage:\n\t<destination IP address> <destination port> <local port> [optional rng seed]");
		return 0;
	}

	CVSDTP_Cleanup();
	return 0;
}