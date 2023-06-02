#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <WinSock2.h>
#include <WS2tcpip.h>

#define MAX_TARGETS         16
#define MAX_DISTRACTORS     32

#define COORD_BOUNDS 		(Vec2){0, 2048}
#define SHAPE_BOUNDS		(Vec2){0,	 4}
#define ROTATION_BOUNDS		(Vec2){0,	 3}
#define COLOR_BOUNDS		(Vec2){0,	 4}

// sizeof CVSDTP_Packet
#define PACKET_SIZE			416

// Checks if a given bit is set
#define IS_SET(var, offset) (var >> offset) & 1
// Determines packetType by header bits (checks the
// first 2 bits; see 'Communication Standard.md')
#define IS_INIT(header)	       ~IS_SET(header, 1) && ~IS_SET(header, 0)
#define IS_UPDATE(header)      ~IS_SET(header, 1) &&  IS_SET(header, 0)
#define IS_INITIALIZED(header)  IS_SET(header, 1) && ~IS_SET(header, 0)
#define IS_COMPLETE(header)		IS_SET(header, 1) &&  IS_SET(header, 0)

#define EXPORT				__declspec(dllimport)

// For printing binary numbers
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

/* Object Properties */

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
    PC1,
    PC2
} Device;

#pragma pack(push, 1)

/* Objects */

// 22 bits
typedef struct {
    /* 11-bit uints store up to 2048,
    which is enough for our coord system */
    uint32_t x : 11;
	uint32_t y : 11;
} Vec2;

// 31 bits
typedef struct {
    Vec2 coords;
    ObjectShape shape : 3;
    ObjectColor color : 3;
    ObjectRotation rotation : 2;
    bool found : 1;
} Target;

// 30 bits
typedef struct {
    Vec2 coords;
    ObjectShape shape : 3;
    ObjectColor color : 3;
    ObjectRotation rotation : 2;
} Distractor;

/* ObjectStates */

typedef struct {
	uint8_t numTargets : 7;
	Target targets[MAX_TARGETS];
} TargetState;

typedef struct {
	uint8_t numDistractors : 7;
	Distractor distractors[MAX_DISTRACTORS];
} DistractorState;

#pragma pack(pop)

/* CVSDPT_Packets */

typedef enum {
	INITIALIZATION,
	UPDATE,
	INITIALIZED,
	COMPLETE
} CVSDTP_PacketType;

typedef struct {
	CVSDTP_PacketType type : 2;
	Device sender : 1;
	Device receiver : 1;
	Target target;
	TargetState targetState;
	DistractorState distractorState;
} CVSDTP_Packet;

/* Boilerplate Functions */

/*EXPORT*/ int CVSDTP_Startup(const char* dstIpAddr, const uint16_t dstPort, const uint16_t localPort, int seed);
/*EXPORT*/ void CVSDTP_Cleanup();
/*EXPORT*/ DWORD WINAPI CVSDTP_StartSenderThread();
/*EXPORT*/ DWORD WINAPI CVSDTP_StartReceiverThread();

/* CVSDPT_Packet Transfer Functions */

/*EXPORT*/ int CVSDTP_SendPacket(const CVSDTP_Packet* packet);
/*EXPORT*/ int CVSDTP_RecvPacket(CVSDTP_Packet* receivedPacket);

/* Object Creation Functions */

/*EXPORT*/ Target CVSDTP_CreateTarget(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color);
/*EXPORT*/ Distractor CVSDTP_CreateDistractor(Vec2 coords, ObjectShape shape, ObjectRotation rotation, ObjectColor color);
/*EXPORT*/ CVSDTP_Packet CVSDTP_CreateRandomState(int numTargets, int numDistractors);

// TODO: move this somewhere more appropriate
Vec2 getRandomCoords();
void printCVSDTP_Packet(const CVSDTP_Packet* packet);

#ifdef __cplusplus
}
#endif