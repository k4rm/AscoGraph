/**
 * ofxTLZoomer2D : a 2D zoom track for ofxTimeline
 * part of AscoGraph : an Antescofo musical score editor
 *
 * http://repmus.ircam.fr/mutant/ascograph
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

#include "ofxTLZoomer2D.h"
#include "ofxTimeline.h"
#include "ofxXmlSettings.h"
#include "ofRange.h"

ofxTLZoomer2D::ofxTLZoomer2D()
:	minSelected(false),
	maxSelected(false),
	midSelected(false),
	currentViewRange(ofRange(0.0, 1.0)),
	zoomExponent(1.0)
{
}

ofxTLZoomer2D::~ofxTLZoomer2D(){
	
}

void ofxTLZoomer2D::draw() {
	ofPushStyle();
	ofSetColor(timeline->getColors().textColor);
	//draw min
	float screenY = bounds.y + bounds.height/2.0;
	float minScreenX = normalizedXtoScreenX(currentViewRange.min, ofRange(0,1.0));
	float maxScreenX = normalizedXtoScreenX(currentViewRange.max, ofRange(0,1.0));

	ofSetLineWidth(1);

	ofSetColor(0, 0, 0, 255);
	ofRange actualZoom = getViewRange();
	ofRectangle zoomRegion = ofRectangle(bounds.x + bounds.width*actualZoom.min, bounds.y,
			bounds.width*actualZoom.span(),bounds.height);
	ofFill();
	ofSetColor(timeline->getColors().outlineColor, 52);
	ofRect(zoomRegion);
	ofNoFill();
	ofSetLineWidth(2);
	ofSetColor(0, 0, 0, 255);
	ofRectangle zoomRegionL = zoomRegion; 
	zoomRegionL.y -= 1;
	zoomRegionL.height += 2;
	ofRect(zoomRegionL);
	ofLine(bounds.x+bounds.width*timeline->getPercentComplete(), bounds.y,
			bounds.x+bounds.width*timeline->getPercentComplete(), bounds.y+bounds.height);

	ofPopStyle();
}

void ofxTLZoomer2D::load() {

	notifyZoomStarted();
	
	ofxXmlSettings settings;
	if(!settings.loadFile(xmlFileName)){
		ofLog(OF_LOG_NOTICE, "ofxTLZoomer2D -- couldn't load zoom settings file " + xmlFileName);
        currentViewRange = ofRange(0., 1.0);
		return;
	}
    
	settings.pushTag("zoom");
	currentViewRange = ofRange(settings.getValue("min", 0.0),
							   settings.getValue("max", 1.0));

	settings.popTag();
	
	notifyZoomEnded();
}

void ofxTLZoomer2D::zoomin()
{
	notifyZoomStarted();
	float r = currentViewRange.max - currentViewRange.min;
	
	currentViewRange.min += r/100.;
	currentViewRange.max -= r/100.;
	currentViewRange.min = fmax(0., currentViewRange.min);
	currentViewRange.max = fmin(currentViewRange.max, 1.);

	notifyZoomEnded();
}

void ofxTLZoomer2D::zoomout()
{
	notifyZoomStarted();
	float r = currentViewRange.max - currentViewRange.min;
	
	currentViewRange.min -= r/100.;
	currentViewRange.max += r/100.;
	currentViewRange.min = fmax(0., currentViewRange.min);
	currentViewRange.max = fmin(currentViewRange.max, 1.);

	notifyZoomEnded();
}


void ofxTLZoomer2D::save() {
	ofxXmlSettings savedSettings;
	savedSettings.addTag("zoom");
	savedSettings.pushTag("zoom");
	savedSettings.addValue("min", currentViewRange.min);
	savedSettings.addValue("max", currentViewRange.max);
	savedSettings.popTag();//zoom
	savedSettings.saveFile(xmlFileName);
}

void ofxTLZoomer2D::mouseMoved(ofMouseEventArgs& args) {
	
}

void ofxTLZoomer2D::mousePressed(ofMouseEventArgs& args) {
	if (args.button == 3) {  // scroll events
		notifyZoomStarted();
	}
	if (!bounds.inside(args.x, args.y)) return;
	if(!enabled) return;

	ofHideCursor();
	minSelected = maxSelected = midSelected = false;
	if (pointInScreenBounds(ofVec2f(args.x, args.y))) {
		mouseIsDown = true;
		float minScreenX = normalizedXtoScreenX(currentViewRange.min, ofRange(0,1.0));
		xMinGrabOffset = args.x - minScreenX;
		float maxScreenX = normalizedXtoScreenX(currentViewRange.max, ofRange(0,1.0));
		xMaxGrabOffset = args.x - maxScreenX;
		yGrabOffset = args.y;
		notifyZoomStarted();
		midSelected = true;
		mClickedX = args.x-bounds.x;
		mClickedY = args.y;
		return;
	}
}

void ofxTLZoomer2D::mouseDragged(ofMouseEventArgs& args) {
	if (args.button == 3) { // scroll events
		float d = currentViewRange.max - currentViewRange.min;
		float minScreenX = normalizedXtoScreenX(currentViewRange.min, ofRange(0,1.0));
		float maxScreenX = normalizedXtoScreenX(currentViewRange.max, ofRange(0,1.0));
		float xmin = ofClamp( screenXtoNormalizedX(minScreenX + 3*args.x, ofRange(0, 1.0)), 0, currentViewRange.max-.01);
		//float xmax = ofClamp( screenXtoNormalizedX(minScreenX + args.x, ofRange(0, 1.0)), currentViewRange.min+.01, 1.0);
		float xmax = xmin + d;

		currentViewRange.min = fmax(0., xmin);
		currentViewRange.max = fmin(xmax, 1.);
		notifyZoomDragged(currentViewRange);
		return;

	}
	if(!enabled || !mouseIsDown) return;

	bool notify = false;
	ofRange oldRange = getViewRange();
	if(midSelected){
		// x
		float originalMin = currentViewRange.min;
		float xmin = ofClamp( screenXtoNormalizedX(args.x-xMinGrabOffset, ofRange(0, 1.0)), 0, currentViewRange.max-.01);
		float originalMax = currentViewRange.max;
		float xmax = ofClamp( screenXtoNormalizedX(args.x-xMaxGrabOffset, ofRange(0, 1.0)), currentViewRange.min+.01, 1.0);

		currentViewRange.min = fmax(0., xmin);
		currentViewRange.max = fmin(xmax, 1.);

		// y
		float yd = -(args.y - yGrabOffset)*6;
		if (yd > 0) yd *= 10;
		float nyd = screenXtoNormalizedX(yd, ofRange(0, 1.));
		float d = screenXtoNormalizedX(xMinGrabOffset - currentViewRange.min);
		xmin = currentViewRange.min - nyd * d;

		d = screenXtoNormalizedX(currentViewRange.max - xMaxGrabOffset);
		xmax = currentViewRange.max + nyd * d;

		currentViewRange.min = fmax(0., xmin);
		currentViewRange.max = fmin(xmax, 1.);

		if (currentViewRange.max < currentViewRange.min)
			currentViewRange.max = currentViewRange.min + 0.05;
		notify = true;
	}

	if(notify)
		notifyZoomDragged(currentViewRange);
}

bool ofxTLZoomer2D::isActive(){
	return mouseIsDown && ( maxSelected || minSelected || midSelected);
}

void ofxTLZoomer2D::mouseReleased(ofMouseEventArgs& args){
	if (args.button == 3) {
		notifyZoomEnded();
	}
	if(!enabled) return;
	CGPoint point; 
	point.x = args.x;
	point.y = bounds.y+60;// + bounds.height/2;
	//CGSetLocalEventsSuppressionInterval(0);
	CGWarpMouseCursorPosition(point);
	CGAssociateMouseAndMouseCursorPosition(true);
	
	ofShowCursor();
	
	if(mouseIsDown){
		mouseIsDown = false;
		notifyZoomEnded();
		save(); //intentionally ignores auto save since this is just a view parameter
	}
	mClickedY = mClickedX = 0;
}

void ofxTLZoomer2D::notifyZoomStarted(){
	ofxTLZoomEventArgs zoomEvent;
	zoomEvent.sender = timeline;
	zoomEvent.currentZoom = zoomEvent.oldZoom = getViewRange();
	ofNotifyEvent(events().zoomStarted, zoomEvent);		
}

void ofxTLZoomer2D::notifyZoomDragged(ofRange oldRange){
	ofxTLZoomEventArgs zoomEvent;
	zoomEvent.sender = timeline;
	zoomEvent.oldZoom = oldRange;
	zoomEvent.currentZoom = getViewRange();
	ofNotifyEvent(events().zoomDragged, zoomEvent);
}

void ofxTLZoomer2D::notifyZoomEnded(){
	ofxTLZoomEventArgs zoomEvent;
	zoomEvent.sender = timeline;    
	zoomEvent.currentZoom = getViewRange();
	ofNotifyEvent(events().zoomEnded, zoomEvent);
}

void ofxTLZoomer2D::keyPressed(ofKeyEventArgs& args){
	//TODO: Nudging?
}

ofRange ofxTLZoomer2D::getViewRange() {
	float logSpan = powf(currentViewRange.span(),zoomExponent);
	float centerPosition = currentViewRange.center();
	//recompute view range
	if(centerPosition != .5){
		centerPosition = ofMap(centerPosition, currentViewRange.span()/2, 1.0 - currentViewRange.span()/2, logSpan/2, 1.0-logSpan/2);
	}
	return ofRange(centerPosition - logSpan/2, centerPosition + logSpan/2);
}

void ofxTLZoomer2D::setViewRange(ofRange newRange){

	ofxTLZoomEventArgs zoomEvent;
	zoomEvent.oldZoom = getViewRange();
	zoomEvent.sender = timeline;

	currentViewRange = newRange;
	zoomEvent.currentZoom = getViewRange();
    ofNotifyEvent(events().zoomDragged, zoomEvent);
}

ofRange ofxTLZoomer2D::getSelectedRange(){
	return currentViewRange;
}

void ofxTLZoomer2D::setSelectedRange(ofRange newRange){
	ofxTLZoomEventArgs zoomEvent;
	zoomEvent.oldZoom = getViewRange();
	zoomEvent.sender = timeline;

	currentViewRange = newRange;
	zoomEvent.currentZoom = getViewRange();
	ofNotifyEvent(events().zoomEnded, zoomEvent); 
}

