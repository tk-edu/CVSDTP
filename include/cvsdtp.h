#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
Maybe enable -fshort-enums gcc flag to
see if it speeds things up enough,
considering this data goes over a wire
*/

#define MAX_TARGETS     16
#define MAX_DISTRACTORS 32

/*
* Maybe in the end it'll make more sense to
* eat the extra bytes and just send the data
* these constants represent over the wire. If
* the extra time dedicated to doing logic on
* them is slowing things down, maybe try that.
*/

typedef enum {
    TRIANGLE,
    SQUARE,
    CIRCLE,
    T,
	L
} ObjectShape;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} ObjectRotation;

typedef enum {
    RED,
    GREEN,
    BLUE,
	PINK,
    ORANGE
} ObjectColor;

typedef enum {
    INITIALIZATION,
    UPDATE,
    UNUSED,
    COMPLETE
} CVSDTP_PacketType;

typedef enum {
    PC1,
    PC2
} Device;

#pragma pack(push, 1)

// 22 bits
typedef struct {
    /* 11-bit uints store up to 2048,
    which is enough for our coord system */
    uint32_t x : 11;
	uint32_t y : 11;
} Vec2;

// 29 bits
typedef struct {
    Vec2 coords;
    ObjectShape shape : 2;
    ObjectColor color : 2;
    ObjectRotation rotation : 2;
    bool found : 1;
} Target;

// 28 bits
typedef struct {
    Vec2 coords;
    ObjectShape shape : 2;
    ObjectColor color : 2;
    ObjectRotation rotation : 2;
} Distractor;

typedef struct {
	uint8_t numTargets : 7;
	Target targets[MAX_TARGETS];
} TargetState;

typedef struct {
	uint8_t numDistractors : 7;
	Distractor distractors[MAX_DISTRACTORS];
} DistractorState;

#pragma pack(pop)

// CVSDPT_Packets

typedef struct {
	uint8_t header : 4;
	TargetState targetState;
	DistractorState distractorState;
} CVSDTP_InitializationPacket;

typedef struct {
	uint8_t header : 4;
	Target target;
	TargetState targetState;
} CVSDTP_UpdatePacket;

typedef struct {
	uint8_t header : 4;
} CVSDTP_CompletionPacket;

typedef union {
	CVSDTP_InitializationPacket initPacket;
	CVSDTP_UpdatePacket updatePacket;
	CVSDTP_CompletionPacket completePacket;
} CVSDTP_Packet;

// CVSDPT_Packet creation functions

CVSDTP_InitializationPacket createInitPacket(const TargetState* targetState, const DistractorState* distractorState, 
								Device sender, Device receiver);
CVSDTP_UpdatePacket		    createUpdatePacket(const Target* target, const TargetState* targetState,
								Device sender, Device receiver);
CVSDTP_CompletionPacket		createCompletionPacket(Device sender, Device receiver);

void writeInitializationPacketData(CVSDTP_InitializationPacket* packet, const TargetState* targetState,
	const DistractorState* distractorState);
void writeUpdatePacketData(CVSDTP_UpdatePacket* packet, const Target* target,
	const TargetState* targetState);

int sendPacket(CVSDTP_Packet packet);
int recvPacket();

void printCVSDTP_Packet(const CVSDTP_Packet* packet, CVSDTP_PacketType packetType);

// Target and Distractor creation functions

Target createTarget(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color);
Distractor createDistractor(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color);

Vec2 getRandomCoords(Vec2 xBounds, Vec2 yBounds, uint32_t seed);