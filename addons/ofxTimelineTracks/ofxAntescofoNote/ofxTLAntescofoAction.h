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
class Curve;
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

class ActionGroupHeader;
class ActionGroup;
class ActionMessage;

class ofxTLAntescofoAction : public ofxTLTrack
{
	public:
		ofxTLAntescofoAction(ofxAntescofog* Antescofog);
		~ofxTLAntescofoAction();

		friend class ofxTLAntescofoNote;

		virtual void setup();
		virtual void draw();
		virtual void update();
		void update_groups();
		int update_sub(ActionGroup *ag);
		float update_sub_duration(ActionGroup *ag);
		int update_sub_width(ActionGroup *ag);
		int update_sub_y(ActionGroup *ag);
		void update_avoid_overlap();
		void update_avoid_overlap_rec(ActionGroup* g, int w);

		virtual bool mousePressed(ofMouseEventArgs& args, long millis);
		virtual void mouseMoved(ofMouseEventArgs& args, long millis);
		virtual void mouseDragged(ofMouseEventArgs& args, long millis);//bool snapped);
		virtual void mouseReleased(ofMouseEventArgs& args, long millis);

		virtual void keyPressed(ofKeyEventArgs& args);
		void keyPressed(int key);

		void windowResized(int w, int h);

		virtual void save();
		virtual void load();

		/*
		float normalizedXtoScreenX_(float x, ofRange z) { return normalizedXtoScreenX(x, z); }
		ofRectangle getBounds() { return bounds; }
		*/
		ofRange getZoomBounds() { return zoomBounds; }
		ofRectangle getBoundedRect(ofRectangle& r);
		
		void setNoteTrack(ofxTLAntescofoNote* o) { ofxAntescofoNote = o; }
		ofxTLAntescofoNote *ofxAntescofoNote;
		void setScore(Score* s);
		//bool mousePressed_In_Arrow(ofMouseEventArgs& args, list<ActionRect*> actionrects);
		bool mousePressed_In_Arrow(ofMouseEventArgs& args, ActionGroup* group);
		void add_action(float beatnum, string action, Event *e);
		void add_action_curves(float beatnum, ActionGroup *ar, Curve *c);

		int get_max_note_beat();
		void clear_actions();
		void move_action();
		void attribute_header_colors(list<ActionGroupHeader*> actiongroups);
		ofColor get_random_color();
		void drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground);

		int get_x(float beat);
		string cut_str(int w, string in);

		ofTrueTypeFont mFont;
		Score *mScore;
		ofxAntescofog *mAntescofog;

		list<ActionGroupHeader*> mActionGroups;
		bool bEditorShow;
		bool draggingSelectionRange, movingAction;
		ofRectangle movingActionRect;
		ofPoint selectionRangeAnchor;
		ofRectangle dragSelection;
		ActionGroup* groupFromScreenPoint(int x, int y);
		ActionGroup* groupFromScreenPoint_rec(ActionGroup* group, int x, int y);
		void regionSelected(ofLongRange timeRange, ofRange valueRange);
};

class ActionGroup {
	public:
		ActionGroup(Gfwd* g, Event *e, ActionGroupHeader* header_);
		ActionGroup() {}
		
		virtual ~ActionGroup();

		string get_period();
		double get_delay(Action* tmpa);
		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void print();
		bool is_in_bounds(ofxTLAntescofoAction *tlAction);

		list<ActionGroup*> sons;
		ActionGroupHeader *header;
		Gfwd *gfwd;
		Event *event;
		string trackName;
		float period;
		bool selected;
		float delay;
};


class ActionMessage : public ActionGroup {
	public:
		ActionMessage(Message* g, float delay_, Event* e, ActionGroupHeader* header_);
		virtual ~ActionMessage() {}

		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void print();
		string action;
		int x, y;
};

class ActionMultiCurves : public ActionGroup {
	public:
		ActionMultiCurves(Curve* c, float delay_, Event* e, ActionGroupHeader* header_);
		virtual ~ActionMultiCurves();

		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void print();

		int howmany;
		string label;
		Curve* curve;
};

class ActionCurve : public ActionGroup {
	public:
		ActionCurve(string var, vector<SimpleContFunction>* s_vect, vector<AnteDuration*>* dur_vect, float delay_, Event *e, ActionGroupHeader* header_, ActionMultiCurves* parentCurve_=NULL);
		virtual ~ActionCurve();

		virtual void draw(ofxTLAntescofoAction *tlAction) {}
		virtual void print();
		void addKeyframeAtBeat(float beat, float val);
		void moveKeyframeAtBeat(float to_beat, float from_beat, float to_val, float from_val);
		void changeKeyframeEasing(float beat, string type);
		bool set_dur_val(double d, AnteDuration* a);
		FloatValue* get_new_y(Expression* y);
		bool set_y(Expression* y, double val);
		

		string action;

		string label;
		string varname;
		double grain;
		string symb;
		ActionMultiCurves* parentCurve;
		vector< vector<double> > values;
		SeqContFunction* seq;
		//std::vector<double> values; 
		vector<double> delays;
		
		// parser strucs:
		vector<SimpleContFunction>* simple_vect;
		vector<AnteDuration*>* dur_vect;

};

/*
class ActionLoop : public ActionGroup {
	public:
		ActionLoop(Lfwd *l, float delay_, Event *e, ActionGroupHeader* header_);
		virtual ~ActionLoop() {}

		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void print();
		string action;
		double delay;
		ActionGroup *group;
		Lfwd *lfwd;

		string label;
		float period;
};
*/

class ActionGroupHeader {
	public:
		ActionGroupHeader(float beatnum_, float delay_, Action* a_, Event *e_);
		~ActionGroupHeader();

		virtual void draw(ofxTLAntescofoAction *tlAction);
		void drawArrow(); 
		void print();
		bool is_in_arrow(int x, int y);

		// display
		ofColor headerColor;
		string title, realtitle;
		float duration;
		float beatnum;
		double delay;
		bool hidden;
		ofRectangle rect;
		ofPath arrow;
		int ARROW_LEN, LINE_HEIGHT, LINE_SPACE;
		int HEADER_HEIGHT;
		bool top_level_group;
		// TODO float bpm tempo local a un groupe

		ActionGroup *group;

		// antescofo internal
		Action *action;
		Event *event;
		unsigned int lineNum_begin;
		unsigned int lineNum_end;
};


