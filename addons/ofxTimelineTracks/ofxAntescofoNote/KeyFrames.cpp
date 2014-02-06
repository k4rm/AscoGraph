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

#include "Keyframes.h"
#include "ofxTimeline.h"
#include "ofxHotKeys.h"

bool keyframesort(Keyframe* a, Keyframe* b){
	return a->time < b->time;
}

Keyframes::Keyframes()
:	hoverKeyframe(NULL),
	keysAreDraggable(false),
	keysDidDrag(false),
	keysDidNudge(false),
	lastKeyframeIndex(1),
	lastSampleTime(0),
	shouldRecomputePreviews(false),
	createNewOnMouseup(false),
	useBinarySave(false),
	valueRange(ofRange(0,1.))
{
}

Keyframes::~Keyframes(){
	clear();
}

void Keyframes::recomputePreviews(){
	preview.clear();
	
//	cout << "Keyframes::recomputePreviews " << endl;
	
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

void Keyframes::draw(){
	
        //cout << "Keyframes::draw(): bw:"<< bounds.width << " bh:" << bounds.height  << endl;
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
	float currentPercent = sampleAtTime(track->currentTrackTime());
	ofFill();
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
                    string frameString = timeline->formatTime(selectedKeyframes[i]->time);
                    timeline->getFont().drawString(ofToString(keysValue, 4), screenpoint.x+5, screenpoint.y-5);
                }
                //ofCircle(screenpoint.x, screenpoint.y, 4);
                //ofCurveVertex(screenpoint.x, screenpoint.y);
                ofVertex(screenpoint.x, screenpoint.y);
            }
        }
	if (selectedKeyframes.size()) {
		ofVec2f screenpoint = screenPositionForKeyframe(selectedKeyframes[0]);
		//ofCurveVertex(screenpoint.x, screenpoint.y);
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
		ofSetColor(255, 0, 0, 255);
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
        //cout << "Keyframes::draw(): selectedKeyframes.size:"<< selectedKeyframes.size() << endl;
	for(int i = 0; i < selectedKeyframes.size(); i++){
		if(isKeyframeIsInBounds(selectedKeyframes[i])){
			ofVec2f screenpoint = screenPositionForKeyframe(selectedKeyframes[i]);
			float keysValue = ofMap(selectedKeyframes[i]->value, 0, 1.0, valueRange.min, valueRange.max, true);
			if(keysAreDraggable){
				string frameString = timeline->formatTime(selectedKeyframes[i]->time);
				timeline->getFont().drawString(ofToString(keysValue, 4), screenpoint.x+5, screenpoint.y-5);
			}
			ofSetColor(255, 0, 0, 255);
			ofCircle(screenpoint.x, screenpoint.y, 5);
                        // cout << "Keyframes::draw(): circle "<<screenpoint.x << ", "<< screenpoint.y << endl;
		}
	}

	ofPopStyle();
}

//TODO: potentially scale internal values at this point
void Keyframes::setValueRange(ofRange range, float newDefaultValue){
	valueRange = range;
    defaultValue = newDefaultValue;
}

void Keyframes::setValueRangeMin(float min){
	valueRange.min = min;
}

void Keyframes::setValueRangeMax(float max){
	valueRange.max = max;
}

void Keyframes::setDefaultValue(float newDefaultValue){
	defaultValue = newDefaultValue;
}

void Keyframes::quantizeKeys(int step){
	for(int i = 0; i < keyframes.size(); i++){
		setKeyframeTime(keyframes[i], timeline->getQuantizedTime(keyframes[i]->time, step));
	}
}

ofRange Keyframes::getValueRange(){
	return valueRange;
}

float Keyframes::getValue(){
	return getValueAtTimeInMillis(track->currentTrackTime());
}

//main function to get values out of the timeline, operates on the given value range
float Keyframes::getValueAtPercent(float percent){
	//	return ofMap(sampleAt(percent), 0.0, 1.0, valueRange.min, valueRange.max, false);
    return getValueAtTimeInMillis(percent*timeline->getDurationInMilliseconds());
}

float Keyframes::getValueAtTimeInMillis(long sampleTime){
	return ofMap(sampleAtTime(sampleTime), 0.0, 1.0, valueRange.min, valueRange.max, false);
}

float Keyframes::sampleAtPercent(float percent){
	return sampleAtTime(percent * timeline->getDurationInMilliseconds());
}

float Keyframes::sampleAtTime(long sampleTime){
	sampleTime = ofClamp(sampleTime, 0, timeline->getDurationInMilliseconds());
	
	//edge cases
	if(keyframes.size() == 0){
		return ofMap(defaultValue, valueRange.min, valueRange.max, 0, 1.0, true);
	}
	
	if(sampleTime <= keyframes[0]->time){
		return evaluateKeyframeAtTime(keyframes[0], sampleTime);
//		return keyframes[0]->value;
	}
	
	if(sampleTime >= keyframes[keyframes.size()-1]->time){
		//return keyframes[keyframes.size()-1]->value;
		return evaluateKeyframeAtTime(keyframes[keyframes.size()-1], sampleTime);
	}
	
	//optimization for linear playback
	int startKeyframeIndex = 1;
	if(sampleTime >= lastSampleTime){
		startKeyframeIndex = lastKeyframeIndex;
	}
	
	for(int i = startKeyframeIndex; i < keyframes.size(); i++){
		if(keyframes[i]->time >= sampleTime){
			lastKeyframeIndex = i;
			lastSampleTime = sampleTime;
			return interpolateValueForKeys(keyframes[i-1], keyframes[i], sampleTime);
		}
	}
	ofLog(OF_LOG_ERROR, "Keyframes --- Error condition, couldn't find keyframe for percent " + ofToString(sampleTime));
	return defaultValue;
}

float Keyframes::evaluateKeyframeAtTime(Keyframe* key, unsigned long sampleTime){
	return key->value;
}

float Keyframes::interpolateValueForKeys(Keyframe* start,Keyframe* end, unsigned long sampleTime){
	return ofMap(sampleTime, start->time, end->time, start->value, end->value);
}

void Keyframes::load(){
    clear();
}

void Keyframes::createKeyframesFromXML(ofxXmlSettings xmlStore, vector<Keyframe*>& keyContainer){
}

void Keyframes::clear(){

	for(int i = 0; i < keyframes.size(); i++){
		willDeleteKeyframe(keyframes[i]);
		delete keyframes[i];
	}
	keyframes.clear();
    selectedKeyframes.clear();
	updateKeyframeSort();
}

void Keyframes::save(){

}

string Keyframes::getXMLStringForKeyframes(vector<Keyframe*>& keys){
//	return "";
	ofxXmlSettings savedkeyframes;
	savedkeyframes.addTag("keyframes");
	savedkeyframes.pushTag("keyframes");

	for(int i = 0; i < keys.size(); i++){
		savedkeyframes.addTag("key");
		savedkeyframes.pushTag("key", i);
        
        //calling store before saving the default values gives the subclass a chance to modify them
        storeKeyframe(keys[i], savedkeyframes);
        savedkeyframes.addValue("time", ofxTimecode::timecodeForMillis(keys[i]->time));
		savedkeyframes.addValue("value", keys[i]->value);
        
		savedkeyframes.popTag(); //key
	}

	savedkeyframes.popTag();//keyframes
	string str;
	savedkeyframes.copyXmlToString(str);
	return str;
}

bool Keyframes::mousePressed(ofMouseEventArgs& args, long millis){
	cout << "Keyframes::mousePressed: error should no be here" << endl;
	return 0;
}

void Keyframes::regionSelected(ofLongRange timeRange, ofRange valueRange){
    for(int i = 0; i < keyframes.size(); i++){
        if(timeRange.contains(keyframes[i]->time) && valueRange.contains(1.-keyframes[i]->value)){
            selectKeyframe(keyframes[i]);
        }
	}
	updateKeyframeSort();
}

//update the grabTimeOffset to prepare for stretching keys
void Keyframes::updateStretchOffsets(ofVec2f screenpoint, long grabMillis){
	for(int k = 0; k < selectedKeyframes.size(); k++){
        selectedKeyframes[k]->grabTimeOffset = selectedKeyframes[k]->time - stretchAnchor;
	}
}

void Keyframes::updateDragOffsets(ofVec2f screenpoint, long grabMillis){
	for(int k = 0; k < selectedKeyframes.size(); k++){
        selectedKeyframes[k]->grabTimeOffset  = grabMillis - selectedKeyframes[k]->time;
        selectedKeyframes[k]->grabValueOffset = screenpoint.y - valueToScreenY(selectedKeyframes[k]->value);
	}
}

void Keyframes::mouseMoved(ofMouseEventArgs& args, long millis){
	//ofxTLTrack::mouseMoved(args, millis);
	hoverKeyframe = keyframeAtScreenpoint( ofVec2f(args.x, args.y));
}

void Keyframes::mouseDragged(ofMouseEventArgs& args, long millis){

	if(keysAreStretchable){
		//cast the stretch anchor to long so that it can be signed
		float stretchRatio = 1.0*(millis-long(stretchAnchor)) / (1.0*stretchSelectPoint-stretchAnchor);

        for(int k = 0; k < selectedKeyframes.size(); k++){
            setKeyframeTime(selectedKeyframes[k], ofClamp(stretchAnchor + (selectedKeyframes[k]->grabTimeOffset * stretchRatio),
														  0, timeline->getDurationInMilliseconds()));
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
            setKeyframeTime(selectedKeyframes[k], ofClamp(millis - selectedKeyframes[k]->grabTimeOffset,
														  screenXToMillis(bounds.getMinX()), screenXToMillis(bounds.getMaxX())));
            selectedKeyframes[k]->value = screenYToValue(args.y - selectedKeyframes[k]->grabValueOffset);
            selectedKeyframes[k]->screenPosition = screenPositionForKeyframe(selectedKeyframes[k]);
        }
        if(selectedKeyframe != NULL && timeline->getMovePlayheadOnDrag()){
            timeline->setCurrentTimeMillis(selectedKeyframe->time);
        }
        timeline->flagUserChangedValue();
        keysDidDrag = true;
        updateKeyframeSort();
    }
	createNewOnMouseup = false;
}

void Keyframes::updateKeyframeSort(){
	//reset these caches because they may no longer be valid
	shouldRecomputePreviews = true;
	lastKeyframeIndex = 1;
	lastSampleTime = 0;
	if(keyframes.size() > 1){

		sort(keyframes.begin(), keyframes.end(), keyframesort);
		for(int i = 0; i < keyframes.size()-1; i++){
			if(keyframes[i]->time == keyframes[i+1]->time){
				if(keyframes[i]->previousTime < keyframes[i+1]->time){
					keyframes[i]->time -= 1;
				}
				else{
					keyframes[i+1]->time+=1;
				}
			}
		}
		if(selectedKeyframes.size() > 1){
			sort(selectedKeyframes.begin(), selectedKeyframes.end(), keyframesort);
		}
	}
}

void Keyframes::mouseReleased(ofMouseEventArgs& args, long millis){
	keysAreDraggable = false;
    if(keysDidDrag){
		//reset these caches because they may no longer be valid
		lastKeyframeIndex = 1;
		lastSampleTime = 0;
        //TODO timeline->flagTrackModified(this);
    }
	
	if(createNewOnMouseup){
		//add a new one
		selectedKeyframe = newKeyframe();
		setKeyframeTime(selectedKeyframe,millis);
		selectedKeyframe->value = screenYToValue(args.y);
		keyframes.push_back(selectedKeyframe);
		selectedKeyframes.push_back(selectedKeyframe);
		updateKeyframeSort();
		//TODO timeline->flagTrackModified(this);
	}
	createNewOnMouseup = false;
}

void Keyframes::setKeyframeTime(Keyframe* key, unsigned long newTime){
	key->previousTime = key->time;
	key->time = newTime;
}

void Keyframes::getSnappingPoints(set<unsigned long>& points){
	for(int i = 0; i < keyframes.size(); i++){
		if (isKeyframeIsInBounds(keyframes[i]) && !isKeyframeSelected(keyframes[i])) {
			points.insert(keyframes[i]->time);
		}
	}
}

string Keyframes::copyRequest(){
	if(selectedKeyframes.size() > 0){
		return getXMLStringForKeyframes(selectedKeyframes);
	}
	return "";
}

string Keyframes::cutRequest(){
	if(selectedKeyframes.size() > 0){
		string xmlrep = getXMLStringForKeyframes(selectedKeyframes);
		deleteSelectedKeyframes();
		return xmlrep;
	}
	return "";
}

void Keyframes::pasteSent(string pasteboard){
	vector<Keyframe*> keyContainer;
	ofxXmlSettings pastedKeys;
	
	if(pastedKeys.loadFromBuffer(pasteboard)){
		createKeyframesFromXML(pastedKeys, keyContainer);
		if(keyContainer.size() != 0){
			timeline->unselectAll();
			int numKeyframesPasted = 0;
			//normalize and add at playhead
			//for(int i = 0; i < keyContainer.size(); i++){
			for(int i = keyContainer.size()-1; i >= 0; i--){
				keyContainer[i]->time -= keyContainer[0]->time;
				keyContainer[i]->time += timeline->getCurrentTimeMillis();
				if(keyContainer[i]->time <= timeline->getDurationInMilliseconds()){
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
			
//			if(timeline->getMovePlayheadOnPaste()){
//				timeline->setCurrentTimeMillis( keyContainer[keyContainer.size()-1]->time );
//			}
		}
	}
}

void Keyframes::addKeyframe(){
	addKeyframe(defaultValue);
}

void Keyframes::addKeyframe(float value){
	//play solo change
	addKeyframeAtMillis(value, track->currentTrackTime());
}

void Keyframes::addKeyframeAtMillis(unsigned long millis){
	addKeyframeAtMillis(defaultValue, millis);
}

void Keyframes::addKeyframeAtMillis(float value, unsigned long millis){
	Keyframe* key = newKeyframe();
	key->time = key->previousTime = millis;
	key->value = ofMap(value, valueRange.min, valueRange.max, 0, 1.0, true);
	keyframes.push_back(key);
	//smart sort, only sort if not added to end
	if(keyframes.size() > 2 && keyframes[keyframes.size()-2]->time > keyframes[keyframes.size()-1]->time){
		updateKeyframeSort();
	}
	lastKeyframeIndex = 1;
	//TODO timeline->flagTrackModified(this);
	shouldRecomputePreviews = true;
}

void Keyframes::selectAll(){
	selectedKeyframes = keyframes;
}

void Keyframes::unselectAll(){
	selectedKeyframes.clear();
}

int Keyframes::getSelectedItemCount(){
    return selectedKeyframes.size();
}

unsigned long Keyframes::getEarliestTime(){
	if(keyframes.size() > 0){
		return keyframes[0]->time;
	}
	else{
		//return ofxTLTrack::getEarliestTime();
		return track->getEarliestTime();
	}
}

unsigned long Keyframes::getLatestTime(){
	if(keyframes.size() > 0){
		return keyframes[keyframes.size()-1]->time;
	}
	else{
		//return ofxTLTrack::getLatestTime();
		return track->getLatestTime();
	}	
}

unsigned long Keyframes::getEarliestSelectedTime(){
	if(selectedKeyframes.size() > 0){
		return selectedKeyframes[0]->time;
	}
	else{
		//return ofxTLTrack::getEarliestSelectedTime();
		return track->getEarliestSelectedTime();
	}
}

unsigned long Keyframes::getLatestSelectedTime(){
	if(selectedKeyframes.size() > 0){
		return selectedKeyframes[selectedKeyframes.size()-1]->time;
	}
	else{
		return track->getLatestSelectedTime();
	}	
}

string Keyframes::getXMLRepresentation(){
    return getXMLStringForKeyframes(keyframes);
}

void Keyframes::loadFromXMLRepresentation(string rep){
    clear();
    ofxXmlSettings buffer;
    buffer.loadFromBuffer(rep);
    createKeyframesFromXML(buffer, keyframes);
    updateKeyframeSort();
    timeline->flagUserChangedValue();    //because this is only called in Undo we don't flag track modified
}

//experimental binary saving. does not work with subclasses yet
void Keyframes::saveToBinaryFile(){
}

void Keyframes::loadFromBinaryFile(){
	
}

void Keyframes::keyPressed(ofKeyEventArgs& args){
	if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE){
		cout << "Keyframes::delete selected" << endl;
		deleteSelectedKeyframes();
	}
}

void Keyframes::nudgeBy(ofVec2f nudgePercent){
	for(int i = 0; i < selectedKeyframes.size(); i++){
		setKeyframeTime(selectedKeyframes[i], ofClamp(selectedKeyframes[i]->time + timeline->getDurationInMilliseconds()*nudgePercent.x,
													  0, timeline->getDurationInMilliseconds()));
		selectedKeyframes[i]->value = ofClamp(selectedKeyframes[i]->value + nudgePercent.y, 0, 1.0);
	}	
	updateKeyframeSort();
    //TODO timeline->flagTrackModified(this);
}

void Keyframes::deleteSelectedKeyframes(){
	cout << "Keyframes::deleteSelectedKeyframes:: keyframes size:"<< keyframes.size() << endl;

	vector<Keyframe*>::iterator selectedIt = selectedKeyframes.end();
	for(int i = keyframes.size() - 1; i >= 0; i--){
		if(isKeyframeSelected(keyframes[i])){
			cout << "Keyframes::delete selected i : " << i << endl;
			if(keyframes[i] != selectedKeyframes[selectedKeyframes.size()-1]){
				ofLogError("Keyframes::deleteSelectedKeyframes") << "keyframe delete inconsistency";
			}
			willDeleteKeyframe(keyframes[i]);
			cout << "Keyframes::delete selected i : " << i << " deleted"<< endl;
			delete keyframes[i];
			keyframes.erase(keyframes.begin()+i);
			selectedKeyframes.erase(--selectedIt);
		}
	}
	
	selectedKeyframes.clear();
	updateKeyframeSort();
    
    //TODO timeline->flagTrackModified(this);
}

void Keyframes::deleteKeyframe(Keyframe* keyframe){
	
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

Keyframe* Keyframes::keyframeAtScreenpoint(ofVec2f p){
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
}

void Keyframes::selectKeyframe(Keyframe* k){
	if(!isKeyframeSelected(k)){
        selectedKeyframes.push_back(k);
    }
}

void Keyframes::deselectKeyframe(Keyframe* k){
	for(int i = 0; i < selectedKeyframes.size(); i++){
        if(selectedKeyframes[i] == k){
            selectedKeyframes.erase(selectedKeyframes.begin() + i);
			return;
        }
    }
}

bool Keyframes::isKeyframeSelected(Keyframe* k){
	
	if(k == NULL) return false;

	return binary_search(selectedKeyframes.begin(), selectedKeyframes.end(), k, keyframesort);
//	for(int i = 0; i < selectedKeyframes.size(); i++){
//		if(selectedKeyframes[i] == k){
//			return true;
//		}
//	}
//	return false;
}

bool Keyframes::isKeyframeIsInBounds(Keyframe* key){
	if(zoomBounds.min == 0.0 && zoomBounds.max == 1.0) return true;
	unsigned long duration = timeline->getDurationInMilliseconds();
	return key->time >= zoomBounds.min*duration && key->time <= zoomBounds.max*duration;
}

ofVec2f Keyframes::screenPositionForKeyframe(Keyframe* keyframe){
    return ofVec2f(millisToScreenX(keyframe->time), 
                   valueToScreenY(keyframe->value));
}

bool Keyframes::screenpointIsInBounds(ofVec2f screenpoint){
	return bounds.inside(screenpoint);
}

float Keyframes::screenYToValue(float screenY){
    return ofMap(screenY, bounds.y, bounds.y+bounds.height, 1.0, 0.0, true);
}

float Keyframes::valueToScreenY(float value){
	return ofMap(value, 1.0, 0.0, bounds.y, bounds.y+bounds.height, true);
}

Keyframe* Keyframes::newKeyframe(){
	Keyframe* k = new Keyframe();
	return k;
}

string Keyframes::getTrackType(){
    return "Keyframes";
}

/////////////////////////////

long Keyframes::screenXToMillis(float x){
	return timeline->screenXToMillis(x);
}

float Keyframes::millisToScreenX(long millis){
    return timeline->millisToScreenX(millis);
}

float Keyframes::screenXtoNormalizedX(float x){
    return timeline->screenXtoNormalizedX(x);
}

float Keyframes::normalizedXtoScreenX(float x){
    return timeline->normalizedXtoScreenX(x);
}

float Keyframes::screenXtoNormalizedX(float x, ofRange outputRange){
	return timeline->screenXtoNormalizedX(x, outputRange);
}

float Keyframes::normalizedXtoScreenX(float x, ofRange inputRange){
	return timeline->normalizedXtoScreenX(x, inputRange);
}

float Keyframes::beatToMillisec(float b) {
    return timeline->beatToMillisec(b);
}

float Keyframes::millisecToBeat(float m) {
    return timeline->millisecToBeat(m);
}

float Keyframes::beatToNormalizedX(float b) {
    return timeline->beatToNormalizedX(b);
}

float Keyframes::normalizedXToBeat(float x) {
    return timeline->normalizedXToBeat(x);
}
