#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "cvsdtp.h"

// https://stackoverflow.com/a/3208376
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)   \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

//void writePacketHeader(CVSDTP_Packet* packet, CVSDTP_PacketType packetType, Device sender, Device receiver) {
//	uint8_t flags = 0;
//
//	// Set packetType bits
//	switch (packetType) {
//		case INITIALIZATION: flags |= INITIALIZATION; break;
//		case UPDATE		   : flags |= UPDATE;		  break;
//		case COMPLETE	   : flags |= COMPLETE;		  break;
//		default			   : break;
//	}
//	// Set device identity bits
//	flags |= sender << 2;
//	flags |= receiver << 3;
//
//	packet->header = flags;
//}

/* hey asshole, you should consider using
cpp for this so that you can use polymorphism
for the createPacket function and be slay */

inline void setDeviceIdentityBits(uint8_t* header, Device sender, Device receiver) {
	*header |= sender << 2;
	*header |= receiver << 3;
}

CVSDTP_InitializationPacket createInitPacket(const TargetState* targetState,
											 const DistractorState* distractorState,
											 Device sender, Device receiver) {
	CVSDTP_InitializationPacket packet;
	uint8_t header = 0;

	header |= INITIALIZATION;
	setDeviceIdentityBits(&header, sender, receiver);
	packet.header = header;

	writeInitializationPacketData(&packet, targetState, distractorState);

	return packet;
}

CVSDTP_UpdatePacket createUpdatePacket(const Target* target, const TargetState* targetState,
									   Device sender, Device receiver) {
	CVSDTP_UpdatePacket packet;
	uint8_t header = 0;

	header |= UPDATE;
	setDeviceIdentityBits(&header, sender, receiver);
	packet.header = header;

	writeUpdatePacketData(&packet, target, targetState);

	return packet;
}

CVSDTP_CompletionPacket createCompletionPacket(Device sender, Device receiver) {
	CVSDTP_CompletionPacket packet;
	uint8_t header = 0;

	header |= COMPLETE;
	setDeviceIdentityBits(&header, sender, receiver);
	packet.header = header;

	return packet;
}

void writeInitializationPacketData(CVSDTP_InitializationPacket* packet, const TargetState* targetState,
								   const DistractorState* distractorState) {
	int i = 0;
	// Copy TargetState to the InitializationPacket
	for (i = 0; i < targetState->numTargets; i++) {
		packet->targetState.targets[i] = targetState->targets[i];
	}
	packet->targetState.numTargets = targetState->numTargets;

	// Copy DistractorState to the InitializationPacket
	for (i = 0; i < distractorState->numDistractors; i++) {
		packet->distractorState.distractors[i] = distractorState->distractors[i];
	}
	packet->distractorState.numDistractors = distractorState->numDistractors;
}

void writeUpdatePacketData(CVSDTP_UpdatePacket* packet, const Target* target,
						   const TargetState* targetState) {
	// Copy TargetState to the UpdatePacket
	for (int i = 0; i < targetState->numTargets; i++) {
		packet->targetState.targets[i] = targetState->targets[i];
	}
	packet->targetState.numTargets = targetState->numTargets;

	// Copy Target to the UpdatePacket
	packet->target = *target;
}

int sendPacket(CVSDTP_Packet packet) {
	;
}

int recvPacket() {
	;
}

void printCVSDTP_Packet(const CVSDTP_Packet* packet, CVSDTP_PacketType packetType) {
	if (packetType == INITIALIZATION) {
		printf("packetType: INITIALIZATION\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(packet->initPacket.header));
		printf("numTargets: %d\nnumDistractors: %d\n", packet->initPacket.targetState.numTargets,
			packet->initPacket.distractorState.numDistractors);
	}
	else if (packetType == UPDATE) {
		printf("packetType: UPDATE\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(packet->updatePacket.header));
		printf("target: (%d, %d)\n", packet->updatePacket.target.coords.x, packet->updatePacket.target.coords.y);
		printf("numTargets: %d\n", packet->updatePacket.targetState.numTargets);
	}
	else {
		printf("packetType: COMPLETE\n");
		printf("Header: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(packet->completePacket.header));
	}
}

Target createTarget(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color) {
	return (Target){.coords = coords, .shape = shape, .rotation = rotation, .color = color, .found = false};
}

Distractor createDistractor(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color) {
	return (Distractor){.coords = coords, .shape = shape, .rotation = rotation, .color = color};
}

/* Generate random coordinates in range xBounds,
yBounds (set seed to NULL to omit seeding) */
Vec2 getRandomCoords(Vec2 xBounds, Vec2 yBounds, uint32_t seed) {
	Vec2 coords;

	if (seed != NULL)
		srand(seed);

	// https://stackoverflow.com/a/1202706
	coords.x = rand() % (xBounds.y + 1 - xBounds.x) + xBounds.x;
	coords.y = rand() % (yBounds.y + 1 - yBounds.x) + yBounds.x;

	return coords;
}

int main() {
	//CVSDTP_Packet packet = createPacket(INITIALIZATION, PC1, PC2);
	Vec2 targetCoords = getRandomCoords((Vec2){0, 1000}, (Vec2){0, 1000}, 7);
	Target target = createTarget(targetCoords, L, UP, RED);

	TargetState targetState = {.numTargets = 1, .targets = {target}};

	Vec2 distractorCoords = getRandomCoords((Vec2){0, 1000}, (Vec2){0, 1000}, NULL);
	Distractor distractor = createDistractor(distractorCoords, T, RIGHT, PINK);

	DistractorState distractorState = {.numDistractors = 1, .distractors = {distractor}};

	CVSDTP_InitializationPacket initPacket = createInitPacket((const TargetState*)&targetState,
		(const DistractorState*)&distractorState, PC1, PC2);
	CVSDTP_UpdatePacket updatePacket = createUpdatePacket((const Target*)&target,
		(const TargetState*)&targetState, PC1, PC2);
	CVSDTP_CompletionPacket completePacket = createCompletionPacket(PC2, PC1);

	printCVSDTP_Packet((const CVSDTP_Packet*)&initPacket, INITIALIZATION);
	printf("\n");
	printCVSDTP_Packet((const CVSDTP_Packet*)&updatePacket, UPDATE);
	printf("\n");
	printCVSDTP_Packet((const CVSDTP_Packet*)&completePacket, COMPLETE);

	return 0;
}