#include "ofxAntescofog.h"
#include "ofxCocoaWindow.h"

//--------------------------------------------------------------
int main(int argc, char* argv[]) {

	ofxCocoaWindow cocoaWindow;

	ofSetupOpenGL(&cocoaWindow, 1024, 768, OF_WINDOW);				

 	cocoaWindow.registerForDraggedTypes();
	
	ofRunApp(new ofxAntescofog(argc, argv, &cocoaWindow)); // start the app
}
