/**
 * Copyright (c) 2012-2013 Thomas Coffy - thomas.coffy@ircam.fr
 * http://repmus.ircam.fr/mutant/ascograph
 *
 * based on ofxTimeline
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
#include "ofxTimeline.h"

Curves::Curves(){
	initializeEasings();
	valueRange = ofRange(0.0, 1.0);
	drawingEasingWindow = false;
}

float Curves::interpolateValueForKeys(Keyframe* start,Keyframe* end, unsigned long sampleTime){
	cout << "Curves:: interpolateValueForKeys: " << sampleTime << endl;
	TweenKeyframe* tweenKeyStart = (TweenKeyframe*)start;
	TweenKeyframe* tweenKeyEnd = (TweenKeyframe*)end;
	return ofxTween::map(sampleTime, tweenKeyStart->time, tweenKeyEnd->time,
						 			 tweenKeyStart->value, tweenKeyEnd->value,
						 false, *tweenKeyStart->easeFunc->easing, tweenKeyStart->easeType->type);
}

string Curves::getTrackType(){
	return "Curves";    
}

Keyframe* Curves::newKeyframe(){
	TweenKeyframe* k = new TweenKeyframe();
	k->easeFunc = easingFunctions[0];
	k->easeType = easingTypes[0];
	return k;
}


#if 0
//easetype: ofSetColor(150, 100, 10);
void Curves::setColor_EasetypeSel(ofColor &col) {
    mColor_easeTypeSel = col;
}
 //ofSetColor(80, 80, 80);
void Curves::setColor_EasetypeDefault(ofColor &col) {
    mColor_easeTypeDefault = col;
}
#endif


void Curves::drawModalContent(){
	
	//****** DRAW EASING CONTROLS
	if(!drawingEasingWindow){
    	return;
    }
	
    TweenKeyframe* tweenFrame = (TweenKeyframe*) selectedKeyframe;
	if(tweenFrame == NULL){
		if(selectedKeyframes.size() == 0){
			return;
		}
		tweenFrame = (TweenKeyframe*)selectedKeyframes[0];
	}
	
	for(int i = 0; i < easingTypes.size(); i++){
        //TODO turn into something like selectionContainsEaseType();
        //so that we can show the multi-selected easies
        if(easingTypes[i] ==  ((ofxTLTweenKeyframe*)selectedKeyframes[0])->easeType){
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

bool Curves::mousePressed(ofMouseEventArgs& args, long millis){
	if(drawingEasingWindow){
		return true;
	}
	else {
		return Keyframes::mousePressed(args,millis);
	}
}

void Curves::mouseDragged(ofMouseEventArgs& args, long millis){
	if(!drawingEasingWindow){
        Keyframes::mouseDragged(args, millis);
    }
}

void Curves::mouseReleased(ofMouseEventArgs& args, long millis){
	if(drawingEasingWindow && args.button == 0){
		drawingEasingWindow = false;
		timeline->dismissedModalContent();
		ofVec2f screenpoint(args.x,args.y);
		for(int i = 0; i < easingFunctions.size(); i++){
			if(easingFunctions[i]->bounds.inside(screenpoint-easingWindowPosition)){
				for(int k = 0; k < selectedKeyframes.size(); k++){
					((ofxTLTweenKeyframe*)selectedKeyframes[k])->easeFunc = easingFunctions[i];
				}
				//TODO timeline->flagTrackModified(this);
				shouldRecomputePreviews = true;
				return;
			}
		}
		
		for(int i = 0; i < easingTypes.size(); i++){
			if(easingTypes[i]->bounds.inside(screenpoint-easingWindowPosition)){
				for(int k = 0; k < selectedKeyframes.size(); k++){
					((ofxTLTweenKeyframe*)selectedKeyframes[k])->easeType = easingTypes[i];
				}
				//TODO timeline->flagTrackModified(this);
				shouldRecomputePreviews = true;
				return;
			}
		}
	}
	else{
		Keyframes::mouseReleased(args, millis);
	}
}

void Curves::selectedKeySecondaryClick(ofMouseEventArgs& args){
    easingWindowPosition = ofVec2f(MIN(args.x, bounds.width - easingBoxWidth),
                                   MIN(args.y, timeline->getBottomLeft().y - (tweenBoxHeight*easingFunctions.size())));
    
    drawingEasingWindow = true;
    //timeline->presentedModalContent(track);
}

void Curves::restoreKeyframe(Keyframe* key, ofxXmlSettings& xmlStore){
    TweenKeyframe* tweenKey =  (TweenKeyframe*)key;    
    tweenKey->easeFunc = easingFunctions[ofClamp(xmlStore.getValue("easefunc", 0), 0, easingFunctions.size()-1)];
    tweenKey->easeType = easingTypes[ofClamp(xmlStore.getValue("easetype", 0), 0, easingTypes.size()-1)];
}

void Curves::storeKeyframe(Keyframe* key, ofxXmlSettings& xmlStore){
    ofxTLTweenKeyframe* tweenKey =  (ofxTLTweenKeyframe*)key;
    xmlStore.addValue("easefunc", tweenKey->easeFunc->id);
    xmlStore.addValue("easetype", tweenKey->easeType->id);
}

void Curves::initializeEasings(){
    
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

