//
//  ofxAntescofoAction.h
//  part of AscoGraph : graphical editor for Antescofo musical scores.
//
//  Created by Thomas Coffy on 06/12/12.
//  Licensed under the Apache License : http://www.apache.org/licenses/LICENSE-2.0
//

#pragma once

#include <iostream>
#include <list>
#include "Action.h"
#include "ActionCompound.h"
#include "ActionAtomic.h"
#include "ofxTimeline.h"
#include "ofxTLTrack.h"
#include "pre_Antescofo.h"
#include "Score.h"
#include "Action.h"


using namespace std;

class ofxAntescofog;
class Score;
class ofxTLAntescofoNote;
class Curve;
class ActionRect;
class BeatCurve;
class ActionGroupHeader;
class ActionGroup;
class ActionMessage;
class ActionCurve;
class ActionMultiCurves;

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
		int update_sub_height(ActionGroup *ag);
		int update_sub_height_curve(ActionMultiCurves* c, int& cury, int& curh);
		float update_sub_duration(ActionGroup *ag);
		int update_sub_width(ActionGroup *ag);
		void update_avoid_overlap();
		void update_avoid_overlap_rec(ActionGroup* g, int w);

		virtual bool mousePressed(ofMouseEventArgs& args, long millis);
		bool mousePressed_In_Arrow(ofMouseEventArgs& args, ActionGroup* group);
		virtual bool mousePressed_curve_rec(ActionGroup* a, ofMouseEventArgs& args, long millis);
		virtual bool mousePressed_search_curve_rec(ActionGroup* a, ofMouseEventArgs& args, long millis);
		virtual void mouseMoved(ofMouseEventArgs& args, long millis);
		virtual void mouseDragged(ofMouseEventArgs& args, long millis);//bool snapped);
		virtual void mouseReleased(ofMouseEventArgs& args, long millis);
		vector<ActionMultiCurves*> clickedCurves;
		bool shouldDrawModalContent;

		virtual void keyPressed(ofKeyEventArgs& args);
		void keyPressed_curve_rec(ActionGroup* a, ofKeyEventArgs& key);
		
		void windowResized(int w, int h);

		virtual void save();
		virtual void load();
		ofRange getZoomBounds() { return zoomBounds; }
		ofRectangle getBoundedRect(ofRectangle& r);
		ofRectangle getBounds();
		
		void setNoteTrack(ofxTLAntescofoNote* o) { ofxAntescofoNote = o; }
		ofxTLAntescofoNote *ofxAntescofoNote;
		void setScore(Score* s);
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
		void show(string label);
		void show_rec(ActionGroup* a, string label);
		void replaceEditorScore(ActionCurve* actioncurve);

		ofTrueTypeFont mFont;
		Score *mScore;
		ofxAntescofog *mAntescofog;

		list<ActionGroupHeader*> mActionGroups;
		bool bEditorShow;
		bool draggingSelectionRange, movingAction;
		ofRectangle movingActionRect;
		ofPoint selectionRangeAnchor;
		ofRectangle dragSelection;
		ofRectangle mRectCross;
		ActionGroup* groupFromScreenPoint(int x, int y);
		ActionGroup* groupFromScreenPoint_rec(ActionGroup* group, int x, int y);
		void regionSelected(ofLongRange timeRange, ofRange valueRange);
		void show_all_curves();
		void show_all_groups(bool bJustCurves=false);
		void show_all_groups_rec(bool bJustCurves, ActionGroup* gf);
};

class ActionGroup {
	public:
		ActionGroup(Action* a, Event *e, ActionGroupHeader* header_);
		ActionGroup() {}
		
		virtual ~ActionGroup();

		void createActionGroup(Action* tmpa, Event* e, float delay_);
		string get_period();
		double get_delay(Action* tmpa);
		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void drawModalContent(ofxTLAntescofoAction *tlAction);
		virtual void print();
		virtual string dump();
		virtual int getHeight();
		bool is_in_bounds(ofxTLAntescofoAction *tlAction);

		list<ActionGroup*> sons;
		ActionGroupHeader *header;
		Action *action;
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
		ActionMultiCurves(/*ofxTLAntescofoAction *tlAction_, */ Curve* c, float delay_, Event* e, ActionGroupHeader* header_);
		virtual ~ActionMultiCurves();

		virtual void draw(ofxTLAntescofoAction *tlAction_);
		virtual void drawModalContent(ofxTLAntescofoAction *tlAction);
		virtual void print();
		virtual string dump();
		void show();
		void hide();
		int getWidth();
		virtual int getHeight();
		void setWidth(int w);

		float resize_factor;
		ofxTLAntescofoAction *tlAction;
		Curve* antescofo_curve;
		vector<ActionCurve* > curves;
		int howmany, nbvects;
		string label;
		bool isValid;
};

class ActionCurve {
	public:
		ActionCurve(list<Var*> &var, SeqContFunction* seq, vector<AnteDuration*>* dur_vect, float delay_, Event *e, ActionMultiCurves* parentCurve_=NULL);
		virtual ~ActionCurve();

		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void drawModalContent(ofxTLAntescofoAction *tlAction);
		virtual void drawSplitBtn(ofxTLAntescofoAction *tlAction);
		virtual void print();
		bool addKeyframeAtBeat(float beat, float val);
		void deleteKeyframeAtBeat(float beat);
		void moveKeyframeAtBeat(float to_beat, float from_beat, float to_val, float from_val);
		void changeKeyframeEasing(float beat, string type);
		bool set_dur_val(double d, AnteDuration* a);
		FloatValue* get_new_y(Expression* y);
		bool set_y(Expression* y, double val);
		virtual void split();
		int getWidth();
		int getHeight();
		void setWidth(int w);
		void setHeight(int h);
		bool create_from_parser_objects(list<Var*> &var, vector<AnteDuration*>* dur_vect_, ActionMultiCurves* parentCurve_);
		ofRectangle mSplitBtnRect;

		string action;

		string label;
		string varname;
		double grain;
		string symb;
		ActionMultiCurves* parentCurve;
		vector<BeatCurve* > beatcurves;
		vector< vector<double> > values;
		SeqContFunction* seq;
		vector<double> delays;
		list<Var*> &vars;
		
		// parser strucs:
		vector<SimpleContFunction>* simple_vect;
		vector<AnteDuration*>* dur_vect;
};


class ActionGroupHeader {
	public:
		ActionGroupHeader(float beatnum_, float delay_, Action* a_, Event *e_, ActionGroup* parentGroup_ = NULL);
		~ActionGroupHeader();

		virtual void draw(ofxTLAntescofoAction *tlAction);
		void drawArrow(); 
		void print();
		virtual string dump();
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
		ActionGroup *parentGroup; // used for MultiCurves delay passing, null for others objects

		// antescofo internal
		Action *action;
		Event *event;
		unsigned int lineNum_begin;
		unsigned int lineNum_end;
		unsigned int colNum_begin;
		unsigned int colNum_end;
};


