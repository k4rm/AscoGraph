//
//  ofxAntescofoAction.h
//  ofxAntescofog
//
//  Created by Thomas Coffy on 06/12/12.
//
//
#pragma once

#include <iostream>
#include <list>
#include "ofxTimeline.h"
#include "ofxTLTrack.h"
#include "Score.h"
#include "Action.h"

class ofxAntescofog;
class Score;
//class Event;
class ofxTLAntescofoNote;
class Cfwd;
class ActionRect;

using namespace std;

class Display_cfwd {
public:
		string label;
		double grain;
		vector< vector<double> > values;
		//std::vector<double> values; 
		vector<double> delays;
};


class ofxTLAntescofoAction : public ofxTLTrack
{
public:
	ofxTLAntescofoAction(ofxAntescofog* Antescofog);
	~ofxTLAntescofoAction();
	
    friend class ofxTLAntescofoNote;
    
	virtual void setup();
	virtual void draw();
	virtual void update();
	
	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
 	virtual void mouseDragged(ofMouseEventArgs& args, long millis);//bool snapped);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);
	
	virtual void keyPressed(ofKeyEventArgs& args);
    void keyPressed(int key);

    void windowResized(int w, int h);

	virtual void save();
	virtual void load();

  float normalizedXtoScreenX_(float x, ofRange z) { return normalizedXtoScreenX(x, z); }
	ofRange getZoomBounds() { return zoomBounds; }
	ofRectangle getBounds() { return bounds; }
	

    void setNoteTrack(ofxTLAntescofoNote* o) { ofxAntescofoNote = o; }
    ofxTLAntescofoNote *ofxAntescofoNote;
    void setScore(Score* s);
		bool mousePressed_In_Arrow(ofMouseEventArgs& args, list<ActionRect*> actionrects);
    void add_action(float beatnum, string action, Event *e);
		void add_action_curves(float beatnum, ActionRect *ar, Cfwd *c);
    void clear_actions();
		void attribute_header_colors(list<ActionRect*> actionrects);
		ofColor get_random_color();
    void drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground);

    ofTrueTypeFont mFont;
    Score *mScore;
    ofxAntescofog *mAntescofog;
    
    list<ActionRect*> mActionRects;
    bool bEditorShow;
};

class ActionRect
{
public:
    ActionRect(string action_, float beatnum_, Action* a_, Event *e_, ofRectangle rect_) 
			: beatnum(beatnum_), w(0), a(a_), cfwd(0), e(e_), rect(rect_),
			hidden(true), duration(0), nblines(0), /*top_level_group(false),*/
			HEADER_HEIGHT(14), ARROW_LEN(15), LINE_HEIGHT(12), LINE_SPACE(12)
		{
			if (a) {
				string lab = a->label();
				cout << "ActionRect: adding rect with label : " << lab << endl;
				//if (lab.size() && strncmp(lab.c_str(), "top_gfwd_", 9)) {
					title = a->label();
					//top_level_group = true;
					//hidden = false;
				//}
				realtitle = a->label();
				rect.height = HEADER_HEIGHT;

				add_if_present_message(a, e);
				add_if_present_gfwd(a, e);
				add_if_present_lfwd(a, e);
				//add_if_present_cfwd(a, e);
			}
			if (action_.size())
				actions.push_back(action_);
		}
    ~ActionRect() {}

		// message
		void add_if_present_message(Action *a, Event *e) {
			Message *m = dynamic_cast<Message*>(a);
			if (m) {
				ostringstream oss;
				oss << *m;
				string act = oss.str();
				cout << "ActionRect: adding message : " << act <<  endl;
				//ofRectangle tmprect = rect; tmprect.y += HEADER_HEIGHT;
				ActionRect *newact = new ActionRect(act, beatnum + get_delay(a), NULL, e, rect);
				newact->hidden = false;
				ActionRects.push_back(newact);

				duration += get_delay(a);
			}
		}

		// GFWD
		void add_if_present_gfwd(Action *a, Event *e) {
			Gfwd *g = dynamic_cast<Gfwd*>(a);
			if (g) {
				vector<Action*>::const_iterator i;
				for (i = g->actions().begin(); i != g->actions().end(); i++)
				{
					Action *tmpa = *i;
					
					Gfwd *gg = dynamic_cast<Gfwd*>(tmpa);
					if (gg) {
						//ofRectangle tmprect = rect; tmprect.y += HEADER_HEIGHT;
						ActionRect *newact = new ActionRect("", beatnum + get_delay(a), gg, e, rect);
						duration += newact->duration;
						ActionRects.push_back(newact);
					}
					add_if_present_message(tmpa, e);
					// TODO add_if_present_lfwd() whenever, cfwd
				}
				duration += get_delay(a);
			}
		}

		// LFWD
		void add_if_present_lfwd(Action *a, Event *e) {
			Lfwd *l = dynamic_cast<Lfwd*>(a);
			if (l) {
				add_if_present_gfwd(l->_group, e);
				// TODO un truc
			}
		}
		// TODO whenever, cfwd
  
		float get_delay(Action* tmpa) {
			float d = 0.;
			if (tmpa && tmpa->delay() && tmpa->delay()->value()) {
				Expression *e = tmpa->delay()->value();
				if (e && e->is_constant()) {
					d = (float)e->eval().get_double();
					//cout << "ActionRect: duration:" << duration << " delay:" << d << endl;
				}
			}
			return d;
		}
		int get_x(ofxTLAntescofoAction *tlAction, float beat) {
				return tlAction->normalizedXtoScreenX_( tlAction->getTimeline()->beatToNormalizedX(beat), tlAction->getZoomBounds());
		}

  	void draw( ofxTLAntescofoAction *tlAction ) {
			update_rects(tlAction);
			print(tlAction);
			draw_rec(tlAction);
		}

		void update_rects(ofxTLAntescofoAction *tlAction) {
			rect.x =  tlAction->getBounds().x + get_x(tlAction, beatnum);
			rect.y =  tlAction->getBounds().y + 3; // TODO change y
			if (duration == 0)
				rect.width = 200; // TODO make it dynamic depending on actions len
			else
				rect.width = get_x(tlAction, duration);
			// check we're not out of screen
			if (rect.x < tlAction->getBounds().x)
				rect.x = tlAction->getBounds().x;
			if (rect.width > tlAction->getBounds().width)
				rect.width = tlAction->getBounds().width;
			if (hidden) {
				rect.height = HEADER_HEIGHT;
			} else {
				rect.height = get_height(tlAction);
			}
			//for (i = ActionRects.begin(); i != ActionRects.end(); i++) { }
		}

		// get total height with subgroups hidden
		// used as well to propagate x,y
		int get_height(ofxTLAntescofoAction *tlAction) {
			int actionslen = actions.size() * (LINE_HEIGHT + LINE_SPACE) + HEADER_HEIGHT;
			if (ActionRects.empty())
				return actionslen + LINE_SPACE;
			int toth = 0, curh = 0; // find max h
			list<ActionRect*>::const_iterator i;
			int cury = rect.y;
			for (i = ActionRects.begin(); i != ActionRects.end(); i++) {
				(*i)->rect.x = tlAction->getBounds().x + get_x(tlAction, (*i)->beatnum);
				cury += HEADER_HEIGHT;
				(*i)->rect.y = cury;
				if (duration == 0)
					(*i)->rect.width = 200; // TODO make it dynamic depending on actions len
				else (*i)->rect.width = get_x(tlAction, (*i)->duration);
				curh = (*i)->get_height(tlAction);
				if (curh > toth)
					toth += curh;
			}
			return toth;
		}

		void draw_rec(ofxTLAntescofoAction *tlAction) {
			ofNoFill(); // border
			ofSetColor(0, 0, 0, 255);

			//cout << "ActionRects.draw: label:"<< realtitle<< " x:"<<rect.x << " y:" << rect.y << " " << rect.width <<  "x"<< rect.height << endl;
			if (!hidden) {
				int sizec = tlAction->mFont.stringWidth(string("a"));
				if (!actions.size())
					ofRect(rect);
				for (list<string>::const_iterator i = actions.begin(); i != actions.end(); i++) {
					//rect.y += LINE_HEIGHT;
					string c = *i;
					cout << "DRAWING: " << c << " x:" << rect.x << " y:" << rect.y << endl;
					rect.width = sizec * c.size(); // TODO limiter la largeur en fct du suivant
					tlAction->mFont.drawString(c, rect.x, rect.y + 15);
					//ofRect(rect);
				}
				//ofRect(rect);
				list<ActionRect*>::const_iterator i;
				for (i = ActionRects.begin(); i != ActionRects.end(); i++)
				{
					//if (!top_level_group) {
					ofFill(); // rect color filled
					ofSetColor(headerColor);
					int savedh = rect.height;
					rect.height = HEADER_HEIGHT;
					if (duration == 0) // groups with no delay
						rect.width = 200;
					else rect.width = get_x(tlAction, (*i)->duration);
					ofRect(rect);
					rect.height = savedh;
					ofNoFill();
					ofSetColor(0, 0, 0, 255);
					drawArrow(); // arrow
					tlAction->mFont.drawString(title, rect.x, rect.y + LINE_HEIGHT);
					cout << "ActionRects.draw !hidden: ("<<rect.x<<","<<rect.y<<") : " << title << endl;
					//}
					(*i)->draw_rec(tlAction);
				}
			} else { // hidden, just draw header and arrow
				ofFill(); // rect color filled
				ofSetColor(headerColor);
				int savedh = rect.height;
				rect.height = HEADER_HEIGHT;
				if (duration == 0) // groups with no delay
					rect.width = 200;
				else rect.width = get_x(tlAction, duration);
				ofRect(rect);
				rect.height = savedh;
				ofNoFill();
				ofSetColor(0, 0, 0, 255);
				drawArrow(); // arrow
				tlAction->mFont.drawString(title, rect.x, rect.y + LINE_HEIGHT);
				cout << "ActionRects.draw hidden: ("<<rect.x<<","<<rect.y<<") : " << title << endl;
			}
		}

		/*
					2
			 1
					3
		*/
		void drawArrow() {
			cout << "ActionRects.draw arrow ("<<rect.x<<","<<rect.y<<") : hidden:" << hidden << endl; 
			ofFill();
			int xlen = ARROW_LEN;
			int space = 3;
			int halfHeight = HEADER_HEIGHT / 2;

			int x1, y1, x2, y2, x3, y3;
			if (hidden) {
				x1 = rect.x + rect.width - xlen - space;
				y1 = rect.y + halfHeight;
				x2 = rect.x + rect.width - space;
				y2 = rect.y + space;
				x3 = x2;
				y3 = rect.y + HEADER_HEIGHT - space;
			} else {
				x1 = rect.x + rect.width - xlen - space;
				y1 = rect.y + space;
				x2 = rect.x + rect.width - space;
				y2 = y1;
				x3 = rect.x + rect.width - space - xlen/2;
				y3 = rect.y + HEADER_HEIGHT - space;
			}
	
			arrow.setFillColor(ofColor(0, 0, 0, 200));
			arrow.lineTo(x1, y1);
			arrow.lineTo(x2, y2);
			arrow.lineTo(x3, y3);
			arrow.lineTo(x1, y1);
			arrow.draw();
		}

		void print(ofxTLAntescofoAction *tlAction) {
			cout << "print -- ActionRect:" << realtitle << " beat:" <<
				beatnum<<" dur:" << duration << " hidden:"<< hidden; //" height:" << get_height(tlAction);
			for (list<string>::const_iterator i = actions.begin(); i != actions.end(); i++) {
				cout << " action:\""<< *i << "\"";
			}
			cout << " x:" << rect.x << " y:" << rect.y << " " << rect.width << "x" << rect.height << endl;
			list<ActionRect*>::const_iterator i;
			for (i = ActionRects.begin(); i != ActionRects.end(); i++) {
				(*i)->print(tlAction);
			}
		}

		bool is_in_arrow(int x, int y)
		{
			bool res = false;
			if (x > rect.x + rect.width - ARROW_LEN && y < rect.y + HEADER_HEIGHT)
				res = true;
			return res;
		}
#if 0
		// update sets duration to max duration 
		void update() {
			//duration = 0.;
			for (list<ActionRect*>::const_iterator i = ActionRects.begin(); 
					i != ActionRects.end(); i++) {
				cout << "----> duration: "<< (*i)->duration << endl;
				if ((*i)->duration > duration) {
					duration = (*i)->duration;
				}
				(*i)->update();
			}
		}
#endif
		int type;
		list<ActionRect*> ActionRects; 
    list<string> actions;
    ofRectangle rect;
		string trackName;
		string title, realtitle;
    string drawn_action;
    float beatnum;
    int w, nblines;
    Action *a;
		Event *e;
		Display_cfwd* cfwd;
		bool hidden;
		float duration;
		ofColor headerColor;
		//bool top_level_group;
		ofPath arrow;
		int ARROW_LEN, LINE_HEIGHT, LINE_SPACE;
		int HEADER_HEIGHT;
};

