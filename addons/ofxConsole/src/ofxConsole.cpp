#include "ofxConsole.h"

ofxConsole* console;

ofxConsole::ofxConsole(int x_, int y_, int w_, int h_, int maxNbLines_) : maxNbLines(maxNbLines_), x(x_), y(y_), w(w_), h(h_)
{
	maxLineSize = 100;
	firstLineDisplayed = 1;
	contentSize = 0;
	setup();
	bShow = false;
}


void ofxConsole::setup()
{
	//mFont.loadFont ("DroidSansMono.ttf", 7, true, true, true);
	mFont.loadFont ("menlo.ttf", 8, true, true, true);
	string s("L");
	fontHeight = mFont.stringHeight(s) + mFont.getSpaceSize();

	maxy = 0;
  canvas = new ofxUICanvas(x, y, 20, h);
	slide = new ofxUIRangeSlider(x+1, y+1, 12, h, 0, 25, 0, 255, "slide", OFX_UI_FONT_SMALL);
	slide->setLabelPrecision(0);
	slide->setLabelVisible(false);
	canvas->addWidgetDown(slide);
	//canvas->setTheme(OFX_UI_THEME_BLUEBLUE);
	/*
	label = new ofxUILabel(w, "label", 100);
	label->setFont(&mFont);
  canvas->addWidgetRight(label);
	*/


	canvas->getRect()->height = h;//maxy + 100;
	//sliderEnable();
	ofAddListener(canvas->newGUIEvent, this, &ofxConsole::guiEvent);

	disable();
}

void ofxConsole::enable()
{
	bShow = true;
	canvas->enable();
	sliderEnable();
}


void ofxConsole::disable()
{
	bShow = false;
	canvas->disable();
	sliderDisable();
}


void ofxConsole::toggleShow()
{
	if (bShow)
		disable();
	else
		enable();
}


void ofxConsole::update()
{
	//vector<string> conlines = ofSplitString(content, "\n");
}

void ofxConsole::draw()
{
	if (!bShow) return;
	ofNoFill();
	ofSetColor(0, 0, 0, 200);
	//ofSetColor(255, 255, 255, 200);
	ofRect(x, y, w, h);
	ofFill();
	ofSetColor(0, 0, 0, 10);

	ofRect(x, y, w, h);
	ofSetColor(0, 0, 0, 210);

	int b = 0, d = 0;
	int len;
	for (int i = firstLineDisplayed; i < contentlines.size(); i++)
	{
		int n = i - firstLineDisplayed + 1;
		string s;
		if (contentlines[i].size() > maxLineSize) {
			s = contentlines[i];
			int len = s.size();
			while (b > maxLineSize) {
				len = ( (contentlines[i]).size() - b > maxLineSize ? maxLineSize : (contentlines[i]).size() - b);
				s = (contentlines[i]).substr(b, len);
				s += "\n";
				b += s.size();
				mFont.drawString(s, x + 27, y + (n+d)*fontHeight);
			}
			d = b;
			b = 0;
		} else {	
			s = contentlines[i] ;//.substr(maxLineSize);
			mFont.drawString(s, x + 27, y + (n+d)*fontHeight);
		}
	}
}

void ofxConsole::addln(string strin)
{
	strin += "\n";
	add(strin);
}

void ofxConsole::add(string strin)
{
	cout << "console:> "<< strin;
	vector<string> lines = ofSplitString(strin, "\n");

	contentlines.insert( contentlines.end(), lines.begin(), lines.end() );
		
	while (contentlines.size() > maxNbLines) { contentlines.erase(contentlines.begin()); }
	for (vector<string>::iterator i = contentlines.begin(); i != contentlines.end(); i++) {
		content += *i;
	}
	//label->setLabel(content);
	maxy = fontHeight * contentlines.size();
	contentSize = contentlines.size();

	if (contentSize < maxNbLines)
		firstLineDisplayed = 0;
	//return c;
	//return strin;
}


void ofxConsole::guiEvent(ofxUIEventArgs &e)
{
	if(e.widget->getName() == "slide")
	{
		//if (_maxy > ofGetHeight()) {
			ofxUIRangeSlider *r = (ofxUIRangeSlider*)e.widget;
			float m = (r->getPercentValueLow() + r->getPercentValueHigh()) / 2;
			/*
			cout << "maxy: " << maxy << endl;
			float ny = (m - 0.75) * maxy;
			//float ny = (m - 0.75) * contentlines.size();
			cout << "got slide clicked: medium percent = " << m << " oldy:"<< canvas->getRect()->y << " ny:" << ny << endl;
			if (ny + maxy + 200 < 0) ny = maxy + 200;
			if (ny + maxy - 200 > ofGetHeight()) ny = 0;
			canvas->getRect()->y = ny + y;
			*/
			cout << "got slide clicked: medium percent = " << m << " firstline was::"<< firstLineDisplayed << " newfirstline:"<<firstLineDisplayed << endl;
			firstLineDisplayed = (1 - m) * contentSize;
		//} 
	}

}


void ofxConsole::sliderEnable()
{
	slide->setVisible(true);
	slide->setLabelVisible(false);
	canvas->enable();
}

void ofxConsole::sliderDisable()
{
	slide->setVisible(false);
	canvas->disable();
	canvas->setVisible(false);
	//guiContent->getRect()->x = 0;
	//guiContent->getRect()->width = ofGetWidth();
}


