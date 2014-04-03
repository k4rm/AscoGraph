//
//  ofxTLBeatTicker.cpp
//  part of AscoGraph : graphical editor for Antescofo musical scores.
//
//  Created by Thomas Coffy on 06/12/12.
//  Licensed under the Apache License : http://www.apache.org/licenses/LICENSE-2.0
//
#include <algorithm>
#include <string>
#include "ofxTimeline.h"
#include "ofxTLBeatTicker.h"
#ifdef ASCOGRAPH_IOS
# include "iOSAscoGraph.h"
#else
# include "ofxAntescofog.h"
#endif

template <typename T> string tostr(const T& t) { ostringstream os; os<<t; return os.str(); } 

#ifdef ASCOGRAPH_IOS
ofxTLBeatTicker::ofxTLBeatTicker(iOSAscoGraph* Antescofog)
#else
ofxTLBeatTicker::ofxTLBeatTicker(ofxAntescofog* Antescofog)
#endif
: mAntescofog(Antescofog), ofxTLTicker()
{
	isSetup = false;
}

void ofxTLBeatTicker::setup() {
	bpmScreenPoints.clear();
	ofxTLTicker::setup();
	(*(getTimeline()->getPages().begin()))->setTicker(this);
	getTimeline()->setTicker(this);
	setBPM(getTimeline()->getBPM());
	isSetup = true;
	refreshTickMarks();
}

void ofxTLBeatTicker::draw(){
	if (!isSetup)
		return;

	ofPushStyle();

	int textH, textW;
	string text;

	if(viewIsDirty){
		refreshTickMarks();
	}

	drawBPMGrid = true;
	tickerMarks.setStrokeColor( ofColor(0, 0, 240) );
	tickerMarks.setStrokeWidth(1);
	tickerMarks.draw(bounds.x, bounds.y);

	if(drawBPMGrid){
		if(viewIsDirty){
			updateBPMPoints();
		}
		ofPushStyle();

		ofSetColor(0, 0, 0, 200);
		ofSetLineWidth(1);

		int siz = bpmScreenPoints.size();
		int howmany;
		if (siz > 20)
			howmany = siz / 15;
		else if (siz > 12)
			howmany = 4;
		else howmany = 4;
		for(int i = 0; i < bpmScreenPoints.size(); i++) {
			if (isOnScreen(bpmScreenPoints[i].screenX)) {
				int bi = floor(bpmScreenPoints[i].beat);
				//if ((bi) % 4 == 1) { // draw bpms indices
				if ((bi) % howmany == 1) { // draw bpms indices
#if DRAW_FXCKING_GRID
						ofLine(bpmScreenPoints[i].screenX, getBottomEdge(), bpmScreenPoints[i].screenX, totalDrawRect.y+totalDrawRect.height);
#endif
						text = tostr(bi);
						textW = timeline->getFont().stringWidth(text);
						timeline->getFont().drawString(text, bpmScreenPoints[i].screenX - textW/2, getBottomEdge()-20);
				}
			}
		}
		ofPopStyle();
	}

	textH = timeline->getFont().getLineHeight();
	textW = 3;


	//draw current frame
	int currentFrameX;
	if (timeline->getIsFrameBased()) {
		//text = ofToString(timeline->getCurrentFrame());
		text = tostr(timeline->millisecToBeat(hoverTime));
		currentFrameX = screenXForIndex(timeline->getCurrentFrame());
	} else{
		//text = timeline->formatTime(timeline->getCurrentTime());
		text = tostr(timeline->millisecToBeat(hoverTime));
		currentFrameX = screenXForTime(timeline->getCurrentTime());
		//currenttimeline->normalizedXtoScreenX(timeline->beatToNormalizedX(currentPoint), zoomBounds); //;timeline->millisToScreenX(timeline->beatToMillisec(measures[0].beat));
	}
	currentFrameX = ofClamp(currentFrameX, bounds.getMinX(), bounds.getMaxX());

	//draw playhead line
	ofSetLineWidth(1);
	ofLine(currentFrameX, totalDrawRect.y, currentFrameX, totalDrawRect.y+totalDrawRect.height);
	//text = tostr(timeline->millisecToBeat(hoverTime)+1 - startBeat);
	unsigned long startBeat = timeline->normalizedXToBeat(zoomBounds.min);// * timeline->getDurationInMilliseconds();
	text = tostr( timeline->normalizedXToBeat( screenXtoNormalizedX( millisToScreenX(hoverTime), zoomBounds) ) + 1);
	//cout << "ofxTLBeatTicker: hoverTime: " << hoverTime << " text:"<< text << endl;
	float screenX = ofClamp(millisToScreenX(hoverTime), bounds.getMinX(), bounds.getMaxX());
	timeline->getFont().drawString(text, screenX, bounds.y+textH+25);
	ofPopStyle();

}

void ofxTLBeatTicker::getSnappingPoints(set<unsigned long>& points){

	if(!drawBPMGrid){
		updateBPMPoints();
	}
 
	for(int i = 0; i < bpmScreenPoints.size(); i++){
		points.insert(bpmScreenPoints[i].beat);
	}
	
	points.insert(timeline->millisecToBeat(timeline->getCurrentTimeMillis()));
}

void ofxTLBeatTicker::refreshTickMarks() {
	tickerMarks.clear();
	for(int i = 0; i < bpmScreenPoints.size(); i++) {
		float x = timeline->normalizedXtoScreenX(timeline->beatToNormalizedX( bpmScreenPoints[i].beat ), zoomBounds);
		int height = 0;
		int b = (int)(bpmScreenPoints[i].beat);
		if (b % 4 == 0) {
			//if (b % 10 == 0)
				height = bounds.height * .25;
			//else height = bounds.height * .75;
			tickerMarks.moveTo(x, bounds.height - height);
			tickerMarks.lineTo(x, bounds.height);
		}
	}
}

void ofxTLBeatTicker::updateBPMPoints(){
	//cout << "ofxTLBeatTicker::updateBPMPoints()" << endl;
	bpmScreenPoints.clear();
	float startBeat = timeline->normalizedXToBeat(zoomBounds.min);
	float endBeat = timeline->normalizedXToBeat(zoomBounds.max);
	float currentPoint = 0;

	currentPoint = floor(startBeat);
	while (currentPoint <= endBeat) {
		ofxTLBeatBPMPoint m;
		int numMeasures = 0;
		m.beat = currentPoint + 1;
		m.screenX = timeline->normalizedXtoScreenX(timeline->beatToNormalizedX(currentPoint), zoomBounds);
		m.weight = 4;
		//cout << "adding measure(beat=" << measures[m].beat << ", x="<< measures[m].screenX << ")" << endl;
		bpmScreenPoints.push_back( m );
		currentPoint++;
	}

}


void ofxTLBeatTicker::mousePressed(ofMouseEventArgs& args){
}

void ofxTLBeatTicker::mouseDragged(ofMouseEventArgs& args){
}

void ofxTLBeatTicker::mouseMoved(ofMouseEventArgs& args){
}

void ofxTLBeatTicker::mouseReleased(ofMouseEventArgs& args){
}

