#pragma once

#include "ofMain.h"

#ifdef OF_KEY_SHIFT
#undef OF_KEY_SHIFT
#define OF_KEY_SHIFT 1
#endif

#ifdef OF_KEY_CTRL
#undef OF_KEY_CTRL
#define OF_KEY_CTRL 2
#endif

#ifdef OF_KEY_ALT
#undef OF_KEY_ALT
#define OF_KEY_ALT 4
#endif

#define OF_KEY_SPECIAL 8

bool ofGetModifierPressed(unsigned int mod);
