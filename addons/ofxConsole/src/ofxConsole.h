#pragma once

#include "ofMain.h"
#include "ofxUI.h"

class ofxConsole
{
	public:
		ofxConsole(int x_, int y_, int w_, int h_, int maxNbLines_);
		void setup();
		void draw();
		void enable();
		void disable();
		void toggleShow();

		void update();

		void appendString(string s) { content += s; }
		void add(string strin);
		void addln(string strin);
		void guiEvent(ofxUIEventArgs &e);
		void sliderEnable();
		void sliderDisable();

	protected:

		bool bShow;
		string content;
		vector<string> contentlines;
		int x, y, w, h, maxNbLines, fontHeight, maxLineSize, firstLineDisplayed, contentSize;
		float maxy;
		ofxUILabel* label;
		ofxUICanvas* canvas;
		ofxUIRangeSlider* slide;
		ofTrueTypeFont mFont;
};
