/**
 * ofxTimeline
 * openFrameworks graphical timeline addon
 *
 * Copyright (c) 2011-2012 James George
 * Development Supported by YCAM InterLab http://interlab.ycam.jp/en/
 * http://jamesgeorge.org + http://flightphase.com
 * http://github.com/obviousjim + http://github.com/flightphase
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
 */

#pragma once

#include "ofMain.h"
#include "ofRange.h"
#include "ofxTLTrack.h"
#include "ofxTLKeyframes.h"
#include "ofxXmlSettings.h"

class ofxTLBeatKeyframe : public ofxTLKeyframe {
  public:
		float previousBeat;
		float beat;
		float grabBeatOffset;
};

class ofxTLBeatKeyframes : public ofxTLKeyframes
{
	public:	
	ofxTLBeatKeyframes();
	virtual ~ofxTLBeatKeyframes();

	virtual void draw();

	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);

	virtual void addKeyframe();
	virtual void addKeyframe(float value);
	virtual void addKeyframeAtBeat(float beat);
	virtual void addKeyframeAtBeat(float value, float beat);

	//copy paste
	virtual void pasteSent(string pasteboard);
	virtual void selectAll();
	virtual void unselectAll();

	virtual int getSelectedItemCount();

	virtual float getEarliestBeat();
	virtual float getLatestBeat();
	virtual float getEarliestSelectedBeat();
	virtual float getLatestSelectedBeat();

	//undo
	virtual string getTrackType();

	//sampling
	virtual float getValueAtBeat(float beat);

	//experimental binary saving. does not work with subclasses yet
	void saveToBinaryFile();
	void loadFromBinaryFile();
	bool useBinarySave;

	protected:
	virtual ofxTLBeatKeyframe* newKeyframe();
	vector<ofxTLBeatKeyframe*> keyframes;

	//cached previews for fast drawing of large timelines
	ofPolyline preview;
	vector<ofVec2f> keyPoints;

	virtual void recomputePreviews();
	bool shouldRecomputePreviews;

	virtual float sampleAtPercent(float percent); //less accurate than millis
	virtual float sampleAtBeat(float sampleBeat);
	virtual float interpolateValueForKeys(ofxTLBeatKeyframe* start,ofxTLBeatKeyframe* end, float sampleBeat);
	virtual float evaluateKeyframeAtBeat(ofxTLBeatKeyframe* key, float sampleBeat);

	ofRange valueRange;
	float defaultValue;

	//keep these stored for efficient search through the keyframe array
	int lastKeyframeIndex;
	unsigned long lastSampleBeat;

	bool isKeyframeIsInBounds(ofxTLBeatKeyframe* key);
	bool isKeyframeSelected(ofxTLBeatKeyframe* k);
	void selectKeyframe(ofxTLBeatKeyframe* k);
	void deselectKeyframe(ofxTLBeatKeyframe* k);
	void keyPressed(ofKeyEventArgs& args);

	//don't override these in subclasses
	virtual void deleteSelectedKeyframes();
	virtual void deleteKeyframe(ofxTLBeatKeyframe* keyframe);
	//instead implement special behavior here:
	//this is called before the keyframe is deleted and removed from the keyframes vector
	virtual void willDeleteKeyframe(ofxTLKeyframe* keyframe);

	vector<ofxTLBeatKeyframe*> selectedKeyframes;
	ofxTLBeatKeyframe* selectedKeyframe;
	ofxTLBeatKeyframe* hoverKeyframe;

	int selectedKeyframeIndex;
	bool keysAreDraggable;
	bool keysAreStretchable;
	unsigned long stretchAnchor;
	unsigned long stretchSelectPoint;

	virtual void setKeyframeBeat(ofxTLBeatKeyframe* key, float newBeat);
	virtual void updateKeyframeSort();
	virtual void updateStretchOffsets(ofVec2f screenpoint, float grabBeat);
	virtual void updateDragOffsets(ofVec2f screenpoint, float grabBeat);

	virtual string getXMLStringForKeyframes(vector<ofxTLBeatKeyframe*>& keys);
	virtual void createKeyframesFromXML(ofxXmlSettings xml, vector<ofxTLBeatKeyframe*>& keyContainer);
	virtual void restoreKeyframe(ofxTLBeatKeyframe* key, ofxXmlSettings& xmlStore){};
	virtual void storeKeyframe(ofxTLBeatKeyframe* key, ofxXmlSettings& xmlStore){};

	virtual void selectedKeySecondaryClick(ofMouseEventArgs& args){};

	virtual ofxTLBeatKeyframe* keyframeAtScreenpoint(ofVec2f p);


	bool screenpointIsInBounds(ofVec2f screenpoint);
	ofVec2f screenPositionForKeyframe(ofxTLBeatKeyframe* keyframe);

	float screenYToValue(float screenY);
	float valueToScreenY(float value);

	bool keysDidDrag;
	bool keysDidNudge;
	bool createNewOnMouseup;

};
