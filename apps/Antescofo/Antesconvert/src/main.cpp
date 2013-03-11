#include "testApp.h"
#include "ofAppGlutWindow.h"

//--------------------------------------------------------------
int main(){
	ofAppGlutWindow window; // create a window
	ofSetupOpenGL(&window, 762, 640, OF_WINDOW);
	ofRunApp(new testApp()); // start the app
}
