//
//  iOSAscoGraph.h
//  AscoGraph
//
//  Created by Thomas Coffy on 29/03/14.
//  Copyright (c) 2014 IRCAM. All rights reserved.
//

#ifndef AscoGraph_ofApp_h
#define AscoGraph_ofApp_h
#pragma once

#include "ofxiOS.h"
#include "ofxiOSExtras.h"
#include "ofxGui.h"
#include "ofxTimeline.h"
#include "ofxTLZoomer2D.h"
#include "ofxTLBeatTicker.h"
#include "ofxTLAntescofoNote.h"
//#include "ofxTLAntescofoAction.h"
//#include "ofxTLBeatJump.h"
#include "ofxBonjourIp.h"
#include "ofxOSC.h"



class ofxTLAntescofoNote;
class ofxTLBeatTicker;

class AntescofoTimeline : public ofxTimeline
{
public:
	AntescofoTimeline() : ofxTimeline() {}
	virtual ~AntescofoTimeline(){}
    
	void setZoomer(ofxTLZoomer *z);
    
	void keypressed(ofKeyEventArgs& args) {}
};

class iOSAscoGraph : public ofxiOSApp{
	
public:
    void setup();
    void setupUI();
    void setupOSC();
    void setupBonjour();
    void setupTimeline();
    void update();
    void draw();
    void drawBonjour();
    void exit();
    void loadScore(string score);
	
    void touchDown(ofTouchEventArgs & touch);
    void touchMoved(ofTouchEventArgs & touch);
    void touchUp(ofTouchEventArgs & touch);
    void touchDoubleTap(ofTouchEventArgs & touch);
    void touchCancelled(ofTouchEventArgs & touch);
    
    void lostFocus();
    void gotFocus();
    void gotMemoryWarning();
    void deviceOrientationChanged(int newOrientation);
    
    void display_error() { cerr << "iOSAscoGraph::display_error() TODO" << endl; }
    void showJumpTrack();
    float score_x, score_y, score_w, score_h, mUIbottom_y, bpm;
    ofxTLZoomer2D* ofxAntescofoZoom;
    ofxTLAntescofoNote* ofxAntescofoNote;
    ofxTLBeatTicker* ofxAntescofoBeatTicker;
    //ofxTLBeatJump* ofxJumpTrack;
    bool bHide;
    string current_score;
    
    string TEXT_CONSTANT_TEMP_FILENAME;
    string TEXT_CONSTANT_TEMP_ACTION_FILENAME;

    ofFbo drawCache;
    bool bShouldRedraw, bLoadingScore;
    
    AntescofoTimeline timeline;

    // Bonjour stuff
    ofxBonjourIp* bonjour;
    void onPublishedService(const void* sender, string &serviceIp);
    void onDiscoveredService(const void* sender, string &serviceIp);
    void onRemovedService(const void* sender, string &serviceIp);
    string antescofo_remoteip;
    
    // OSC stuff
    ofxOscReceiver  mOSCreceiver;
    ofxOscSender    mOSCsender;
    void send_OSC_getscore();
    string mOsc_port, mOsc_port_MAX, mOsc_host;//, mLabelBeat, mLabelBPM, mLabelPitch;
    float mOsc_tempo, mOsc_beat, mOsc_rnow, mOsc_pitch;
    unsigned long long mLastOSCmsgDate;
    float fAntescofoTimeSeconds;

};




#endif
