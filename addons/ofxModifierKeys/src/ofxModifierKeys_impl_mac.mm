#include "ofxModifierKeys.h"

#ifdef TARGET_OSX

#include <Cocoa/Cocoa.h>

bool ofGetModifierPressed(unsigned int mod)
{
	unsigned int t = 0;
	
	if ((OF_KEY_CONTROL & mod) == OF_KEY_CONTROL)
		t += NSControlKeyMask;
	
	if ((OF_KEY_ALT & mod) == OF_KEY_ALT)
		t += NSAlternateKeyMask;
	
	if ((OF_KEY_SHIFT & mod) == OF_KEY_SHIFT)
		t += NSShiftKeyMask;

	if ((OF_KEY_SPECIAL & mod) == OF_KEY_SPECIAL)
		t += NSCommandKeyMask;

	return [[NSApp currentEvent] modifierFlags] & t;
}

#endif