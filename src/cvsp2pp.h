#pragma once

#include <stdbool.h>

/*
Maybe enable -fshort-enums gcc flag to
see if it speeds things up enough,
considering this data goes over a wire
*/

enum ObjectShape {
    TRIANGLE,
    SQUARE,
    CIRCLE,
    T
};

enum ObjectRotation {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum ObjectColor {
    RED,
    GREEN,
    BLUE,
    ORANGE
};

typedef struct Target {
    int x, y;
    bool found;
    ObjectShape ts;
    ObjectColor tc;
    ObjectRotation tr;
} Target;

typedef struct Distractor {
    int x, y;
    ObjectShape ts;
    ObjectColor tc;
    ObjectRotation tr;
} Distractor;

typedef struct TargetState {
    // Don't need that many...
    int numTargets;
    Object targets[];
} TargetState;

typedef struct DistractorState {
    int numDistractors;
    Object distractors[];
} DistractorState;

typedef union Object {
    Target target;
    Distractor distractor;
} Object;