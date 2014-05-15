/**
 * ofxTLAntescofoNote : pianoroll/staffview for Antescofo score langage
 * Copyright (c) 2012-2013 Thomas Coffy - thomas.coffy@ircam.fr
 *
 * derived from ofxTimeline
 *
 * Copyright (c) 2011 James George
 * http://jamesgeorge.org + http://flightphase.com
 * http://github.com/obviousjim + http://github.com/flightphase 
 *
 * implementaiton by James George (@obviousjim) and Tim Gfrerer (@tgfrerer) for the 
 * Voyagers gallery National Maritime Museum 
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * ----------------------
 *
 * ofxTimeline 
 * Lightweight SDK for creating graphic timeline tools in openFrameworks
 */

#pragma once 

#include <Antescofo_AscoGraph.h>
#include "ofxMidi.h"
#ifdef USE_MUSICXML
# include "antescofowriter.h"
#endif
#include "rational.h"
#include <sndfile.h>    // Sound-file reader
#ifdef USE_GUIDO
# include "ofxGuido.h"
#endif

#define ANTESCOFO_REST              0
#define ANTESCOFO_CHORD             1
#define ANTESCOFO_NOTE              2
#define ANTESCOFO_TRILL             3
#define ANTESCOFO_MULTI             4
#define ANTESCOFO_MULTI_STOP        5
#define ANTESCOFO_MULTI_DUMMY       6


class Score;
class ParseDriver;
class ofxTLAntescofoAction;
#ifdef ASCOGRAPH_IOS
# include "iOSAscoGraph.h"
class iOSAscoGraph;
#endif

// representation similar to libmusicxml antescofowriter one
class score_elt {
	public:
		score_elt() { pitches.reserve(10); }
		int 		    type;
		rational 	    duration;
		vector<int>         pitches;
		vector<int>         grace_pitches;
		int		    nMeasure; // measure number
		float		    m_pos; // position in part, in beats unit
		int                 flags; // handles tied notes..
		string              bpm;

		bool operator<(const score_elt& rhs) const { return (m_pos < rhs.m_pos); }
		bool operator==(const score_elt& rhs) { return (m_pos == rhs.m_pos); }
		bool operator==(float pos) { return (m_pos == pos); }
};

typedef struct {
	ofRange beat;

	//ui stuff
	ofRange dragOffsets;
	bool startSelected;
	bool endSelected;
	bool startHovered;
	bool endHovered;
	bool isLast;
	double duration;

	int pitch;
	int velocity;
	int channel;
	bool growing;
	bool triggeredOn;
	bool triggeredOff;
	bool missed;
	bool is_tied;
	int type;
	ofRectangle guidoCoords;
	int notenum;

	string label;
	string action;
	string measure;
	unsigned int lineNum_begin;
	unsigned int colNum_begin;
	unsigned int lineNum_end;
	unsigned int colNum_end;
	vector<float>	    jump_dests;
} ofxTLAntescofoNoteOn;


class ofxTLAntescofoNote : public ofxTLTrack //, public ofxMidiListener
{
	public:
#ifdef ASCOGRAPH_IOS
	ofxTLAntescofoNote(iOSAscoGraph *mAntescofog);
#else
	ofxTLAntescofoNote(ofxAntescofog *mAntescofog);
#endif
	~ofxTLAntescofoNote();
	friend class ofxTLBeatTicker;
    friend class ofxTLAntescofoAction;

	virtual void setup();
	virtual void draw();
	void draw_playhead();

	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);//bool snapped);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);

#if 0
	virtual void zoomStarted(ofxTLZoomEventArgs& zoomEvent);
	virtual void zoomDragged(ofxTLZoomEventArgs& zoomEvent);
	virtual void zoomEnded(ofxTLZoomEventArgs& zoomEvent);
#endif

	virtual void keyPressed(ofKeyEventArgs& args);

	virtual void nudgeBy(ofVec2f nudgePercent);

	virtual void save();
	virtual void load();
#ifdef USE_MUSICXML
	virtual int loadscoreMusicXML(string filename, string outfilename);
#endif
	virtual int loadscoreAntescofo(string filename);
	virtual bool loadscoreAntescofo_fromString(string newscore, string filepath="");
	bool getAccompanimentMarkers(vector<float>& map_index, vector<float>& map_markers);
	bool getAccompanimentMarkers_rec_group(Gfwd *g, vector<float>& map_index, vector<float>& map_markers);

	ofxTLAntescofoAction* getActionTrack() { return ofxAntescofoAction; }
    ofxTLAntescofoAction* createActionTrack();
    void add_action(float beatnum, string action, Event *e);
    void clear_actions();
    void deleteActionTrack();

	void update_duration();
	string get_error();
	void set_error(string e);
	void clear_error();
	virtual void clear();
	int note_pitch2y(int p);
	int note_height();

	void playbackStarted(ofxTLPlaybackEventArgs& args);
	void playbackLooped(ofxTLPlaybackEventArgs& args);
	void playbackEnded(ofxTLPlaybackEventArgs& args);

	virtual string copyRequest();
	virtual string cutRequest();
	virtual void pasteSent(string pasteboard);
	virtual void selectAll();
	virtual void unselectAll();
	virtual void missedAll();
	virtual int getSelectedItemCount();

	void roundedRect(float x, float y, float w, float h, float r);
	void quadraticBezierVertex(float cpx, float cpy, float x, float y, float prevX, float prevY);

	// MIDI
	void newMidiMessage(ofxMidiMessage& eventArgs) { /* TODO ? */ };
	virtual void drawRectChanged();
	ofRange noteRange;

	float convertAntescofoOutputToTime(float mOsc_beat, float mOsc_tempo, float mOsc_pitch);
	void setAutoScroll(bool active);
	void toggleView();
	void showNote(int line);
	ofRectangle getBounds();
	float get_max_note_beat();
	void get_rational_duration(rational& r, float f);

	bool change_action(float beatnum, string action);

	ofColor color_staves_bg, color_gui_bg;
	ofColor color_staves_fg;
	ofColor color_range_black;
	ofColor color_range_white;
	ofColor color_range_txt;
	ofColor color_note, color_note_chord, color_note_trill, color_note_multi;
	ofColor color_note_selected;
	ofColor color_resize_note;
	ofColor color_note_rest;
	ofColor color_resize_note_rest, color_key, color_text, color_highlight, color_disabled, color_modalBg, color_outline;
	antescofo_ascograph_offline *mAntescofo;
	vector<string> cuepoints;
	vector<ofxTLAntescofoNoteOn*>& getSwitches() {return switches;}
    ofTrueTypeFont mFont;
    int fontsize;

	protected:
	virtual void update(ofEventArgs& args);
	bool isSwitchInBounds(ofxTLAntescofoNoteOn* s);
	bool hoveringOn(float hoverY);
	void setNoteColor(int n);
	float lastTimelinePoint;
	void autoscroll();

	void updateDragOffsets(float clickX);
	int howManySwitchesAreSelected();
	bool pointsAreDraggable;
	bool draggingSelectionRange;
	float selectionRangeAnchor;
	ofRange dragSelection;

	void setScore(Score* s);

	// Track Header GUI
	void drawLabel(string caption, ofRectangle bounds);
	void drawGuiBG(ofRectangle rect);
	bool portInHover;
	bool portOutHover;
	ofRectangle portInButtonBounds;
	ofRectangle portOutButtonBounds;
	ofRectangle armInButtonBounds;
	ofRectangle armOutButtonBounds;
	ofRectangle rangeLabelBounds;
	ofRectangle rangeMinSliderBounds;
	ofRectangle rangeMaxSliderBounds;
	ofRectangle rangeTrimButtonBounds;
	ofRectangle getBoundsEastOf(ofRectangle anchor, string label);
	int guiHeaderHeight;

	// MIDI
	ofxMidiIn midiIn;
	ofxMidiOut midiOut;
	int currentInPort;
	int currentOutPort;

	// State
	bool inArmed;
	bool outArmed;
	bool changingRangeMin;
	bool changingRangeMax;
	int changingRangeAnchor;
	bool shouldCreateNewSwitch;
	void setNoteRange(ofRange range);
	void addNote(float beat, int pitch, int velocity, bool growing = false, int channel = 1);
	void endActiveNotes();
	void trimRange();
	bool activeNotes[127];
	int pitchForScreenY(int y);
	bool bMousePressed;

	string getXMLStringForSwitches(bool selectedOnly);
	vector<ofxTLAntescofoNoteOn*> switchesFromXML(ofxXmlSettings xml);
#ifdef USE_MUSICXML
	vector<ofxTLAntescofoNoteOn*> switchesFromMusicXML(string filename, string outfilename);
#endif
	ofxTLAntescofoNoteOn* switchForScreenXY(float screenPos, int y);
	ofxTLAntescofoNoteOn* switchForPoint(float percent, int y);
	ofxTLAntescofoNoteOn* nearestGrowingNoteBeforePointWithPitch(float percent, int pitch);
	ofxTLAntescofoNoteOn* switchHandleForScreenPoint(ofPoint screenPos, bool& startTimeSelected);
	ofxTLAntescofoNoteOn* get_switch_for_beatnum(float beatnum);
	

	vector<ofxTLAntescofoNoteOn*> switches;
	ofxTLAntescofoNoteOn* hoverSwitch;
	bool hoveringStartTime;
	bool hoveringHandle;
#ifdef USE_MUSICXML
	MusicXML2::antescofowriter* AntescofoWriter;
#endif
	float mDur_in_secs, mDur_in_beats, mCurSecs, mCurBeat;
	int getNoteType(Event *e);
	void getcues();

	void draw_showPianoRoll();
	void draw_showStaves();
	
#ifdef USE_GUIDO
	void update_guido();
	void draw_guido();
	bool render_guido(float xfactor);
	void update_guido_render();
	void guido_store_notes();
	string getGuidoString(int fromx, int fromi, int tox, int toi);
	string getGuidoStringNote(int switchnb);
	string getGuidoStringNoteName(int pitch);
	int has_next_event_tied_pitch(int curEvent, int curPitch);
	void add_string_staff(string &str1, string &str2, int staff, string str);
	
	ofxGuido *oguido;
	string guido_string;
	int guido_x, guido_y, guido_w, guido_h; // abs coord
	int scrolled_guido_x, scrolled_guido_y, scrolled_guido_w; // in bounds coord
	float guido_span;
	int mCurGuidoId, mCurSwitchId;
	float mRatioGuido;
	map<float, int> beat2switchId;
	map<int, int> switchId2guidoId;
#endif

	bool bShowPianoRoll;
	bool bAutoScroll;
	float mLastBeat;
#ifdef ASCOGRAPH_IOS
    iOSAscoGraph* mAntescofog;
#else
	ofxAntescofog* mAntescofog;
#endif
    ofxTLAntescofoAction* ofxAntescofoAction;

	// Antescofo score support
	Score       *mNetscore;
	map<int,int> line2note;

	ofImage* noteImage;

	bool bLockNotes;
};
