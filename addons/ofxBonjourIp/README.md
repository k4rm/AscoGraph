## ofxBonjourIp ##
This addon uses Bonjour to find ip addresses of devices on the same network. Example broadcasts itself and looks for other devices (only 1 atm). To test run example on your device (iphone/ipad) and in the simulator, or on desktop.

Works with ios and osx. Uses CFNetServices (https://developer.apple.com/library/mac/#documentation/CoreFoundation/Reference/CFNetServiceRef/Reference/reference.html) instead of NSNetServices, as this is recommended for C and C++ applications. So no need to rename .cpp files to .mm or anything. Ios examples needs the CFNetwork framework added to your project (see ofxBonjourIp.h for instructions).

-Trent Brooks