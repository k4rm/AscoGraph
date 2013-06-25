/**
 * ofxTLMultiCurves : continuous Curve objects editing in Antescofo score langage
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
#include "ofxTLBeatKeyframes.h"
#include "ofxTLBeatCurves.h"
#include "ofxTween.h"


class ofxTLMultiCurves : public ofxTLTrack {
  public:
    ofxTLMultiCurves();

    virtual void draw();
    virtual void update();
    virtual void setup();
    virtual void drawModalContent();
    
    //For selecting keyframe type only,
    //the superclass controls keyframe placement
    virtual bool mousePressed(ofMouseEventArgs& args, long millis);
    virtual void mouseDragged(ofMouseEventArgs& args, long millis);
    virtual void mouseReleased(ofMouseEventArgs& args, long millis);
	
    virtual void setHowmany(int howmany_);
    virtual string getTrackType();

    virtual void setValueRange(int which, ofRange range, float defaultValue = 0);
    virtual void setValueRangeMin(int which, float min);
    virtual void setValueRangeMax(int which, float max);
    virtual void setDefaultValue(int which, float defaultValue);

    //virtual void quantizeKeys(int step);

    virtual ofRange getValueRange(int which);

    void addKeyframeAtBeatAtCurveId(int which, float val, float beat);
    void changeKeyframeEasingAtCurveId(int which, float beat, string type);


    ofxTLBeatCurves* getCurve(int which);
    ofRectangle mSplitBtnRect;
  protected:
	
    virtual void selectedKeySecondaryClick(ofMouseEventArgs& args);	
    virtual float interpolateValueForKeys(ofxTLBeatKeyframe* start, ofxTLBeatKeyframe* end, float sampleBeat);

    vector<ofxTLBeatCurves*> curves;
    int howmany;
};
