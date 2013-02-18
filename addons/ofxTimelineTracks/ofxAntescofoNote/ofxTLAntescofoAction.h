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
#include "ofxTLTrack.h"

class ofxAntescofog;
class Score;
class Event;
class ofxTLAntescofoNote;

class Display_cfwd {
public:
		string label;
		double grain;
		vector< vector<double> > values;
		//std::vector<double> values; 
		vector<double> delays;
};


class ActionRect
{
public:
    ActionRect(string action_, float beatnum_, Event* e_) : action(action_), beatnum(beatnum_), w(0), e(e_), cfwd(0) {}
    ~ActionRect() {}
    
    ofRectangle rect;
		string trackName;
    string action;
    string drawn_action;
    float beatnum;
    int w;
    Event *e;
		Display_cfwd* cfwd;
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

    void setNoteTrack(ofxTLAntescofoNote* o) { ofxAntescofoNote = o; }
    ofxTLAntescofoNote *ofxAntescofoNote;
    void setScore(Score* s);
    void add_action(float beatnum, string action, Event *e);
    void clear_actions();
    void drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground);

    ofTrueTypeFont mFont;
    Score *mScore;
    ofxAntescofog *mAntescofog;
    
    list<ActionRect*> mActionRects;
    bool bEditorShow;
};
