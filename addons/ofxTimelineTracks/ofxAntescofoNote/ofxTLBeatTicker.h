//
//  ofxTLBeatTicker.h
//  part of AscoGraph : graphical editor for Antescofo musical scores.
//
//  Created by Thomas Coffy on 06/12/12.
//  Licensed under the Apache License : http://www.apache.org/licenses/LICENSE-2.0
//
#pragma once

#include <iostream>
#include <list>
#include "ofxTLTrack.h"
#include "ofxTLTicker.h"


class ofxAntescofog;

typedef struct{
	float screenX;
  float beat;
	int weight;
} ofxTLBeatBPMPoint;


class ofxTLBeatTicker : public ofxTLTicker {

	public:
    friend class ofxTLAntescofoNote;

		ofxTLBeatTicker(ofxAntescofog* Antescofog);
	virtual void draw();
	virtual void setup();

	virtual void getSnappingPoints(set<unsigned long>& points);
	virtual void refreshTickMarks();
	virtual void updateBPMPoints();
	virtual void mousePressed(ofMouseEventArgs& args);
	virtual void mouseMoved(ofMouseEventArgs& args);
	virtual void mouseDragged(ofMouseEventArgs& args);
	virtual void mouseReleased(ofMouseEventArgs& args);


	vector<ofxTLBeatBPMPoint> bpmScreenPoints;
	ofxAntescofog* mAntescofog;
	bool isSetup;

};
