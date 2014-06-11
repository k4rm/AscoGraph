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
#ifdef ASCOGRAPH_IOS
# include "iOSAscoGraph.h"
#endif
#include "Action.h"
#include "ActionCompound.h"
#include "ActionAtomic.h"
#include "ofxTimeline.h"
#include "ofxTLTrack.h"
#include "pre_Antescofo.h"
#include "Score.h"
#include "Action.h"
#include "Values.h"

#define GROUP_COLOR_ALPHA 80
#define ELEVATOR_WIDTH 12

using namespace std;

#ifndef ASCOGRAPH_IOS
class ofxAntescofog;
#else
class iOSAscoGraph;
#endif
class Score;
class ofxTLAntescofoNote;
class Curve;
class ActionRect;
class BeatCurve;
class ActionGroup;
class ActionMessage;
class ActionCurve;
class ActionMultiCurves;
class TrackState;

class ofxTLAntescofoAction : public ofxTLTrack
{
	public:
#ifndef ASCOGRAPH_IOS
    ofxTLAntescofoAction(ofxAntescofog* Antescofog);
#else
    ofxTLAntescofoAction(iOSAscoGraph* Antescofog);
#endif
		~ofxTLAntescofoAction();

		friend class ofxTLAntescofoNote;

		virtual void setup();
		virtual void draw();
		void windowResized(ofResizeEventArgs& resizeEventArgs);
		virtual void update();
		void update_groups();
		int update_sub_height(ActionGroup *ag);
		int update_sub_height_curve(ActionMultiCurves* c, int& cury, int& curh);
		int update_sub_height_message(ActionMessage* m, int& cury, int& curh);
		float update_sub_duration(ActionGroup *ag);
		int update_sub_width(ActionGroup *ag);
		void update_avoid_overlap();
		void update_avoid_overlap_rec(ActionGroup* g, int w, int h);

		virtual bool mousePressed(ofMouseEventArgs& args, long millis);
		bool mousePressed_in_header(ofMouseEventArgs& args, ActionGroup* group, bool recurs=true);
		virtual bool mousePressed_curve_rec(ActionGroup* a, ofMouseEventArgs& args, long millis);
		virtual bool mousePressed_search_curve_rec(ActionGroup* a, ofMouseEventArgs& args, long millis);
		virtual void mouseMoved(ofMouseEventArgs& args, long millis);
		virtual void mouseDragged(ofMouseEventArgs& args, long millis);//bool snapped);
		virtual void mouseReleased(ofMouseEventArgs& args, long millis);
		vector<ActionMultiCurves*> clickedCurves;
		vector<ActionGroup*> clickedGroups;
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

		void viewWasResized(ofEventArgs& args);
		void resize();
		bool bHasToResize;
		int get_max_note_beat();
		void clear_actions();
		void move_action();
		void attribute_header_colors(vector<ActionGroup*> actiongroups);
		ofColor get_random_color();
		void drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground);

		int get_x(float beat);
		string cut_str(int w, string in);
		void show(string label);
		void show_rec(ActionGroup* a, string label);
		void replaceEditorScore(ActionCurve* actioncurve);

		vector<ActionGroup*> foreground_groups;

		// antescofo tracks
		void draw_tracks_lines();
		void draw_antescofo_tracks_header();
		void draw_antescofo_tracks_header(int x, int y, int sens);
		map<string, TrackState*> mTrackStates;
		bool mFilterActions;
		bool mouseReleased_tracks_header(ofMouseEventArgs& args, long millis);
		void tracks_rec_mark_groups_as_not_displayed(ActionGroup *ag);
		void tracks_mark_group_as_displayed(ActionGroup *ag);
		void tracks_rec_test_if_groups_are_displayed(ActionGroup *ag);
		int mFirstTrackBtn;
		ofRectangle mNextTrackBtn, mPrevTrackBtn;
		int mTrackBtnWidth;
		int mTrackBtnHeight;
		int mTrackBtnSpace;

		ofTrueTypeFont mFont;
		int sizec;
		Score *mScore;
#ifdef ASCOGRAPH_IOS
        iOSAscoGraph *mAntescofog;
#else
		ofxAntescofog *mAntescofog;
#endif
		vector<ActionGroup*> mActionGroups;
		bool bEditorShow;
		bool draggingSelectionRange, movingAction;
		ofRectangle movingActionRect;
		ofPoint selectionRangeAnchor;
		ofRectangle dragSelection;
		ofRectangle mRectCross;
		ActionGroup* groupFromScreenPoint(int x, int y, vector<ActionGroup*>& groups);
		ActionGroup* groupFromScreenPoint_rec(ActionGroup* group, int x, int y);
		void regionSelected(ofLongRange timeRange, ofRange valueRange);
		void show_all_curves();
		void show_all_groups(bool bJustCurves=false);
		void show_all_groups_rec(bool bJustCurves, ActionGroup* gf);
		void hide_all_curves();
		void hide_all_groups(bool bJustCurves=false);
		void hide_all_groups_rec(bool bJustCurves, ActionGroup* gf);

		ofRectangle actualBounds;
		bool is_in_bounds(ActionGroup* g);
		void elevator_enable();
		void elevator_disable();
		void elevator_update();
		void draw_elevator();
		void elevator_mouseMoved(ofMouseEventArgs& args);
		void elevator_mousePressed(ofMouseEventArgs& args);
		void elevator_mouseDragged(ofMouseEventArgs& args);
		void elevator_mouseReleased(ofMouseEventArgs& args);
		bool bElevatorEnabled, bElevatorShowMore;
		ofRectangle mElevatorRect;
		float mElevatorBarHeight, mElevatorBarY;
		int mMaxHeight, mElevatorClickedY, mElevatorStartY, mElevatorBarYClicked, mOldBoundsY;
};

class ActionGroup {
	public:
		ActionGroup(float beatnum_, float delay_, Action* a, Event *e);
		ActionGroup() {}
		virtual ~ActionGroup();

		void createActionGroup(Action* tmpa, Event* e, float delay_);
		void createActionGroup_fill(Action* action);

		string get_period();
		double get_delay(Action* tmpa);
		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual void draw_header(ofxTLAntescofoAction *tlAction, bool draw_rect = true);
		virtual void drawModalContent(ofxTLAntescofoAction *tlAction);
		virtual void print();
		virtual string dump();
		virtual int getHeight();
		bool is_in_bounds(ofxTLAntescofoAction *tlAction);
		bool is_in_bounds_y(ofxTLAntescofoAction *tlAction);

		// hierarchy related
		vector<ActionGroup*> sons;
		string trackName;
		float period;
		string group_type;

		// header related
		void drawArrow(); 
		bool is_in_header(int x, int y);

		// display
		ofColor headerColor;
		string title, realtitle;
		float duration;
		float delay;
		float beatnum;
		bool hidden;
		ofRectangle rect;
		ofPath arrow;
		int ARROW_LEN, LINE_HEIGHT, LINE_SPACE;
		int HEADER_HEIGHT;
		bool top_level_group;
		bool in_selected_track;

		// deep levels
		float deep_level;
		void bringFront() { deep_level = 1.; }
		void bringBack() { deep_level = 0.3; }

		// TODO float bpm tempo local a un groupe

		// antescofo score objects
		Action *action;
		Event *event;
		unsigned int lineNum_begin;
		unsigned int lineNum_end;
		unsigned int colNum_begin;
		unsigned int colNum_end;
};


class ActionMessage : public ActionGroup {
	public:
		ActionMessage(float beatnum_, float delay_, Action* a, Event* e);
		virtual ~ActionMessage() {}

		virtual void draw(ofxTLAntescofoAction *tlAction);
		virtual int getHeight();
		virtual void print();
		string actionstr;
		bool is_kill;
		bool is_proc;
};

class ActionMultiCurves : public ActionGroup {
	public:
		ActionMultiCurves(float beatnum_, float delay_, Curve* c, Event* e);
		virtual ~ActionMultiCurves();

		virtual void draw(ofxTLAntescofoAction *tlAction_);
		virtual void drawModalContent(ofxTLAntescofoAction *tlAction);
		virtual void print();
		virtual string dump();
		int getWidth();
		virtual int getHeight();
		void setWidth(int w);

		float resize_factor;
		ofxTLAntescofoAction *tlAction;
		Curve* antescofo_curve;
		vector<ActionCurve* > curves;
		int howmany, nbvects;
		bool isValid;
		float deep_level;
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

		string action, label, varname;
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


class TrackState {
	public:
		TrackState() : selected(false) {}
		~TrackState() {}

		bool selected;
		ofRectangle rect;
};
