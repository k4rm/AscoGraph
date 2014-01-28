#pragma once

#include "ofMain.h"
#include "ofxNSWindowApp.h"
#include "ofxCocoaDelegate.h"
#include "ofxTimeline.h"
#include "ofxTLZoomer2D.h"
#include "ofxTLAccompAudioTrack.h"
#include "ofxTLBeatTicker.h"
#include "ofxTLAntescofoNote.h"
#include "ofxTLAntescofoAction.h"
#include "ofxTLAntescofoSim.h"
#include "ofxColorPicker.h"
#include "ofxUI.h"
#include "ofxOSC.h"
#include "ofxConsole.h"
#ifdef USE_HTTPD
#include "ofxHTTPServer.h"
#endif
#include "ofxTLBeatJump.h"


#define INT_CONSTANT_BUTTON_RELOAD      0
#define INT_CONSTANT_BUTTON_LOAD        1
#define INT_CONSTANT_BUTTON_SAVE        2
#define INT_CONSTANT_BUTTON_COLORSETUP  3
#define INT_CONSTANT_BUTTON_OSCSETUP    4
#define INT_CONSTANT_BUTTON_TOGGLEVIEW  5
#define INT_CONSTANT_BUTTON_TOGGLEEDIT  6    
#define INT_CONSTANT_BUTTON_QUIT        7
#define INT_CONSTANT_BUTTON_SELECTALL   8
#define INT_CONSTANT_BUTTON_AUTOSCROLL  9
#define INT_CONSTANT_BUTTON_SNAP        10
#define INT_CONSTANT_BUTTON_PLAY        11
#define INT_CONSTANT_BUTTON_START       12
#define INT_CONSTANT_BUTTON_STOP        13
#define INT_CONSTANT_BUTTON_SAVE_AS     14
#define	INT_CONSTANT_BUTTON_LINEWRAP    15
#define	INT_CONSTANT_BUTTON_FIND        16
#define	INT_CONSTANT_BUTTON_CREATE_GROUP	17
#define	INT_CONSTANT_BUTTON_CREATE_WHENEVER	18
#define	INT_CONSTANT_BUTTON_CREATE_CURVE	19
#define	INT_CONSTANT_BUTTON_CREATE_CURVES	20
#define	INT_CONSTANT_BUTTON_CREATE_LOOP		21
#define	INT_CONSTANT_BUTTON_CREATE_OSCSEND	22
#define	INT_CONSTANT_BUTTON_CREATE_OSCRECV	23
#define INT_CONSTANT_BUTTON_PLAYSTRING		24
#define INT_CONSTANT_BUTTON_NEW			25
#define INT_CONSTANT_BUTTON_SIMULATE		26
#define INT_CONSTANT_BUTTON_EDIT		27
#define INT_CONSTANT_BUTTON_PREVEVENT		28
#define INT_CONSTANT_BUTTON_NEXTEVENT		29
#define INT_CONSTANT_BUTTON_ZOOM_IN		30
#define INT_CONSTANT_BUTTON_ZOOM_OUT		31
#define INT_CONSTANT_BUTTON_OPEN_ALL_CURVES	32
#define INT_CONSTANT_BUTTON_OPEN_ALL_GROUPS	33
#define INT_CONSTANT_BUTTON_SHOWHIDE_ACTION  	34
#define INT_CONSTANT_BUTTON_HIDE		35
#define INT_CONSTANT_BUTTON_UNDO		36
#define INT_CONSTANT_BUTTON_REDO		37
#define INT_CONSTANT_BUTTON_CUES_INDEX  	300

#define TEXT_CONSTANT_TITLE                     "Ascograph: score editor"
#define TEXT_CONSTANT_BUTTON_LOAD               "Load score"
#define TEXT_CONSTANT_BUTTON_RELOAD             "Reload score"
#define TEXT_CONSTANT_BUTTON_SAVE               "Save score"
#define TEXT_CONSTANT_BUTTON_SAVE_AS            "Save score as"
#define TEXT_CONSTANT_BUTTON_COLOR_SETUP        "Color preferences"
#define TEXT_CONSTANT_BUTTON_OSC_SETUP          "OSC Setup"
#define TEXT_CONSTANT_BUTTON_COLOR_SETUP_EXIT   "Exit color preferences"
#define TEXT_CONSTANT_BUTTON_OSC_SETUP_EXIT     "Exit OSC Setup"
#define TEXT_CONSTANT_BUTTON_TOGGLE_VIEW        "Toggle view"
#define TEXT_CONSTANT_BUTTON_TOGGLE_EDITOR      "Toggle Editor"
#define TEXT_CONSTANT_BUTTON_BEAT               "Position in score (in beats): "
#define TEXT_CONSTANT_BUTTON_PITCH              "Detected Pitch: "
#define TEXT_CONSTANT_BUTTON_BPM              	"Detected BPM: "
#define TEXT_CONSTANT_BUTTON_SPEED              "Accompaniment speed : "
#define TEXT_CONSTANT_BUTTON_SNAP               "Snap to grid"
#define TEXT_CONSTANT_BUTTON_AUTOSCROLL         "Auto Scroll"
#define TEXT_CONSTANT_BUTTON_LINEWRAP		"Toggle line wrapping"
#define TEXT_CONSTANT_BUTTON_PLAY               "Play"
#define TEXT_CONSTANT_BUTTON_SIMULATE           "Simulate"
#define TEXT_CONSTANT_BUTTON_EDIT		"Edit"
#define TEXT_CONSTANT_BUTTON_PLAYSTRING         "Play string"
#define TEXT_CONSTANT_BUTTON_START              "Start"
#define TEXT_CONSTANT_BUTTON_STOP               "Stop"
#define TEXT_CONSTANT_BUTTON_NEXT_EVENT         "next event"
#define TEXT_CONSTANT_BUTTON_PREV_EVENT         "prev event"
#define TEXT_CONSTANT_BUTTON_SAVE_COLOR         "Save color"
#define TEXT_CONSTANT_BUTTON_CUEPOINTS		"cue points"
#define TEXT_CONSTANT_PARSE_ERROR               " /!\\ Antescofo Score parsing error /!\\  "
#define TEXT_CONSTANT_SIMULATION_ERROR          " /!\\ Antescofo performance simulation error /!\\  "
#define TEXT_CONSTANT_BUTTON_CANCEL             "Cancel"
#define TEXT_CONSTANT_BUTTON_BACK               "Back"
#define TEXT_CONSTANT_BUTTON_FIND               "Find"
#define TEXT_CONSTANT_BUTTON_REPLACE       	"Replace"
#define TEXT_CONSTANT_BUTTON_TEXT               "Text"
#define TEXT_CONSTANT_BUTTON_REPLACE_TEXT	"Replaced Text"
#define TEXT_CONSTANT_BUTTON_REPLACE_NB		"Number of replacement:"
#define TEXT_CONSTANT_TEMP_FILENAME             "/tmp/tmpfile-ascograph.txt"
#define TEXT_CONSTANT_TITLE_LOAD_SCORE          "Select a score : MusicXML2 or Antescofo format"
#define TEXT_CONSTANT_TITLE_SAVE_AS_SCORE       "Save score in Antescofo format"
#define TEXT_CONSTANT_TEMP_ACTION_FILENAME      "/tmp/ascograph_tmp.asco.txt"


@class ofxCodeEditor;
class ofxCocoaWindow;
class ofxTLBeatTicker;
class rational;

@interface ofxCocoaDelegate (ofxAntescofogAdditions)
- (void)menu_item_hit:(id)sender;
- (void) receiveNotification:(NSNotification *) notification;
@end

// simulation structures
class curveval {
public:
	curveval(double now_, double rnow_, double val_)
		: now(now_), rnow(now_), val(val_)
	{}
	double now, rnow, val;
};
class curve_trace {
public:
	curve_trace(string name_, string fathername_, string varname_, double now_, double rnow_, double val_)
		: name(name_), fathername(fathername_), now(now_), rnow(rnow_), varname(varname_)
	{
		values.push_back(new curveval(now, rnow, val_));
		min = val_; 
		max = val_;
	}
	string name;
	string varname;
	string fathername;
	vector<curveval*> values;
	double now, rnow; // time of first element
	ofRectangle rect;
	double min, max;
	ofPolyline line;
};

class action_trace {
  public:
	action_trace(string name_, string fathername_, double now_, double rnow_, string s_)
		: name(name_), fathername(fathername_), now(now_), rnow(rnow_), s(s_), nbcurves(0)
	{}
	ofRectangle rect;
	string name;
	string fathername;
	double now, rnow;
	string s;
	int nbcurves;
};


class AntescofoTimeline : public ofxTimeline
{
	public: 
	AntescofoTimeline() : ofxTimeline() {}
	virtual ~AntescofoTimeline(){}

	void setZoomer(ofxTLZoomer *z);

	void keypressed(ofKeyEventArgs& args) {}
};

//class ofxAntescofog : public ofBaseApp
class ofxAntescofog : public ofxNSWindowApp
#ifdef USE_HTTPD
		      , public ofxHTTPServerListener
#endif
{
	public:
#ifdef TARGET_OSX
		ofxAntescofog(int argc, char* argv[], ofxCocoaWindow* window);
#else
		ofxAntescofog(int argc, char* argv[]);
#endif

		AntescofoTimeline timeline, timelineSim;

		void setup();
		void setupTimeline();
		void setupTimelineSim();
		void setupUI();
		void setupOSC();
		void update();
		void draw();
		void load();
		void save();
		void menu_item_hit(int n);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved( int x, int y);
		void mouseDragged( int x, int y, int button);
		void mousePressed( int x, int y, int button);
		void mouseReleased( int x, int y, int button);
		void windowResized(ofResizeEventArgs& resizeEventArgs);//int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void addTrackCurve(string trackname);

		void parse_AntescofoScore(const string filename);
		void setEditorMode(bool state, float beatn);
		ofxCodeEditor* editor;
		void editorShowLine(int linea, int lineb, int cola, int colb);
		void editorDoubleclicked(int line);
		void replaceEditorScore(int linebegin, int lineend, int cola, int colb, string actstr);
		void createCodeTemplate(int which);
		void showJumpTrack();
		
		ofxTLAntescofoNote* ofxAntescofoNote, *ofxAntescofoNoteSim;
		ofxTLBeatTicker *ofxAntescofoBeatTicker, *ofxAntescofoBeatTickerSim;
		ofxTLZoomer2D *ofxAntescofoZoom, *ofxAntescofoZoomSim;
		ofxTLAccompAudioTrack* audioTrack;
		ofxTLAntescofoSim* ofxAntescofoSim;
		ofxTLBeatJump* ofxJumpTrack;

	protected:
		// display properties
		int score_x, score_y, score_w, score_h, mUIbottom_y;
		float score_bg_color, score_fg_color;
		int score_line_x, score_line_y, score_line_w, score_line_h;
		int score_line_space;

		int bpm;
		bool bSnapToGrid, bAutoScroll, bSetupDone, bLineWrapMode, bScoreFromCommandLine;
		string tmpfilename; // file to store converted MusicXML file to Antescofo score
		string mScore_filename;

		vector<float> accomp_map_index, accomp_map_markers;

#ifdef TARGET_OSX
		ofxCocoaWindow*	cocoaWindow;
#endif

		// UI
		id mCuesMenuItem, mCuesMenu;
		ofxUICanvas *guiTop, *guiBottom, *guiSetup_OSC, *guiElevator;
		ofxUICanvas *guiSetup_Colors, *guiFind;
		ofxUIScrollableCanvas *guiError;
		ofxUISlider *mSliderBPM;
		ofxUILabel  *mLabelBeat, *mLabelBPM, *mLabelPitch, *mLabelAccompSpeed, *mFindReplaceOccur;
		ofxUILabelButton *mSaveColorButton;
		ofxUILabelToggle *mEditButton;
		void exit();
		void guiEvent(ofxUIEventArgs &e);
		ofxUIDropDownList *mUImenu;
		ofFbo	drawCache;
		bool bShouldRedraw;
		//ofRectangle logoInria;
		ofImage mLogoInria, mLogoIrcam;
		vector<string> cuepoints;
		string mPlayLabel;
		ofxUIDropDownList* mCuepointsDdl;
		float* mBPMbuffer;
		void push_tempo_value();
		map<int, string> mCuesIndexToString;
		int mCuesMaxIndex;
		void cues_clear_menu();
		void cues_add_menu(string s);
		void newWindow();
		ofxCocoaWindow* subWindow;

		// OpenSoundControl communication with MAX/MSP or PureData
		ofxOscReceiver  mOSCreceiver;
		ofxOscSender    mOSCsender, mOSCsender_www;
		bool            mHasReadMessages;
		string          mOsc_host, mOsc_port, mOsc_port_MAX;
		float           mOsc_beat, mOsc_tempo, mOsc_pitch, mOsc_rnow, mOsc_accomp_speed;
		float           fAntescofoTimeSeconds;

		// color chooser
		ofxColorPicker	mColorPicker;
		map<string, ofColor*> colorString2var;
		bool bShowColorSetup, bColorSetupInitDone;
		void draw_ColorSetup();
		void draw_ColorPicker(string name);
		void save_ColorPicker(string name);
		void draw_ColorAsset(string name, ofColor *color);
		string mColorChanged;
		ofxUIRangeSlider* elevator;
		void elevatorEnable();
		void elevatorDisable();

		// OSC setup
		void draw_OSCSetup();
		bool bShowOSCSetup, bOSCSetupInitDone;
		map<string, string *> oscString2var;
		unsigned long long mLastOSCmsgDate;
		ofxUITextInput* mTextOscPort, *mTextOscHost, *mTextOscPortRemote;

		// error handling
		void display_error();
		void draw_error();
		bool bShowError, bErrorInitDone;

		// open/save file
		int loadScore(string filename, bool reloadEditor, bool sendOsc = true);
		void saveScore(bool stopSimu = true);
		void saveAsScore();
		void newScore();
		void askToSaveScore();
		bool edited();

		// MIDI file conversion
		void setup_Midi(string& midifile, bool do_actions);
		string convertMidiFileToActions(string& midifile);
		string convertMidiFileToNotes(string& midifile);

		// simulation
		bool bIsSimulating;
		void simulate();
		void stop_simulate_and_goedit();
		void draw_simulate();

		bool bEditorShow;

		struct timeval last_draw_time;

		// find text
		bool bFindTextInitDone, bShowFind;
		void draw_FindText();

#ifdef USE_HTTPD
		// httpd
		ofxHTTPServer * httpd_server;
		ofPoint radius[20]; // for drawing
		ofImage image;
		bool imageServed;
		bool imageSaved;
		void setup_httpd();
		void getRequest(ofxHTTPServerResponse & response);
		void postRequest(ofxHTTPServerResponse & response);
		void fileNotFound(ofxHTTPServerResponse& response){}
		void draw_http_image();
		void update_http_image();
		void httpd_update_beatpos();
		string postedImgName;
		string postedImgFile;
		string prevPostedImg;
		ofImage postedImg;
#endif

};

