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
	//bounds.height = 25;
}

ofxTLZoomer2D::~ofxTLZoomer2D(){
	
}

void ofxTLZoomer2D::draw() {
	//cout << "zoom: " << bounds.x << ", "<< bounds.y << " : " << bounds.width << "x" << bounds.height << endl;
	//cout << "zoom2d :" << currentViewRange.min << "->" << currentViewRange.max  << endl<<endl; 
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
	ofSetColor(timeline->getColors().outlineColor, 72);
	ofRect(zoomRegion);
	ofNoFill();
	ofSetColor(0, 0, 0, 255);
	ofRectangle zoomRegionL = zoomRegion; 
	zoomRegionL.y -= 1;
	zoomRegionL.height += 2;
	ofRect(zoomRegionL);
	ofLine(bounds.x+bounds.width*timeline->getPercentComplete(), bounds.y,
			bounds.x+bounds.width*timeline->getPercentComplete(), bounds.y+bounds.height);


#if 0
	ofSetLineWidth(1);
	ofFill();
	//ofSetColor(timeline->getColors().textColor, 122);
	ofSetColor(timeline->getColors().keyColor, 122);
	//ofLine(minScreenX, screenY, maxScreenX, screenY);
	ofRect(minScreenX, bounds.y, maxScreenX-minScreenX, bounds.height);

	if(minSelected){
		ofFill();
	}
	else{
		ofNoFill();
	}
	// left handle
	//ofCircle(minScreenX, screenY, 5);
	ofSetColor(timeline->getColors().outlineColor, 122);
	ofRect(minScreenX, bounds.y, 10, bounds.height);

	if(maxSelected){
		ofFill();
	}
	else{
		ofNoFill();
	}
	// right handle
	//ofCircle(maxScreenX, screenY, 5);
	ofSetColor(timeline->getColors().outlineColor, 122);
	ofRect(maxScreenX, bounds.y, 10, bounds.height);

	//cout << "zoomer bounds width " << bounds.width << endl;
	//draw playhead reference
	ofLine(bounds.x+bounds.width*timeline->getPercentComplete(), bounds.y,
			bounds.x+bounds.width*timeline->getPercentComplete(), bounds.y+bounds.height);
	//draw zoom region reference
	ofSetColor(timeline->getColors().backgroundColor);
	ofRange actualZoom = getViewRange();
	ofRectangle zoomRegion = ofRectangle(bounds.x + bounds.width*actualZoom.min, bounds.y,
			bounds.width*actualZoom.span(),bounds.height);
	ofFill();
	ofSetColor(timeline->getColors().keyColor, 65);
	ofRect(zoomRegion);
#endif
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
	if(!enabled) return;

	minSelected = maxSelected = midSelected = false;
	if (pointInScreenBounds(ofVec2f(args.x, args.y))) {
		mouseIsDown = true;

		float minScreenX = normalizedXtoScreenX(currentViewRange.min, ofRange(0,1.0));
		xMinGrabOffset = args.x - minScreenX;
		float maxScreenX = normalizedXtoScreenX(currentViewRange.max, ofRange(0,1.0));
		xMaxGrabOffset = args.x - maxScreenX;
		yGrabOffset = args.y;
		/*
		//did we click on the min-left handle?
		float minScreenX = normalizedXtoScreenX(currentViewRange.min, ofRange(0,1.0));
		minGrabOffset = args.x - minScreenX;
		if(fabs(minScreenX - args.x) < 5){
			minSelected = true;
			notifyZoomStarted();
			return;
		}

		//did we click on the max-right handle?
		float maxScreenX = normalizedXtoScreenX(currentViewRange.max, ofRange(0,1.0));
		maxGrabOffset = args.x - maxScreenX;
		if(fabs(maxScreenX - args.x) < 5){
			maxSelected = true;
			notifyZoomStarted();
			return;
		}
		*/

		//did we click in the middle?
		//if(args.x > minScreenX && args.x < maxScreenX){
			notifyZoomStarted();
			midSelected = true;
			return;
		//}

		/*
		//did we click to the right?
		if(args.x > maxScreenX){
			maxSelected = true;
			maxGrabOffset = 0;
			currentViewRange.max = screenXtoNormalizedX(args.x, ofRange(0,1.0));
			notifyZoomStarted();
			return;
		}

		//did we click to the left?
		if(args.x < minScreenX){
			minSelected = true;
			minGrabOffset = 0;
			currentViewRange.min = screenXtoNormalizedX(args.x, ofRange(0,1.0));
			notifyZoomStarted();
			return;
		}
		*/
	}
}

void ofxTLZoomer2D::mouseDragged(ofMouseEventArgs& args) {
	if(!enabled) return;

	bool notify = false;
	ofRange oldRange = getViewRange();
	if(midSelected){
		//currentViewRange.min = ofClamp( screenXtoNormalizedX(args.x-minGrabOffset, ofRange(0, 1.0)), 0, currentViewRange.max-.01);
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
		//cout << "yd: " << yd << " nyd: " << nyd << endl;
		xmin = currentViewRange.min - nyd * d;
		//xmin = ofClamp( nyd, 0, currentViewRange.min - 0.1*d); // , 0, currentViewRange.min);

		d = screenXtoNormalizedX(currentViewRange.max - xMaxGrabOffset);
		xmax = currentViewRange.max + nyd * d;
		//xmax = ofClamp( nyd, currentViewRange.max + d*0.1, 1.);

		if (xmin < xmax) {
			currentViewRange.min = fmax(0., xmin);
			currentViewRange.max = fmin(xmax, 1.);
		}

		if (currentViewRange.max < currentViewRange.min)
			currentViewRange.max = currentViewRange.min + 0.2;
		notify = true;
	}

	if(notify){
		//notifyZoomDragged(oldRange);
		notifyZoomDragged(currentViewRange);
	}
}

bool ofxTLZoomer2D::isActive(){
	return mouseIsDown && ( maxSelected || minSelected || midSelected);
}

void ofxTLZoomer2D::mouseReleased(ofMouseEventArgs& args){
	if(!enabled) return;
	
	if(mouseIsDown){
		mouseIsDown = false;
		notifyZoomEnded();
//		timeline->flagTrackModified(this);
		save(); //intentionally ignores auto save since this is just a view parameter
	}
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
	//zoomEvent.currentZoom = currentViewRange;
	zoomEvent.currentZoom = getViewRange();
	ofNotifyEvent(events().zoomDragged, zoomEvent);
}

void ofxTLZoomer2D::notifyZoomEnded(){
	ofxTLZoomEventArgs zoomEvent;
	zoomEvent.sender = timeline;    
	//zoomEvent.currentZoom = currentViewRange;
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
	ofNotifyEvent(events().zoomEnded, zoomEvent); 
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

