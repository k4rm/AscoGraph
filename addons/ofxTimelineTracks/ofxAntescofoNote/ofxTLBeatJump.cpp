//
//  ofxTLBeatJump.cpp
//  part of AscoGraph : graphical editor for Antescofo musical scores.
//
//  Created by Thomas Coffy on 06/12/12.
//  Licensed under the Apache License : http://www.apache.org/licenses/LICENSE-2.0
//
#include <algorithm>
#include <string>
#include "ofxTimeline.h"
#include "ofxTLBeatJump.h"
//#include "ofxAntescofog.h"

vector<ofPoint> circlePoints;
ofPolyline circlePolyline;

#ifndef ASCOGRAPH_IOS
ofxTLBeatJump::ofxTLBeatJump(ofxAntescofog* Antescofog)
#else
ofxTLBeatJump::ofxTLBeatJump(iOSAscoGraph* Antescofog)
#endif
: mAntescofog(Antescofog)
{
	isSetup = false;
}

void ofxTLBeatJump::setup() {
	oldBoundsH = bounds.height;
	circlePolyline.arc(0,0,0,1,1,0,360,400);
	circlePoints.resize(circlePolyline.size());
	isSetup = true;
}

void ofxTLBeatJump::add_jump(float beat_, float destBeat_, string destLabel_)
{
	enable();
	ofColor c = ofColor(ofRandom(128, 255), ofRandom(128, 255), ofRandom(128, 255), 255);
	jumpList.push_back(new antescofoJump(beat_, destBeat_, destLabel_, c));
}

void ofxTLBeatJump::clear_jumps()
{
	for (vector<antescofoJump*>::iterator i = jumpList.begin(); i != jumpList.end(); i++) {
		delete *i;
	}
	jumpList.clear();
}

void ofxTLBeatJump::refreshArrows() {
}


void ofxTLBeatJump::update(){
}

// fleches en avant d'une couleur,
// en arriere d'une autre
void ofxTLBeatJump::draw(){
	if (bounds.height == 0) 
		return;
	if (!isSetup)
		return;

	ofPushStyle();
	ofSetLineWidth(2);

	int step = bounds.height / jumpList.size();
	int n = 0;
        vector<ofPoint> & circleCache = circlePolyline.getVertices();
	ofSetColor(255, 204, 0);
	for (vector<antescofoJump*>::iterator i = jumpList.begin(); i != jumpList.end(); i++, n++) {
		antescofoJump* j = *i;
		ofSetColor(j->color);
		ofVec3f start, mid, end;
		start.x = normalizedXtoScreenX( timeline->beatToNormalizedX(j->beat), zoomBounds);
		start.y = bounds.y + bounds.height -2;
		start.z = 0;
		end.x = normalizedXtoScreenX( timeline->beatToNormalizedX(j->destBeat), zoomBounds);
		end.y = bounds.y + bounds.height - 2;
		end.z = 0;
		mid.x = (end.x + start.x) / 2;
		mid.y = bounds.y + bounds.height - 2;
		mid.z = 0;

		float width = (end.x - start.x);
		float height = bounds.height * 2 -12;
		float radiusX = width*0.5;
		float radiusY = height*0.5;
		float x = mid.x;
		float y = mid.y;
		float z = 0;
	
		ofBeginShape();
		for(int i=0;i<(int)circleCache.size();i++){
			if (radiusY*circlePolyline[i].y+y < y)
				circlePoints[i].set(radiusX*circlePolyline[i].x+x, radiusY*circlePolyline[i].y+y, z);
			else  {
				if (i <= circleCache.size()/2)
					circlePoints[i].set(start);
				else 
					circlePoints[i].set(end);
			}
			//cout << "circle points["<<i<<"]: " << circlePoints[i].x << "  -  " << circlePoints[i].y << endl; 
		}
		circlePolyline.setClosed(false);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);

		//why do we need this?
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(ofVec3f), &circlePoints[0].x);
		glDrawArrays(GL_LINE_STRIP, 0, circlePoints.size());
		ofEndShape(false);

		// draw arrow
		float headSize = 3.;
		ofMatrix4x4 mat;
		mat.makeRotationMatrix(end - start, ofVec3f(0,0,1));
		ofPushMatrix();
		ofTranslate(end);
		ofMultMatrix(mat.getPtr());
		ofTranslate(0, headSize*0.5 ,0);
		ofDrawCone(headSize, headSize);
		ofPopMatrix();
	}
	int textH, textW;
	string text;

	//timeline->getFont().drawString(text, screenX, bounds.y+textH+25);
	ofPopStyle();

}

void ofxTLBeatJump::mousePressed(ofMouseEventArgs& args){
}

void ofxTLBeatJump::mouseDragged(ofMouseEventArgs& args){
}

void ofxTLBeatJump::mouseMoved(ofMouseEventArgs& args){
}

void ofxTLBeatJump::mouseReleased(ofMouseEventArgs& args){
}

