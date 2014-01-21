/**
 * ofxTLBeatCurves : continuous Curve objects editing in Antescofo score langage
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
#include "BeatKeyframes.h"
#include "ofxTween.h"
#include "ofxTLCurves.h"


class TweenBeatKeyframe : public BeatKeyframe{
  public:
    EasingFunction* easeFunc;
	EasingType* easeType;
	int easeTypeId;
};


class BeatCurve : public BeatKeyframes {
	public:
		BeatCurve();
		virtual ~BeatCurve();

		virtual void draw();
		virtual void recomputePreviews();
		virtual void drawModalContent();

		virtual bool mousePressed(ofMouseEventArgs& args, long millis);
		virtual void mouseMoved(ofMouseEventArgs& args, long millis);
		virtual void mouseDragged(ofMouseEventArgs& args, long millis);
		virtual void mouseReleased(ofMouseEventArgs& args, long millis);
		ofxTLAntescofoAction *tlAction;

		virtual string getTrackType();
		void changeKeyframeEasing(float beat, string type);

		ofColor keyColor, highlightColor, disabledColor, mPreviewColor;
		bool get_first_last_displayed_keyframe(ofVec2f* coord1, ofVec2f* coord2, int* firsti, int* lasti);
		bool drawingEasingWindow;
		vector<BeatKeyframe* > getKeyframes() {return keyframes;}
	protected:

		virtual BeatKeyframe* newKeyframe();
		virtual void restoreKeyframe(BeatKeyframe* key, ofxXmlSettings& xmlStore);
		virtual void storeKeyframe(BeatKeyframe* key, ofxXmlSettings& xmlStore);

		virtual void selectedKeySecondaryClick(ofMouseEventArgs& args);	
		virtual float interpolateValueForKeys(BeatKeyframe* start, BeatKeyframe* end, float sampleBeat);
		virtual void willDeleteKeyframe(BeatKeyframe* keyframe);

		//easing dialog stuff
		void initializeEasings();
		ofVec2f easingWindowPosition;
		vector<EasingFunction*> easingFunctions;
		vector<EasingType*> easingTypes;

		float easingBoxWidth;
		float easingBoxHeight;
		float tweenBoxWidth;
		float tweenBoxHeight;

		ofFbo	drawCache;
		ofRectangle boundsCached;
		ofPoint	cursor;

		ofRectangle mApplyBtnRect;
		bool bDrawApplyButton;
};
