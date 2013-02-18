/**
 * ofxTimeline
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
#include "ofxTLTrack.h"
#include "ofxMidi.h"

typedef struct {
	ofRange time;
	
	//ui stuff
	ofRange dragOffsets;
	bool startSelected;
	bool endSelected;
	bool startHovered;
	bool endHovered;
	
	int pitch;
	int velocity;
	int channel;
	bool growing;
	bool triggeredOn;
	bool triggeredOff;
} ofxTLMidiNoteOn;

class ofxTLMidiNote : public ofxTLTrack, public ofxMidiListener
{
  public:
	ofxTLMidiNote();
	~ofxTLMidiNote();
	
	virtual void setup();
	virtual void draw();
	
	virtual void mousePressed(ofMouseEventArgs& args);
	virtual void mouseMoved(ofMouseEventArgs& args);
	virtual void mouseDragged(ofMouseEventArgs& args,bool snapped);
	virtual void mouseReleased(ofMouseEventArgs& args);
	
	virtual void keyPressed(ofKeyEventArgs& args);
	
	virtual void nudgeBy(ofVec2f nudgePercent);
	
	virtual void save();
	virtual void load();
	
	virtual void clear();
	
	void playbackStarted(ofxTLPlaybackEventArgs& args);
	void playbackLooped(ofxTLPlaybackEventArgs& args);
	void playbackEnded(ofxTLPlaybackEventArgs& args);
	
	virtual bool isOn(float percent);
	
	virtual string copyRequest();
	virtual string cutRequest();
	virtual void pasteSent(string pasteboard);
	virtual void selectAll();
	virtual void unselectAll();
	
	// MIDI
	void newMidiMessage(ofxMidiMessage& eventArgs);
	virtual void drawRectChanged();
	ofRange noteRange;
	
  protected:
	virtual void update(ofEventArgs& args);
	bool isSwitchInBounds(ofxTLMidiNoteOn* s);
	bool hoveringOn(float hoverY);
	float lastTimelinePoint;
	
	void updateDragOffsets(float clickX);
	int howManySwitchesAreSelected();
	bool pointsAreDraggable;
	bool draggingSelectionRange;
	float selectionRangeAnchor;
	ofRange dragSelection;
	
	
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
	void setNoteRange(ofRange range);
	void addNote(float time, int pitch, int velocity, bool growing = false, int channel = 1);
	void endActiveNotes();
	void trimRange();
	bool activeNotes[127];
	int pitchForScreenY(int y);
	
	string getXMLStringForSwitches(bool selectedOnly);
	vector<ofxTLMidiNoteOn*> switchesFromXML(ofxXmlSettings xml);
	
	ofxTLMidiNoteOn* switchForScreenXY(float screenPos, int y);
	ofxTLMidiNoteOn* switchForPoint(float percent, int y);
	ofxTLMidiNoteOn* nearestGrowingNoteBeforePointWithPitch(float percent, int pitch);
	ofxTLMidiNoteOn* switchHandleForScreenPoint(ofPoint screenPos, bool& startTimeSelected);
	
	vector<ofxTLMidiNoteOn*> switches;
	ofxTLMidiNoteOn* hoverSwitch;
	bool hoveringStartTime;
	bool hoveringHandle;
	

};
