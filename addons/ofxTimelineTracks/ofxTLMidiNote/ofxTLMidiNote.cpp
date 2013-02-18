/**
 * ofxTimeline
 *	
 * Copyright (c) 2011 James George
 * http://jamesgeorge.org + http://flightphase.com
 * http://github.com/obviousjim + http://github.com/flightphase 
 *
 * implementaiton by James George (@obviousjim) and Tim Gfrerer (@tgfrerer) for the 
 * Voyagers gallery National Maritime Museum 
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
 * ----------------------
 *
 * ofxTimeline 
 * Lightweight SDK for creating graphic timeline tools in openFrameworks
 */

#include "ofxTLMidiNote.h"
#include "ofxTimeline.h"
//#include "ofxTLUtils.h"

int bitmapFontSize = 8;
int guiXPadding = 20;
ofColor guiBGColor;

bool switchsort(ofxTLMidiNoteOn* a, ofxTLMidiNoteOn* b){
	return a->time.min < b->time.min;
}

ofxTLMidiNote::ofxTLMidiNote(){
	changingRangeMin = false;
	changingRangeMax = false;
	draggingSelectionRange = false;
	pointsAreDraggable = false;
	portInHover = false;
	portOutHover = false;
	inArmed = false;
	outArmed = false;
	noteRange = ofRange(0,11);
	guiBGColor = ofColor(30, 110, 110);
}

ofxTLMidiNote::~ofxTLMidiNote(){
}

void ofxTLMidiNote::setup(){
	enable();
	load();	
	
//	ofxTLRegisterPlaybackEvents(this);
	
	ofAddListener(ofEvents().update, this, &ofxTLMidiNote::update);
	
	// print input ports to console
	midiIn.listPorts(); // via instance
	
	// open port by number (you may need to change this)
	currentInPort = 1;
	midiIn.openPort(currentInPort);
	currentOutPort = 1;
	midiOut.openPort(currentOutPort);
	
	// add testApp as a listener
	midiIn.addListener(this);
	
	// print received messages to the console
	midiIn.setVerbose(false);
	
	for (int i = 0; i < 127; i++) {
		activeNotes[i] = false;
	}
}

void ofxTLMidiNote::update(ofEventArgs& args){
	float thisTimelinePoint = timeline->getPercentComplete();
	for(int i = 0; i < switches.size(); i++){
		
		// update out-points of all growing notes
		if(switches[i]->growing) {
			switches[i]->time.max = timeline->getPercentComplete();
			// delete notes which have been overwritten by current growing note
			for (int j = switches.size()-1; j >= 0; j--) {
				if(switches[j]->pitch == switches[i]->pitch && switches[j]->time.intersects(switches[i]->time) && switches[j] != switches[i]) {
					delete switches[j];
					switches.erase(switches.begin()+j);
				}
			}
		}
		
		// check for note on/off
		if(lastTimelinePoint < switches[i]->time.min && thisTimelinePoint > switches[i]->time.min && switches[i]->triggeredOn == false){
//			ofNotifyEvent(ofxTLEvents.trigger, args); //TODO: implement an event with ofxTLBangEventArgs or a simple float event
			switches[i]->triggeredOn = true;
			if(outArmed) 
				midiOut.sendNoteOn(switches[i]->channel, switches[i]->pitch, switches[i]->velocity);
		} else if(lastTimelinePoint < switches[i]->time.max && thisTimelinePoint > switches[i]->time.max && switches[i]->triggeredOff == false) {
//			ofNotifyEvent(ofxTLEvents.trigger, args); //TODO: implement an event with ofxTLBangEventArgs or a simple float event
			switches[i]->triggeredOff = true;
			if(outArmed)
				midiOut.sendNoteOff(switches[i]->channel, switches[i]->pitch, switches[i]->velocity);
		}
		
		// trigger off values and re-enable notes if we've dragged the timeline backwards
		if(lastTimelinePoint > thisTimelinePoint && thisTimelinePoint < switches[i]->time.min) {
			if(outArmed)
				midiOut.sendNoteOff(switches[i]->channel, switches[i]->pitch, switches[i]->velocity);
			switches[i]->triggeredOn = false;
			switches[i]->triggeredOff = false;
		}
	}
	
	lastTimelinePoint = thisTimelinePoint;
}



void ofxTLMidiNote::draw(){
	
	ofPushStyle();
	//**** DRAW BORDER
	ofNoFill();
	if(hover){
		ofSetColor(timeline->getColors().highlightColor);
	}
	else if(focused){
		ofSetColor(timeline->getColors().highlightColor);
	}
	else{
		ofSetColor(timeline->getColors().outlineColor);
	}	
	ofRect(bounds.x, bounds.y, bounds.width, bounds.height);
	
	//************************ Range Rows
	ofFill();
	float rowHeight = bounds.height / (noteRange.span()+1);
	
	for (int i = 0; i <= noteRange.span(); i++) {
		// alternate row colors
		if(i%2 == 1) {
			ofSetColor(255, 255, 255, 50);
		} else {
			ofSetColor(255, 255, 255, 25);
		}
		
		// highlight active rows, overriding row color
		int rowNote = noteRange.max - i;
		if (activeNotes[rowNote]) {
			ofSetColor(0, 255, 255, 100);
		}
		
		ofRect(bounds.x, bounds.y + i * rowHeight, bounds.width, rowHeight);
	}
	
	//************************ Port In GUI
	// Draw BG
	ofFill();
	ofSetColor(guiBGColor);
	drawGuiBG(portInButtonBounds);
	
	// Draw Text
	if(portInHover) {
		ofSetColor(0, 255, 255);
	} else {
		ofSetColor(255);
	}
	string portGuiString = "In: " + midiIn.getName();
	drawLabel(portGuiString, portInButtonBounds);
	//************************
	
	//************************ Arm In GUI
	if(inArmed) {
		ofSetColor(255,0,0);
	} else {
		ofSetColor(guiBGColor);
	}
	// draw state-colored BG
	drawGuiBG(armInButtonBounds);
	// draw text
	ofSetColor(255);
	drawLabel("Arm", armInButtonBounds);
	//************************
	
	//************************ Range GUI
	// draw label
	ofSetColor(255);
	drawLabel("Range:", rangeLabelBounds);
	
	// draw min bg
	ofSetColor(guiBGColor);
	drawGuiBG(rangeMinSliderBounds);
	// draw min text
	ofSetColor(255);
	drawLabel(ofToString(noteRange.min), rangeMinSliderBounds);
	
	// draw max bg
	ofSetColor(guiBGColor);
	drawGuiBG(rangeMaxSliderBounds);
	// draw max text
	ofSetColor(255);
	drawLabel(ofToString(noteRange.max), rangeMaxSliderBounds);
	
	ofSetColor(guiBGColor);
	drawGuiBG(rangeTrimButtonBounds);
	ofSetColor(255);
	drawLabel("Trim", rangeTrimButtonBounds);
	//************************
	
	//************************ Port Out GUI
	
	// Draw BG
	ofFill();
	ofSetColor(guiBGColor);
	drawGuiBG(portOutButtonBounds);
	
	// Draw Text
	if(portOutHover) {
		ofSetColor(0, 255, 255);
	} else {
		ofSetColor(255);
	}
	drawLabel("Out: " + midiOut.getName(), portOutButtonBounds);
	//************************
	
	//************************ Arm In GUI
	if(outArmed) {
		ofSetColor(255,0,0);
	} else {
		ofSetColor(guiBGColor);
	}
	// draw state-colored BG
	drawGuiBG(armOutButtonBounds);
	// draw text
	ofSetColor(255);
	drawLabel("Arm", armOutButtonBounds);
	//************************
	
	//************************ Note Drawing
	if(switches.size() > 0) {
		for(int i = 0; i < switches.size(); i++){
			if(isSwitchInBounds(switches[i])){
				float startX = normalizedXtoScreenX(switches[i]->time.min, zoomBounds);
				float endX = normalizedXtoScreenX( switches[i]->time.max, zoomBounds );
				ofSetColor(0, 220, 255);
				// set color to transparent white when selected
				if(switches[i]->startSelected && switches[i]->endSelected) ofSetColor(255, 255, 255, 190);
				int whichRow = ofMap(switches[i]->pitch, noteRange.max, noteRange.min, 0, noteRange.span());
				ofRectangle noteBounds = ofRectangle(startX, bounds.y + whichRow * rowHeight, endX - startX, rowHeight);
				ofRect(noteBounds);
				int hoverHintWidth = 3;
				if(switches[i]->endHovered) {
					ofSetLineWidth(hoverHintWidth);
					ofSetColor(255);
					ofLine(noteBounds.x + noteBounds.width, noteBounds.y, noteBounds.x + noteBounds.width, noteBounds.y + rowHeight);
				}
				if(switches[i]->startHovered) {
					ofSetLineWidth(hoverHintWidth);
					ofSetColor(255);
					ofLine(noteBounds.x, noteBounds.y, noteBounds.x, noteBounds.y + rowHeight);
				}
			}
		} // end switch loop
	} // end if has switches
	//************************
	
	//************************ Note Number Guides
	ofSetColor(255, 255, 255, 255);
	for (int i = 0; i <= noteRange.span(); i++) {
		string noteNumber = ofToString(ofMap(i, 0, noteRange.span(), noteRange.max, noteRange.min));
		ofDrawBitmapString(noteNumber, bitmapFontSize, bounds.y + i * rowHeight + rowHeight / 2.0f + 4);
	}
	//************************
		
	//************************ Drag Selection
	ofSetLineWidth(2.0);
	if(draggingSelectionRange){
		ofSetColor(timeline->getColors().keyColor);	
		ofLine(dragSelection.min, bounds.y, dragSelection.min, bounds.y+bounds.height);
		ofLine(dragSelection.max, bounds.y, dragSelection.max, bounds.y+bounds.height);
		ofSetColor(timeline->getColors().keyColor, 30);
		ofFill();
		ofRect(dragSelection.min, bounds.y, dragSelection.span(), bounds.height);
	}
	//************************
	
	ofPopStyle();
}

void ofxTLMidiNote::drawGuiBG(ofRectangle rect) {
	rect.x += -0.25 * guiXPadding;
	rect.width += guiXPadding * 0.5;
	ofRect(rect);
}

void ofxTLMidiNote::drawLabel(string caption, ofRectangle bounds){
	ofDrawBitmapString(caption, bounds.x, bounds.y + 10);
}

void ofxTLMidiNote::setNoteRange(ofRange range) {
	noteRange = range;
}

int ofxTLMidiNote::pitchForScreenY(int y) {
	float normalizedY = (y - bounds.y) / bounds.height;
	int pitch = ofClamp(ofMap(normalizedY, 1, 0, noteRange.min, noteRange.max + 1), noteRange.min, noteRange.max); // clamp to range
	return pitch;
}

void ofxTLMidiNote::mousePressed(ofMouseEventArgs& args){
	ofVec2f screenpoint(args.x,args.y);
	
	updateDragOffsets(args.x);
	pointsAreDraggable = !ofGetModifierKeyShift();
	
	//************************ Port In Name Click
	bool clickedPortIn = portInButtonBounds.inside(screenpoint);
	if(clickedPortIn) {
		midiIn.closePort();
		int newPort = currentInPort+ 1;
		// roll over port list if needed
		if (newPort >= midiIn.getNumPorts()) newPort = 0;
		currentInPort = newPort;
		midiIn.openPort(currentInPort);
		drawRectChanged();
		return;
	}
	//************************
	
	//************************ Port In Name Click
	bool clickedPortOut = portOutButtonBounds.inside(screenpoint);
	if(clickedPortOut) {
		midiOut.closePort();
		int newPort = currentOutPort+ 1;
		// roll over port list if needed
		if (newPort >= midiOut.getNumPorts()) newPort = 0;
		currentOutPort = newPort;
		midiOut.openPort(currentOutPort);
		drawRectChanged();
		return;
	}
	//************************
	
	//************************ Arm In Button Click
	bool clickedArmIn = armInButtonBounds.inside(screenpoint);
	if(clickedArmIn) {
		inArmed = !inArmed;
		if(!inArmed) endActiveNotes(); // rare case where user holds down a note and turns off arming
		return;
	}
	//************************
	
	//************************ Arm Out Button Click
	bool clickedArmOut = armOutButtonBounds.inside(screenpoint);
	if(clickedArmOut) {
		outArmed = !outArmed;
		return;
	}
	//************************
	
	//************************ Range Click
	changingRangeMin = rangeMinSliderBounds.inside(screenpoint);
	changingRangeMax = rangeMaxSliderBounds.inside(screenpoint);
	if(changingRangeMin || changingRangeMax) changingRangeAnchor = args.y;
	if(rangeTrimButtonBounds.inside(screenpoint)){
		trimRange();
		return;
	}
	//************************
	
	
	//************************ Track Focusing
	bool clickInRect = bounds.inside(screenpoint);
	if(clickInRect){
		if(!focused){
			focused = true;
//			return;
		}
	}
	
	if(!clickInRect){
		if(!ofGetModifierKeyShift()){
			//unselectAll();
			focused = false;
		}
		return;
	}
	//************************
	bool shouldDeselect = false;
	bool didSelectedStartTime;
	//at this point we are inside the rect and focused, make changes
	ofxTLMidiNoteOn* clickedSwitchA = NULL;
	ofxTLMidiNoteOn* clickedSwitchB = NULL;
	clickedSwitchA = switchHandleForScreenPoint(ofPoint(args.x, args.y), didSelectedStartTime);
	if(clickedSwitchA != NULL){
		if(!ofGetModifierKeyShift()){
			timeline->unselectAll();
		}
		bool startAlreadySelected = clickedSwitchA->startSelected;
		bool endAlreadySelected = clickedSwitchA->endSelected;
		clickedSwitchA->startSelected = didSelectedStartTime || (ofGetModifierKeyShift() && startAlreadySelected);
		clickedSwitchA->endSelected   = !didSelectedStartTime || (ofGetModifierKeyShift() && endAlreadySelected);
		if(didSelectedStartTime){
			timeline->setDragAnchor( clickedSwitchA->dragOffsets.min );
		}
		else{
			timeline->setDragAnchor( clickedSwitchA->dragOffsets.max );
		}
	} else {
		if(ofGetModifierKeyShift()){
			draggingSelectionRange = true;
			selectionRangeAnchor = args.x;
			dragSelection.min = dragSelection.max = selectionRangeAnchor;
			return;
		}
		
		float normalizedCoord = screenXtoNormalizedX(args.x, zoomBounds);
		bool shouldCreateNewSwitch = false;
		clickedSwitchA = switchForScreenXY(args.x, args.y);
		if(clickedSwitchA != NULL){
			//if we haven't already selected these, flag deselect
			if((!clickedSwitchA->startSelected || !clickedSwitchA->endSelected) && !ofGetModifierKeyShift()){
				timeline->unselectAll();
			}
			clickedSwitchA->startSelected = true;
			clickedSwitchA->endSelected   = true;

		} else {
				shouldCreateNewSwitch = true;
		}
		
		//don't create new switches when shift is held down, and don't deselect anything
		if(ofGetModifierKeyShift()){
			shouldCreateNewSwitch = false;
			shouldDeselect = false;
		}
		
		//if we clicked where to create a new switch, but still have a selection, first deselect
		if(shouldCreateNewSwitch && howManySwitchesAreSelected() > 0){
			shouldCreateNewSwitch = false;
			timeline->unselectAll();
		}

		if(shouldCreateNewSwitch){
			ofxTLMidiNoteOn* newNote = new ofxTLMidiNoteOn();
			int pitch = pitchForScreenY(args.y);
			int velocity = 50;
			float startTime = screenXtoNormalizedX(args.x, zoomBounds);
			addNote(startTime, pitch, velocity);
			updateDragOffsets(args.x); //TODO: what does this do? Do we still need it?
		}
	}
}

void ofxTLMidiNote::mouseMoved(ofMouseEventArgs& args){
	hoverSwitch = switchHandleForScreenPoint(ofPoint(args.x, args.y), hoveringStartTime);
	if(hoverSwitch == NULL){
		hoverSwitch = switchForScreenXY(args.x, args.y);
		hoveringHandle = false;
		for (int i = 0; i < switches.size(); i++) {
			switches[i]->startHovered = false;
			switches[i]->endHovered = false;
		}
	}
	else{
		hoveringHandle = true;
		hoverSwitch->startHovered = hoveringStartTime;
		hoverSwitch->endHovered = !hoveringStartTime;
	}
	
	portInHover = portInButtonBounds.inside(args.x, args.y);
	portOutHover = portOutButtonBounds.inside(args.x, args.y);
}

//TODO: account for snapping
void ofxTLMidiNote::mouseDragged(ofMouseEventArgs& args, bool snapped){
	bool oneSwitchSelected = howManySwitchesAreSelected() == 1;
	
	if(draggingSelectionRange){
		dragSelection.min = MIN(args.x, selectionRangeAnchor);
		dragSelection.max = MAX(args.x, selectionRangeAnchor);
	}
	
	else if(pointsAreDraggable){
	
		for(int i = 0; i < switches.size(); i++){
			if(switches[i]->startSelected){
				switches[i]->time.min = MIN( screenXtoNormalizedX(args.x - switches[i]->dragOffsets.min, zoomBounds), switches[i]->time.max);
			}
			if(switches[i]->endSelected){
				switches[i]->time.max = MAX( screenXtoNormalizedX(args.x - switches[i]->dragOffsets.max, zoomBounds), switches[i]->time.min);
			}
			// allow pitch changes if we're dragging the whole note
			if(switches[i]->startSelected && switches[i]->endSelected && oneSwitchSelected) {
				switches[i]->pitch = pitchForScreenY(args.y);
			}
		}
		
		bool didSelectedStartTime;
		ofxTLMidiNoteOn* switchHandle = switchHandleForScreenPoint(ofPoint(args.x, args.y), didSelectedStartTime);
		if(timeline->getMovePlayheadOnPaste()){
			if(switchHandle != NULL){
				timeline->setPercentComplete(didSelectedStartTime ? switchHandle->time.min : switchHandle->time.max);
			}
			else{
				timeline->setPercentComplete(screenXtoNormalizedX(args.x, zoomBounds));
			}
		}
	}
	
	// Drag Note Range
	if(changingRangeMin) {
		noteRange.min += floor((changingRangeAnchor - args.y) * 0.1f);
		noteRange.min = ofClamp(noteRange.min, 0, noteRange.max);
		drawRectChanged();
	} else if(changingRangeMax) {
		noteRange.max += floor((changingRangeAnchor - args.y) * 0.1f);
		noteRange.max = ofClamp(noteRange.max, noteRange.min, 126);
		drawRectChanged();
	}
}

void ofxTLMidiNote::mouseReleased(ofMouseEventArgs& args){
	// don't look for changing ranges after mouse button is released
	changingRangeMin = false;
	changingRangeMax = false;
	
	if(draggingSelectionRange){
		for(int i = 0; i < switches.size(); i++){
			if(dragSelection.contains( normalizedXtoScreenX(switches[i]->time.min, zoomBounds))){
				switches[i]->startSelected = true;				
			}
			if(dragSelection.contains( normalizedXtoScreenX(switches[i]->time.max, zoomBounds))){
				switches[i]->endSelected = true;
			}
		}
	}
	
	
	// If we've dragged a switch such that it overlaps other notes, delete the other notes
	for (int i = 0; i < switches.size(); i++) {
		if(switches[i]->endSelected || switches[i]->startSelected) {
			for (int j = switches.size()-1; j >= 0; j--) {
				if(switches[j]->pitch == switches[i]->pitch && switches[j]->time.intersects(switches[i]->time) && switches[j] != switches[i]) {
					// we have overlapping switches - delete the old one if we overlap the start, or trim it if we overlap the end
					if(switches[i]->time.contains(switches[j]->time.min)) {
						delete switches[j];
						switches.erase(switches.begin()+j);
					} else {
						switches[j]->time.max = switches[i]->time.min - 0.001;
					}
				}
			}
		}
	}
	
	
	for(int i = switches.size()-1; i >= 0; i--){
		// deselect all switches if we're not dragging and we haven't released over a note
		if(!draggingSelectionRange && !switches[i]->time.contains(screenXtoNormalizedX(args.x))){
			switches[i]->startSelected = false;
			switches[i]->endSelected = false;
		}
		// delete all switches that have been collapsed to nothing
		if(switches[i]->time.min == switches[i]->time.max){
			delete switches[i];
			switches.erase(switches.begin()+i);
		}
	}	
	draggingSelectionRange = false;
//	if(autosave) save();
}

void ofxTLMidiNote::keyPressed(ofKeyEventArgs& args){
	
	if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE){
		
		//if an 'on' is selected, delete it
		for(int i = switches.size()-1; i >= 0; i--){
			if(switches[i]->startSelected && switches[i]->endSelected){
				delete switches[i];
				switches.erase(switches.begin()+i);
			}
		}
		//if an off valley is selected, merge the adjascent switches
		for(int i = switches.size()-1; i >= 1; i--){
			if(switches[i-1]->endSelected && switches[i]->startSelected){
				//merge
				switches[i-1]->time.max = switches[i]->time.max;
				delete switches[i];
				switches.erase(switches.begin()+i);
			}
		}
//		if(autosave) save();
	}
}

void ofxTLMidiNote::nudgeBy(ofVec2f nudgePercent){
	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->startSelected){
			switches[i]->time.min += nudgePercent.x; 
		}
		if(switches[i]->endSelected){
			switches[i]->time.max += nudgePercent.x; 
		}		
	}
	
//	if(autosave) save();
}

// TODO: return notes in all rows
bool ofxTLMidiNote::isOn(float percent){
//	return switchForPoint(percent) != NULL;
}

ofxTLMidiNoteOn* ofxTLMidiNote::switchForScreenXY(float screenPos, int y){
	return switchForPoint(screenXtoNormalizedX(screenPos, zoomBounds), y);
}

ofxTLMidiNoteOn* ofxTLMidiNote::switchForPoint(float percent, int y){
	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->time.contains(percent) && switches[i]->pitch == pitchForScreenY(y)){
			return switches[i];
		}
	}
	return NULL;	
}

void ofxTLMidiNote::save(){
	ofxXmlSettings settings;
	string xmlRep = getXMLStringForSwitches(false);
	settings.loadFromBuffer(xmlRep);
	settings.saveFile(xmlFileName);
}

void ofxTLMidiNote::load(){
	clear();
	setXMLFileName(name + ".xml");
	ofxXmlSettings settings;
	if(settings.loadFile(xmlFileName)){
		switches = switchesFromXML(settings);
	}
	else{
		ofLogError("ofxTLMidiNote -- Error loading from xml file " + xmlFileName);
	}
}

string ofxTLMidiNote::getXMLStringForSwitches(bool selectedOnly){
	ofxXmlSettings settings;
	settings.addTag("notes");
	settings.pushTag("notes");
	int notesAdded = 0;
	for(int i = 0; i < switches.size(); i++){
		if(!selectedOnly || (switches[i]->startSelected && switches[i]->endSelected)){
			settings.addTag("note");
			settings.pushTag("note",notesAdded);
			settings.addValue("startTime", switches[i]->time.min);
			settings.addValue("endTime", switches[i]->time.max);
			settings.addValue("pitch", switches[i]->pitch);
			settings.addValue("velocity", switches[i]->velocity);
			settings.addValue("channel", switches[i]->channel);
			settings.popTag();
			notesAdded++;
		}
	}
	settings.popTag();

	string ret;
	settings.copyXmlToString(ret);
//	cout << " xml request rep " << ret << endl;
	return ret;
}

vector<ofxTLMidiNoteOn*> ofxTLMidiNote::switchesFromXML(ofxXmlSettings xmlStore){
	vector<ofxTLMidiNoteOn*> newSwitches;

	int numSwitchStores = xmlStore.getNumTags("notes");
	for(int s = 0; s < numSwitchStores; s++){
		xmlStore.pushTag("notes", s);
		int numSwitches = xmlStore.getNumTags("note");

		for(int i = 0; i < numSwitches; i++){
			ofxTLMidiNoteOn* newSwitch = new ofxTLMidiNoteOn();
			newSwitch->startSelected = newSwitch->endSelected = false;
			xmlStore.pushTag("note", i);
			newSwitch->time.min = xmlStore.getValue("startTime",0.0);
			newSwitch->time.max = xmlStore.getValue("endTime",0.0);
			newSwitch->pitch = xmlStore.getValue("pitch",0);
			newSwitch->velocity = xmlStore.getValue("velocity",0);
			newSwitch->channel = xmlStore.getValue("channel",1);
			newSwitches.push_back(newSwitch);
			xmlStore.popTag();
		}
		xmlStore.popTag();
	}
	
//	cout << "loaded " << newSwitches.size() << endl;
	
	sort(newSwitches.begin(), newSwitches.end(), switchsort);
	return newSwitches;
}

string ofxTLMidiNote::copyRequest(){
//	cout << "copy request" << endl;
	return getXMLStringForSwitches(true);
}

string ofxTLMidiNote::cutRequest(){
	string switchString = getXMLStringForSwitches(true);
	
	//if an 'on' is selected, delete it
	for(int i = switches.size()-1; i >= 0; i--){
		if(switches[i]->startSelected && switches[i]->endSelected){
			delete switches[i];
			switches.erase(switches.begin()+i);
		}
	}	
	
//	if(autosave) save();
	
	return switchString;
}

void ofxTLMidiNote::pasteSent(string pasteboard){
//	cout << "pasting " << pasteboard << endl;
	ofxXmlSettings pasted;
	pasted.loadFromBuffer(pasteboard);
	vector<ofxTLMidiNoteOn*> newSwitches = switchesFromXML(pasted);
	if(newSwitches.size() == 0){
		return;
	}
	
	timeline->unselectAll();
	//move switches relative to the playhead
	float playheadPercent = timeline->getPercentComplete();
	float basePercent = newSwitches[0]->time.min;
	for(int i = 0; i < newSwitches.size(); i++){
//		cout << "pasted time is " << newSwitches[i]->time << endl;
		newSwitches[i]->time += playheadPercent - basePercent;
//		cout << "repositioned time " << newSwitches[i]->time << endl;
		newSwitches[i]->time.clamp(ofRange(0,1.0)); 
//		cout << "clamped time " << newSwitches[i]->time << endl;
	}
	
	//validate switch as not overlapping with any other switches
	float playheadJumpPoint = -1;
	for(int i = 0; i < newSwitches.size(); i++){
		for(int s = 0; s < switches.size(); s++){
			if(newSwitches[i]->time.intersects(switches[s]->time)){
				delete newSwitches[i];
				newSwitches[i] = NULL;
				break;
			}
		}
		if(newSwitches[i] != NULL){
			switches.push_back(newSwitches[i]);
			playheadJumpPoint = newSwitches[i]->time.max; 
		}
		
	}
	if(playheadJumpPoint != -1 && timeline->getMovePlayheadOnPaste()){
		timeline->setPercentComplete( playheadJumpPoint );
	}
	
	sort(switches.begin(), switches.end(), switchsort);
//	if(autosave) save();
}

void ofxTLMidiNote::selectAll(){
	for(int i = 0; i < switches.size(); i++){
		switches[i]->startSelected = switches[i]->endSelected = true;
	}
}

void ofxTLMidiNote::clear(){
	for(int i = 0; i < switches.size(); i++){
		delete switches[i];
	}
	switches.clear();
}

void ofxTLMidiNote::playbackStarted(ofxTLPlaybackEventArgs& args){
	lastTimelinePoint = timeline->getPercentComplete();
}
void ofxTLMidiNote::playbackLooped(ofxTLPlaybackEventArgs& args){
	for(int i = 0; i < switches.size(); i++){
		// stop all notes at end of timeline
		if(switches[i]->growing){
			switches[i]->growing = false;
			switches[i]->time.max = 1.0;
		}
		// reset trigger states to allow retriggers
		switches[i]->triggeredOn = false;
		switches[i]->triggeredOff = false;
	}
}
void ofxTLMidiNote::playbackEnded(ofxTLPlaybackEventArgs& args){
	endActiveNotes();
}

void ofxTLMidiNote::endActiveNotes() {
	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->growing) {
			switches[i]->growing = false;
			switches[i]->time.max = timeline->getPercentComplete();
		}
	}
}

bool ofxTLMidiNote::isSwitchInBounds(ofxTLMidiNoteOn* s){
	return zoomBounds.intersects(s->time);
}
		
void ofxTLMidiNote::updateDragOffsets(float clickX){
	for(int i = 0; i < switches.size(); i++){
		switches[i]->dragOffsets = ofRange(clickX - normalizedXtoScreenX(switches[i]->time.min, zoomBounds),
										   clickX - normalizedXtoScreenX(switches[i]->time.max, zoomBounds));										   
	}
}

int ofxTLMidiNote::howManySwitchesAreSelected(){
	int numSelected = 0;
	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->startSelected || switches[i]->endSelected){
			numSelected++;
		}
	}
	return numSelected;
}

void ofxTLMidiNote::unselectAll(){
//	if(ofGetModifierKeyShift()){
//		return;
//	}
	
	for(int i = 0; i < switches.size(); i++){
		switches[i]->startSelected = false;
		switches[i]->endSelected = false;
	}
}

ofxTLMidiNoteOn* ofxTLMidiNote::switchHandleForScreenPoint(ofPoint screenPos, bool& startTimeSelected){
	for(int i = 0; i < switches.size(); i++){
		float xPos = normalizedXtoScreenX(switches[i]->time.min, zoomBounds);
		if(abs(xPos - screenPos.x) < 7 && switches[i]->pitch == pitchForScreenY(screenPos.y)){
			startTimeSelected = true;
			return switches[i];
		}
		xPos = normalizedXtoScreenX(switches[i]->time.max, zoomBounds);
		if(abs(xPos - screenPos.x) < 7 && switches[i]->pitch == pitchForScreenY(screenPos.y)){
			startTimeSelected = false;
			return switches[i];
		}
	}
	return NULL;
}


//Finds the closest switch behind a given point
//if the point is inside a switch, it will return NULL.
ofxTLMidiNoteOn* ofxTLMidiNote::nearestGrowingNoteBeforePointWithPitch(float percent, int pitch){
	float nearest = 1.0;
	ofxTLMidiNoteOn* nearestSwitch = NULL;
	for(int i = 0; i < switches.size(); i++){
//		if (switches[i]->time.max > percent) {
//			break;
//		}
		if(percent - switches[i]->time.min < nearest && switches[i]->pitch == pitch && switches[i]->growing){
			nearest = percent - switches[i]->time.min;
			nearestSwitch = switches[i];
		}
	}
	return nearestSwitch;
}

bool ofxTLMidiNote::hoveringOn(float hoverY){
	return hoverY < bounds.y+bounds.height/2;
}

void ofxTLMidiNote::newMidiMessage(ofxMidiMessage& msg) {
	/*
	 Note On: Highlight row, Add note on if armed
	 Note Off: Set end of active note
	 */
	
	switch (msg.status) {
		case MIDI_NOTE_ON:
			//////// Set Row Highlight
			activeNotes[msg.pitch] = true;
			
			//////// Add Note
			if(inArmed) {
				addNote(timeline->getPercentComplete(), msg.pitch, msg.velocity, true, msg.channel);
				trimRange();
			}
			break;
			
		case MIDI_NOTE_OFF:
		{
			//////// Set Highlight
			activeNotes[msg.pitch] = false;
			
			//////// Set Note Off Point
			// Find closest note back
			ofxTLMidiNoteOn* activeNote = nearestGrowingNoteBeforePointWithPitch(timeline->getPercentComplete(), msg.pitch);
			if(activeNote) {
				if(activeNote->growing) {
					activeNote->growing = false;
					activeNote->time.max = timeline->getPercentComplete();
				}
			}
			break;
		}	
		default:
			break;
	}
//	printf("heard p:%i v:%i val:%i\n", msg.pitch, msg.velocity, msg.control);
}

void ofxTLMidiNote::addNote(float time, int pitch, int velocity, bool growing, int channel){
	ofxTLMidiNoteOn* newNote = new ofxTLMidiNoteOn();
	newNote->pitch = pitch;
	newNote->velocity = velocity;
	newNote->time.min = time;
	newNote->time.max = time + 0.001;
	if(!growing) {
		double oneMeasure = 60 / timeline->getBPM();
		double halfMeasure = oneMeasure/2;
		double quarterMeasure = halfMeasure/2;
		newNote->time.max = time + ofMap(quarterMeasure, 0, timeline->getDurationInSeconds(), 0, 1);
	}
	newNote->growing = growing;
	newNote->channel = channel;
	newNote->startHovered = false;
	newNote->endHovered = false;
	newNote->triggeredOn = false;
	newNote->triggeredOff = false;
	switches.push_back(newNote);
	sort(switches.begin(), switches.end(), switchsort);
//	if(autosave) save();
	drawRectChanged();
}

void ofxTLMidiNote::trimRange() {
	if(switches.size() > 0) {
		ofRange newRange = ofRange(switches[0]->pitch, switches[0]->pitch);
		for (int i = 0; i < switches.size(); i++) {
			if (switches[i]->pitch > newRange.max) {
				newRange.max = switches[i]->pitch;
			} else if(switches[i]->pitch < newRange.min) {
				newRange.min = switches[i]->pitch;
			}
		}
		setNoteRange(newRange);
	}
}

void ofxTLMidiNote::drawRectChanged(){
	guiHeaderHeight = 11;
	ofRectangle startRect = ofRectangle(bounds.x + 100 - guiXPadding, 0, 0, guiHeaderHeight);
	portInButtonBounds = getBoundsEastOf(startRect, "In: " + midiIn.getName());
	armInButtonBounds = getBoundsEastOf(portInButtonBounds, "Arm");
	rangeLabelBounds = getBoundsEastOf(armInButtonBounds, "Range");
	rangeMinSliderBounds = getBoundsEastOf(rangeLabelBounds, ofToString(noteRange.min));
	rangeMaxSliderBounds = getBoundsEastOf(rangeMinSliderBounds, ofToString(noteRange.max));
	rangeTrimButtonBounds = getBoundsEastOf(rangeMaxSliderBounds, "Trim");
	portOutButtonBounds = getBoundsEastOf(rangeTrimButtonBounds, "Out: " + midiOut.getName());
	armOutButtonBounds = getBoundsEastOf(portOutButtonBounds, "Arm");
}

ofRectangle ofxTLMidiNote::getBoundsEastOf(ofRectangle anchor, string label){
	return ofRectangle(anchor.x + anchor.width + guiXPadding, bounds.y - guiHeaderHeight, label.size() * bitmapFontSize, guiHeaderHeight);
}