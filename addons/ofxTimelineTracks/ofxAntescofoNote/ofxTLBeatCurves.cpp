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
#include "ofxHotKeys.h"


ofxTLBeatCurves::ofxTLBeatCurves()
: ofxTLBeatKeyframes()
{
	initializeEasings();
	valueRange = ofRange(0.0, 1.0);
	drawingEasingWindow = false;

	bDrawApplyButton = false;
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
		//return ofxTLBeatKeyframes::mousePressed(args,millis);
		cout <<"ofxTLBeatCurves: mousepressed inside" << endl;
		ofVec2f screenpoint = ofVec2f(args.x, args.y);
		keysAreStretchable = ofGetModifierShiftPressed() && ofGetModifierControlPressed();
		keysDidDrag = false;
		if(keysAreStretchable && timeline->getTotalSelectedItems() > 1){
			unsigned long minSelected = timeline->getEarliestSelectedTime();
			unsigned long maxSelected = timeline->getLatestSelectedTime();
			if(minSelected == maxSelected){
				keysAreStretchable = false;
			}
			else {
				unsigned long midSelection = (maxSelected-minSelected)/2 + minSelected;
				//the anchor is the selected key opposite to where we are stretching
				stretchAnchor = midSelection <= millis ? minSelected : maxSelected;
				//			cout << "Min selected " << ofxTimecode::timecodeForMillis(minSelected) << " Mid Selected " << ofxTimecode::timecodeForMillis(midSelection) << " Max selected " << ofxTimecode::timecodeForMillis(maxSelected) << " anchor "  << ofxTimecode::timecodeForMillis(stretchAnchor) << " millis down " << ofxTimecode::timecodeForMillis(millis) << endl;
				stretchSelectPoint = millis;
				//don't do anything else, like create or deselect keyframes
				updateStretchOffsets(screenpoint, timeline->millisecToBeat(millis));
			}
			return true;
		}

		keysAreDraggable = !ofGetModifierShiftPressed();
		selectedKeyframe =  keyframeAtScreenpoint(screenpoint);
		//if we clicked OFF of a keyframe OR...
		//if we clicked on a keyframe outside of the current selection and we aren't holding down shift, clear all
		if(!ofGetModifierSelection() && (isActive() || selectedKeyframe != NULL) ){
			bool didJustDeselect = false;
			if( selectedKeyframe == NULL || !isKeyframeSelected(selectedKeyframe)){
				//settings this to true causes the first click off of the timeline to deselct rather than create a new keyframe
				didJustDeselect = timeline->getTotalSelectedItems() > 1;
				timeline->unselectAll();
			}

			//if we didn't just deselect everything and clicked in an empty space add a new keyframe there
			if(selectedKeyframe == NULL && !didJustDeselect){
				createNewOnMouseup = args.button == 0 && !ofGetModifierControlPressed();
			}
		}

		if(selectedKeyframe != NULL){
			//add the keyframe to the selection, whether it was just generated or not
			if(!isKeyframeSelected(selectedKeyframe)){
				selectedKeyframes.push_back(selectedKeyframe);
				selectedKeyframe->orig_beat = selectedKeyframe->beat;
				selectedKeyframe->orig_value = selectedKeyframe->value;
				updateKeyframeSort();
				cout << "--------------------------------------- MOUSEDPRESSED origvalue:" << selectedKeyframe->orig_value << endl;
				//			selectKeyframe(selectedKeyframe);
			}
			//unselect it if it's selected and we clicked the key with shift pressed
			else if(ofGetModifierSelection()){
				deselectKeyframe(selectedKeyframe);
				selectedKeyframe = NULL;
			}
		}

		//if we have any keyframes selected update the grab offsets and check for showing the modal window
		if(selectedKeyframes.size() != 0){
			updateDragOffsets(screenpoint, millis);
			if(selectedKeyframe != NULL){
				selectedKeyframe->orig_beat = selectedKeyframe->beat;
				selectedKeyframe->orig_value = selectedKeyframe->value;
				cout << "--------------------------------------- MOUSEDPRESSED 2 origvalue:" << selectedKeyframe->orig_value << endl;

				if(args.button == 0 && !ofGetModifierSelection() && !ofGetModifierControlPressed()){
					timeline->setDragTimeOffset(timeline->beatToMillisec(selectedKeyframe->grabBeatOffset));
					//move the playhead
					if(timeline->getMovePlayheadOnDrag()){
						timeline->setCurrentTimeMillis(timeline->beatToMillisec(selectedKeyframe->beat));
					}
				}
				if(args.button == 2 || ofGetModifierControlPressed()){
					selectedKeySecondaryClick(args);
				}
			}
		}
		return selectedKeyframe != NULL;
	}
}

void ofxTLBeatCurves::mouseMoved(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	if(!drawingEasingWindow){
		ofxTLBeatKeyframes::mouseMoved(args, millis);
	} else cursor.set(args.x, args.y);
}
void ofxTLBeatCurves::mouseDragged(ofMouseEventArgs& args, long millis){
	//if (!bounds.inside(args.x, args.y)) return;
	//if(!drawingEasingWindow){ ofxTLBeatKeyframes::mouseDragged(args, millis); }
	float beat = timeline->millisecToBeat(millis);
	if(keysAreStretchable){
		//cast the stretch anchor to long so that it can be signed
		float stretchRatio = 1.0*(beat-long(stretchAnchor)) / (1.0*stretchSelectPoint-stretchAnchor);

		for(int k = 0; k < selectedKeyframes.size(); k++){
			setKeyframeBeat(selectedKeyframes[k], ofClamp(stretchAnchor + (selectedKeyframes[k]->grabBeatOffset * stretchRatio),
						0, timeline->millisecToBeat(timeline->getDurationInMilliseconds())));
			selectedKeyframes[k]->screenPosition = screenPositionForKeyframe(selectedKeyframes[k]);
		}
		timeline->flagUserChangedValue();
		keysDidDrag = true;
		updateKeyframeSort();
	}

	if(keysAreDraggable && selectedKeyframes.size() != 0){
		ofVec2f screenpoint(args.x,args.y);
		for(int k = 0; k < selectedKeyframes.size(); k++){
			ofVec2f newScreenPosition;
			//cout << "mouseDragged: clamp: " <<   ofClamp(beat - selectedKeyframes[k]->grabBeatOffset, timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMinX())), timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMaxX()))) << endl;
			setKeyframeBeat(selectedKeyframes[k], ofClamp(beat - selectedKeyframes[k]->grabBeatOffset,
						timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMinX())), 
						timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMaxX()))));
			selectedKeyframes[k]->value = screenYToValue(args.y - selectedKeyframes[k]->grabValueOffset);
			//selectedKeyframes[k]->orig_value = ofMap(selectedKeyframes[k]->value, 0, 1.0, valueRange.min, valueRange.max, true); // ADDED
			selectedKeyframes[k]->tmp_value = ofMap(selectedKeyframes[k]->value, 0, 1.0, valueRange.min, valueRange.max, true);
			selectedKeyframes[k]->screenPosition = screenPositionForKeyframe(selectedKeyframes[k]);
			//selectedKeyframes[k]->beat = 
		}
		if(selectedKeyframe != NULL && timeline->getMovePlayheadOnDrag()){
			timeline->setCurrentTimeMillis(timeline->beatToMillisec(selectedKeyframe->beat));
		}
		timeline->flagUserChangedValue();
		keysDidDrag = true;
		updateKeyframeSort();
	}
	createNewOnMouseup = false;

}

void ofxTLBeatCurves::mouseReleased(ofMouseEventArgs& args, long millis){
		cout << "ofxTLBeatCurves::mouseReleased: should apply" << endl;
	keysAreDraggable = false;
	if(bDrawApplyButton && mApplyBtnRect.inside(args.x, args.y)) {
		cout << "ofxTLBeatCurves::mouseReleased: should apply" << endl;
		cout << "-----> ligne " << ref->header->lineNum_begin << " - " << ref->header->lineNum_end << " <------" << endl;
		if (tlAction) tlAction->replaceEditorScore(ref);
		bDrawApplyButton = false;
	}
	if(keysDidDrag){
		//reset these caches because they may no longer be valid
		lastKeyframeIndex = 1;
		lastSampleBeat = 0;
		timeline->flagTrackModified(this);
		for(int i = 0; i < selectedKeyframes.size(); i++){
			cout << "ofxTLBeatCurves::mouseReleased: selectedkeyframe: origbeat: " <<  selectedKeyframes[i]->orig_beat << " beat:" << selectedKeyframes[i]->beat << endl;

			//ref->moveKeyframeAtBeat(selectedKeyframes[i]->beat, selectedKeyframes[i]->orig_beat, selectedKeyframes[i]->tmp_value, selectedKeyframe->orig_value);
			ref->deleteKeyframeAtBeat(selectedKeyframes[i]->orig_beat);
			ref->addKeyframeAtBeat(selectedKeyframes[i]->beat, selectedKeyframes[i]->tmp_value);
			bDrawApplyButton = true;

			selectedKeyframes[i]->orig_value = selectedKeyframes[i]->tmp_value;
			selectedKeyframes[i]->tmp_value = 0;
			selectedKeyframes[i]->value = ofMap(selectedKeyframes[i]->orig_value, valueRange.min, valueRange.max, 0, 1.0, true);
			setKeyframeBeat(selectedKeyframes[i], selectedKeyframes[i]->beat);
			selectedKeyframes[i]->orig_beat = selectedKeyframes[i]->beat;
		}
	}

	if(createNewOnMouseup) {
		float beat = timeline->millisecToBeat(millis);
		//add a new one
		selectedKeyframe = newKeyframe();
		setKeyframeBeat(selectedKeyframe, beat);
		selectedKeyframe->value = screenYToValue(args.y);
		selectedKeyframe->orig_value = ofMap(selectedKeyframe->value, 0, 1.0, valueRange.min, valueRange.max, true);
		keyframes.push_back(selectedKeyframe);
		selectedKeyframes.push_back(selectedKeyframe);
		updateKeyframeSort();
		timeline->flagTrackModified(this);

		// when new breakpoint is created, we should reduce next breakpoint duration, before adding
		ref->addKeyframeAtBeat(beat, selectedKeyframe->orig_value);
		bDrawApplyButton = true;
	}


	createNewOnMouseup = false;

#if OLDSHIT
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
					// modify easing type in curve
					float beat = selectedKeyframes[k]->beat;
					ref->changeKeyframeEasing(beat, ((ofxTLTweenBeatKeyframe*)selectedKeyframes[k])->easeFunc->name);// XXX

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
					// modify easing type in curve
					float beat = selectedKeyframes[k]->beat;
					ref->changeKeyframeEasing(beat, ((ofxTLTweenBeatKeyframe*)selectedKeyframes[k])->easeFunc->name);// XXX
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
#endif
}

void ofxTLBeatCurves::willDeleteKeyframe(ofxTLBeatKeyframe* keyframe){
	bDrawApplyButton = true;
	ref->deleteKeyframeAtBeat(keyframe->beat);
	cout << "ofxTLBeatKeyframes::willDeleteKeyframe" << endl;
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

	//cout << "ofxTLBeatCurves::recomputePreviews " << endl; cout << "bounds.getMinX:" << bounds.getMinX() << " maxx::"<< bounds.getMaxX() << endl;

	//	if(keyframes.size() == 0 || keyframes.size() == 1){
	//		preview.addVertex(ofPoint(bounds.x, bounds.y + bounds.height - sampleAtPercent(.5f)*bounds.height));
	//		preview.addVertex(ofPoint(bounds.x+bounds.width, bounds.y + bounds.height - sampleAtPercent(.5f)*bounds.height));
	//	}
	//	else{
	for(int p = bounds.getMinX(); p <= bounds.getMaxX(); p++){
		preview.addVertex(p,  bounds.y + bounds.height - sampleAtPercent(screenXtoNormalizedX(p, zoomBounds)) * bounds.height);
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
	
	// Draw Apply btn 
	if (bDrawApplyButton) {
		ofPushStyle();
		ofFill();
		ofSetColor(timeline->getColors().backgroundColor);
		mApplyBtnRect.x = bounds.x + bounds.width - 80;
		mApplyBtnRect.y = bounds.y - 13 - 4;
		mApplyBtnRect.width = 70;
		mApplyBtnRect.height = 14;
		ofRect(mApplyBtnRect);
		ofSetColor(0);
		ofNoFill();
		ofRect(mApplyBtnRect);

		ofDrawBitmapString("Apply",  mApplyBtnRect.x + 4, mApplyBtnRect.y + 10);

		// draw min bg
		ofSetColor(200, 0, 0, 160);
		ofFill();
		ofRect(bounds);
		ofPopStyle();
	}

	ofPushStyle();


        //draw current value indicator as a big transparent rectangle
	//ofSetColor(timeline->getColors().disabledColor, 30);
	ofSetColor(timeline->getColors().outlineColor, 170);
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
		//ofVertex(bounds.getMaxX(), screenpoint.y); // right low corner of rect
		//ofVertex(bounds.getMaxX(), bounds.getMaxY()); // right low corner of rect
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

