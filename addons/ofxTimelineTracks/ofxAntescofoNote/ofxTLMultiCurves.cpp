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

#include "ofxTLMultiCurves.h"
#include "ofxTimeline.h"

ofxTLMultiCurves::ofxTLMultiCurves() {

}

float ofxTLMultiCurves::interpolateValueForKeys(ofxTLBeatKeyframe* start, ofxTLBeatKeyframe* end, float sampleBeat){
	ofxTLTweenBeatKeyframe* tweenKeyStart = (ofxTLTweenBeatKeyframe*)start;
	ofxTLTweenBeatKeyframe* tweenKeyEnd = (ofxTLTweenBeatKeyframe*)end;
	return ofxTween::map(sampleBeat, tweenKeyStart->beat, tweenKeyEnd->beat,
						 			 tweenKeyStart->value, tweenKeyEnd->value,
						 false, *tweenKeyStart->easeFunc->easing, tweenKeyStart->easeType->type);
}

string ofxTLMultiCurves::getTrackType(){
	return "MultiCurves";
}

/*
ofxTLKeyframe* ofxTLMultiCurves::newKeyframe(){
	ofxTLTweenKeyframe* k = new ofxTLTweenKeyframe();
	k->easeFunc = easingFunctions[0];
	k->easeType = easingTypes[0];
	return k;
}
*/

void ofxTLMultiCurves::update() {
    if (viewIsDirty && isEnabled()) {
        for (int i = 0; i < howmany; i++) {
            if (curves[i])
                curves[i]->setDrawRect(bounds);
        }
    }
}
 

void ofxTLMultiCurves::draw() {
    if (isEnabled()) {
        for (int i = 0; i < howmany; i++) {
            if (curves[i])
                curves[i]->draw();
        }
    }
}


void ofxTLMultiCurves::setup() {
    return;
    for (int i = 0; i < howmany; i++) {
        curves[i]->setTimeline(timeline);
        curves[i]->setDrawRect(bounds);
				curves[i]->setZoomBounds(zoomBounds);
        curves[i]->setup();
    }
}

void ofxTLMultiCurves::drawModalContent() {
	
    for (int i = 0; i < howmany; i++) {
        curves[i]->drawModalContent();
    }
        
}

void ofxTLMultiCurves::setHowmany(int howmany_)
{
        howmany = howmany_;
        cout << "Instanciating " << howmany << " curves in a MultiCurves obj." << endl;
        //cout << "--> bounds: w:" << bounds.width << " h:"<<bounds.height << endl;  
        for (int i = 0; i < howmany; i++) {
            ofxTLBeatCurves *c = new ofxTLBeatCurves();
            c->setTimeline(timeline);
            c->setDrawRect(bounds);
            c->setZoomBounds(zoomBounds);
            c->selectAll();
            c->setTimeline(timeline);
            c->setDrawRect(bounds);
            int r = 10*i;
            int g = 70*i;
            int b = 60*i; 
            c->highlightColor = ofColor(r % 255, g % 255, b % 255, 140);
            c->disabledColor = ofColor((r-3) % 255, (g-3) % 255, (b-3) % 255, 120);
            c->keyColor = ofColor((r+20) % 255, (g+20) % 255, (b+30) % 255, 140);
            c->setup();
            c->clear();

            curves.push_back(c);
        }

}


bool ofxTLMultiCurves::mousePressed(ofMouseEventArgs& args, long millis){
    cout << "ofxTLMultiCurves: moussePressed"<<endl;
    int selected = -1;
    bool r = false;
    for (int i = 0; i < howmany; i++) {
        cout << "mousePressed: "<< i << endl;
        r = curves[i]->mousePressed(args,millis);
        if (r) {
            selected = i;
            break;
        }
    }

    if (!r && isEnabled())  {
        for (int i = 0; i < howmany; i++) {
            if (i != selected)
                curves[i]->unselectAll();
        }
    }
    return r;
}

void ofxTLMultiCurves::mouseDragged(ofMouseEventArgs& args, long millis){
    for (int i = 0; i < howmany; i++)
        curves[i]->mouseDragged(args,millis);
}

void ofxTLMultiCurves::mouseReleased(ofMouseEventArgs& args, long millis){
    for (int i = 0; i < howmany; i++) {
        curves[i]->mouseReleased(args,millis);
    }
}

void ofxTLMultiCurves::selectedKeySecondaryClick(ofMouseEventArgs& args){
    // TODO
}

void ofxTLMultiCurves::setValueRange(int which, ofRange range, float defaultValue)
{
    curves[which]->setValueRange(range, defaultValue);
}

void ofxTLMultiCurves::setValueRangeMin(int which, float min) {
    curves[which]->setValueRangeMin(min);
}

void ofxTLMultiCurves::setValueRangeMax(int which, float max) {
    curves[which]->setValueRangeMax(max);
}

void ofxTLMultiCurves::setDefaultValue(int which, float defaultValue) {
    curves[which]->setDefaultValue(defaultValue);
}

ofRange ofxTLMultiCurves::getValueRange(int which)
{
    return curves[which]->getValueRange();
}

void ofxTLMultiCurves::addKeyframeAtBeatAtCurveId(int which, float value, float beat){
    //cout << "ofxTLMultiCurves::addKeyframeAtMillisAtCurveId : id:" << which << " val:" << value << " millis:" << millis << endl;
    curves[which]->addKeyframeAtBeat(value, beat);
}

