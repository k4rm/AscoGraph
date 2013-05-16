#pragma once

#include "ofMain.h"
#ifdef TARGET_OSX
//# include "ofxCocoaWindowNibless.h"
//# include "ofxCocoaWindow.h"
//# include "ofxCocoa.h"
//# include "Editor.h"
#endif
#include "ofxCocoaDelegate.h"
#include "ofxTimeline.h"
#include "ofxTLZoomer2D.h"
#include "ofxTLAccompAudioTrack.h"
#include "ofxTLBeatTicker.h"
#include "ofxTLAntescofoNote.h"
#include "ofxTLAntescofoAction.h"
#include "ofxColorPicker.h"
#include "ofxUI.h"
#include "ofxOSC.h"
#include "ofxConsole.h"

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
#define TEXT_CONSTANT_BUTTON_PITCH              "Detected Pitch : "
#define TEXT_CONSTANT_BUTTON_SPEED              "Accompaniment speed : "
#define TEXT_CONSTANT_BUTTON_SNAP               "Snap to grid"
#define TEXT_CONSTANT_BUTTON_AUTOSCROLL         "Auto Scroll"
#define TEXT_CONSTANT_BUTTON_PLAY               "Play"
#define TEXT_CONSTANT_BUTTON_START              "Start"
#define TEXT_CONSTANT_BUTTON_STOP               "Stop"
#define TEXT_CONSTANT_BUTTON_NEXT_EVENT         "next event"
#define TEXT_CONSTANT_BUTTON_PREV_EVENT         "prev event"
#define TEXT_CONSTANT_BUTTON_BPM                "BPM"
#define TEXT_CONSTANT_BUTTON_SAVE_COLOR         "Save color"
#define TEXT_CONSTANT_PARSE_ERROR               " /!\\ Antescofo Score parsing error /!\\  "
#define TEXT_CONSTANT_BUTTON_CANCEL             "Cancel"
#define TEXT_CONSTANT_BUTTON_BACK               "Back"
#define TEXT_CONSTANT_TEMP_FILENAME             "/tmp/tmpfile-ascograph.txt"
#define TEXT_CONSTANT_TITLE_LOAD_SCORE          "Select a score : MusicXML2 or Antescofo format"
#define TEXT_CONSTANT_TITLE_SAVE_AS_SCORE       "Save score in Antescofo format"
#define TEXT_CONSTANT_TEMP_ACTION_FILENAME      "/tmp/ascograph_tmp.asco.txt"


@class ofxCodeEditor;
class ofxCocoaWindow;
class ofxTLBeatTicker;


@interface ofxCocoaDelegate (ofxAntescofogAdditions)
- (void)menu_item_hit:(id)sender;
- (void) receiveNotification:(NSNotification *) notification;
@end

class AntescofoTimeline : public ofxTimeline
{
	public: 
	AntescofoTimeline() : ofxTimeline() {}
	virtual ~AntescofoTimeline(){}

	void setZoomer(ofxTLZoomer *z);
};

class ofxAntescofog : public ofBaseApp{
	public:
#ifdef TARGET_OSX
		ofxAntescofog(int argc, char* argv[], ofxCocoaWindow* window);
#else
		ofxAntescofog(int argc, char* argv[]);
#endif

		AntescofoTimeline timeline;

		void setup();
		void setupTimeline();
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
		void editorShowLine(int linea, int lineb);
		void editorDoubleclicked(int line);
		void replaceEditorScore(int linebegin, int lineend, string actstr);

		ofxTLAntescofoNote* ofxAntescofoNote;
		//ofxTLAntescofoAction* ofxAntescofoAction;
		ofxTLBeatTicker *ofxAntescofoBeatTicker;
		ofxTLZoomer2D *ofxAntescofoZoom;
		ofxTLAccompAudioTrack* audioTrack;

	protected:
		// display properties
		int score_x, score_y, score_w, score_h, mUIbottom_y;
		float score_bg_color, score_fg_color;
		int score_line_x, score_line_y, score_line_w, score_line_h;
		int score_line_space;

		int bpm;
		bool bSnapToGrid, bAutoScroll, bSetupDone;
		string tmpfilename; // file to store converted MusicXML file to Antescofo score
		string mScore_filename;


		vector<float> accomp_map_index, accomp_map_markers;

#ifdef TARGET_OSX
		ofxCocoaWindow*	cocoaWindow;
#endif

		// UI
		ofxUICanvas *guiTop, *guiBottom, *guiSetup_OSC;
		ofxUICanvas *guiSetup_Colors;
		ofxUIScrollableCanvas *guiError;
		ofxUISlider *mSliderBPM;
		ofxUILabel  *mLabelBeat, *mLabelPitch, *mLabelAccompSpeed;
		ofxUILabelButton *mSaveColorButton;
		void exit();
		void guiEvent(ofxUIEventArgs &e);
		ofxUIDropDownList *mUImenu;
		ofFbo	drawCache;
		bool bShouldRedraw;
		//ofRectangle logoInria;
		ofImage mLogoInria, mLogoIrcam;

		// OpenSoundControl communication with MAX/MSP or PureData
		ofxOscReceiver  mOSCreceiver;
		ofxOscSender    mOSCsender;
		bool            mHasReadMessages;
		string          mOsc_host, mOsc_port, mOsc_port_MAX;
		char            mOSCmsg_string[20];
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

		// OSC setup
		void draw_OSCSetup();
		bool bShowOSCSetup, bOSCSetupInitDone;
		map<string, string *> oscString2var;
		unsigned long long mLastOSCmsgDate;

		// error handling
		void display_error();
		void draw_error();
		bool bShowError, bErrorInitDone;

		// open/save file
		int loadScore(string filename);
		void saveScore();
		void saveAsScore();

		bool bEditorShow;

		struct timeval last_draw_time;
};
