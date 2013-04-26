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

#include "ofxTLCurves.h"
#include "ofxTLBeatCurves.h"
#include "ofxTimeline.h"

ofxTLBeatCurves::ofxTLBeatCurves()
: ofxTLBeatKeyframes()
{
	initializeEasings();
	valueRange = ofRange(0.0, 1.0);
	drawingEasingWindow = false;
}

float ofxTLBeatCurves::interpolateValueForKeys(ofxTLBeatKeyframe* start, ofxTLBeatKeyframe* end, float sampleBeat){
	//cout << "interpolate: " << sampleBeat << endl;; 
	ofxTLTweenBeatKeyframe* tweenKeyStart = (ofxTLTweenBeatKeyframe*)start;
	ofxTLTweenBeatKeyframe* tweenKeyEnd = (ofxTLTweenBeatKeyframe*)end;
	return ofxTween::map(sampleBeat, tweenKeyStart->beat, tweenKeyEnd->beat, tweenKeyStart->value, tweenKeyEnd->value,
			false, *tweenKeyStart->easeFunc->easing, tweenKeyStart->easeType->type);
}

string ofxTLBeatCurves::getTrackType(){
	return "Beat Curves";    
}

ofxTLBeatKeyframe* ofxTLBeatCurves::newKeyframe(){
	ofxTLTweenBeatKeyframe* k = new ofxTLTweenBeatKeyframe();
	k->easeFunc = easingFunctions[0];
	k->easeType = easingTypes[0];
	return k;
}


#if 0
//easetype: ofSetColor(150, 100, 10);
void ofxTLCurves::setColor_EasetypeSel(ofColor &col) {
	mColor_easeTypeSel = col;
}
//ofSetColor(80, 80, 80);
void ofxTLCurves::setColor_EasetypeDefault(ofColor &col) {
	mColor_easeTypeDefault = col;
}
#endif


void ofxTLBeatCurves::drawModalContent(){
	//cout << "ofxTLBeatCurves::drawModalContent() " << endl;

	//****** DRAW EASING CONTROLS
	if(!drawingEasingWindow){
		return;
	}

	ofxTLTweenBeatKeyframe* tweenFrame = (ofxTLTweenBeatKeyframe*) selectedKeyframe;
	if(tweenFrame == NULL){
		if(selectedKeyframes.size() == 0){
			return;
		}
		tweenFrame = (ofxTLTweenBeatKeyframe*)selectedKeyframes[0];
	}

	for(int i = 0; i < easingTypes.size(); i++){
		//TODO turn into something like selectionContainsEaseType();
		//so that we can show the multi-selected easies
		if(easingTypes[i] ==  ((ofxTLTweenBeatKeyframe*)selectedKeyframes[0])->easeType){
			//ofSetColor(mColor_easeTypeSel);
			ofSetColor(150, 100, 10);
		}
		else{
			//ofSetColor(mColor_easeTypeDefault); 
			ofSetColor(80, 80, 80);
		}
		ofFill();
		ofRect(easingWindowPosition.x + easingTypes[i]->bounds.x, easingWindowPosition.y + easingTypes[i]->bounds.y,
				easingTypes[i]->bounds.width, easingTypes[i]->bounds.height);
		ofSetColor(200, 200, 200);
		timeline->getFont().drawString(easingTypes[i]->name,
				easingWindowPosition.x + easingTypes[i]->bounds.x+11,
				easingWindowPosition.y + easingTypes[i]->bounds.y+10);
		ofNoFill();
		ofSetColor(40, 40, 40);
		ofRect(easingWindowPosition.x + easingTypes[i]->bounds.x,
				easingWindowPosition.y + easingTypes[i]->bounds.y,
				easingTypes[i]->bounds.width, easingTypes[i]->bounds.height);
	}

	for(int i = 0; i < easingFunctions.size(); i++){
		//TODO: turn into something like selectionContainsEaseFunc();
		if(easingFunctions[i] == tweenFrame->easeFunc){
			//ofSetColor(mColor_easeTypeSel);
			ofSetColor(150, 100, 10);
		}
		else{
			//ofSetColor(mColor_easeTypeDefault);
			ofSetColor(80, 80, 80);
		}
		ofFill();
		ofRect(easingWindowPosition.x + easingFunctions[i]->bounds.x, easingWindowPosition.y +easingFunctions[i]->bounds.y, 
				easingFunctions[i]->bounds.width, easingFunctions[i]->bounds.height);
		ofSetColor(200, 200, 200);
		//        timeline->getFont().drawString(easingFunctions[i]->name,
		//                           easingWindowPosition.x + easingFunctions[i]->bounds.x+10, 
		//                           easingWindowPosition.y + easingFunctions[i]->bounds.y+15);			
		ofPushMatrix();
		ofTranslate(easingWindowPosition.x + easingFunctions[i]->bounds.x,
				easingWindowPosition.y + easingFunctions[i]->bounds.y);
		if(tweenFrame->easeType->type == ofxTween::easeIn){
			easingFunctions[i]->easeInPreview.draw();
		}
		else if(tweenFrame->easeType->type == ofxTween::easeOut){
			easingFunctions[i]->easeOutPreview.draw();
		}
		else {
			easingFunctions[i]->easeInOutPreview.draw();
		}

		ofPopMatrix();
		ofNoFill();
		ofSetColor(40, 40, 40);
		ofRect(easingWindowPosition.x + easingFunctions[i]->bounds.x, easingWindowPosition.y +easingFunctions[i]->bounds.y, 
				easingFunctions[i]->bounds.width, easingFunctions[i]->bounds.height);	
	}
}

bool ofxTLBeatCurves::mousePressed(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return false;
	if(drawingEasingWindow){
		return true;
	}
	else {
		return ofxTLBeatKeyframes::mousePressed(args,millis);
	}
}

void ofxTLBeatCurves::mouseMoved(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	if(!drawingEasingWindow){
		ofxTLBeatKeyframes::mouseMoved(args, millis);
	} else cursor.set(args.x, args.y);
}
void ofxTLBeatCurves::mouseDragged(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	if(!drawingEasingWindow){
		ofxTLBeatKeyframes::mouseDragged(args, millis);
	}
}

void ofxTLBeatCurves::mouseReleased(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	cout << "ofxTLBeatCurves::mouseReleased: button:" << args.button <<endl;
	if (drawingEasingWindow && args.button == 0){
		drawingEasingWindow = false;
		timeline->dismissedModalContent();
		ofVec2f screenpoint(args.x,args.y);
		for(int i = 0; i < easingFunctions.size(); i++){
			if(easingFunctions[i]->bounds.inside(screenpoint-easingWindowPosition)){
				for(int k = 0; k < selectedKeyframes.size(); k++){
					((ofxTLTweenBeatKeyframe*)selectedKeyframes[k])->easeFunc = easingFunctions[i];
				}
				timeline->flagTrackModified(this);
				shouldRecomputePreviews = true;
				return;
			}
		}

		for(int i = 0; i < easingTypes.size(); i++){
			if(easingTypes[i]->bounds.inside(screenpoint-easingWindowPosition)){
				for(int k = 0; k < selectedKeyframes.size(); k++){
					((ofxTLTweenBeatKeyframe*)selectedKeyframes[k])->easeType = easingTypes[i];
				}
				timeline->flagTrackModified(this);
				shouldRecomputePreviews = true;
				return;
			}
		}
	}
	else{

		//bool hastoupdate = createNewOnMouseup || keysDidDrag;
		ofxTLBeatKeyframes::mouseReleased(args, millis);
		//if (hastoupdate) updateEditorContent();
	}
}

// called when a keyframe is mouse moved, : this fct should:
// 1/ re-fill the Curve antescofo parser object,
// 2/ pretty print the action
// 3/ eplace it in the text score
/*
void ofxTLBeatCurves::updateEditorContent()
{
}
*/

void ofxTLBeatCurves::selectedKeySecondaryClick(ofMouseEventArgs& args){
	easingWindowPosition = ofVec2f(MIN(args.x, bounds.width - easingBoxWidth),
			MIN(args.y, timeline->getBottomLeft().y - (tweenBoxHeight*easingFunctions.size())));

	drawingEasingWindow = true;
	timeline->presentedModalContent(this);
}

void ofxTLBeatCurves::restoreKeyframe(ofxTLBeatKeyframe* key, ofxXmlSettings& xmlStore){
	ofxTLTweenBeatKeyframe* tweenKey =  (ofxTLTweenBeatKeyframe*)key;    
	tweenKey->easeFunc = easingFunctions[ofClamp(xmlStore.getValue("easefunc", 0), 0, easingFunctions.size()-1)];
	tweenKey->easeType = easingTypes[ofClamp(xmlStore.getValue("easetype", 0), 0, easingTypes.size()-1)];
}

void ofxTLBeatCurves::storeKeyframe(ofxTLBeatKeyframe* key, ofxXmlSettings& xmlStore){
	ofxTLTweenBeatKeyframe* tweenKey =  (ofxTLTweenBeatKeyframe*)key;
	xmlStore.addValue("easefunc", tweenKey->easeFunc->id);
	xmlStore.addValue("easetype", tweenKey->easeType->id);
}

void ofxTLBeatCurves::initializeEasings(){

	//FUNCTIONS ----
	EasingFunction* ef;
	ef = new EasingFunction();
	ef->easing = new ofxEasingLinear();
	ef->name = "linear";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingSine();
	ef->name = "sine";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingCirc();
	ef->name = "circular";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingQuad();
	ef->name = "quadratic";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingCubic();
	ef->name = "cubic";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingQuart();
	ef->name = "quartic";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingQuint();
	ef->name = "quintic";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingExpo();
	ef->name = "exponential";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingBack();
	ef->name = "back";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingBounce();
	ef->name = "bounce";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingElastic();
	ef->name = "elastic";
	easingFunctions.push_back(ef);

	///TYPES -------
	EasingType* et;
	et = new EasingType();
	et->type = ofxTween::easeIn;
	et->name = "ease in";
	easingTypes.push_back(et);

	et = new EasingType();
	et->type = ofxTween::easeOut;
	et->name = "ease out";
	easingTypes.push_back(et);

	et = new EasingType();
	et->type = ofxTween::easeInOut;
	et->name = "ease in-out";
	easingTypes.push_back(et);


	tweenBoxWidth = 40;
	tweenBoxHeight = 30;
	easingBoxWidth  = 80;
	easingBoxHeight = 15;

	//	easingWindowSeperatorHeight = 4;

	for(int i = 0; i < easingTypes.size(); i++){
		easingTypes[i]->bounds = ofRectangle(0, i*easingBoxHeight, easingBoxWidth, easingBoxHeight);
		easingTypes[i]->id = i;
	}

	for(int i = 0; i < easingFunctions.size(); i++){
		easingFunctions[i]->bounds = ofRectangle(easingBoxWidth, i*tweenBoxHeight, tweenBoxWidth, tweenBoxHeight);
		easingFunctions[i]->id = i;
		//build preview
		for(int p = 1; p < tweenBoxWidth-1; p++){
			float percent;
			percent = ofxTween::map(1.0*p/tweenBoxWidth, 0, 1.0, tweenBoxHeight-5, 5, false, *easingFunctions[i]->easing, ofxTween::easeIn);
			easingFunctions[i]->easeInPreview.addVertex(ofPoint(p, percent));
			percent = ofxTween::map(1.0*p/tweenBoxWidth, 0, 1.0, tweenBoxHeight-5, 5, false, *easingFunctions[i]->easing, ofxTween::easeOut);
			easingFunctions[i]->easeOutPreview.addVertex(ofPoint(p, percent));
			percent = ofxTween::map(1.0*p/tweenBoxWidth, 0, 1.0, tweenBoxHeight-5, 5, false, *easingFunctions[i]->easing, ofxTween::easeInOut);
			easingFunctions[i]->easeInOutPreview.addVertex(ofPoint(p, percent));
		}

		easingFunctions[i]->easeInPreview.simplify();
		easingFunctions[i]->easeOutPreview.simplify();
		easingFunctions[i]->easeInOutPreview.simplify();

	}

}

#if 0
void ofxTLBeatCurves::draw(){
	//cout << "ofxTLBeatCurves::draw" << endl;

	//cout << "ofxTLBeatCurves::draw(): bw:"<< bounds.width << " bh:" << bounds.height  << endl;
	if(bounds.width == 0 || bounds.height < 2 || keyframes.empty()){
		return;
	}

	if(shouldRecomputePreviews || viewIsDirty){
		recomputePreviews();
	}

	ofPushStyle();
	ofFill();

	// draw rect around curves
	//ofVec2f beginPoint = screenPositionForKeyframe(keyframes[0]);
	//ofVec2f endPoint = screenPositionForKeyframe(keyframes[keyframes.size() - 1]);

	ofSetColor(keyColor, 30);
	//ofSetColor(0, 0, 0, 30);
	//ofRect(beginPoint.x , bounds.y, endPoint.x - beginPoint.x, bounds.height);

	//draw current value indicator as a big transparent rectangle
	//ofSetColor(timeline->getColors().disabledColor, 30);
	ofSetColor(disabledColor);


	//******* DRAW FILL CURVES
	ofSetPolyMode(OF_POLY_WINDING_NONZERO);
	ofBeginShape();
	ofVec2f screenpoint;
	for (int i = 0; i < keyframes.size(); i++){
		if (isKeyframeIsInBounds(keyframes[i])){
			ofVec2f screenpoint = screenPositionForKeyframe(keyframes[i]);
			float keysValue = ofMap(keyframes[i]->value, 0, 1.0, valueRange.min, valueRange.max, true);
			if(keysAreDraggable){
				//string frameString = timeline->formatTime(keyframes[i]->beat);
				timeline->getFont().drawString(ofToString(keysValue, 2), screenpoint.x+5, screenpoint.y-5);
			}
			//ofCircle(screenpoint.x, screenpoint.y, 200);
			//cout << "x:" <<screenpoint.x << " y:" << screenpoint.y << endl;
			ofVertex(screenpoint.x, screenpoint.y);
		}
	}
	// draw edges
	if (keyframes.size()) {
		// last point
		screenpoint = screenPositionForKeyframe(keyframes[keyframes.size()-1]);
		ofVertex(screenpoint.x, bounds.getMaxY()); // right low corner of rect
		// first point
		screenpoint = screenPositionForKeyframe(keyframes[0]);
		ofVertex(screenpoint.x, bounds.getMaxY()); // right low corner of rect
		ofVertex(screenpoint.x, screenpoint.y);  
	}
	ofEndShape();
	//***** DRAW KEYFRAME LINES
	//ofSetColor(timeline->getColors().keyColor);
	ofSetColor(keyColor);
	ofNoFill();

	preview.draw();

	//**** DRAW KEYFRAME DOTS

	//**** HOVER FRAME
	if(hoverKeyframe != NULL){
		ofPushStyle();
		ofFill();
		//ofSetColor(timeline->getColors().highlightColor);
		ofSetColor(highlightColor);
		ofVec2f hoverKeyPoint = screenPositionForKeyframe( hoverKeyframe );
		cout << "ofxTLBeatCurves::highlight: " << hoverKeyPoint.x << " : " << hoverKeyPoint.y << endl;
		ofCircle(hoverKeyPoint.x, hoverKeyPoint.y, 6);
		ofPopStyle();
	}

	//**** ALL CACHED VISIBLE KEYS
	ofSetColor(timeline->getColors().textColor);
	ofNoFill();
	for(int i = 0; i < keyPoints.size(); i++){
		ofRect(keyPoints[i].x-1, keyPoints[i].y-1, 3, 3);
	}

	//**** SELECTED KEYS
	ofSetColor(timeline->getColors().textColor);
	ofFill();
	for(int i = 0; i < selectedKeyframes.size(); i++){
		if(isKeyframeIsInBounds(selectedKeyframes[i])) {
			ofVec2f screenpoint = screenPositionForKeyframe(selectedKeyframes[i]);
			float keysValue = ofMap(selectedKeyframes[i]->value, 0, 1.0, valueRange.min, valueRange.max, true);
			if(keysAreDraggable){
				//string frameString = timeline->formatTime(selectedKeyframes[i]->time);
				timeline->getFont().drawString(ofToString(keysValue, 2), screenpoint.x+5, screenpoint.y-5);
			}
			ofCircle(screenpoint.x, screenpoint.y, 4);
			//cout << "ofxTLKeyframes::draw(): circle "<<screenpoint.x << ", "<< screenpoint.y << endl;
		}
	}

	ofPopStyle();
}
#else


void ofxTLBeatCurves::recomputePreviews(){
	preview.clear();

	cout << "ofxTLBeatCurves::recomputePreviews " << endl;

	//	if(keyframes.size() == 0 || keyframes.size() == 1){
	//		preview.addVertex(ofPoint(bounds.x, bounds.y + bounds.height - sampleAtPercent(.5f)*bounds.height));
	//		preview.addVertex(ofPoint(bounds.x+bounds.width, bounds.y + bounds.height - sampleAtPercent(.5f)*bounds.height));
	//	}
	//	else{
	for(int p = bounds.getMinX(); p <= bounds.getMaxX(); p++){
		preview.addVertex(p,  bounds.y + bounds.height - sampleAtPercent(screenXtoNormalizedX(p)) * bounds.height);
	}
	//	}
	//	int size = preview.getVertices().size();
	preview.simplify();
	//cout << "simplify pre " << size << " post: " << preview.getVertices().size() << " dif: " << (size - preview.getVertices().size()) << endl;

	ofVec2f lastPoint;
	keyPoints.clear();
	for(int i = 0; i < keyframes.size(); i++){
		if(!isKeyframeIsInBounds(keyframes[i])){
			continue;
		}
		ofVec2f screenpoint = screenPositionForKeyframe(keyframes[i]);
		if(lastPoint.squareDistance(screenpoint) > 5*5){
			keyPoints.push_back(screenpoint);
		}

		lastPoint = screenpoint;
	}

	shouldRecomputePreviews = false;

}

void ofxTLBeatCurves::draw(){
        cout << "ofxTLBeatCurves::draw(): bw:"<< bounds.width << " bh:" << bounds.height << " valueRange:" << valueRange.min << ":" << valueRange.max << endl;
	if(bounds.width == 0 || bounds.height < 2){
		return;
	}
	
	if(shouldRecomputePreviews || viewIsDirty){
		recomputePreviews();
	}
	
	ofPushStyle();


        //draw current value indicator as a big transparent rectangle
	//ofSetColor(timeline->getColors().disabledColor, 30);
	ofSetColor(timeline->getColors().disabledColor, 70);
	//jg play solo change
	//float currentPercent = sampleAtTime(timeline->getCurrentTimeMillis());
	//float currentPercent = sampleAtTime(currentTrackTime());
	ofFill();
	//ofRect(bounds.x, bounds.getMaxY(), bounds.width, -bounds.height*currentPercent);

        //******* DRAW FILL CURVES
        ofSetPolyMode(OF_POLY_WINDING_NONZERO);


	//cout << "keyframe size:" << keyframes.size() << endl;
	//cout << "selectedKeyframe size:" << selectedKeyframes.size() << endl;
        ofFill();
        ofBeginShape();
        for (int i = 0; i < keyframes.size(); i++) {
            if (isKeyframeIsInBounds(keyframes[i])){
                ofVec2f screenpoint = screenPositionForKeyframe(keyframes[i]);
                float keysValue = ofMap(keyframes[i]->value, 0, 1.0, valueRange.min, valueRange.max, true);
                if(keysAreDraggable){
                    //timeline->getFont().drawString(ofToString(keysValue, 4), screenpoint.x+5, screenpoint.y-5);
                    timeline->getFont().drawString(ofToString(keyframes[i]->orig_value, 4), screenpoint.x+5, screenpoint.y-5);
                }
                //ofCircle(screenpoint.x, screenpoint.y, 4);
                //ofCurveVertex(screenpoint.x, screenpoint.y);
                ofVertex(screenpoint.x, screenpoint.y);
            }
        }
	/*if (keyframes.size()) {
		ofVec2f screenpoint = screenPositionForKeyframe(keyframes[0]);
		ofCurveVertex(screenpoint.x, screenpoint.y);
	}*/
	// draw edges
	if (keyframes.size()) {
		// last point
		ofVec2f screenpoint = screenPositionForKeyframe(keyframes[keyframes.size()-1]);
		ofVertex(screenpoint.x, bounds.getMaxY()); // right low corner of rect
		// first point
		screenpoint = screenPositionForKeyframe(keyframes[0]);
		ofVertex(screenpoint.x, bounds.getMaxY()); // right low corner of rect
		ofVertex(screenpoint.x, screenpoint.y);  
	}

        ofEndShape();
	//***** DRAW KEYFRAME LINES
	ofSetColor(timeline->getColors().keyColor);
	ofNoFill();
	
	preview.draw();
	
	//**** DRAW KEYFRAME DOTS
	
	//**** HOVER FRAME
	if(hoverKeyframe != NULL){
		ofPushStyle();
		ofFill();
		ofSetColor(timeline->getColors().highlightColor);
		ofVec2f hoverKeyPoint = screenPositionForKeyframe( hoverKeyframe );
		ofCircle(hoverKeyPoint.x, hoverKeyPoint.y, 6);
		ofPopStyle();
	}

	//**** ALL CACHED VISIBLE KEYS
	ofSetColor(timeline->getColors().textColor);
	ofNoFill();
	for(int i = 0; i < keyPoints.size(); i++){
		ofRect(keyPoints[i].x-1, keyPoints[i].y-1, 3, 3);
	}
	
	//**** SELECTED KEYS
	ofSetColor(timeline->getColors().textColor);
	ofFill();
        //cout << "ofxTLKeyframes::draw(): selectedKeyframes.size:"<< selectedKeyframes.size() << endl;
	for(int i = 0; i < selectedKeyframes.size(); i++){
		if(isKeyframeIsInBounds(selectedKeyframes[i])){
			ofVec2f screenpoint = screenPositionForKeyframe(selectedKeyframes[i]);
			float keysValue = ofMap(selectedKeyframes[i]->value, 0, 1.0, valueRange.min, valueRange.max, true);
			if(keysAreDraggable){
				//timeline->getFont().drawString(ofToString(keysValue, 4), screenpoint.x+5, screenpoint.y-5);
				if (selectedKeyframes[i]->tmp_value)
					timeline->getFont().drawString(ofToString(selectedKeyframes[i]->tmp_value, 4), screenpoint.x+5, screenpoint.y-5);
				else
					timeline->getFont().drawString(ofToString(selectedKeyframes[i]->orig_value, 4), screenpoint.x+5, screenpoint.y-5);
			}
			ofCircle(screenpoint.x, screenpoint.y, 4);
                        // cout << "ofxTLKeyframes::draw(): circle "<<screenpoint.x << ", "<< screenpoint.y << endl;
		}
	}

	ofPopStyle();
}
#endif


// save

