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

bool ofxTLAntescofoSim::isCurve(string act) 
{
	return curvesmap.find(act) != curvesmap.end();
}

void ofxTLAntescofoSim::update()
{
	int lasty = bounds.y + 50;
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		action_trace* at = *i;
		at->rect.x = get_x(at->rnow);
		lasty = at->rect.y = lasty + 20;
	}
/*
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
			at->rect = r;
		}
		lasty = y;
	}*/
}

//#define LINESHIT

void ofxTLAntescofoSim::update_curve(curve_trace* ct)
{
#ifdef LINESHIT
	ct->line.clear();
#endif
	// x
	ct->rect.x = get_x(ct->rnow);
	// y
	ct->rect.y = bounds.y + 10;
	// ???
	// w
	vector<curveval*>::iterator i = ct->values.end();
	i--;
	ct->rect.width = get_x((*i)->rnow) - get_x(ct->rnow);
	//h
	ct->rect.height = bounds.height - 10;

#ifdef LINESHIT
	for (vector<curveval*>::iterator i = ct->values.begin(); i != ct->values.end(); i++) {
		int x = get_x((*i)->rnow);
		int y = bounds.y + bounds.height - get_y(ct, (*i)->val);
		ct->line.addVertex(x, y);//bounds.y + bounds.height - y);
	}
	//ct->line.close();
	ct->line.simplify();
#endif
}

void ofxTLAntescofoSim::draw_curve(curve_trace* ct)
{
	//cout << "draw curve: " << ct->name << " got " << ct->values.size() << " values. Rect: " << ct->rect.width << "x" << ct->rect.height << endl;

	ofNoFill();
	// draw curve
	ofSetColor(255, 0, 0, 255);

#ifdef LINESHIT
	ofPushStyle(); ofSetPolyMode(OF_POLY_WINDING_NONZERO);
	ct->line.draw();
	ofPopStyle();
#else
	int lastx = -1;
	for (vector<curveval*>::iterator i = ct->values.begin(); i != ct->values.end(); i++) {
		int x = get_x((*i)->rnow);
		if (x != lastx) {
			int y = bounds.y + bounds.height - get_y(ct, (*i)->val);
			//ofRectangle r(x, y, 1, 1);
			//ofRect(r);
			ofCircle(x, y, 1);
			lastx = x;
		}
	}
#endif

	// draw box around
	ofNoFill();
	ofSetColor(0, 0, 0, 255);
	ofRect(ct->rect);

}


void ofxTLAntescofoSim::draw()
{
	ofNoFill();
	ofSetColor(0, 0, 0, 255);
	//cout << "ofxTLAntescofoSim::draw()" << endl;
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		action_trace* at = *i;
		if (!is_in_bounds(at)) continue;
		if (isCurve(at->name) || isCurve(at->fathername)) {
			curve_trace* ct = curvesmap[at->name];
			if (ct && ct->values.size()) {
				update_curve(ct);
				draw_curve(ct);
			}
			continue;
		}
		//cout << "draw: " << at->name << endl;
		string txt(at->name + at->s);
		mFont.drawString(txt, at->rect.x, at->rect.y);
		// draw rect around groups
		if (at->s.empty()) {
			int sizec = mFont.stringWidth(string("_"));
			int h = 18;
			int w = (txt.size() + 1) * sizec;
			ofRectangle r(at->rect.x - 2, at->rect.y - 12, w, h);
			ofRect(r);
		}
	}
}


bool ofxTLAntescofoSim::is_in_bounds(action_trace* at) {
	ofRange r(timeline->screenXtoNormalizedX(at->rect.x, zoomBounds), timeline->screenXtoNormalizedX(at->rect.x + at->rect.width, zoomBounds));
	return zoomBounds.intersects(r);
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


void ofxTLAntescofoSim::add_curveval(string str, curve_trace* ct) {
	if (ct->fathername != "Top_level") {
		curvesmap[str] = ct;
	}
}

void ofxTLAntescofoSim::add_action(action_trace* at) {
	if (at->fathername != "Top_level") {
		//cout << "sim: add action: now:" << at->now << " rnow:" << at->rnow << " action name:" <<  at->name << " father:" << at->fathername <<  " msg: '" << at->s << "'"<<endl;
		actions.push_back(at);
	}
}

int ofxTLAntescofoSim::get_x(float beat) {
	//cout << "get_x: beat:" << beat << " zoomBounds:" << zoomBounds.min << "-" << zoomBounds.max << " -> " << normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds)<< endl;
	return normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds);
}

int ofxTLAntescofoSim::get_y(curve_trace* ct, double v) {
	//return normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds);
	return ofMap(v, ct->min, ct->max, 0, bounds.height-10);
}

void ofxTLAntescofoSim::clear_actions()
{
	actions.clear();
	curvesmap.clear();
}
