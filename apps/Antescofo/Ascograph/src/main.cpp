#include "ofxAntescofog.h"
#include "ofAppGlutWindow.h"

//--------------------------------------------------------------
int main(int argc, char* argv[]){
	ofAppGlutWindow window; // create a window
	window.setGlutDisplayString("rgba double samples>=4");
	window.setWindowTitle("Antescofog : Antescofo Score Editor");
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	ofRunApp(new ofxAntescofog(argc, argv)); // start the app
}
