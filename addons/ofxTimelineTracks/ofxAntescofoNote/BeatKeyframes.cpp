/**
 * BeatKeyframes : continuous Curve objects editing in Antescofo score langage
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

#include "BeatKeyframes.h"
#include "ofxTimeline.h"
#include "ofxHotKeys.h"

bool beatkeyframesort(BeatKeyframe* a, BeatKeyframe* b){
	return a->beat < b->beat;
}

BeatKeyframes::BeatKeyframes()
: Keyframes()
{
	hoverKeyframe = NULL; 
}

BeatKeyframes::~BeatKeyframes(){
}

void BeatKeyframes::recomputePreviews(){
	preview.clear();

	//cout << "BeatKeyframes::recomputePreviews " << endl;

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

void BeatKeyframes::draw(){
	cout << "BeatKeyframes::draw" << endl;
	if(bounds.width == 0 || bounds.height < 2){
		return;
	}

	if(shouldRecomputePreviews || viewIsDirty){
		recomputePreviews();
	}

	ofPushStyle();


	//draw current value indicator as a big transparent rectangle
	ofSetColor(timeline->getColors().disabledColor, 30);
	//jg play solo change
	//float currentPercent = sampleAtTime(timeline->getCurrentTimeMillis());
	//float currentPercent = sampleAtTime(currentTrackTime());
	//ofFill();
	//ofRect(bounds.x, bounds.getMaxY(), bounds.width, -bounds.height*currentPercent);


	//******* DRAW FILL CURVES
	ofSetPolyMode(OF_POLY_WINDING_NONZERO);

	ofFill();
	ofBeginShape();
	for (int i = 0; i < selectedKeyframes.size(); i++) {
		if (isKeyframeIsInBounds(selectedKeyframes[i])){
			ofVec2f screenpoint = screenPositionForKeyframe(selectedKeyframes[i]);
			float keysValue = ofMap(selectedKeyframes[i]->value, 0, 1.0, valueRange.min, valueRange.max, true);
			if(keysAreDraggable){
				string frameString = ofToString(selectedKeyframes[i]->beat);
				timeline->getFont().drawString(ofToString(keysValue, 4), screenpoint.x+5, screenpoint.y-5);
			}
			//ofCircle(screenpoint.x, screenpoint.y, 4);
			ofCurveVertex(screenpoint.x, screenpoint.y);
		}
	}
	if (selectedKeyframes.size()) {
		ofVec2f screenpoint = screenPositionForKeyframe(selectedKeyframes[0]);
		ofCurveVertex(screenpoint.x, screenpoint.y);
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
				string frameString = ofToString(selectedKeyframes[i]->beat);
				timeline->getFont().drawString(ofToString(keysValue, 4), screenpoint.x+5, screenpoint.y-5);
			}
			ofCircle(screenpoint.x, screenpoint.y, 4);
			// cout << "ofxTLKeyframes::draw(): circle "<<screenpoint.x << ", "<< screenpoint.y << endl;
		}
	}

	ofPopStyle();
}


float BeatKeyframes::getValueAtBeat(float sampleBeat){
	return ofMap(sampleAtBeat(sampleBeat), 0.0, 1.0, valueRange.min, valueRange.max, false);
}

float BeatKeyframes::sampleAtPercent(float percent){
	return sampleAtBeat(percent * timeline->millisecToBeat(timeline->getDurationInMilliseconds()));//*timeline->getBPM());
}

float BeatKeyframes::sampleAtBeat(float sampleBeat){
	//cout << "BeatKeyframes:: sampleAtBeat:" << sampleBeat << endl;
	//sampleBeat = ofClamp(sampleBeat, 0, timeline->getDurationInMilliseconds());

	//edge cases
	if(keyframes.size() == 0){
		return ofMap(defaultValue, valueRange.min, valueRange.max, 0, 1.0, true);
	}

	if(sampleBeat <= keyframes[0]->beat){
		return evaluateKeyframeAtBeat(keyframes[0], sampleBeat);
		//		return keyframes[0]->value;
	}

	if(sampleBeat >= keyframes[keyframes.size()-1]->beat){
		//return keyframes[keyframes.size()-1]->value;
		return evaluateKeyframeAtBeat(keyframes[keyframes.size()-1], sampleBeat);
	}

	//optimization for linear playback
	int startKeyframeIndex = 1;
	if(sampleBeat >= lastSampleBeat){
		startKeyframeIndex = lastKeyframeIndex;
	}

	for(int i = startKeyframeIndex; i < keyframes.size(); i++){
		if(keyframes[i]->beat >= sampleBeat){
			lastKeyframeIndex = i;
			lastSampleBeat = sampleBeat;
			return interpolateValueForKeys(keyframes[i-1], keyframes[i], sampleBeat);
		}
	}
	ofLog(OF_LOG_ERROR, "BeatKeyframes --- Error condition, couldn't find keyframe for percent " + ofToString(sampleBeat));
	return defaultValue;
}

float BeatKeyframes::evaluateKeyframeAtBeat(BeatKeyframe* key, float sampleBeat){
	return key->value;
}

float BeatKeyframes::interpolateValueForKeys(BeatKeyframe* start,BeatKeyframe* end, float sampleBeat){
	return ofMap(sampleBeat, start->beat, end->beat, start->value, end->value);
}

void BeatKeyframes::createKeyframesFromXML(ofxXmlSettings xmlStore, vector<BeatKeyframe*>& keyContainer){

	int numKeyframeStores = xmlStore.getNumTags("keyframes");
	for(int store = 0; store < numKeyframeStores; store++){
		xmlStore.pushTag("keyframes",store);
		int numKeyTags = xmlStore.getNumTags("key");

		for(int i = 0; i < numKeyTags; i++){
			xmlStore.pushTag("key", i);
			//ofxTLKeyframe* key = newKeyframe(ofVec2f(xmlStore.getValue("x", 0.0),xmlStore.getValue("y", 0.0)));
			BeatKeyframe* key = newKeyframe();

			string legacyX = xmlStore.getValue("x", "");
			//if there is a decimal this is most likely an old save so let's 
			//convert it based on the current duration
			if(legacyX != ""){
				ofLogNotice() << "BeatKeyframes::createKeyframesFromXML -- Found legacy beat " + legacyX << endl;
				float normalizedBeat = ofToFloat(legacyX);
				key->beat = key->previousBeat =  normalizedBeat*timeline->getDurationInMilliseconds();
			}
			else {
				string bbeat = xmlStore.getValue("beat", "0.0");
				//key->time = key->previousTime = timeline->getTimecode().millisForTimecode(timecode);
				//key->beat = key->previousBeat = bbeat;
				cerr << "TODO or not TODO that is the question" << endl;
			}

			float legacyYValue = xmlStore.getValue("y", 0.0);
			if(legacyYValue != 0.0){
				ofLogNotice() << "BeatKeyframes::createKeyframesFromXML -- Found legacy value " << legacyYValue << endl;
				key->value = legacyYValue;
			}
			else{
				key->value = xmlStore.getValue("value", 0.0f);
			}
			restoreKeyframe(key, xmlStore);			
			xmlStore.popTag(); //key
			keyContainer.push_back( key );
		}

		xmlStore.popTag(); //keyframes
	}
	sort(keyContainer.begin(), keyContainer.end(), beatkeyframesort);
}


string BeatKeyframes::getXMLStringForKeyframes(vector<BeatKeyframe*>& keys){
	//	return "";
	ofxXmlSettings savedkeyframes;
	savedkeyframes.addTag("keyframes");
	savedkeyframes.pushTag("keyframes");

	for(int i = 0; i < keys.size(); i++){
		savedkeyframes.addTag("key");
		savedkeyframes.pushTag("key", i);

		//calling store before saving the default values gives the subclass a chance to modify them
		storeKeyframe(keys[i], savedkeyframes);
		savedkeyframes.addValue("beat", keys[i]->beat);
		savedkeyframes.addValue("value", keys[i]->value);

		savedkeyframes.popTag(); //key
	}

	savedkeyframes.popTag();//keyframes
	string str;
	savedkeyframes.copyXmlToString(str);
	return str;
}

BeatKeyframe* BeatKeyframes::keyframeAtScreenpoint(ofVec2f p){
	if(!bounds.inside(p)){
		return NULL;
	}
	float minDistanceSquared = 15*15;
	for(int i = 0; i < keyframes.size(); i++){
		if(isKeyframeIsInBounds(keyframes[i]) &&
		   p.squareDistance(screenPositionForKeyframe(keyframes[i])) < minDistanceSquared)
		{
			return keyframes[i];
		}
	}
	return NULL;
	//return (BeatKeyframe*)ofxTLKeyframes::keyframeAtScreenpoint(p);
}

bool BeatKeyframes::mousePressed(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return false;
	cout <<"BeatKeyframes: mousepressed" << endl;
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
	if(!ofGetModifierSelection() && (1 || /*isActive() || */ selectedKeyframe != NULL) ){
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
			updateKeyframeSort();
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

//update the grabBeatOffset to prepare for stretching keys
void BeatKeyframes::updateStretchOffsets(ofVec2f screenpoint, float grabBeat){
	for(int k = 0; k < selectedKeyframes.size(); k++){
		selectedKeyframes[k]->grabBeatOffset = selectedKeyframes[k]->beat - stretchAnchor;
	}
}

void BeatKeyframes::updateDragOffsets(ofVec2f screenpoint, float grabMillis){
	float grabBeat = timeline->millisecToBeat(grabMillis);
	for(int k = 0; k < selectedKeyframes.size(); k++){
		selectedKeyframes[k]->grabBeatOffset  = grabBeat - selectedKeyframes[k]->beat;
		selectedKeyframes[k]->grabValueOffset = screenpoint.y - valueToScreenY(selectedKeyframes[k]->value);
	}
}

void BeatKeyframes::mouseMoved(ofMouseEventArgs& args, long millis){

	//ofxTLTrack::mouseMoved(args, millis);
	hoverKeyframe = keyframeAtScreenpoint( ofVec2f(args.x, args.y));
}


void BeatKeyframes::mouseDragged(ofMouseEventArgs& args, long millis){
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
			//selectedKeyframes[k]->orig_value = ofMap(selectedKeyframes[k]->value, 0, 1.0, valueRange.min, valueRange.max, true);
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

void BeatKeyframes::updateKeyframeSort(){
	//reset these caches because they may no longer be valid
	shouldRecomputePreviews = true;
	lastKeyframeIndex = 1;
	lastSampleBeat = 0;
	if(keyframes.size() > 1){

		sort(keyframes.begin(), keyframes.end(), beatkeyframesort);
		for(int i = 0; i < keyframes.size()-1; i++){
			if(keyframes[i]->beat == keyframes[i+1]->beat){
				if(keyframes[i]->previousBeat < keyframes[i+1]->beat){
					keyframes[i]->beat -= 1;
				}
				else{
					keyframes[i+1]->beat +=1;
				}
			}
		}
		if(selectedKeyframes.size() > 1){
			sort(selectedKeyframes.begin(), selectedKeyframes.end(), beatkeyframesort);
		}
	}
}

void BeatKeyframes::mouseReleased(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	keysAreDraggable = false;
	if(keysDidDrag){
		//reset these caches because they may no longer be valid
		lastKeyframeIndex = 1;
		lastSampleBeat = 0;
		//TODO timeline->flagTrackModified(this);
		for(int i = 0; i < selectedKeyframes.size(); i++){
			cout << "mouseReleased: selectedkeyframe: origbeat: " <<  selectedKeyframes[i]->orig_beat << " beat:" << selectedKeyframes[i]->beat << endl;

			//ref->moveKeyframeAtBeat(selectedKeyframes[i]->beat, selectedKeyframes[i]->orig_beat, selectedKeyframes[i]->tmp_value, selectedKeyframe->orig_value);
			ref->deleteKeyframeAtBeat(selectedKeyframes[i]->orig_beat);
			ref->addKeyframeAtBeat(selectedKeyframes[i]->beat, selectedKeyframes[i]->tmp_value);

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
		//TODO timeline->flagTrackModified(this);

		// when new breakpoint is created, we should reduce next breakpoint duration, before adding
		ref->addKeyframeAtBeat(beat, selectedKeyframe->orig_value);
	}


	createNewOnMouseup = false;
}

void BeatKeyframes::setKeyframeBeat(BeatKeyframe* key, float newBeat){
	key->previousBeat = key->beat;
	key->beat = newBeat;
}

void BeatKeyframes::pasteSent(string pasteboard){
	vector<BeatKeyframe*> keyContainer;
	ofxXmlSettings pastedKeys;

	if(pastedKeys.loadFromBuffer(pasteboard)){
		createKeyframesFromXML(pastedKeys, keyContainer);
		if(keyContainer.size() != 0){
			timeline->unselectAll();
			int numKeyframesPasted = 0;
			//normalize and add at playhead
			//for(int i = 0; i < keyContainer.size(); i++)
			for(int i = keyContainer.size()-1; i >= 0; i--){
				keyContainer[i]->beat -= keyContainer[0]->beat;
				keyContainer[i]->beat += timeline->millisecToBeat(timeline->getCurrentTimeMillis());
				if(keyContainer[i]->beat <= timeline->millisecToBeat(timeline->getDurationInMilliseconds())){
					selectedKeyframes.push_back(keyContainer[i]);
					keyframes.push_back(keyContainer[i]);
					numKeyframesPasted++;
				}
				else{
					delete keyContainer[i];
				}
			}

			if(numKeyframesPasted > 0){
				updateKeyframeSort();
				//TODO timeline->flagTrackModified(this);
			}

			//			if(timeline->getMovePlayheadOnPaste())
			//				timeline->setCurrentTimeMillis( keyContainer[keyContainer.size()-1]->time );
		}
	}
}

void BeatKeyframes::addKeyframe(){
	addKeyframe(defaultValue);
}

void BeatKeyframes::addKeyframe(float value){
	//play solo change
	//addKeyframeAtBeat(value, timeline->millisecToBeat(currentTrackTime()));
	cout << "!!!!!!!!!!!!!!!!!!!! BeatKeyframes: ERROR Undefined method" << endl;
}

void BeatKeyframes::addKeyframeAtBeat(float beat){
	addKeyframeAtBeat(defaultValue, beat);
}

void BeatKeyframes::addKeyframeAtBeat(float value, float beat){
	BeatKeyframe* key = newKeyframe();
	key->time = 0;
	key->beat = key->previousBeat = beat;
	key->value = ofMap(value, valueRange.min, valueRange.max, 0, 1.0, true);
	key->orig_value = value;
	keyframes.push_back(key);
	//smart sort, only sort if not added to end
	if(keyframes.size() > 2 && keyframes[keyframes.size()-2]->beat > keyframes[keyframes.size()-1]->beat){
		updateKeyframeSort();
	}
	lastKeyframeIndex = 1;
	//TODO timeline->flagTrackModified(this);
	shouldRecomputePreviews = true;
}

void BeatKeyframes::selectAll(){
	selectedKeyframes = keyframes;
}

void BeatKeyframes::unselectAll(){
	selectedKeyframes.clear();
}

int BeatKeyframes::getSelectedItemCount(){
	return selectedKeyframes.size();
}

float BeatKeyframes::getEarliestBeat(){
	if(keyframes.size() > 0){
		return keyframes[0]->beat;
	}
	else{
		return INT_MAX;
	}
}

float BeatKeyframes::getLatestBeat(){
	if(keyframes.size() > 0){
		return keyframes[keyframes.size()-1]->beat;
	}
	else{
		return 0;
	}	
}

float BeatKeyframes::getEarliestSelectedBeat(){
	if(selectedKeyframes.size() > 0){
		return selectedKeyframes[0]->beat;
	}
	else{
		return INT_MAX;
	}
}

float BeatKeyframes::getLatestSelectedBeat(){
	if(selectedKeyframes.size() > 0){
		return selectedKeyframes[selectedKeyframes.size()-1]->beat;
	}
	else{
		return 0;
	}	
}


void BeatKeyframes::selectKeyframe(BeatKeyframe* k){
	if(!isKeyframeSelected(k)){
		selectedKeyframes.push_back(k);
	}
}

void BeatKeyframes::deselectKeyframe(BeatKeyframe* k){
	for(int i = 0; i < selectedKeyframes.size(); i++){
		if(selectedKeyframes[i] == k){
			selectedKeyframes.erase(selectedKeyframes.begin() + i);
			return;
		}
	}
}

bool BeatKeyframes::isKeyframeSelected(BeatKeyframe* k){

	if(k == NULL) return false;

	return binary_search(selectedKeyframes.begin(), selectedKeyframes.end(), k, beatkeyframesort);
	//	for(int i = 0; i < selectedKeyframes.size(); i++){
	//		if(selectedKeyframes[i] == k){
	//			return true;
	//		}
	//	}
	//	return false;
}

bool BeatKeyframes::isKeyframeIsInBounds(BeatKeyframe* key){
	if(zoomBounds.min == 0.0 && zoomBounds.max == 1.0) return true;
	unsigned long duration = timeline->getDurationInMilliseconds();
	return key->beat >= timeline->millisecToBeat(zoomBounds.min*duration) 
		&& key->beat <= timeline->millisecToBeat(zoomBounds.max*duration);
}

ofVec2f BeatKeyframes::screenPositionForKeyframe(BeatKeyframe* keyframe){
	return ofVec2f( timeline->normalizedXtoScreenX( timeline->beatToNormalizedX( keyframe->beat), zoomBounds), valueToScreenY(keyframe->value));
}

bool BeatKeyframes::screenpointIsInBounds(ofVec2f screenpoint){
	return bounds.inside(screenpoint);
}

float BeatKeyframes::screenYToValue(float screenY){
	return ofMap(screenY, bounds.y, bounds.y+bounds.height, 1.0, 0.0, true);
}

float BeatKeyframes::valueToScreenY(float value){
	return ofMap(value, 1.0, 0.0, bounds.y, bounds.y+bounds.height, true);
}

BeatKeyframe* BeatKeyframes::newKeyframe(){
	BeatKeyframe* k = new BeatKeyframe();
	return k;
}

string BeatKeyframes::getTrackType(){
	return "Beat Keyframes";
}

void BeatKeyframes::keyPressed(ofKeyEventArgs& args){
	if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE){
		cout << "BeatKeyframes::delete selected" << endl;
		deleteSelectedKeyframes();
	}
}
void BeatKeyframes::deleteSelectedKeyframes(){
	if (keyframes.size() == 2) {
		cerr << "BeatKeyframes::deleteSelectedKeyframes: can not delete keyframe because curves need minimum 2 points." << endl;
		return;
	}
	cout << "BeatKeyframes::deleteSelectedKeyframes:: keyframes size:"<< keyframes.size() << endl;

	vector<BeatKeyframe*>::iterator selectedIt = selectedKeyframes.end();
	for(int i = keyframes.size() - 1; i >= 0; i--){
		if(isKeyframeSelected(keyframes[i])){
			cout << "BeatKeyframes::delete selected i : " << i << endl;
			if(keyframes[i] != selectedKeyframes[selectedKeyframes.size()-1]){
				ofLogError("ofxTLKeyframes::deleteSelectedKeyframes") << "keyframe delete inconsistency";
			}
			willDeleteKeyframe(keyframes[i]);
			cout << "BeatKeyframes::delete selected i : " << i << " deleted"<< endl;
			delete keyframes[i];
			keyframes.erase(keyframes.begin()+i);
			selectedKeyframes.erase(--selectedIt);
		}
	}
	
	selectedKeyframes.clear();
	updateKeyframeSort();
    
    //TODO timeline->flagTrackModified(this);
}

void BeatKeyframes::deleteKeyframe(BeatKeyframe* keyframe){
	
	if(keyframe == NULL) return;
	
	for(int i = keyframes.size() - 1; i >= 0; i--){
		if(keyframe == keyframes[i]){
			deselectKeyframe(keyframe);
			willDeleteKeyframe(keyframes[i]);
			delete keyframes[i];
			keyframes.erase(keyframes.begin()+i);
			return;
		}
	}
}

