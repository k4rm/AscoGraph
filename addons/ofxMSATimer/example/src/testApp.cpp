#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetFrameRate(60);

	ofBackground(0,0,0);
	TTF.loadFont("mono.ttf", 7);
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){

	ofSetColor(240, 240, 240);

	message = "ofxMSATimer.getAppTime(): " + ofToString(timer.getAppTimeSeconds()) + " sec";
	TTF.drawString(message, 100, 120);

	double elapsedTimeSec = timer.getElapsedSeconds();
	message = "ofxMSATimer.getElapsedTime(): " + ofToString(elapsedTimeSec*1000) + " ms"; //To millis
	TTF.drawString(message, 100, 160);

	message = "ofxMSATimer.getAppTimeMillis(): " + ofToString(timer.getAppTimeMillis()) + " ms";
	TTF.drawString(message, 100, 180);

	message = "ofxMSATimer.getTimeSinceLastCall(): " + ofToString(timer.getSecondsSinceLastCall()) + " sec";
	TTF.drawString(message, 100, 200);

	message = "FPS: " + ofToString(1/elapsedTimeSec);
	TTF.drawString(message, 100, 100);

	timer.setStartTime();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}