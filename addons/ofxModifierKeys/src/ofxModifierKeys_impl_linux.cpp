#include "ofxModifierKeys.h"

#ifdef TARGET_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <GL/glx.h>

bool ofGetModifierPressed(unsigned int mod)
{
	unsigned int t = 0;

	//return false;
	Display* disp = glXGetCurrentDisplay();
	char key_map_stat[32];
	XQueryKeymap(disp, key_map_stat);

	if ((OF_KEY_ALT & mod) == OF_KEY_ALT){
		const KeyCode alt_l = XKeysymToKeycode(disp,XK_Alt_L);
		const KeyCode alt_r = XKeysymToKeycode(disp,XK_Alt_R);

		return (((key_map_stat[alt_l >> 3] >> (alt_l & 7)) & 1)
			  ||((key_map_stat[alt_r >> 3] >> (alt_r & 7)) & 1));
	}

	if ((OF_KEY_CONTROL & mod) == OF_KEY_CONTROL){
		const KeyCode control_l = XKeysymToKeycode(disp,XK_Control_L);
		const KeyCode control_r = XKeysymToKeycode(disp,XK_Control_R);

		return (((key_map_stat[control_l >> 3] >> (control_l & 7)) & 1)
			  ||((key_map_stat[control_r >> 3] >> (control_r & 7)) & 1));
	}

	if((OF_KEY_SHIFT & mod) == OF_KEY_SHIFT){
		const KeyCode shift_l = XKeysymToKeycode(disp,XK_Shift_L);
		const KeyCode shift_r = XKeysymToKeycode(disp,XK_Shift_R);

		return (((key_map_stat[shift_l >> 3] >> (shift_l & 7)) & 1)
			  ||((key_map_stat[shift_r >> 3] >> (shift_r & 7)) & 1));
	}

	if ((OF_KEY_SPECIAL & mod) == OF_KEY_SPECIAL){
		const KeyCode control_l = XKeysymToKeycode(disp,XK_Super_L);
		const KeyCode control_r = XKeysymToKeycode(disp,XK_Super_R);

		return (((key_map_stat[control_l >> 3] >> (control_l & 7)) & 1)
			  ||((key_map_stat[control_r >> 3] >> (control_r & 7)) & 1));
	}


	return false;
}

#endif
