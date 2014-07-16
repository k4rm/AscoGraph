/**
 * BeatKeyframes : continuous Curve objects editing in Antescofo score langage
 *
 * Copyright (c) 2012-2013 Thomas Coffy - thomas.coffy@ircam.fr
 *
 * derived from ofxTimeline
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
#include "Keyframes.h"
#include "ofxXmlSettings.h"
#include "ofxTLAntescofoAction.h"


class BeatKeyframe : public Keyframe {
  public:
		float previousBeat;
		float beat, orig_beat;
		float grabBeatOffset;
		float orig_value;
		float tmp_value; // when dragging
};

class BeatKeyframes : public Keyframes
{
	public:	
	BeatKeyframes();
	virtual ~BeatKeyframes();

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

	ActionCurve* ref;
	void keyPressed(ofKeyEventArgs& args);
	vector<BeatKeyframe*> keyframes;

	protected:
	virtual BeatKeyframe* newKeyframe();
	vector<BeatKeyframe*> selectedKeyframes;

	//cached previews for fast drawing of large timelines
	ofPolyline preview;
	vector<ofVec2f> keyPoints;

	virtual void recomputePreviews();
	bool shouldRecomputePreviews;

	virtual float sampleAtPercent(float percent); //less accurate than millis
	virtual float sampleAtBeat(float sampleBeat);
	virtual float interpolateValueForKeys(BeatKeyframe* start,BeatKeyframe* end, float sampleBeat);
	virtual float evaluateKeyframeAtBeat(BeatKeyframe* key, float sampleBeat);

	//ofRange valueRange; float defaultValue;

	//keep these stored for efficient search through the keyframe array
	int lastKeyframeIndex;
	unsigned long lastSampleBeat;

	bool isKeyframeIsInBounds(BeatKeyframe* key);
	bool isKeyframeSelected(BeatKeyframe* k);
	void selectKeyframe(BeatKeyframe* k);
	void deselectKeyframe(BeatKeyframe* k);

	//don't override these in subclasses
	virtual void deleteSelectedKeyframes();
	virtual void deleteKeyframe(BeatKeyframe* keyframe);
	//instead implement special behavior here:
	//this is called before the keyframe is deleted and removed from the keyframes vector
	virtual void willDeleteKeyframe(BeatKeyframe* keyframe) {}

	BeatKeyframe* selectedKeyframe;
	BeatKeyframe* hoverKeyframe;

	int selectedKeyframeIndex;
	bool keysAreDraggable;
	bool keysAreStretchable;
	unsigned long stretchAnchor;
	unsigned long stretchSelectPoint;

	virtual void setKeyframeBeat(BeatKeyframe* key, float newBeat);
	virtual void updateKeyframeSort();
	virtual void updateStretchOffsets(ofVec2f screenpoint, float grabBeat);
	virtual void updateDragOffsets(ofVec2f screenpoint, float grabBeat);

	virtual string getXMLStringForKeyframes(vector<BeatKeyframe*>& keys);
	virtual void createKeyframesFromXML(ofxXmlSettings xml, vector<BeatKeyframe*>& keyContainer);
	virtual void restoreKeyframe(BeatKeyframe* key, ofxXmlSettings& xmlStore){};
	virtual void storeKeyframe(BeatKeyframe* key, ofxXmlSettings& xmlStore){};

	virtual void selectedKeySecondaryClick(ofMouseEventArgs& args){};

	virtual BeatKeyframe* keyframeAtScreenpoint(ofVec2f p);


	bool screenpointIsInBounds(ofVec2f screenpoint);
	ofVec2f screenPositionForKeyframe(BeatKeyframe* keyframe);

	float screenYToValue(float screenY);
	float valueToScreenY(float value);

	bool keysDidDrag;
	bool keysDidNudge;
	bool createNewOnMouseup;

};
