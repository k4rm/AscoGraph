/**
 * BeatCurve : continuous Curve objects editing in Antescofo score langage
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

#include "Curves.h"
#include "BeatCurve.h"
#include "ofxTimeline.h"
#include "ofxHotKeys.h"

bool curve_debug_ = false;

BeatCurve::BeatCurve()
: BeatKeyframes()
{
	initializeEasings();
	valueRange = ofRange(0.0, 1.0);
	drawingEasingWindow = false;

	boundsCached = ofRectangle(0, 0, 0, 0);
	bDrawApplyButton = false;
}

BeatCurve::~BeatCurve()
{
	while(!easingFunctions.empty()) delete easingFunctions.back(), easingFunctions.pop_back();
	while(!easingTypes.empty()) delete easingTypes.back(), easingTypes.pop_back();
}


float BeatCurve::interpolateValueForKeys(BeatKeyframe* start, BeatKeyframe* end, float sampleBeat){
	//cout << "BeatCurve: interpolateValueForKeys: sampleBeat="<<sampleBeat << " start:" << start->beat << "/" << start->value << " end:" << end->beat << "/" << end->value<< endl;
	TweenBeatKeyframe* tweenKeyStart = (TweenBeatKeyframe*)start;
	TweenBeatKeyframe* tweenKeyEnd = (TweenBeatKeyframe*)end;
	return ofxTween::map(sampleBeat, tweenKeyStart->beat, tweenKeyEnd->beat, tweenKeyStart->value, tweenKeyEnd->value,
			false, *tweenKeyStart->easeFunc->easing, tweenKeyStart->easeType->type);
}

string BeatCurve::getTrackType(){
	return "Beat Curves";    
}

BeatKeyframe* BeatCurve::newKeyframe(){
	TweenBeatKeyframe* k = new TweenBeatKeyframe();
	k->easeFunc = easingFunctions[0];
	k->easeType = easingTypes[0];
	return k;
}


void BeatCurve::drawModalContent(){
	//cout << "BeatCurve::drawModalContent() " << endl;
	//****** DRAW EASING CONTROLS
	if(!drawingEasingWindow){
		return;
	}

	TweenBeatKeyframe* tweenFrame = (TweenBeatKeyframe*) selectedKeyframe;
	if(tweenFrame == NULL){
		if(selectedKeyframes.size() == 0){
			return;
		}
		tweenFrame = (TweenBeatKeyframe*)selectedKeyframes[0];
	}

	for(int i = 0; i < easingTypes.size(); i++){
		//TODO turn into something like selectionContainsEaseType();
		//so that we can show the multi-selected easies
		if(easingTypes[i] ==  ((TweenBeatKeyframe*)selectedKeyframes[0])->easeType){
			ofSetColor(150, 100, 10);
		}
		else{
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
			ofSetColor(150, 100, 10);
		}
		else{
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

bool BeatCurve::mousePressed(ofMouseEventArgs& args, long millis){
	//cout << "BeatCurve::mousePressed: " << args.x << ", " << args.y << " bounds:" << bounds.x << ", " << bounds.y << " " << bounds.width << " x " << bounds.height << endl;
	if (!bounds.inside(args.x, args.y)) return false;
	if(drawingEasingWindow){
		return true;
	}
	if(bDrawApplyButton && mApplyBtnRect.inside(args.x, args.y))
		return true;
	else {
		//return ofxTLBeatKeyframes::mousePressed(args,millis);
		if (curve_debug_) cout <<"BeatCurve: mousepressed inside" << endl;
		ofVec2f screenpoint = ofVec2f(args.x, args.y);
		keysAreStretchable = ofGetModifierShiftPressed() && ofGetModifierControlPressed();
		keysDidDrag = false;
		//if(keysAreStretchable && timeline->getTotalSelectedItems() > 1){
		if(keysAreStretchable && selectedKeyframes.size() > 1) {
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
				stretchAnchor = 0;
				//updateStretchOffsets(screenpoint, timeline->millisecToBeat(millis));
				//updateStretchOffsets(screenpoint, timeline->normalizedXToBeat(timeline->screenXtoNormalizedX(args.x, zoomBounds)));//timeline->millisecToBeat());
				for(int k = 0; k < selectedKeyframes.size(); k++){
					selectedKeyframes[k]->grabBeatOffset  = selectedKeyframes[k]->beat;
					selectedKeyframes[k]->grabValueOffset = selectedKeyframes[k]->value;
				}
			}
			return true;
		}

		keysAreDraggable = !ofGetModifierShiftPressed();
		selectedKeyframe =  keyframeAtScreenpoint(screenpoint);
		//if we clicked OFF of a keyframe OR...
		//if we clicked on a keyframe outside of the current selection and we aren't holding down shift, clear all
		if(!ofGetModifierSelection() && (1 || /*isActive() ||*/ selectedKeyframe != NULL) ){
			bool didJustDeselect = false;
			if( selectedKeyframe == NULL || !isKeyframeSelected(selectedKeyframe)){
				//settings this to true causes the first click off of the timeline to deselct rather than create a new keyframe
				//didJustDeselect = timeline->getTotalSelectedItems() > 1;
				didJustDeselect = selectedKeyframes.size() > 1;
				timeline->unselectAll();
				selectedKeyframes.clear();
			}

			//if we didn't just deselect everything and clicked in an empty space add a new keyframe there
			if(selectedKeyframe == NULL && !didJustDeselect){
				createNewOnMouseup = args.button == 0 && !ofGetModifierControlPressed();
				if (curve_debug_) cout << "---------- should create new keyframe:" << createNewOnMouseup << endl;
			}
		}

		if(selectedKeyframe != NULL){
			//add the keyframe to the selection, whether it was just generated or not
			if(!isKeyframeSelected(selectedKeyframe)){
				selectedKeyframes.push_back(selectedKeyframe);
				selectedKeyframe->orig_beat = selectedKeyframe->beat;
				selectedKeyframe->orig_value = selectedKeyframe->value;
				updateKeyframeSort();
				if (curve_debug_) cout << "--------------------------------------- MOUSEDPRESSED origvalue:" << selectedKeyframe->orig_value << endl;
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
				if (curve_debug_) cout << "--------------------------------------- MOUSEDPRESSED 2 origvalue:" << selectedKeyframe->orig_value << endl;

				if(args.button == 0 && !ofGetModifierSelection() && !ofGetModifierControlPressed()){
					//timeline->setDragTimeOffset(timeline->beatToMillisec(selectedKeyframe->grabBeatOffset));
					for(int k = 0; k < selectedKeyframes.size(); k++){
						selectedKeyframes[k]->grabBeatOffset  = selectedKeyframes[k]->beat;
						selectedKeyframes[k]->grabValueOffset = selectedKeyframes[k]->value;
					}
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

void BeatCurve::mouseMoved(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	if(!drawingEasingWindow){
		BeatKeyframes::mouseMoved(args, millis);
	} else cursor.set(args.x, args.y);
}

void BeatCurve::mouseDragged(ofMouseEventArgs& args, long millis){
	if (ref->parentCurve->howmany != 1) return;
	//if (!bounds.inside(args.x, args.y)) return;
	//if(!drawingEasingWindow){ ofxTLBeatKeyframes::mouseDragged(args, millis); }
	//float beat = timeline->millisecToBeat(millis);
	float beat = timeline->normalizedXToBeat(timeline->screenXtoNormalizedX(args.x, zoomBounds));
	if (curve_debug_) cout << "BeatCurve::mouseDragged: beat clicked:" << beat << endl;
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


	// if dragging last key, extend bounds
	int maxi = 0;
	for (int i = 0; i < selectedKeyframes.size(); i++) {
		int newbeat = timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(args.x, zoomBounds));
		if (selectedKeyframes[i]->beat < newbeat) {
			if (curve_debug_) cout << "--------------------------------------------> growing from: "<< bounds.width << " to " << args.x - bounds.x + 160  << endl;
			bounds.width += args.x - bounds.x + 160;
			//timeline->normalizedXtoScreenX( timeline->beatToNormalizedX()) 
			break;
		}
	}


	if(keysAreDraggable && selectedKeyframes.size() != 0){
		ofVec2f screenpoint(args.x,args.y);
		for(int k = 0; k < selectedKeyframes.size(); k++){
			ofVec2f newScreenPosition;
			//cout << "mouseDragged: clamp: " <<   ofClamp(beat - selectedKeyframes[k]->grabBeatOffset, timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMinX())), timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMaxX()))) << endl;
			setKeyframeBeat(selectedKeyframes[k], ofClamp(beat,// - selectedKeyframes[k]->grabBeatOffset,
						timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMinX(), zoomBounds)),  //XXX FUCKED UP BOUNDS SHIT
						timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(bounds.getMaxX(), zoomBounds))));
			if (curve_debug_) cout << "BeatCurve: mouseDragged new beat: " << selectedKeyframes[k]->beat << endl;
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

void BeatCurve::mouseReleased(ofMouseEventArgs& args, long millis){
	if (ref->parentCurve->howmany != 1) return;

	keysAreDraggable = false;
	//cout << "BeatCurve::mouseReleased: x="<< args.x << " y=" << args.y << " mApplyRect: " << mApplyBtnRect.x << ", " << mApplyBtnRect.y << " - " << mApplyBtnRect.width << "-" << mApplyBtnRect.height << " isInside: " << mApplyBtnRect.inside(args.x, args.y) << endl;
	if(bDrawApplyButton && mApplyBtnRect.inside(args.x, args.y)) {
		if (curve_debug_) cout << "BeatCurve::mouseReleased: should apply" << endl;
		if (curve_debug_) cout << "-----> ligne " << ref->parentCurve->lineNum_begin << " - " << ref->parentCurve->lineNum_end << " <------" << endl;
		if (tlAction) tlAction->replaceEditorScore(ref);
		bDrawApplyButton = false;
		return;
	}

	if (drawingEasingWindow) {
		//drawingEasingWindow = false; timeline->dismissedModalContent();
		ofVec2f screenpoint(args.x,args.y);

		bool clickedEasingType = false;
		for(int i = 0; i < easingTypes.size(); i++){
			if(easingTypes[i]->bounds.inside(screenpoint-easingWindowPosition)){
				clickedEasingType = true;
				for(int k = 0; k < selectedKeyframes.size(); k++){
					((TweenBeatKeyframe*)selectedKeyframes[k])->easeType = easingTypes[i];
					((TweenBeatKeyframe*)selectedKeyframes[k])->easeTypeId = i; // store ease type id
					bDrawApplyButton = false;
					// modify easing type in curve
					//float beat = selectedKeyframes[k]->beat;
					//ref->changeKeyframeEasing(beat, ((ofxTLTweenBeatKeyframe*)selectedKeyframes[k])->easeFunc->name);// XXX
					/*
					   switch(i) {
					   case 1:
					   easetype = "_out";
					   break;
					   case 2:
					   easetype = "_in_out";
					   break;
					   default:
					   easetype = "";
					   }
					   cout << "BeatCurve::mouseReleased: easingType : " << i <<  " easetype:" << easetype <<  endl;
					   */
					//drawingEasingWindow = false; timeline->dismissedModalContent();
				}
				//timeline->flagTrackModified(this);
				//shouldRecomputePreviews = true;
				//return;
			}
		}

		for(int i = 0; i < easingFunctions.size(); i++){
			if(easingFunctions[i]->bounds.inside(screenpoint-easingWindowPosition)){
				for(int k = 0; k < selectedKeyframes.size(); k++){
					((TweenBeatKeyframe*)selectedKeyframes[k])->easeFunc = easingFunctions[i];
					// modify easing type in curve
					float beat = selectedKeyframes[k]->beat;
					string easetype;
					switch(((TweenBeatKeyframe*)selectedKeyframes[k])->easeTypeId) {
						case 1:
							easetype = "_out";
							break;
						case 2:
							easetype = "_in_out";
							break;
						default:
							easetype = "";
					}

					if (curve_debug_) cout << "BeatCurve::mouseReleased: easetype:" << easetype << endl;
					string newtype = ((TweenBeatKeyframe*)selectedKeyframes[k])->easeFunc->name;
					newtype += easetype;
					cout << "BeatCurve::mouseReleased: easingFunc:" << i << " newtype:" << newtype << " easeType:" << easetype <<endl;
					ref->changeKeyframeEasing(beat, newtype);
					drawingEasingWindow = false;
					tlAction->shouldDrawModalContent = false;
					timeline->dismissedModalContent();
					bDrawApplyButton = true;
				}
				//TODO timeline->flagTrackModified(this);
				shouldRecomputePreviews = true;
				return;
			}
		}
		if (clickedEasingType)
			return;
	}

	if (!bounds.inside(args.x, args.y) && drawingEasingWindow) {
		// click is out of easingWindow
		drawingEasingWindow = false;
		timeline->dismissedModalContent();
		tlAction->shouldDrawModalContent = false;
		return;
	}

	// if dragging last key, extend bounds
	int maxi = 0;
	for (int i = 0; i < selectedKeyframes.size(); i++) {
		int newbeat = timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(args.x, zoomBounds));
		if (selectedKeyframes[i]->beat < newbeat) {
			cout << "--------------------------------------------> growing from: "<< bounds.width << " to " << args.x - bounds.x + 160  << endl;
			bounds.width += args.x - bounds.x + 160;
			//timeline->normalizedXtoScreenX( timeline->beatToNormalizedX()) 
			break;
		}
	}

	if(keysDidDrag && !drawingEasingWindow){
		//reset these caches because they may no longer be valid
		lastKeyframeIndex = 1;
		lastSampleBeat = 0;
		for (int i = 0; i < selectedKeyframes.size(); i++) {
			if (curve_debug_) cout << "BeatCurve::mouseReleased: selectedkeyframe: origbeat: " << selectedKeyframes[i]->orig_beat << " beat:" << selectedKeyframes[i]->beat << " tmpval:" << selectedKeyframes[i]->tmp_value << " origvalue:"<< selectedKeyframes[i]->orig_value << endl;

			//ref->moveKeyframeAtBeat(selectedKeyframes[i]->beat, selectedKeyframes[i]->orig_beat, selectedKeyframes[i]->tmp_value, selectedKeyframe->orig_value);
			EasingFunction* func = ((TweenBeatKeyframe*)(selectedKeyframes[i]))->easeFunc;
			EasingType* type = ((TweenBeatKeyframe*)(selectedKeyframes[i]))->easeType;

			ref->deleteKeyframeAtBeat(selectedKeyframes[i]->orig_beat);
			ref->addKeyframeAtBeat(selectedKeyframes[i]->beat, selectedKeyframes[i]->tmp_value);
			string easetype;
			switch(((TweenBeatKeyframe*)selectedKeyframes[i])->easeTypeId) {
				case 1:
					easetype = "_out";
					break;
				case 2:
					easetype = "_in_out";
					break;
				default:
					easetype = "";
			}

			cout << "BeatCurve::mouseReleased: easetype:" << easetype << endl;
			string newtype = ((TweenBeatKeyframe*)selectedKeyframes[i])->easeFunc->name;
			newtype += easetype;
			ref->changeKeyframeEasing(selectedKeyframes[i]->beat, newtype);

			bDrawApplyButton = true;

			selectedKeyframes[i]->orig_value = selectedKeyframes[i]->tmp_value;
			//selectedKeyframes[i]->tmp_value = 0;
			if (curve_debug_) cout << "BeatCurve::mouseReleased: selectedkeyframe: valureRange.min=" << valueRange.min << " valueRange.max=" << valueRange.max << endl;
			selectedKeyframes[i]->value = ofMap(selectedKeyframes[i]->orig_value, valueRange.min, valueRange.max, 0, 1.0, true);
			if (curve_debug_) cout << "BeatCurve::mouseReleased: selectedkeyframe: orig_value=" << selectedKeyframes[i]->orig_value << " tmp_value=" << selectedKeyframes[i]->tmp_value<< endl;
			setKeyframeBeat(selectedKeyframes[i], selectedKeyframes[i]->beat);
			if (curve_debug_) cout << "BeatCurve::mouseReleased: selectedkeyframe: value=" << selectedKeyframes[i]->value << endl;
			selectedKeyframes[i]->orig_beat = selectedKeyframes[i]->beat;
		}
	}

	if (curve_debug_) cout << "mouseReleased:---------- should create new keyframe:" << createNewOnMouseup << endl;
	if (createNewOnMouseup && !drawingEasingWindow) {
		if (ref->parentCurve->howmany == 1) {
			//float beat = timeline->millisecToBeat(millis);
			float beat = timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(args.x, zoomBounds));
			//add a new one
			selectedKeyframe = newKeyframe();
			setKeyframeBeat(selectedKeyframe, beat);
			selectedKeyframe->value = screenYToValue(args.y);
			selectedKeyframe->orig_value = ofMap(selectedKeyframe->value, 0, 1.0, valueRange.min, valueRange.max, true);
			keyframes.push_back(selectedKeyframe);
			selectedKeyframes.push_back(selectedKeyframe);
			updateKeyframeSort();
			//TODO timeline->flagTrackModified(this);

			// when new breakpoint is created, we should reduce next breakpoint duration, before adding
			cout << "BeatCurve::mouseReleased: howmany:" << ref->parentCurve->howmany << endl;
			ref->addKeyframeAtBeat(beat, selectedKeyframe->orig_value);
			bDrawApplyButton = true;
		}
	}

	createNewOnMouseup = false;
}

// change keyframe easing type
void BeatCurve::changeKeyframeEasing(float beat, string type) {
	std::transform(type.begin(), type.end(), type.begin(), ::tolower); // to_lower case
	float dcumul = 0;//ref->parentCurve->header->beatnum;
	int i = 0;
	bool done = false;
	int easeType = 0;
	//cout << "BeatCurve::changeKeyframeEasing: beat:"<< beat << " type:"<< type << " easingType:"<< easeType << endl;
	if (type[0] == '\"') type.erase(0, 1);
	if (type[type.size()-1] == '\"') type.erase(type.size()-1, 1);
	string roottype = type;
	//cout << "------> type:" << type << endl;
	if (type.size() > 5) {
		if (type.size() > 8 && type.compare(type.size()-7, 7, "_in_out") == 0) {
			easeType = 2;
			roottype = type.substr(0, type.size()-7);
		} else if (type.compare(type.size()-4, 4, "_out") == 0) {
			easeType = 1;
			roottype = type.substr(0, type.size()-4);
		} else if (type.compare(type.size()-3, 3, "_in") == 0) {
			easeType = 0;
			roottype = type.substr(0, type.size()-3);
		}
	}
	if (curve_debug_) cout << "BeatCurve::changeKeyframeEasing: beat:"<< beat << " type:"<< type << " easingType:"<< easeType << endl;

	//if (type.size() && type[0] == '\"') type = type.substr(1, type.size() - 2);

	for(int i = 0; i < keyframes.size(); i++) {
		//if (debug_edit_curve) cout << "BeatCurve:: change keyframe easing at beat: looping : " << i << " curdur:" << (*k)->eval() <<  " dcumul:" << dcumul<< endl;

		dcumul = keyframes[i]->beat;
		if (dcumul < beat)
			continue;
		else {
			for(int j = 0; j < easingFunctions.size(); j++){
				//cout << "BeatCurve::changeKeyframeEasing: got for keyframe: " << i << " name:" << easingFunctions[j]->name << endl;
				string str = easingFunctions[j]->name;
				//cout << "roottype: "<< roottype<< endl;
				if(roottype == str) {
					//cout << "BeatCurve::changeKeyframeEasing: /!\\ changed to type " << type << " n:"<< i+1 <<endl;
					// modify easing type
					((TweenBeatKeyframe*)keyframes[i])->easeFunc = easingFunctions[j];
					((TweenBeatKeyframe*)keyframes[i])->easeType = easingTypes[easeType];
				}
			}
			/*
			   for(int j = 0; j < easingTypes.size(); j++){
			   if(easingTypes[j]->easeType == easingTypes[i]) {
			   ((ofxTLTweenBeatKeyframe*)keyframes[i])->easeType = easingTypes[j];
			   return;
			   }
			   } */
		}
		break;
	}
}


void BeatCurve::willDeleteKeyframe(BeatKeyframe* keyframe){
	bDrawApplyButton = true;
	ref->deleteKeyframeAtBeat(keyframe->beat);
	if (curve_debug_) cout << "BeatKeyframes::willDeleteKeyframe" << endl;
}

// called when a keyframe is mouse moved, : this fct should:
// 1/ re-fill the Curve antescofo parser object,
// 2/ pretty print the action
// 3/ eplace it in the text score
/*
   void BeatCurve::updateEditorContent()
   {
   }
   */

void BeatCurve::selectedKeySecondaryClick(ofMouseEventArgs& args){
	if (ref->parentCurve->howmany != 1) return;
	easingWindowPosition = ofVec2f(MIN(args.x, bounds.width + bounds.x - easingBoxWidth),
			MIN(args.y, timeline->getBottomLeft().y - (tweenBoxHeight*easingFunctions.size())));

	drawingEasingWindow = true;
	timeline->presentedModalContent(track);
}

void BeatCurve::restoreKeyframe(BeatKeyframe* key, ofxXmlSettings& xmlStore){
	TweenBeatKeyframe* tweenKey =  (TweenBeatKeyframe*)key;    
	tweenKey->easeFunc = easingFunctions[ofClamp(xmlStore.getValue("easefunc", 0), 0, easingFunctions.size()-1)];
	tweenKey->easeType = easingTypes[ofClamp(xmlStore.getValue("easetype", 0), 0, easingTypes.size()-1)];
}

void BeatCurve::storeKeyframe(BeatKeyframe* key, ofxXmlSettings& xmlStore){
	TweenBeatKeyframe* tweenKey =  (TweenBeatKeyframe*)key;
	xmlStore.addValue("easefunc", tweenKey->easeFunc->id);
	xmlStore.addValue("easetype", tweenKey->easeType->id);
}

void BeatCurve::initializeEasings(){

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
	ef->name = "circ";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingQuad();
	ef->name = "quad";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingCubic();
	ef->name = "cubic";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingQuart();
	ef->name = "quart";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingQuint();
	ef->name = "quint";
	easingFunctions.push_back(ef);

	ef = new EasingFunction();
	ef->easing = new ofxEasingExpo();
	ef->name = "exp";
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

void BeatCurve::recomputePreviews(){
	preview.clear();

	//cout << "BeatCurve::recomputePreviews " << endl; cout << "bounds.getMinX:" << bounds.getMinX() << " maxx::"<< bounds.getMaxX() << endl;

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

bool BeatCurve::get_first_last_displayed_keyframe(ofVec2f* coord1, ofVec2f* coord2, int* firsti, int* lasti) {
	//cout <<"BeatCurve::get_first_last_displayed_keyframe(): tlbounds: "<<  tlBounds.x << ", " << tlBounds.y << " wxh:" << tlBounds.width << "x" << tlBounds.height << endl;
	int i = 0;

	*firsti = 0;
	*lasti = 1;
	bool found = false;
	coord1->x = coord1->y = 0;
	coord2->x = bounds.x + bounds.width + 1;
	coord2->y = 0;
	for (int i = 0; i < keyframes.size(); i++) {
		if (isKeyframeIsInBounds(keyframes[i])) {
			*coord1 = screenPositionForKeyframe(keyframes[i]);
			*firsti = i;
			//cout << "get_first_last_displayed_keyframe: found first=" << i << endl;
			found = true;
			break;
		}
	}
	for (int i = keyframes.size() - 1; i >= 0; i--) {
		if (isKeyframeIsInBounds(keyframes[i])) {
			*coord2 = screenPositionForKeyframe(keyframes[i]);
			*lasti = i; 
			//cout << "get_first_last_displayed_keyframe: found last=" << i << " beat:" << keyframes[i]->beat << endl;
			found = true;
			break;
		}
	}
	// if not found in bounds, find out bounds
	if (!found) { 
		// find which keyframe is needed for interpolation on bounds.x
		ofVec2f bx(bounds.x, 0);

		for (int i = 0; i < keyframes.size(); i++) {
			//float x = timeline->normalizedXtoScreenX( timeline->beatToNormalizedX( keyframes[i]->beat), zoomBounds);
			float x = tlAction->get_x( keyframes[i]->beat );
			if ( x > bounds.x && x > tlBounds.x) {
				//cout << "get_first_last_displayed_keyframe: not found first=" << i-1 << endl;
				*firsti = i - 1;
				break;
			}
		}

		for (int i = keyframes.size() - 1; i >= 0; i--) {
			//float x = timeline->normalizedXtoScreenX( timeline->beatToNormalizedX( keyframes[i]->beat), zoomBounds);
			float x = tlAction->get_x( keyframes[i]->beat );
			if (x < bounds.x + bounds.width && x < tlBounds.x + tlBounds.width) {
				*lasti = i;
				//cout << "get_first_last_displayed_keyframe: not found last=" << i << endl;
				*coord2 = ofVec2f(bounds.x + bounds.width + 1, 0);
				break;
			}
		}
	}
	if (*firsti == *lasti) {
		if (*firsti)
			*firsti = *firsti - 1;
		else *lasti = 1;
	}
	if (*lasti == -1) *lasti = *firsti + 1;
	if (curve_debug_) cout << "get first last:==========> [ "<< *firsti << " - " << *lasti << " ] : coord1:("<< coord1->x << ", " << coord1->y << "), coord2:("<< coord2->x << ", " << coord2->y << ")"<< endl;

	return found;
}


void BeatCurve::draw(){
	if (curve_debug_) cout << "BeatCurve::draw(): bounds: x:"<<bounds.x << " y:" << bounds.y << " " << bounds.width << "x" << bounds.height << " valueRange:" << valueRange.min << ":" << valueRange.max << endl;
	if(bounds.width == 0 || bounds.height < 2 || keyframes.empty()){
		return;
	}

	bool redraw = false;
	if(shouldRecomputePreviews || viewIsDirty){
		recomputePreviews();
		redraw = true;
	}

	/*
	   if (bounds == boundsCached && !redraw) {
	   drawCache.draw(0,0);
	   return;
	   }

	   redraw = true;
	   bounds = boundsCached;

	   drawCache.begin();
	   */

	ofVec2f screenpoint_first, screenpoint_last;
	int firsti = 0, lasti = 0;
	bool inbounds = get_first_last_displayed_keyframe(&screenpoint_first, &screenpoint_last, &firsti, &lasti);

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

	//***** DRAW KEYFRAME LINES
	ofSetLineWidth(2);
	ofSetColor(keyColor);
	ofNoFill();

	preview.draw();
	ofSetLineWidth(1);

	//**** DRAW KEYFRAME DOTS

	//**** HOVER FRAME
	if(hoverKeyframe != NULL){
		ofPushStyle();
		ofFill();
		ofSetColor(timeline->getColors().highlightColor);
		ofVec2f hoverKeyPoint = screenPositionForKeyframe( hoverKeyframe );
		ofSetColor(255, 0, 0, 255);
		ofCircle(hoverKeyPoint.x, hoverKeyPoint.y, 6);
		ofPopStyle();
	}

	//**** ALL CACHED VISIBLE KEYS
	ofSetColor(timeline->getColors().textColor);
	ofNoFill();
	for(int i = 0; i < keyPoints.size(); i++){
		if (curve_debug_) cout << "BeatCurve::draw(): "<<keyPoints[i].x << ", "<< keyPoints[i].y << endl;
		ofRect(keyPoints[i].x-1, keyPoints[i].y-1, 3, 3);
	}

	//**** SELECTED KEYS
	ofSetColor(timeline->getColors().textColor);
	ofFill();
	//cout << "Keyframes::draw(): selectedKeyframes.size:"<< selectedKeyframes.size() << endl;
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
			ofSetColor(255, 0, 0, 255);
			ofNoFill();
			ofCircle(screenpoint.x, screenpoint.y, 5);
			//if (curve_debug_) cout << "Keyframes::draw(): circle "<<screenpoint.x << ", "<< screenpoint.y << endl;
		}
	}

	ofPopStyle();

	// Draw Apply btn 
	if (bDrawApplyButton) {
		ofPushStyle();

		// draw min bg
		ofSetColor(0, 0, 250, 20);
		ofFill();
		ofRect(bounds);


		ofFill();
		ofSetColor(timeline->getColors().backgroundColor);
		mApplyBtnRect.x = bounds.x + bounds.width - 54;
		mApplyBtnRect.y = bounds.y + 4;// - 13 - 4;
		mApplyBtnRect.width = 48;
		mApplyBtnRect.height = 14;
		ofRect(mApplyBtnRect);
		ofSetColor(0);
		ofNoFill();
		ofRect(mApplyBtnRect);

		ofDrawBitmapString("Apply",  mApplyBtnRect.x + 4, mApplyBtnRect.y + 10);
		ofPopStyle();
	}

}

void BeatCurve::draw_big(ofRectangle& notebounds){
	cout << "BeatCurve::draw_big" << endl;

}
