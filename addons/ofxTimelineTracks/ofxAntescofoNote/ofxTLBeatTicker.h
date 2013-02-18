//
//  ofxTLBeatTicker.h
//  ofxAntescofog
//
//  Created by Thomas Coffy on 06/12/12.
//
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

	vector<ofxTLBeatBPMPoint> bpmScreenPoints;
	ofxAntescofog* mAntescofog;
	bool isSetup;

};
