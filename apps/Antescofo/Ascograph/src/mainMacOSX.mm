#include "ofxAntescofog.h"
#include "ofxCocoaWindow.h"

ofxCocoaWindow *cocoaWindow;
int stored_argc;
char** stored_argv;

//--------------------------------------------------------------
int main(int argc, char* argv[]) {


	cocoaWindow = new ofxCocoaWindow();
	ofSetupOpenGL(cocoaWindow, 1024, 768, OF_WINDOW);				

 	cocoaWindow->registerForDraggedTypes();
	
	ofPtr<ofBaseApp> app = ofPtr<ofBaseApp>(new ofxAntescofog(argc, argv, cocoaWindow));
	//ofRunApp(new ofxAntescofog(argc, argv, cocoaWindow)); // start the app
	ofRunApp(app);

	stored_argc = argc;
	stored_argv = argv;
	//return NSApplicationMain(argc, (const char **) argv);
	return 0;
}

#include "ofxNSWindower.h"

#import <Cocoa/Cocoa.h>


@interface AppDelegate : NSObject <NSApplicationDelegate> {

        
}

@end

@implementation AppDelegate


- (void) applicationDidFinishLaunching: (NSNotification*) notification {
        
        //ofxNSWindower::instance()->addWindow(new ofxAntescofog(stored_argc, stored_argv, cocoaWindow), "AscoGraph"); //new graphicsExample(), "graphicsExample");
}

- (void) applicationWillTerminate: (NSNotification*) notification {
        ofxNSWindower::destroy();
}

@end
