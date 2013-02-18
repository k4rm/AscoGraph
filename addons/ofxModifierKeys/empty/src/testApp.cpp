#include "testApp.h"

#include "ofxModifierKeys.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	
	ofBackground(30);
	
	
}

//--------------------------------------------------------------
void testApp::update()
{
	
	
}

//--------------------------------------------------------------
void testApp::draw()
{
	string str;
	
	str += "OF_KEY_SHIFT: " + string(ofGetModifierPressed(OF_KEY_SHIFT) ? "ON" : "OFF") + "\n";
	str += "OF_KEY_CTRL: " + string(ofGetModifierPressed(OF_KEY_CTRL) ? "ON" : "OFF") + "\n";
	str += "OF_KEY_ALT: " + string(ofGetModifierPressed(OF_KEY_ALT) ? "ON" : "OFF") + "\n";
	str += "OF_KEY_SPECIAL: " + string(ofGetModifierPressed(OF_KEY_SPECIAL) ? "ON" : "OFF") + "\n";
	
	ofDrawBitmapString(str, 10, 20);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{

}