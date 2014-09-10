#include <sstream>
#include <string>
#include <sys/time.h>

//#include "ofxConsole.h"
#include "ofxAntescofog.h"
#include "ofxCocoaWindow.h"
#include "ofxTLBeatTicker.h"

static vector<action_trace* > simul_actions;
static vector<curve_trace* > simul_curves;

extern string str_error;

bool debug_trace_msgs = false;

// called from antescofo when simulating
void ascograph_send_action_trace(const string& action_name,
                                 const string& fathername,
                                 double now,
                                 double rnow,
                                 const string& s)
{
	//if (debug_trace_msgs) cout << " now: " << now << " rnow:" << rnow << " action name:" <<  action_name << " father:" << fathername <<  " msg:" << s << endl;
	simul_actions.push_back(new action_trace(action_name, fathername, now, rnow, s));
}

string get_fathername(string str) {
	for (vector<action_trace* >::iterator i = simul_actions.begin(); i != simul_actions.end(); i++) {
		if ((*i)->name == str)
			return (*i)->fathername;
	}
	return string("");
}



action_trace* find_action_trace(string name) {

//	find(simul_actions.begin(), simul_actions.end(), fathername);
	for (vector<action_trace* >::iterator i = simul_actions.begin(); i != simul_actions.end(); i++) {
		if ((*i)->name == name) {
			return *i;
		}
	}
	return 0;
}

curve_trace* find_curve_trace(string curvename, string fathername, string varname) {
	for (vector<curve_trace*>::iterator i = simul_curves.begin(); i != simul_curves.end(); i++) {
		if (curvename == (*i)->name && fathername == (*i)->fathername && varname == (*i)->varname)
			return *i;
	}
	return 0;
}

// called from antescofo when simulating curve var change
void ascograph_send_cont_trace(const string& var_name,
                               const string& curvename,
                               double now,double rnow,
                               double val)
{
	string fathername = get_fathername(curvename);
	if (debug_trace_msgs) 
		cout << "now: " << now << " rnow:" << rnow << " curve:" << curvename << "[" << fathername <<"] var:" << var_name << " = " << val << endl;

	curve_trace* c = find_curve_trace(curvename, fathername, var_name);
	// find the curve_trace or create it
	if (!c) {
		// new curve
		c = new curve_trace(curvename, fathername, var_name, now, rnow, val);
		simul_curves.push_back(c);
		// count curves by group
		action_trace* t = find_action_trace(fathername);
		if (t) {
			t->nbcurves++;
			cout << "Simulation: create new curve detected: " << t->name << " var:" << var_name << " nbcurves="<<t->nbcurves<< endl;
		}
	}  
	// curves already exists
	c->values.push_back(new curveval(now, rnow, val));
	if (c->min > val)
		c->min = val;
	if (c->max < val)
		c->max = val;
}



//--------------------------------------------------------------
void ofxAntescofog::setupTimelineSim(){

	ofxAntescofoZoomSim = new ofxTLZoomer2D(this);
	ofxAntescofoNoteSim = new ofxTLAntescofoNote(this);
	ofxAntescofoBeatTickerSim = new ofxTLBeatTicker(this);
	ofxAntescofoSim = new ofxTLAntescofoSim(this);

	//ofSetBackgroundAuto(false);
	timelineSim.setOffset(ofVec2f(score_x, score_y));
	timelineSim.setup(); //registers events

	//timeline.setWidth(score_w - 100);
	timelineSim.setFrameRate(24);
	timelineSim.setShowTicker(false);
	timelineSim.setShowBPMGrid(bSnapToGrid);
	timelineSim.enableSnapToBPM(bSnapToGrid);
	timelineSim.setDurationInSeconds(60);
	//timeline.moveToThread(); //increases accuracy of bang call backs

	timelineSim.setLoopType(OF_LOOP_NORMAL);//LOOP_NONE); ///NORMAL); //turns the timeline to loop
	timelineSim.setBPM(bpm);
	timelineSim.setLockWidthToWindow(false);
	//timeline.setWidth(ofGetWidth());

	// use custom zoomer :
	timelineSim.addTrack("zoom", ofxAntescofoZoomSim);
	timelineSim.setZoomer(ofxAntescofoZoomSim);

	//timelineSim.addTrack("zoom", ofxAntescofoZoom);
	//timelineSim.setZoomer(ofxAntescofoZoom);

	timelineSim.addTrack("Beats", ofxAntescofoBeatTickerSim);
	timelineSim.addTrack("Notes", ofxAntescofoNoteSim);
	//timelineSim.addTrack("Notes", ofxAntescofoNote);
	ofxAntescofoNoteSim->setDrawRect(ofRectangle(0, 0, score_w, 300));
	ofxAntescofoNoteSim->enable();
	timelineSim.addTrack("Performance simulation", ofxAntescofoSim);
	ofxAntescofoBeatTickerSim->setup();
	ofxAntescofoSim->setup();
	ofxAntescofoSim->enable();
	ofxAntescofoSim->update();
	ofxAntescofoZoomSim->setTimeline(&timelineSim);
	timelineSim.setShowTicker(true);
	timelineSim.setBPM(bpm);

	//timeline.addCurves("test", "test.xml");
	timelineSim.enable();
	timelineSim.setFrameBased(false);
}




void ofxAntescofog::draw_simulate()
{
	while (simul_actions.size()) {
		// copy actions
		ofxAntescofoSim->add_action(simul_actions[0]);
		simul_actions.erase(simul_actions.begin());
	}
	while (simul_curves.size()) {
		// copy curves
		ofxAntescofoSim->add_curveval(simul_curves[0]);
		simul_curves.erase(simul_curves.begin());
	}

	timelineSim.draw();
	guiBottom->draw();
}

void ofxAntescofog::stop_simulate_and_goedit() {
	timelineSim.disable();
	timeline.enable();
	ofxAntescofoSim->clear_actions();
	ofxAntescofoNote->createActionTrack();
	ofxAntescofoNote->loadscoreAntescofo(mScore_filename);
}


void ofxAntescofog::simulate()
{
	// TODO changer le bouton simulate en edit

	if (edited())
		saveScore(false);

	timeline.disable();

	if (!ofxAntescofoSim) {
		setupTimelineSim();
	} else {
		timelineSim.enable();
		if (ofxAntescofoNoteSim->mAntescofo) 
		{ 
			//ofxAntescofoNote->deleteActionTrack();
			delete ofxAntescofoNoteSim->mAntescofo; 
			ofxAntescofoNoteSim->mAntescofo = new antescofo_ascograph_offline();
		}
	}
	ofxAntescofoNoteSim->mAntescofo->antescofo_stop_run();
	ofxAntescofoNoteSim->mAntescofo->antescofo_stop();
	ofxAntescofoNoteSim->loadscoreAntescofo(mScore_filename);

	if (ofxAntescofoNote->getActionTrack())
		ofxAntescofoNote->deleteActionTrack();
	if (ofxAntescofoNoteSim->getActionTrack())
		ofxAntescofoNoteSim->deleteActionTrack();
	ofxAntescofoNoteSim->mAntescofo->set_score_file(mScore_filename.c_str());
	ofxAntescofoNoteSim->mAntescofo->set_verbosity_level(1);
	ofxAntescofoNoteSim->mAntescofo->set_trace(true);
	//TODO set_audio_file
	if (ofxAntescofoNoteSim->mAntescofo->countActions()) {
		cout << endl << "Launching performance simulation:" << endl;
		ofxAntescofoNoteSim->mAntescofo->play_mode();
		ofxAntescofoSim->update();
		if (str_error.size()) {
			bShowError = true;
			display_error();
		}
	} else {
		//TODO display error no action in score to simulate
	}
}

