//
//  iOSAscoGraph.mm
//  AscoGraph
//
//  Created by Thomas Coffy on 29/03/14.
//  Copyright (c) 2014 IRCAM. All rights reserved.
//

#include "iOSAscoGraph.h"
//#include "ofxTLAntescofoSim.h"
#include "ofxColorPicker.h"
#include "ofxUI.h"
#include "ofxOSC.h"
#include "ofxConsole.h"

#include "Function.h"
#include "Environment.h"

bool _debug = true;

extern ofxConsole* console;

// callback for traces... unused yet.
void ascograph_send_action_trace(const string& action_name, const string& fathername, double now, double rnow, const string& s) {}
void ascograph_send_cont_trace(const string& action_name, const string& fathername, double now, double rnow, double s) {}



void AntescofoTimeline::setZoomer(ofxTLZoomer *z)
{
	//XXX if (zoomer) removeTrack(zoomer);
	delete zoomer;
	zoomer = z;
	zoomer->setTimeline(this);
	zoomer->setup();
    //zoomer->setDrawRect(ofRectangle(offset.y, tabs->getBottomEdge(), width, 160));

	bringTrackToTop(zoomer);
	bringTrackToTop(zoomer);
}


//--------------------------------------------------------------
void iOSAscoGraph::setup(){
	console = new ofxConsole(4, 500, 800, 300, 10);

    score_x = 5;
	score_y = 82;
	mUIbottom_y = 40;
    
	bpm = 120;
    
	ofSetEscapeQuitsApp(false);
	score_w = ofGetWindowWidth() - score_x - 5;
	score_h = ofGetWindowHeight()/3;

    ofSetOrientation(OF_ORIENTATION_90_LEFT);
	ofSetVerticalSync(true);
	
    ofxAntescofoZoom = new ofxTLZoomer2D();
	ofxAntescofoNote = new ofxTLAntescofoNote(this);
	ofxAntescofoBeatTicker = new ofxTLBeatTicker(this);
    
	setupTimeline();
    
	setupUI();

    setupBonjour();

	setupOSC();
    
    TEXT_CONSTANT_TEMP_ACTION_FILENAME = ofxNSStringToString(NSTemporaryDirectory()) + "ascograph_tmp.asco.txt";
    TEXT_CONSTANT_TEMP_FILENAME = ofxNSStringToString(NSTemporaryDirectory()) + "tmpfile-ascograph.txt";
    
	remove(TEXT_CONSTANT_TEMP_FILENAME.c_str());
	remove(TEXT_CONSTANT_TEMP_ACTION_FILENAME.c_str());
    
	drawCache.allocate(ofGetWindowWidth(), ofGetHeight(), GL_RGBA);
	drawCache.begin();
	ofClear(255,255,255, 0);
	drawCache.end();
    
	//if (mScore_filename.size()) loadScore(mScore_filename, true);


    bLoadingScore = false;
	bHide = true;
    
    //screenSize = ofToString(w) + "x" + ofToString(h);
    drawBonjour();

}

void iOSAscoGraph::setupOSC(){
    // listen for OSC
	int port = 0;
	/*std::istringstream is(mOsc_port);
	is >> port;
	if (! is.good() && port <= 0)
    */mOsc_port = "6789";
	ofLog() << "Listening on OSC port " << mOsc_port << endl;
	try {
		mOSCreceiver.setup(atoi(mOsc_port.c_str()));
	} catch(...) {
		ofSetColor(0, 0, 0, 100);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		ofSetColor(255, 255, 255, 240);
        
		string err = "Error can not listen on port ";
		err += port;
		err += " ! Please verify port is available (is another application blocking this UDP port ?";
		cerr << err << endl;
		cout << err << endl;
		ofLog() << err << endl;
		ofDrawBitmapString(err, 100, 300);
		ofxAntescofoNote->set_error(err);
		//TODO guiError->draw();
		//TODO display_error();
	}
    std::istringstream is;
	is.str("");
	is.str(mOsc_port_MAX);
	is.clear();
	is >> port;
	if (! is.good() && port <= 0)
	{
		//cerr << "Not a number, try again." << endl;
		mOsc_port_MAX = "5678";
	}
	//save();
	std::cout << "Connecting OSC on " << mOsc_host << ":"<< atoi(mOsc_port_MAX.c_str()) << endl;
	try {
		mOSCsender.setup(mOsc_host, atoi(mOsc_port_MAX.c_str()));
	} catch (...)
	{ cerr << "ERROR OSC EXCEPTION" << endl; }

}

void iOSAscoGraph::setupUI(){
}

void iOSAscoGraph::setupTimeline(){
    //ofSetBackgroundAuto(false);
    timeline.setupFont("data/GUI/NewMedia Fett.ttf", 10);
	timeline.setOffset(ofVec2f(score_x, score_y));
	timeline.setup(); //registers events
    
	timeline.setFrameRate(24);
	timeline.setShowTicker(false);
	timeline.setShowBPMGrid(true);
	timeline.enableSnapToBPM(false);
	timeline.setDurationInSeconds(60);
	//timeline.moveToThread(); //increases accuracy of bang call backs
    
	timeline.setLoopType(OF_LOOP_NORMAL);
	timeline.setBPM(bpm);
	timeline.setLockWidthToWindow(false);
    timeline.getColors().load("data/GUI/Ascograph.xml");

	// use custom zoomer :
	timeline.addTrack("zoom", ofxAntescofoZoom);
	timeline.setZoomer(ofxAntescofoZoom);
	timeline.addTrack("Beats", ofxAntescofoBeatTicker);
	timeline.addTrack("Notes", ofxAntescofoNote);
	ofxAntescofoNote->setDrawRect(ofRectangle(0, 0, score_w, 400));
    ofxAntescofoNote->color_note.set(255, 0, 0, 255);
    ofxAntescofoNote->color_note_chord.set(0, 255, 0, 255);
    ofxAntescofoNote->color_note_trill.set(0, 0, 255, 255);
    ofxAntescofoNote->color_note_multi.set(255, 255, 0, 255);

    
	ofxAntescofoBeatTicker->setup();
	timeline.setShowTicker(true);
	timeline.setBPM(bpm);
    
	timeline.enable();
	timeline.setFrameBased(false);
	ofxAntescofoNote->enable();
}

void iOSAscoGraph::setupBonjour(){
    // start bonjour
    bonjour = new ofxBonjourIp();
    bonjour->addEventListeners(this); // optional
    
    // find me (server)
    bonjour->startService(); // make device 'discoverable' with defaults.
    //bonjour->startService("_ofxBonjourIp._tcp.", "", 7777, "local");
    
    // find another device (client)- note will not discover itself
    bonjour->discoverService(); // discover other device with defaults.
    //bonjour->discoverService(type, domain);
}

void iOSAscoGraph::drawBonjour(){
    ofSetColor(0);
    
    ofDrawBitmapString("BONJOUR IP: ", 20, 20);
    
    // device name- can use this to connect via osc or udp or tcp
    ofDrawBitmapString("Device name: ", 20, 45);
    ofDrawBitmapStringHighlight(bonjour->getDeviceHostName(), 20, 70);
    
    // device ip- can use this to connect via osc or udp or tcp
    ofDrawBitmapString("Device IP: ", 20, 95);
    ofDrawBitmapStringHighlight(bonjour->getDeviceIp(), 20, 120);
    
    // is connected to a service
    ofDrawBitmapString("Connected to other device: ", 20, 145);
    ofDrawBitmapStringHighlight((bonjour->isConnectedToService()) ? "YES" : "NO", 20, 170);
    
    // device name- can use this to connect via osc or udp or tcp
    ofDrawBitmapString("Other device's name: ", 20, 195);
    ofDrawBitmapStringHighlight(bonjour->getServerHostName(), 20, 220);
    
    // device ip- can use this to connect via osc or udp or tcp
    ofDrawBitmapString("Other device's IP: ", 20, 245);
    ofDrawBitmapStringHighlight(bonjour->getServerIp(), 20, 270);
}


//--------------------------------------------------------------
void iOSAscoGraph::update(){
    bool mHasReadMessages = false;

    // check for waiting messages
	try {
		while( mOSCreceiver.hasWaitingMessages() )
		{
			// get the next message
			ofxOscMessage m;
			mOSCreceiver.getNextMessage( &m );
			if (_debug) ofLog() << "OSC received: '" << m.getAddress() << endl;
			if(m.getAddress() == "/antescofo/current_score") {
                if(m.getArgType(0) == OFXOSC_TYPE_STRING){
                    current_score += m.getArgAsString(0);
                    if (!bLoadingScore) loadScore(current_score);
                    bLoadingScore = true;
                }
            } else if(m.getAddress() == "/antescofo/current_score_append") {
                if(m.getArgType(0) == OFXOSC_TYPE_STRING){
                    current_score += m.getArgAsString(0);
                }
            } else if(m.getAddress() == "/antescofo/tempo" && m.getArgType(0) == OFXOSC_TYPE_FLOAT) {
				mOsc_tempo = m.getArgAsFloat(0);
				if (_debug) cout << "OSC received: tempo: "<< mOsc_tempo << endl;
				bpm = mOsc_tempo;
				//TODO mLabelBPM->setLabel(ofToString(mOsc_tempo));
				//if (bpm) timeline.setBPM(bpm);
				//mSliderBPM->setValue(bpm);
				mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/event_beatpos" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_beat = m.getArgAsFloat(0);
				//TODO mLabelBeat->setLabel(ofToString(mOsc_beat));
				if (_debug) cout << "OSC received: beat: "<< mOsc_beat << endl;
				mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/rnow" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_rnow = m.getArgAsFloat(0);
				//mLabelBeat->setLabel(ofToString(mOsc_rnow));
				if (_debug) cout << "OSC received: rnow: "<< mOsc_rnow << endl;
			} else if(m.getAddress() == "/antescofo/pitch"  && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_pitch = m.getArgAsFloat(0);
				//TODO mLabelPitch->setLabel(ofToString(mOsc_pitch));
				if (_debug) cout << "OSC received: pitch: "<< mOsc_pitch << endl;
				mHasReadMessages = true;
			} else {
				// unrecognized message: display it
				string msg_string;
				msg_string = m.getAddress();
				msg_string += ": ";
				for(int i = 0; i < m.getNumArgs(); i++){
					// get the argument type
					msg_string += m.getArgTypeName(i);
					msg_string += ":";
					// display the argument - make sure we get the right type
					if(m.getArgType(i) == OFXOSC_TYPE_INT32){
						msg_string += ofToString(m.getArgAsInt32(i));
					}
					else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
						msg_string += ofToString(m.getArgAsFloat(i));
					}
					else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
						msg_string += m.getArgAsString(i);
					}
					else{
						msg_string += "unknown";
					}
				}
				cout << "OSC received: unknown msg: "<< msg_string << endl;
			}
			//no break in order to eat every available messages
        }
	} catch (exception& e) {
		cerr << "OSC HasWaitingMessage exception raised" <<  endl;
	}
    
	// if we read something, advance playhead
	if (mHasReadMessages) {
		mLastOSCmsgDate = ofGetSystemTime();
		if (mOsc_beat != -1 ) {
			fAntescofoTimeSeconds = ofxAntescofoNote->convertAntescofoOutputToTime(mOsc_beat, mOsc_tempo, mOsc_pitch);
            
			if (_debug) cout << "Moving playHead to beat:"<<mOsc_beat << " tempo:"<<mOsc_tempo << " => "<<fAntescofoTimeSeconds << "sec"<<endl;
		}
		mHasReadMessages = false;
		bShouldRedraw = true;
	}

}

//--------------------------------------------------------------
void iOSAscoGraph::draw(){
    ofBackgroundGradient(ofColor::white, ofColor::gray);
    timeline.draw();
}

//--------------------------------------------------------------
void iOSAscoGraph::exit(){
}


void iOSAscoGraph::loadScore(string score)
{
    ofxAntescofoNote->clear_error();

    ofLog() << "iOSAscoGraph::loadScore: " << score << endl;
    
    bool res = ofxAntescofoNote->loadscoreAntescofo_fromString(score, TEXT_CONSTANT_TEMP_FILENAME);
    
    if (res) {
        // jumps track
        showJumpTrack();
        
        bShouldRedraw = true;
    }
}


void iOSAscoGraph::showJumpTrack() {
#if 0
	// check if we need to show jump tracks:
	bool bShowJumpTrack = false;
	vector<ofxTLAntescofoNoteOn*>& switches = ofxAntescofoNote->getSwitches();
	for (vector<ofxTLAntescofoNoteOn*>::iterator i = switches.begin();
         i!= switches.end(); i++)
		if ((*i)->jump_dests.size()) {
			bShowJumpTrack = true;
			break;
		}
    
	if (!bShowJumpTrack) {
		if (ofxJumpTrack) timeline.removeTrack(ofxJumpTrack);
		if (ofxJumpTrack) delete ofxJumpTrack;
		ofxJumpTrack = NULL;
		return;
	}
    
	if (ofxJumpTrack == 0) {
		ofxJumpTrack = new ofxTLBeatJump(this);
		timeline.addTrack("Jumps", ofxJumpTrack);
		timeline.bringTrackToPos(ofxJumpTrack, 2);
	}
    
	ofxJumpTrack->setZoomBounds(ofxAntescofoZoom->getViewRange());
	ofxJumpTrack->clear_jumps();
    
	// for every events
	switches = ofxAntescofoNote->getSwitches();
	for (vector<ofxTLAntescofoNoteOn*>::iterator i = switches.begin();
         i!= switches.end(); i++) {
		// for every jump dest
		for (int n = 0; n < (*i)->jump_dests.size(); n++) {
			float destBeat = (*i)->jump_dests[n];
			cout << "showJumpTrack: adding jump: beat:" << (*i)->beat.min << " destBeat:" << destBeat << " label:" << (*i)->label <<endl;
			ofxJumpTrack->add_jump((*i)->beat.min, destBeat, "");
		}
	}
#endif
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

// Bonjour events handlers
void iOSAscoGraph::onPublishedService(const void* sender, string &serviceIp) {
    ofLog() << "Received published service event: " << serviceIp;
}

void iOSAscoGraph::onDiscoveredService(const void* sender, string &serviceIp) {
    ofLog() << "Received discovered service event: " << serviceIp;
    mOsc_host = serviceIp;
    send_OSC_getscore();
}

void iOSAscoGraph::onRemovedService(const void* sender, string &serviceIp) {
    ofLog() << "Received removed service event: " << serviceIp;
}

void iOSAscoGraph::send_OSC_getscore() {
    if (mOsc_host.size()) {
        std::cout << "Connecting OSC on " << mOsc_host << ":"<< atoi(mOsc_port_MAX.c_str()) << endl;
        try {
            mOSCsender.setup(mOsc_host, atoi(mOsc_port_MAX.c_str()));
        } catch (...)
        { cerr << "ERROR OSC EXCEPTION" << endl; }

        ofxOscMessage m;
        m.setAddress("/antescofo/cmd");
        bLoadingScore = false;
        ofLog() << "Sending OSC get_current_score..." << endl;
        current_score.clear();
        m.addStringArg("get_current_score");
        mOSCsender.sendMessage(m);
    }
}


