#pragma once

#include <stdbool.h>

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
    T
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
    ORANGE
} ObjectColor;

#pragma pack(push, 1)

// 22 bits
typedef struct {
    /* 11-bit uints store up to 2048,
    which is enough for our coord system */
    unsigned int x : 11;
    unsigned int y : 11;
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

#pragma pack(pop)

typedef struct {
    unsigned char numTargets : 7;
    Target targets[MAX_TARGETS];
} TargetState;

typedef struct {
    unsigned char numDistractors : 7;
    Distractor distractors[MAX_DISTRACTORS];
} DistractorState;

typedef struct {
    unsigned char header;
    char data[];
} CVSDTP_Packet;

Vec2 getRandomCoords();