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
#include "ofxUI.h"
#include "ofxTimeline.h"
#include "ofxTLZoomer2D.h"
#include "ofxTLBeatTicker.h"
#include "ofxTLAntescofoNote.h"
#include "ofxTLAntescofoAction.h"
//#include "ofxTLBeatJump.h"
#include "ofxBonjourIp.h"
#include "ofxOSC.h"
#include "iOSAscoGraphMenu.h"


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

#define TEXT_CONSTANT_BUTTON_BEAT               "Position in score (in beats): "
#define TEXT_CONSTANT_BUTTON_PITCH              "Detected Pitch: "
#define TEXT_CONSTANT_BUTTON_BPM              	"Detected BPM: "
#define TEXT_CONSTANT_BUTTON_PLAY               "Play"
#define TEXT_CONSTANT_BUTTON_START              "Start"
#define TEXT_CONSTANT_BUTTON_STOP               "Stop"
#define TEXT_CONSTANT_BUTTON_NEXT_EVENT         "next event"
#define TEXT_CONSTANT_BUTTON_PREV_EVENT         "prev event"
#define TEXT_CONSTANT_BUTTON_SETTINGS           "settings"
#define TEXT_CONSTANT_BUTTON_TOGGLEVIEW         "Toggle view"
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
    void push_tempo_value();
    float score_x, score_y, score_w, score_h, mUIbottom_y, bpm;
    ofxTLZoomer2D* ofxAntescofoZoom;
    ofxTLAntescofoNote* ofxAntescofoNote;
    ofxTLBeatTicker* ofxAntescofoBeatTicker;
    ofxTLAntescofoAction* ofxAntescofoAction;
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
    void send_OSC_getscore(string host);
    string mOsc_port, mOsc_port_MAX, mOsc_host;//, mLabelBeat, mLabelBPM, mLabelPitch;
    float mOsc_tempo, mOsc_beat, mOsc_rnow, mOsc_pitch;
    unsigned long long mLastOSCmsgDate;
    float fAntescofoTimeSeconds;
    struct timeval last_draw_time;

    // buttons...
    ofxUICanvas *guiBottom;
    ofxUISlider *mSliderBPM;
    ofxUILabel  *mLabelBeat, *mLabelBPM, *mLabelPitch;
    float* mBPMbuffer;
    ofxUIDropDownList *mDdl_host_lists, *mDdl_cues_list;
    void guiEvent(ofxUIEventArgs &e);
    vector<string> antescofo_hostnames, antescofo_cuepoints;
    void setAutoscroll(bool newstate);
    void setFastForwardOnOff(bool newstate);
    void cues_add_menu(string& str);
    bool bFastForwardOnOff;
    
    bool is_retina;
    int fontsize;

};




#endif
