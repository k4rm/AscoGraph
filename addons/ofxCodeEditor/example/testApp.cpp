#include "testApp.h"

testApp::testApp(int argc, char** argv, ofxCocoaWindow* window)
: mCocoaWindow(window)
{
}


void testApp::setup()
{
				float editor_x = 50;
				float editor_y = 0;

				mCodeEditor = [ ofxCodeEditor alloc];

				NSWindow *nswin = [mCocoaWindow->delegate getNSWindow]; 
				NSView *nsview_ = [mCocoaWindow->delegate getNSView];
				ofRectangle r(editor_x, editor_y, ofGetWidth() - editor_x, ofGetHeight());
				[ mCodeEditor setup: nswin glview:nsview_ rect:r];
}

void testApp::update()
{
}

void testApp::draw()
{
}

void testApp::keyPressed(int key)
{
	if (key == 'l') { 
		[ mCodeEditor loadFile:"/etc/passwd"];
	}

	if (key == 's') {
		[ mCodeEditor showLine:15 lineb:20];
	}
}

void testApp::keyReleased(int key)
{

}

void testApp::mouseMoved(int x, int y )
{

}

void testApp::mouseDragged(int x, int y, int button)
{

}

void testApp::mousePressed(int x, int y, int button)
{

}

void testApp::mouseReleased(int x, int y, int button)
{

}

void testApp::windowResized(int w, int h)
{

}

/*
void testApp::dragEvent(ofDragInfo dragInfo){
	if( dragInfo.files.size() > 0 ){
		for(int i = 0; i < dragInfo.files.size(); i++){
			ofLogVerbose("ofxAntescofog: trying to load Music XML file :" + dragInfo.files[i]);
			mCodeEditor->loadFile(dragInfo.files[i]);
			break; // only one file supported yet
		}
	}
}
*/
