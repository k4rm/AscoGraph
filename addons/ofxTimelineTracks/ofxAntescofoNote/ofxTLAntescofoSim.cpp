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

string ofxTLAntescofoSim::get_fathername(string str) {
	for (vector<action_trace* >::iterator i = actions.begin(); i != actions.end(); i++) {
		if ((*i)->name == str)
			return (*i)->fathername;
	}
	return string("");
}



curve_trace* ofxTLAntescofoSim::getCurve(string act) 
{
	for (vector<curve_trace*>::iterator i = curves.begin(); i != curves.end(); i++) {
		if ((*i)->name == act)
			return *i;
	}
	return 0;
}


action_trace* ofxTLAntescofoSim::getAction(string act) 
{
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		if ((*i)->name == act)
			return *i;
	}
	return 0;
}

void ofxTLAntescofoSim::update()
{
	update_curves();
	int lasty = bounds.y + 50;
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		action_trace* at = *i;
		at->rect.x = get_x(at->rnow);
		lasty = at->rect.y = lasty + 20;
	}
}
void ofxTLAntescofoSim::print_actions() {
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		action_trace* a = *i;
		cout << "print_actions: " << a->rect.x << "," << a->rect.y << " "<< a->name << " ["<<a->fathername << "] nbcurves:"<< a->nbcurves << endl;
	}

}
void ofxTLAntescofoSim::print_curves() {
	for (vector<curve_trace*>::iterator i = curves.begin(); i != curves.end(); i++) {
		curve_trace* a = *i;
		cout << "print_curves: " << a->rect.x << "," << a->rect.y << " " << a->rect.width << "x" << a->rect.height << " " << a->name << " ["<<a->fathername << "] var: "<< a->varname<< endl;
	}

}

void ofxTLAntescofoSim::checkOverlapCurve()
{
	//for (map<string,curve_trace*>::iterator c = curvesmap.begin(); c != curvesmap.end(); c++) { }
	
}

//#define LINESHIT

void ofxTLAntescofoSim::update_curves()
{
#ifdef LINESHIT
	ct->line.clear();
#endif
	//print_actions();
	//print_curves();
	int lasty = bounds.y + 1;
	int lasth = 0;
	string lastname;
	for (vector<curve_trace*>::iterator j = curves.begin(); j != curves.end(); j++) {
		curve_trace* ct = *j;
		// new curve
		if (ct->fathername != lastname) {
			lasty = bounds.y + 2;
			lasth = 0;
		}
		// x
		ct->rect.x = get_x(ct->rnow);
		// y
		ct->rect.y = lasty + lasth + 1;
		// w
		vector<curveval*>::iterator i = ct->values.end();
		i--;
		ct->rect.width = get_x((*i)->rnow) - get_x(ct->rnow);
		//h
		string fathername = get_fathername(ct->name);
		//cout << "Searching number of curve for obj:" << ct->name << " father:" <<fathername << endl;
		int n = getAction(ct->name)->nbcurves;
		action_trace* a = getAction(fathername);
		if (!a) return;
		n = a->nbcurves;
		if (!n) n = 1;
		lasth = ct->rect.height = (bounds.height-2) / n;
		lastname = ct->fathername;
	}

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
			//int y = bounds.y + bounds.height - get_y(ct, (*i)->val);
			int y = ct->rect.y + ct->rect.height - get_y(ct, (*i)->val);
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

	// draw varname
	mFont.drawString(ct->varname, ct->rect.x + 5, ct->rect.y + 15);
}


void ofxTLAntescofoSim::draw()
{
	update();
	checkOverlapCurve();
	ofNoFill();
	ofSetColor(0, 0, 0, 255);
	//cout << "ofxTLAntescofoSim::draw()" << endl;
	for (vector<action_trace*>::iterator i = actions.begin(); i != actions.end(); i++) {
		action_trace* at = *i;
		if (!is_in_bounds(at)) continue;

		curve_trace* ct = getCurve(at->name);
		if (ct) {
			//if (ct->values.size()) { draw_curve(ct); }
			continue;
		}
		if (at->name.substr(0, 10) == "_topgroup_")
			continue;
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
	for (vector<curve_trace*>::iterator j = curves.begin(); j != curves.end(); j++) {
		draw_curve(*j);
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
	string fontfile = ofFilePath::getCurrentExeDir() + "../Resources/DroidSansMono.ttf";
	mFont.loadFont (fontfile, 8);
}


void ofxTLAntescofoSim::add_curveval(curve_trace* ct) {
	//if (ct->fathername != "Top_level") {
		curves.push_back(ct);
	//}
}

void ofxTLAntescofoSim::add_action(action_trace* at) {
	//if (at->fathername != "Top_level") {
		//cout << "sim: add action: now:" << at->now << " rnow:" << at->rnow << " action name:" <<  at->name << " father:" << at->fathername <<  " msg: '" << at->s << "'"<<endl;
		actions.push_back(at);
	//}
}

int ofxTLAntescofoSim::get_x(float beat) {
	//cout << "get_x: beat:" << beat << " zoomBounds:" << zoomBounds.min << "-" << zoomBounds.max << " -> " << normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds)<< endl;
	return normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds);
}

int ofxTLAntescofoSim::get_y(curve_trace* ct, double v) {
	//return normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds);
	//return ofMap(v, ct->min, ct->max, 0, bounds.height-10);
	return ofMap(v, ct->min, ct->max, 0, ct->rect.height);
}

void ofxTLAntescofoSim::clear_actions()
{
	actions.clear();
	curves.clear();
}
