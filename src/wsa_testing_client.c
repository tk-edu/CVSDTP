///*
//* thanks https://gist.github.com/sunmeat/02b60c8a3eaef3b8a0fb3c249d8686fd
//* 
//* wireshark filter: udp && !lsd && !icmp && !mdns && !ssdp
//*/
//
//#include <stdlib.h>
//#include <string.h>
//#include <stdint.h>
//#include <stdio.h>
//
//#include <WinSock2.h>
//#include <Ws2tcpip.h>
//
//#include "cvsdtp.h"
//
//int main() {
//	// Initialize WinSock
//	WSADATA wsaData;
//	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
//		printf("WinSock failed to initialize; Error: %d", WSAGetLastError());
//		return 1;
//	}
//
//	// Create UDP socket
//	SOCKET mySocket = INVALID_SOCKET;
//	mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	if (mySocket == INVALID_SOCKET) {
//		printf("Failed to initialize mySocket; Error: %d", WSAGetLastError());
//		WSACleanup();
//		return 1;
//	}
//
//	// Create sockaddr_in to hold destination address info
//	struct sockaddr_in dstAddr;
//	const uint16_t dstPort = 4567; // see wsa_testing_server.c
//	dstAddr.sin_family = AF_INET;
//	dstAddr.sin_port = htons(dstPort);
//	inet_pton(AF_INET, "127.0.0.1", &dstAddr.sin_addr.s_addr);
//	int dstAddrLen = sizeof(dstAddr);
//
//	Vec2 targetCoords = getRandomCoords((Vec2){0, 1000}, (Vec2){0, 1000}, 7);
//	Target target = createTarget(targetCoords, L, UP, RED);
//
//	TargetState targetState = {.numTargets = 1, .targets = {target}};
//
//	Vec2 distractorCoords = getRandomCoords((Vec2){0, 1000}, (Vec2){0, 1000}, NULL);
//	Distractor distractor = createDistractor(distractorCoords, T, RIGHT, PINK);
//
//	DistractorState distractorState = {.numDistractors = 1, .distractors = {distractor}};
//
//	CVSDTP_Packet initPacket = createInitPacket((const TargetState*)&targetState,
//		(const DistractorState*)&distractorState, PC1, PC2);
//	CVSDTP_Packet updatePacket = createUpdatePacket((const Target*)&target,
//		(const TargetState*)&targetState, PC1, PC2);
//	CVSDTP_Packet completePacket = createCompletionPacket(PC2, PC1);
//	
//	uint8_t initPacketBytes[DATA_BUF_LEN];
//	uint8_t updatePacketBytes[DATA_BUF_LEN];
//	uint8_t completePacketBytes[DATA_BUF_LEN];
//	memcpy(initPacketBytes, &initPacket, PACKET_SIZE);
//	memcpy(updatePacketBytes, &updatePacket, PACKET_SIZE);
//	memcpy(completePacketBytes, &completePacket, PACKET_SIZE);
//
//	printf("initPacketBytes: %d\nupdatePacketBytes: %d\ncompletePacketBytes: %d\n", sizeof(initPacketBytes),
//		sizeof(updatePacketBytes), sizeof(completePacketBytes));
//
//	// Send data to dstAddr
//	/*if (sendto(mySocket, initPacketBytes, sizeof(initPacketBytes), 0, (SOCKADDR*)&dstAddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
//		printf("Failed to send data to dstAddr; Error: %d", WSAGetLastError());
//	else
//		printf("Sent "BYTE_TO_BINARY_PATTERN" to dstAddr\n", BYTE_TO_BINARY(initPacket.header));
//	if (sendto(mySocket, updatePacketBytes, sizeof(updatePacketBytes), 0, (SOCKADDR*)&dstAddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
//		printf("Failed to send data to dstAddr; Error: %d", WSAGetLastError());
//	else
//		printf("Sent "BYTE_TO_BINARY_PATTERN" to dstAddr\n", BYTE_TO_BINARY(updatePacket.header));
//	if (sendto(mySocket, completePacketBytes, sizeof(completePacketBytes), 0, (SOCKADDR*)&dstAddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
//		printf("Failed to send data to dstAddr; Error: %d", WSAGetLastError());
//	else
//		printf("Sent "BYTE_TO_BINARY_PATTERN" to dstAddr\n", BYTE_TO_BINARY(completePacket.header));
//
//	uint8_t* packets[3] = {initPacketBytes, updatePacketBytes, completePacketBytes};*/
//
//	while (true) {
//		uint8_t myBuf[DATA_BUF_LEN];
//		memset(myBuf, '\0', DATA_BUF_LEN);
//		gets(myBuf);
//
//		sendCVSDPT_Packet((const SOCKET*)&socket, &myBuf, (const SOCKADDR*)&dstAddr, (const CVSDTP_Packet*)&packet);
//
//		if (myBuf[0] == 'z') {
//			printf("Closing socket...");
//			break;
//		}
//	}
//	closesocket(mySocket);
//	WSACleanup();
//	return 0;
//}