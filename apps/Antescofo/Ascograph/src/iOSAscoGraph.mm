//
//  iOSAscoGraph.mm
//  AscoGraph
//
//  Created by Thomas Coffy on 29/03/14.
//  Copyright (c) 2014 IRCAM. All rights reserved.
//


#include "iOSAscoGraph.h"
#include "ofxTimeline.h"
#include "ofxTLZoomer2D.h"
#include "ofxTLAccompAudioTrack.h"
#include "ofxTLBeatTicker.h"
#include "ofxTLAntescofoNote.h"
#include "ofxTLAntescofoAction.h"
//#include "ofxTLAntescofoSim.h"
#include "ofxColorPicker.h"
#include "ofxUI.h"
#include "ofxOSC.h"
#include "ofxConsole.h"
#include "ofxTLBeatJump.h"

#include "Function.h"
#include "Environment.h"

extern void forceload_value();

//--------------------------------------------------------------
void iOSAscoGraph::setup(){
    
    ofSetOrientation(OF_ORIENTATION_90_LEFT);
	ofSetVerticalSync(true);
	
	bHide = true;
    
    //screenSize = ofToString(w) + "x" + ofToString(h);
}

//--------------------------------------------------------------
void iOSAscoGraph::update(){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::draw(){
    ofBackgroundGradient(ofColor::white, ofColor::gray);

}

//--------------------------------------------------------------
void iOSAscoGraph::exit(){
}

//--------------------------------------------------------------
void iOSAscoGraph::touchDown(ofTouchEventArgs & touch){
    cout << "touchDown:" << touch.x << ", " << touch.y << endl;
}

//--------------------------------------------------------------
void iOSAscoGraph::touchMoved(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::touchUp(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::touchDoubleTap(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::touchCancelled(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::lostFocus(){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::gotFocus(){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::gotMemoryWarning(){
    
}

//--------------------------------------------------------------
void iOSAscoGraph::deviceOrientationChanged(int newOrientation){
    
}

