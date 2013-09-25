#include <sstream>
#include <algorithm>
#include <string>
#include "ofxTLAntescofoNote.h"
#include "ofxTLAntescofoAction.h"
#include "Score.h"
#include "Values.h"
#include "Action.h"
#include <location.hh>
#include <position.hh>
#include <ofxAntescofog.h>
#include "ofxCodeEditor.h"



ofxTLAntescofoSim::ofxTLAntescofoSim(ofxAntescofog *Antescofog)
{
	mAntescofog = Antescofog;
	bEditorShow = false;
}


ofxTLAntescofoSim::~ofxTLAntescofoSim()
{
}

void ofxTLAntescofoSim::setup()
{
	load();

	update();
	disable();
}


void ofxTLAntescofoSim::update()
{

}


void ofxTLAntescofoSim::draw()
{
	ofNoFill();
	ofSetColor(0, 0, 0, 255);
	//cout << "ofxTLAntescofoSim::draw()" << endl;
	int lasty = bounds.y + 50;
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		action_trace* at = *i;
		int y = lasty + 20;
		int x = get_x(at->rnow);
		string txt(at->name + at->s);
		mFont.drawString(txt, x, y);
		// draw rect around groups
		if (at->s.empty()) {
			int sizec = mFont.stringWidth(string("_"));
			int h = 18;
			int w = (txt.size() + 1) * sizec;
			ofRectangle r(x - 2, y - 12, w, h);
			ofRect(r);
		}
		lasty = y;
	}
}




bool ofxTLAntescofoSim::mousePressed(ofMouseEventArgs& args, long millis) {
	return false;
}
void ofxTLAntescofoSim::mouseMoved(ofMouseEventArgs& args, long millis) {
}
void ofxTLAntescofoSim::mouseDragged(ofMouseEventArgs& args, long millis) {
}

void ofxTLAntescofoSim::mouseReleased(ofMouseEventArgs& args, long millis) {
}

void ofxTLAntescofoSim::keyPressed(ofKeyEventArgs& args) {
}

void ofxTLAntescofoSim::windowResized(int w, int h) {
}

void ofxTLAntescofoSim::save() {}
void ofxTLAntescofoSim::load() {
	mFont.loadFont ("DroidSansMono.ttf", 8);
}


void ofxTLAntescofoSim::add_action(action_trace* at) {
	if (at->fathername != "Top_level") {
		cout << "sim: add action: now:" << at->now << " rnow:" << at->rnow << " action name:" <<  at->name << " father:" << at->fathername <<  " msg: '" << at->s << "'"<<endl;
		actions.push_back(at);
	}
}

int ofxTLAntescofoSim::get_x(float beat) {
	//cout << "get_x: beat:" << beat << " zoomBounds:" << zoomBounds.min << "-" << zoomBounds.max << " -> " << normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds)<< endl;
	return normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds);
}

void ofxTLAntescofoSim::clear_actions()
{
	actions.clear();
}
