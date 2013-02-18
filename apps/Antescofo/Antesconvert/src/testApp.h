#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxUI.h"
// libmusicxcml includes
#include "libmusicxml.h"
#include "xml.h"
#include "xmlfile.h"
#include "xmlreader.h"
#include "xml2antescofovisitor.h"
#include "pre_Antescofo.h"
#include "antescofowriter.h"

#define TEXT_CONSTANT_VERSION                    "v0.01"


#define ANTESCOFO_REST              0
#define ANTESCOFO_CHORD             1
#define ANTESCOFO_NOTE              2
#define ANTESCOFO_TRILL             3
#define ANTESCOFO_MULTI             4
#define ANTESCOFO_MULTI_STOP        5

#define TEXT_CONSTANT_TITLE                     "     Antesconvert "
#define TEXT_CONSTANT_SUBTITLE                  " MusicXML score to Antescofo langage:"
#define TEXT_CONSTANT_SUBTITLE2                 "    Drag&drop a MusicXml file"
#define TEXT_CONSTANT_SUBTITLE3                 " then select then click on 'Load score'"
#define TEXT_CONSTANT_BUTTON_LOAD               "Load score"
#define TEXT_CONSTANT_BUTTON_SAVE               "Save score"
#define TEXT_CONSTANT_BUTTON_CONVERT            "Convert"
#define TEXT_CONSTANT_BUTTON_QUIT               "Quit"
#define TEXT_CONSTANT_TEXTINPUT_MEASURES				"measures:"
#define TEXT_CONSTANT_PARSE_ERROR               " /!\\ Score parsing error /!\\  "
#define TEXT_CONSTANT_BUTTON_CANCEL             "Cancel"
#define TEXT_CONSTANT_TEMP_FILENAME             "/tmp/tmpfile-antescofog.txt"
#define TEXT_CONSTANT_TITLE_LOAD_SCORE          "Select a score : MusicXML2 format"
#define TEXT_CONSTANT_TITLE_SAVE_SCORE          "Antesconverted.asco.txt"
#define TEXT_CONSTANT_DONE                      "Conversion done, click on Save score"

class Score;
class ParseDriver;
class antescofo;


// representation similar to libmusicxml antescofowriter one
class score_elt {
public:
	score_elt() { pitches.reserve(10); }
    int 				type;
    rational 			duration;
    vector<int>         pitches;
    vector<int>         grace_pitches;
    int					nMeasure; // measure number
    float				m_pos; // position in part, in beats unit
    int                 flags; // handles tied notes..
    string              bpm;
    
	bool operator<(const score_elt& rhs) const { return (m_pos < rhs.m_pos); }
	bool operator==(const score_elt& rhs) { return (m_pos == rhs.m_pos); }
	bool operator==(float pos) { return (m_pos == pos); }
};


class ofxUIScrollableCanvasMine : public ofxUIScrollableCanvas 
{
	public:
	ofxUIScrollableCanvasMine(float x, float y, float w, float h) : ofxUIScrollableCanvas(x, y, w, h) {}
	void setY(int y) {
		cout << "ofxUIScrollableCanvasMine: oldy: " << rect->y << endl;
		rect->y = y;
		cout << "ofxUIScrollableCanvasMine: y: " << rect->y << endl;
		update();
	}
};

class testApp : public ofBaseApp{
	public:
		void setup();
		void setupContent();
		void cleanContent();
		void update();
		void draw();
		void clear();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void guiEvent(ofxUIEventArgs &e);

		int loadScore(string filename, string outfilename);
		int convertScore(string filename, string outfilename);
		MusicXML2::antescofowriter* AntescofoWriter;

		vector<string> getVoicesVect(map<string, pair<string,bool> > &radios_val);
		vector<string> getStavesVect(map<string, pair<string,bool> > &radios_val);
		vector<string> getPartsVect(map<string, pair<string,bool> > &parts_val);
		vector<int> getMeasuresVect();
		
		// error handling
		string get_error();
		void display_error();
		void draw_error();
		bool bShowError, bErrorInitDone;
		ofxUIScrollableCanvas *guiError;
		ofxUICanvas *guiContent;
		ofxUICanvas *guiSlider;
		ofxUILabelToggle *mWSave;
		ofxUILabelToggle *mWConvert;
		ofxUIRangeSlider *slide;

		list<ofxUIWidget*> widgets;
		string mScore_filename;
};
