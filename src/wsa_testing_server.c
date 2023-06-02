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
//	// Define socket info
//	struct sockaddr_in myAddr;
//	const uint16_t myPort = 4567;
//	myAddr.sin_family = AF_INET;
//	myAddr.sin_port = htons(myPort);
//	inet_pton(AF_INET, "127.0.0.1", &myAddr.sin_addr.s_addr);
//
//	// Bind the socket
//	if (bind(mySocket, (SOCKADDR*)&myAddr, sizeof(myAddr))) {
//		printf("Failed to bind myAddr to mySocket; Error: %d", WSAGetLastError());
//		closesocket(mySocket);
//		WSACleanup();
//		return 1;
//	}
//
//	int bytesReceived = 0;
//	uint8_t myBuf[UDP_PACKET_LEN];
//
//	struct sockaddr_in theirAddr;
//	int theirAddrLen = sizeof(theirAddr);
//	int isFirst = 1;
//
//	// Receive data from their socket
//	printf("Listening on port %d...\n\n", myPort);
//	while (true) {
//		bytesReceived = recvfrom(mySocket, myBuf, UDP_PACKET_LEN - 1, 0, (SOCKADDR*)&theirAddr, &theirAddrLen);
//
//		if (bytesReceived == SOCKET_ERROR) {
//			printf("Failed to receive data from theirAddr; Error: %d", WSAGetLastError());
//			closesocket(mySocket);
//			WSACleanup();
//			return 1;
//		}
//		myBuf[bytesReceived + 1] = '\0';
//
//		if (myBuf[0] == 'z') {
//			printf("Closing socket...");
//			break;
//		}
//		CVSDTP_Packet packet;
//		memcpy(&packet, myBuf, DATA_BUF_LEN);
//
//		switch (packet.type) {
//			case INITIALIZATION: packet.packetContainer.initPacket = receiveInitPacket()
//		}
//		CVSDTP_PacketType packetType;
//		uint8_t packetHeader = myBuf[0];
//
//		if (IS_INIT(packetHeader)) {
//			packet.initPacket = receiveInitPacket(myBuf);
//			packetType = INITIALIZATION;
//		}
//		else if (IS_UPDATE(packetHeader)) {
//			packet.updatePacket = receiveUpdatePacket(myBuf);
//			packetType = UPDATE;
//		}
//		else if (IS_COMPLETE(packetHeader)) {
//			packet.completePacket = receiveCompletionPacket(myBuf);
//			packetType = COMPLETE;
//		}
//
//		printPacket(&packet, packetType);
//		printf("\n");
//	}
//	closesocket(mySocket);
//	WSACleanup();
//	return 0;
//}