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

#include <iostream>
#include <string>

#include "ofMain.h"
#include "ofxTLAntescofoNote.h"
#include <ofxAntescofog.h>
#include "ofxModifierKeys.h"

#define RT_TEMPO_VAR_OK

#include <Action.h>
#include <FakeRunTime.h>
#include <pre_antescofo.h>
#include <parsedriver.h>
#include <Score.h>
#include <location.hh>
#include <position.hh>

#define ofGetModifierKeyShift()   ofGetModifierPressed(OF_KEY_SHIFT)

int bitmapFontSize = 8;
int guiXPadding = 20;

extern ofxConsole* console;

static string str_error; // filled by our error()
void pre_antescofo::error(const char *fmt,...)
{
	va_list ap;
	va_start(ap, fmt);

	char buf[1024];
	vsnprintf(buf,sizeof buf,fmt, ap);
	buf[sizeof buf-1] = 0; // in case of full buffer
	cerr << "---------------- PARSING ERROR ----------------" << endl;
	cerr << buf << endl;
	cerr.flush();
	str_error += buf;
	str_error += "\n";
	va_end(ap);
}



bool switchsort(ofxTLAntescofoNoteOn* a, ofxTLAntescofoNoteOn* b){
	return a->beat.min < b->beat.min;
}

ofxTLAntescofoNote::ofxTLAntescofoNote(ofxAntescofog* g) {
	mAntescofog = g;
	changingRangeMin = false;
	changingRangeMax = false;
	draggingSelectionRange = false;
	pointsAreDraggable = false;
	portInHover = false;
	portOutHover = false;
	inArmed = false;
	outArmed = false;
	shouldCreateNewSwitch = false;
	noteRange = ofRange(58, 88) ;// ofRange(10,110);
	color_gui_bg = ofColor(30, 110, 110);

	bounds.height = 275;

	ofxAntescofoAction = 0;
	AntescofoWriter = new MusicXML2::antescofowriter();
	mDur_in_secs = 0;
	bAutoScroll = true;
	bShowPianoRoll = true;
	bLockNotes = true;

	mAntescofo = new antescofo();
	mNetscore = 0;
	mParseDriver = new ParseDriver(mAntescofo);
}

ofxTLAntescofoNote::~ofxTLAntescofoNote(){
}

void ofxTLAntescofoNote::setup(){

	enable();
	load();	

	//	ofxTLRegisterPlaybackEvents(this);

	ofAddListener(ofEvents().update, this, &ofxTLAntescofoNote::update);

	// print input ports to console
	//midiIn.listPorts(); // via instance
	// open port by number (you may need to change this)
	//currentInPort = 1;
	//midiIn.openPort(currentInPort);
	//currentOutPort = 1;
	//midiOut.openPort(currentOutPort);

	// add testApp as a listener
	//midiIn.addListener(this);
	// print received messages to the console
	//midiIn.setVerbose(false);

	//noteFont.loadFont ("Boulez.ttf", 10, true, true, true);
	noteImage = new ofImage();
	if (!noteImage->loadImage("note.png")) {
		cerr << "Can not load note.png" << endl;
		abort();
	}

	mFont.loadFont ("DroidSansMono.ttf", 15);


	for (int i = 0; i < 127; i++) {
		activeNotes[i] = false;
	}
}

void ofxTLAntescofoNote::update(ofEventArgs& args){

}

void ofxTLAntescofoNote::setAutoScroll(bool active)
{
	bAutoScroll = active;
}


void ofxTLAntescofoNote::toggleView(){
	bShowPianoRoll = !bShowPianoRoll;
	trimRange();
}

void ofxTLAntescofoNote::setNoteColor(int n) {
	switch (switches[n]->type) {
		case ANTESCOFO_CHORD:
			ofSetColor(color_note_chord);
			return;
		case ANTESCOFO_MULTI:
		case ANTESCOFO_MULTI_STOP:
			ofSetColor(color_note_multi);
			return;
		case ANTESCOFO_TRILL:
			ofSetColor(color_note_trill);
			return;
		case ANTESCOFO_NOTE:
			ofSetColor(color_note);
			return;
	}
	ofSetColor(color_note);
}


void ofxTLAntescofoNote::draw_showPianoRoll() {
	//cout << "ofxTLAntescofoNote::draw_showPianoRoll: "  << bounds.x <<"," << bounds.y << " " << bounds.width << "x" << bounds.height << endl;
	//************************ Range Rows
	float rowHeight = bounds.height / (noteRange.span()+1);

	ofFill();
	// draw piano roll
	int piano_width = 20;
	for (int i = noteRange.max; i >= noteRange.min; i--) { // && i * rowHeight < bounds.height; i++) 
		int j = noteRange.max - i;
		int n = i % 12;
		if (n == 0 || n == 2 || n == 4 || n == 5 || n == 7 || n == 9 || n == 11) {
			ofSetColor(255, 255, 255, 90);
			ofRect(bounds.x, bounds.y + j * rowHeight, piano_width, rowHeight);

			ofSetColor(color_range_white);
		} else {
			ofSetColor(0, 0, 0, 100);
			ofRect(bounds.x, bounds.y + j * rowHeight, piano_width, rowHeight);

			ofSetColor(color_range_black);

		}
		ofRect(bounds.x + piano_width, bounds.y + j * rowHeight, bounds.width - piano_width, rowHeight);
		ofSetColor(0, 0, 0, 10);
		ofLine(bounds.x, bounds.y + j * rowHeight, bounds.x + bounds.width, bounds.y + j * rowHeight);
	}

	/* // draw note number guides
		 ofSetColor(255, 255, 255, 160);
		 for (int i = noteRange.max; i >= noteRange.min; i--) {
		 string noteNumber = ofToString(i);
	//string noteNumber = ofToString(ofMap(i, 0, noteRange.span(), noteRange.max, noteRange.min));
	//ofDrawBitmapString(noteNumber, 3, bounds.y + (i-noteRange.min) * rowHeight + rowHeight / 2.0f + 4);
	ofDrawBitmapString(noteNumber, 3, bounds.y + (noteRange.max - i) * rowHeight + rowHeight / 2.0f + 4);
	}*/
	// draw beat bars
	/*    ofSetColor(0);
				for (int i = 0; i < 200; i++) {
				float startX =  normalizedXtoScreenX( beatToNormalizedX(i%4), zoomBounds);
				ofLine(startX, bounds.y, startX+2, bounds.y+bounds.height);
				}
				*/
	/* commented by karm
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

	******************* Arm In GUI
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
	*/
	//************************ Range GUI
	// draw label
	ofSetColor(255);
	drawLabel("Range:", rangeLabelBounds);

	// draw min bg
	ofSetColor(color_gui_bg);
	drawGuiBG(rangeMinSliderBounds);
	// draw min text
	ofSetColor(255);
	drawLabel(ofToString(noteRange.min), rangeMinSliderBounds);

	// draw max bg
	ofSetColor(color_gui_bg);
	drawGuiBG(rangeMaxSliderBounds);
	// draw max text
	ofSetColor(255);
	drawLabel(ofToString(noteRange.max), rangeMaxSliderBounds);

	ofSetColor(color_gui_bg);
	drawGuiBG(rangeTrimButtonBounds);
	ofSetColor(255);
	drawLabel("Trim", rangeTrimButtonBounds);
	//************************
	/*t
	****** Port Out GUI

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

	// ********* Arm In GUI
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
	*/	//************************

	// draw notes
	if(switches.size() > 0) {
		float zoomMinX = bounds.x;
		float zoomMaxX = bounds.x + bounds.width;
		for(int i = 0; i < switches.size(); i++){
			if(isSwitchInBounds(switches[i])){
				float startX =  max(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds), zoomMinX);
				float endX = min(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds), zoomMaxX);

				// measure bar
				if (!switches[i]->label.empty()) {
					//cout << "ofxTLAntescofoNote: label:"<< switches[i]->label << endl;
					ofSetColor(0, 0, 0, 75);
					ofSetLineWidth(1);
					ofLine(startX, bounds.y, startX, bounds.y + bounds.height);
					ofDrawBitmapString(switches[i]->label, startX, bounds.y+10);
				}

				// manage rest notes
				if (switches[i]->pitch != 0) {
					if (abs(switches[i]->pitch) > noteRange.max || abs(switches[i]->pitch) < noteRange.min) continue; // don't draw note outside range
					ofFill();

					setNoteColor(i); //ofSetColor(color_note);
					// set color to transparent when selected
					if(switches[i]->startSelected && switches[i]->endSelected) ofSetColor(color_note_selected);
					if (abs(switches[i]->pitch) && switches[i]->beat.span() == 0) ofSetColor(color_note, 100); // grace note
					int whichRow = ofMap(abs(switches[i]->pitch), noteRange.max, noteRange.min, 1, noteRange.span());
					ofRectangle noteBounds = ofRectangle(startX, bounds.y + whichRow * rowHeight, endX - startX, rowHeight);

					// note heads
					float y = bounds.y + whichRow * rowHeight;
					float w = endX - startX;
					float h = rowHeight;
#ifdef CIRCLE_NOTE_HEADS
					ofCircle(startX + h/2, y+ h/2, 1*h/2);// un cercle au debut
					ofCircle(endX - h/2, y + h/2, 1*h/2); // un a la fin
					ofRectangle r(startX + h/2, y, endX - startX - h, rowHeight);
#else
					ofRectangle r(startX, y, endX - startX, rowHeight);
					ofRect(r);// et un rect entre les deux
					ofNoFill();
					ofSetColor(0, 0, 0, 200);
					ofRectangle rb(startX, y, endX - startX, rowHeight); // black border around note
					ofRect(rb);
					ofFill();
#endif
					setNoteColor(i);

					int hoverHintWidth = 3;
					if(switches[i]->endHovered && noteBounds.width > 3) {
						ofSetLineWidth(hoverHintWidth);
						ofSetColor(color_resize_note);
						ofLine(noteBounds.x + noteBounds.width, noteBounds.y, noteBounds.x + noteBounds.width, noteBounds.y + rowHeight);
					}
					if(switches[i]->startHovered && noteBounds.width > 3) {
						ofSetLineWidth(hoverHintWidth);
						ofSetColor(color_resize_note);
						ofLine(noteBounds.x, noteBounds.y, noteBounds.x, noteBounds.y + rowHeight);
					}

					// glissando : draw line
					if (switches[i]->type == ANTESCOFO_MULTI && i+1 < switches.size()
							&& (switches[i+1]->type == ANTESCOFO_MULTI || switches[i+1]->type == ANTESCOFO_MULTI_STOP)) {
						setNoteColor(i);
						// next note:
						float startX2 =  normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.min), zoomBounds);
						float endX2 = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.max), zoomBounds);
						int whichRow2 = ofMap(abs(switches[i+1]->pitch), noteRange.max, noteRange.min, 0, noteRange.span());
						ofRectangle noteBounds2 = ofRectangle(startX2, bounds.y + whichRow2 * rowHeight, endX2 - startX2, rowHeight);

						ofLine(noteBounds.x + noteBounds.width/2, noteBounds.y + noteBounds.height/2,
								noteBounds2.x + noteBounds2.width/2, noteBounds2.y + noteBounds2.height/2);
					}
					/* if (switches[i]->type == ANTESCOFO_TRILL) {
						 ostringstream str;
						 str << "got trill: pitch:" << switches[i]->pitch <<  " min:" << switches[i]->beat.min <<  " max:" << switches[i]->beat.max;
						 console->addln(str.str());
						 }*/
				} else { // draw a rest note
					ofNoFill();
					int rest_pseudoPitch_ymin = noteRange.min;
					int rest_pseudoPitch_ymax = noteRange.max;
					//if (rest_pseudoPitch > noteRange.max || rest_pseudoPitch < noteRange.min) continue; // don't draw note outside y range

					ofSetColor(color_note_rest, 75);
					int whichRowMin = ofMap(rest_pseudoPitch_ymin, noteRange.max, noteRange.min, 0, noteRange.span());
					int whichRowMax = ofMap(rest_pseudoPitch_ymax, noteRange.max, noteRange.min, 0, noteRange.span());
					int w = (noteRange.max - noteRange.min + 1)* rowHeight;
					ofRectangle noteBounds = ofRectangle(startX, bounds.y + w/3, endX - startX, w/3);
					ofRect(noteBounds);
					int hoverHintWidth = 2;
					if(switches[i]->endHovered) {
						ofSetLineWidth(hoverHintWidth);
						ofSetColor(color_resize_note_rest, 75);
						ofLine(noteBounds.x + noteBounds.width, bounds.y + w/3, noteBounds.x + noteBounds.width, bounds.y + w/3 + noteBounds.height);
					}
					if(switches[i]->startHovered) {
						ofSetLineWidth(hoverHintWidth);
						ofSetColor(color_resize_note_rest, 75);
						ofLine(noteBounds.x, bounds.y + w/3, noteBounds.x, bounds.y + w/3 + noteBounds.height);
					}

				}
				if (switches[i]->action.size()) {
					// ?
				}
			}
		} // end switch loop
	} // end if has switches

	//************************ Drag Selection
	ofSetLineWidth(2.0);
	if(draggingSelectionRange){
		ofFill();
		ofSetColor(timeline->getColors().keyColor);
		ofLine(dragSelection.min, bounds.y, dragSelection.min, bounds.y+bounds.height);
		ofLine(dragSelection.max, bounds.y, dragSelection.max, bounds.y+bounds.height);
		ofSetColor(timeline->getColors().keyColor, 30);
		ofFill();
		ofRect(dragSelection.min, bounds.y, dragSelection.span(), bounds.height);
	}
}


void ofxTLAntescofoNote::draw_showStaves() {
	// bg
	ofFill();
	ofSetColor(255, 255, 255, 225);
	ofRect(bounds.x, bounds.y, bounds.width, bounds.height);
	noteRange.set(21, 100);
	float score_line_interval = 40;
	float score_line_space = 40;
	float score_line_x = bounds.x + score_line_interval;
	float score_line_y = bounds.y + score_line_interval;
	float score_line_w = bounds.width;// - score_line_x - score_line_interval;
	float score_line_h = 1;

	/*
	// buttons
	ofSetColor(255);
	drawLabel("Range:", rangeLabelBounds);

	// draw min bg
	ofSetColor(color_gui_bg);
	drawGuiBG(rangeMinSliderBounds);
	// draw min text
	ofSetColor(255);
	drawLabel(ofToString(noteRange.min), rangeMinSliderBounds);

	// draw max bg
	ofSetColor(color_gui_bg);
	drawGuiBG(rangeMaxSliderBounds);
	// draw max text
	ofSetColor(255);
	drawLabel(ofToString(noteRange.max), rangeMaxSliderBounds);

	ofSetColor(color_gui_bg);
	drawGuiBG(rangeTrimButtonBounds);
	ofSetColor(255);
	drawLabel("Trim", rangeTrimButtonBounds);
	*/
	float rowHeight = bounds.height / (noteRange.span()+1);

	ofSetColor(color_staves_fg, 180);
	//ofRect(score_line_x, score_line_y, score_line_w, score_line_h);
	/* for (int i = 0; i < 5; i++) {
		 ofRect(score_line_x, score_line_y + i * score_line_space, score_line_w, score_line_h);
		 std::cout << "Drawing line : " << score_line_x << ", " << score_line_y + i * score_line_space << ", " << score_line_w<< ", " << score_line_h << std::endl;
		 }*/
	// notes
	/* for (int i = 0; i <= noteRange.span() && i * rowHeight < bounds.height; i++) {
		 if (i)

		 int n = i % 12; // 0:C

		 if (n == 0) || n == 2 || n == 4 || n == 5 || n == 7 || n == 9 || n == 11) {
		 ofSetColor(255, 255, 255, 240);
		 ofRect(bounds.x, bounds.y + i * rowHeight, piano_width, rowHeight);

		 ofSetColor(color_range_white);
		 */


	// draw staves :
	ofSetColor(0, 0, 0, 155);
	int staves[10] = { 77, 74, 71, 67, 64, 57, 53, 50, 47, 43 };
	for (int i = 0; i < 10; i++) {
		if (staves[i] > noteRange.max || staves[i] < noteRange.min) continue; // don't draw note outside range

		int whichRow = ofMap(staves[i], noteRange.max, noteRange.min, 0, noteRange.span());

		ofRect(bounds.x, bounds.y + whichRow * rowHeight, score_line_w, score_line_h);
	}
	bool bShowGraceNote = false;
	ofSetColor(0, 0, 0, 255);
	if(switches.size() > 0) {
		for(int i = 0; i < switches.size(); i++){
			if(isSwitchInBounds(switches[i])){
				float startX =  normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds);
				float endX = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds);

				// measure bar
				if (switches[i]->label.size()) {
					ofSetColor(0, 0, 0, 105);
					int yy = bounds.y + rowHeight* ofMap(staves[0], noteRange.max, noteRange.min, 0, noteRange.span());
					int hh = rowHeight* ofMap(staves[9], noteRange.max, noteRange.min, 0, noteRange.span());
					if (switches[i]->label.size()) {
						//cout << "ofxTLAntescofoNote: label:"<< switches[i]->label << endl;

						ofLine(startX, yy, startX, bounds.y + hh);
					}
					//ofDrawBitmapString(switches[i]->label.substr(7, switches[i]->label.size()), startX, yy - 4);
					ofDrawBitmapString(switches[i]->label, startX, yy - 4);
					ofSetColor(0, 0, 0, 255);
				}

				if (abs(switches[i]->pitch) != 0) {
					if (abs(switches[i]->pitch) > noteRange.max || abs(switches[i]->pitch) < noteRange.min) continue; // don't draw note outside range

					ofSetColor(color_note);
					// set color to transparent when selected
					if(switches[i]->startSelected && switches[i]->endSelected) ofSetColor(color_note_selected);
					int p = abs(switches[i]->pitch);
					int c = p % 12;
					bool accident = false;
					if (c == 1 || c == 3 || c == 6 || c == 8 || c == 10) {
						p--; c--;
						accident = true;
					}
					int whichRow = ofMap(p, noteRange.max, noteRange.min, 0, noteRange.span());
					ofRectangle noteBounds = ofRectangle(startX, bounds.y + whichRow * rowHeight, endX - startX, rowHeight);
					int y = bounds.y + whichRow*rowHeight;

					// draw note, if grace note: halfsize gray note
					int notehead_w = 10, notehead_h = 9;
					if (switches[i]->beat.span() == 0 && p) {
						bShowGraceNote = true; notehead_w /= 1.3; notehead_h /= 1.3;
						ofSetColor(0, 0, 0, 100);
					} else { bShowGraceNote = false; ofSetColor(0, 0, 0, 255); }

#if 0
					ofEllipse(startX+5, y + rowHeight/2, notehead_w, notehead_h);
#else
					// draw note image
					ofSetColor(0, 0, 0, 255);
					int sp = rowHeight*4;/// noteImage->getHeight();
					if (bShowGraceNote)
						sp /= 2;
					noteImage->draw(startX, y - sp/2, sp, sp);//noteImage->getWidth(), noteImage->getHeight());
#endif

					if (accident)
						mFont.drawString("#", startX - 10, y + 2*rowHeight);

					// draw little rect displaying duration after the note if not grace note
					ofSetColor(0, 0, 0, 99);
					if (!bShowGraceNote) ofRect(startX+2, y, endX-startX-2, 1);

					ofSetColor(0, 0, 0, 255);
					// draw little lines if note if out of staves
					if (p == 60) {
						whichRow = ofMap(p, noteRange.max, noteRange.min, 0, noteRange.span());
						ofRect(startX - 3, y, notehead_w + 6, score_line_h);
					}
					if (p >= 81) {
						for (int k = p; k >= 81 ; ) {
							whichRow = ofMap(k, noteRange.max, noteRange.min, 0, noteRange.span());
							y = bounds.y + whichRow*rowHeight;
							ofRect(startX - 3, y, notehead_w + 6, score_line_h);
							int t = k % 12;
							if (t == 1 || t == 3 || t == 6 || t == 8 || t == 10)
								k -= 2;
							else k -= 1;
						}
					} else if (p <= 41) {
						for (int k = p; k <= 41 ;) {
							whichRow = ofMap(k, noteRange.max, noteRange.min, 0, noteRange.span());
							y = bounds.y + whichRow*rowHeight;
							ofRect(startX - 3, y, notehead_w + 6, score_line_h);
							int t = k % 12;
							if (t == 1 || t == 3 || t == 6 || t == 8 || t == 10)
								k += 2;
							else k += 1;
						}
					}

					// glissando : draw line
					if (switches[i]->type == ANTESCOFO_MULTI && i+1 < switches.size()
							&& (switches[i+1]->type == ANTESCOFO_MULTI || switches[i+1]->type == ANTESCOFO_MULTI_STOP)) {
						//setNoteColor(i);
						// next note:
						float startX2 =  normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.min), zoomBounds);
						float endX2 = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.max), zoomBounds);
						int whichRow2 = ofMap(abs(switches[i+1]->pitch), noteRange.max, noteRange.min, 0, noteRange.span());
						ofRectangle noteBounds2 = ofRectangle(startX2, bounds.y + whichRow2 * rowHeight, endX2 - startX2, rowHeight);

						ofLine(startX+notehead_w, noteBounds.y + noteBounds.height/2,
								startX2, noteBounds2.y + noteBounds2.height/2);
					}

				} else { // rest
					int whichRow = ofMap(71, noteRange.max, noteRange.min, 0, noteRange.span());
					int y = bounds.y + whichRow*rowHeight;
					int h = 7;
					ofSetColor(0, 0, 0, 255);
					ofRect(startX, y - h + 2, endX-startX, h);
				}
			}
		}
	}
}


void ofxTLAntescofoNote::autoscroll() {
	static float lastpos = 0;
//#ifdef AUTOSCROLL_BROKEN
	float pos = timeline->getPercentComplete();
	if (bAutoScroll && mDur_in_secs && lastpos != pos) {
		ofxTLZoomer2D *zoom = (ofxTLZoomer2D*)timeline->getZoomer();

		ofRange z = zoom->getViewRange();
		ofRange oldz = z;
		//cout << endl << "pos:"<< pos <<" got zoomrange: "<< z.min << "->"<< z.max;
		// continuous scrolling : keep playhead on center
		if (pos) {
			float c = z.center(); 
			float d = pos - c;

			z.min = ofClamp(z.min + d, 0, 1); z.max = ofClamp(z.max + d, 0, 1);
			if (z.min == .0 && z.span() < oldz.span())
				z.max = oldz.max - oldz.min;
			if (z.max == 1. && z.span() < oldz.span())
				z.min = z.max - oldz.max + oldz.min;


			//cout <<" to zoomrange: "<< z.min << "->"<< z.max<<endl;
			zoom->setViewRange(z);
			//zoom->setSelectedRange(z);
#if 0
			float logSpan = powf(z.span(), 2.0);
			//recompute view range
			c = ofMap(pos, z.span()/2, 1.0 - z.span()/2, logSpan/2, 1.0-logSpan/2);
			cout << "c="<<c << endl;

			//z.min = ofClamp(pos - c, 0, 1); z.max = ofClamp(pos + c, 0, 1);
			z = ofRange(c - logSpan/2, c + logSpan/2);
			//zoom->setSelectedRange(ofRange(c - logSpan/2, c + logSpan/2));
			zoom->setSelectedRange(z);
#endif

			//zoom->setSelectedRange(z);
			//zoom->setViewRange(z);
			/*
			ofRange n;//(pos - d/2, pos + d/2);

			//if (r.min - d/2 <= pos) n.min = ofClamp(pos-d/2, 0, r.min + .01);
			//if (r.max + d/2 >= pos) n.max = ofClamp(pos+d/2, r.max + .01, 1);

			//if (pos >= r.max || pos <= r.min)
			{
				//  cout << "EXTRAAAAAAAAAAAA"<<endl;

				if (n.min <= 0) n.min = 0;
				if (n.max >= 1.0) n.max = 1;

				zoom->setViewRange(n);
			} 
			*/
			lastpos = pos;
		}

		// page by page scrolling : when the playhead gets close to the end of the page, move zoom to next page and playhead to beginning
	}
//#endif


}

void ofxTLAntescofoNote::draw() {
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

	autoscroll();
	if (bShowPianoRoll) {
		draw_showPianoRoll();
	} else
		draw_showStaves();

	ofPopStyle();
}



void ofxTLAntescofoNote::drawGuiBG(ofRectangle rect) {
	rect.x += -0.25 * guiXPadding;
	rect.width += guiXPadding * 0.5;
	ofRect(rect);
}

void ofxTLAntescofoNote::drawLabel(string caption, ofRectangle bounds){
	ofDrawBitmapString(caption, bounds.x, bounds.y + 10);
}

void ofxTLAntescofoNote::setNoteRange(ofRange range) {
	noteRange = range;
}

int ofxTLAntescofoNote::pitchForScreenY(int y) {
	float normalizedY = (y - bounds.y) / bounds.height;
	int pitch = ofClamp(ofMap(normalizedY, 1, 0, noteRange.min, noteRange.max + 1), noteRange.min, noteRange.max); // clamp to range
	return pitch;
}

bool ofxTLAntescofoNote::mousePressed(ofMouseEventArgs& args, long millis){

	//cout << "ofxTLAntescofoNote::mousePressed " << endl;
	ofVec2f screenpoint(args.x, args.y);
	shouldCreateNewSwitch = false;



	//************************ Range Click
	changingRangeMin = rangeMinSliderBounds.inside(screenpoint);
	changingRangeMax = rangeMaxSliderBounds.inside(screenpoint);

	if(changingRangeMin || changingRangeMax) {
		changingRangeAnchor = args.y;
		return true;
	}
	if(rangeTrimButtonBounds.inside(screenpoint)){
		trimRange();
		return false;
	}

	if (!bounds.inside(args.x, args.y)) return false;
	pointsAreDraggable = !ofGetModifierKeyShift();
	//************************ Track Focusing
	bool clickInRect = bounds.inside(screenpoint);
	if(clickInRect){
		if(!focused){
			focused = true;
			//			return;
		}
	} else {
		if(pointsAreDraggable){
			//unselectAll();
			focused = false;
		}
		return false;
	}
	updateDragOffsets(args.x);
	//************************
	bool shouldDeselect = false;
	bool didSelectedStartTime;
	//at this point we are inside the rect and focused, make changes
	ofxTLAntescofoNoteOn* clickedSwitchA = NULL;
	ofxTLAntescofoNoteOn* clickedSwitchB = NULL;
	clickedSwitchA = switchHandleForScreenPoint(ofPoint(args.x, args.y), didSelectedStartTime);

	ostringstream str;
	str << "clickedSwitchA:" << clickedSwitchA<< " didSelectedStartTime:" << didSelectedStartTime;
	console->addln(str.str()); str.str("");
	// deja selected
	if(clickedSwitchA != NULL){
		mAntescofog->editorShowLine(clickedSwitchA->lineNum_begin, clickedSwitchA->lineNum_end);
		if(!ofGetModifierKeyShift()){
			timeline->unselectAll();
		}
		bool startAlreadySelected = clickedSwitchA->startSelected;
		bool endAlreadySelected = clickedSwitchA->endSelected;
		clickedSwitchA->startSelected = didSelectedStartTime || (ofGetModifierKeyShift() && startAlreadySelected);
		clickedSwitchA->endSelected   = !didSelectedStartTime || (ofGetModifierKeyShift() && endAlreadySelected);
		if(didSelectedStartTime){
			timeline->setDragTimeOffset( clickedSwitchA->dragOffsets.min );
		}
		else{
			timeline->setDragTimeOffset( clickedSwitchA->dragOffsets.max );
		}
	} else { // premiere selection
		//if(ofGetModifierKeyShift()){
		draggingSelectionRange = true;
		selectionRangeAnchor = args.x;
		dragSelection.min = dragSelection.max = selectionRangeAnchor;
		//	return true; // bool added by karm ? right value ?
		//}

		float normalizedCoord = screenXtoNormalizedX(args.x, zoomBounds);
		clickedSwitchA = switchForScreenXY(args.x, args.y);
		if(clickedSwitchA != NULL){
			mAntescofog->editorShowLine(clickedSwitchA->lineNum_begin, clickedSwitchA->lineNum_end);
			//if we haven't already selected these, flag deselect
			if((!clickedSwitchA->startSelected || !clickedSwitchA->endSelected) && !ofGetModifierKeyShift()){
				timeline->unselectAll();
			}
			clickedSwitchA->startSelected = true;
			clickedSwitchA->endSelected   = true;
		} else {
#if 0 // not yet
			shouldCreateNewSwitch = true;
#endif
		}

		//don't create new switches when shift is held down, and don't deselect anything
		//if(ofGetModifierKeyShift()){
		//	shouldCreateNewSwitch = false;
		//	shouldDeselect = false;
		//}

		//if we clicked where to create a new switch, but still have a selection, first deselect
		if(shouldCreateNewSwitch && howManySwitchesAreSelected() > 0){
			shouldCreateNewSwitch = false;
			timeline->unselectAll();
		}
#if 0 // was here
		if(shouldCreateNewSwitch){
			ofxTLAntescofoNoteOn* newNote = new ofxTLAntescofoNoteOn();
			int pitch = pitchForScreenY(args.y);
			int velocity = 50;
			float startTime = screenXtoNormalizedX(args.x, zoomBounds);
			addNote(startTime, pitch, velocity);
			updateDragOffsets(args.x); //TODO: what does this do? Do we still need it?
		}
		return true;
#endif
	}
	draggingSelectionRange = false;

	return true;
}

void ofxTLAntescofoNote::mouseMoved(ofMouseEventArgs& args, long millis){
	//cout << "mouseMoved: "<< args.x<<", "<<args.y << endl ;
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
void ofxTLAntescofoNote::mouseDragged(ofMouseEventArgs& args, long millis) { //bool snapped){
#if 0
	//cout << "mouseDragged: "<< args.x<<", "<<args.y << ", millis:"<< millis << endl ;

	bool oneSwitchSelected = howManySwitchesAreSelected() == 1;

	if(draggingSelectionRange){
		dragSelection.min = MIN(args.x, selectionRangeAnchor);
		dragSelection.max = MAX(args.x, selectionRangeAnchor);
	}

	else if(pointsAreDraggable){

		for(int i = 0; i < switches.size(); i++){

			// allow pitch changes if we're dragging the whole note
			if(switches[i]->startSelected && switches[i]->endSelected /*&& oneSwitchSelected*/
					&& switches[i]->type != ANTESCOFO_REST) {
				switches[i]->pitch = pitchForScreenY(args.y);
				continue;
			}

			if (switches[i]->type == ANTESCOFO_NOTE || switches[i]->type == ANTESCOFO_REST) {// not a chord
				// check if next note is in the way
				if (i + 1 != switches.size() && switches[i+1]->beat.min > switches[i]->beat.max) {

					switches[i+1]->beat.min = MIN(timeline->normalizedXToBeat( screenXtoNormalizedX(args.x /* - switches[i]->dragOffsets.min*/, zoomBounds)), switches[i+1]->beat.max);
				}
				if (i > 0 && switches[i-1]->beat.max > switches[i]->beat.min) {
					switches[i-1]->beat.max = MIN(timeline->normalizedXToBeat( screenXtoNormalizedX(args.x /* - switches[i]->dragOffsets.min*/, zoomBounds)), switches[i-1]->beat.min);
				}
			}
			if(switches[i]->startSelected){
				//switches[i]->time.min = MIN( screenXtoNormalizedX(args.x - switches[i]->dragOffsets.min, zoomBounds), switches[i]->time.max);
				//TIME switches[i]->time.min = MIN( screenXtoNormalizedX(args.x /* - switches[i]->dragOffsets.min*/, zoomBounds), switches[i]->time.max);
				switches[i]->beat.min = MIN(timeline->normalizedXToBeat( screenXtoNormalizedX(args.x /* - switches[i]->dragOffsets.min*/, zoomBounds)), switches[i]->beat.max);

				cout << "mouseDragged: start selected: newTimeMin:"<< switches[i]->beat.min<<" dragOmin:"<< switches[i]->dragOffsets.min<<", timeMax:"<<switches[i]->beat.max<< endl ;
			}
			if(switches[i]->endSelected){
				//switches[i]->time.max = MAX( screenXtoNormalizedX(args.x - switches[i]->dragOffsets.max, zoomBounds), switches[i]->time.min);
				//TIME switches[i]->time.max = MAX( screenXtoNormalizedX(args.x /* - switches[i]->dragOffsets.max*/, zoomBounds), switches[i]->time.min);
				switches[i]->beat.max = MAX(timeline->normalizedXToBeat( screenXtoNormalizedX(args.x /* - switches[i]->dragOffsets.max*/, zoomBounds)), switches[i]->beat.min);
				cout << "mouseDragged: end selected: newTimeMax:"<< switches[i]->beat.max<<" dragOmin:"<< switches[i]->dragOffsets.min<<", timeMax:"<<switches[i]->beat.max << endl ;

			}
		}
		draggingSelectionRange = false;

		bool didSelectedStartTime;
		ofxTLAntescofoNoteOn* switchHandle = switchHandleForScreenPoint(ofPoint(args.x, args.y), didSelectedStartTime);
		if(timeline->getMovePlayheadOnPaste()){
			if(switchHandle != NULL){
				//timeline->setPercentComplete(didSelectedStartTime ? switchHandle->time.min : switchHandle->time.max);
				//timeline->setPercentComplete(didSelectedStartTime ? timeline->beatToNormalizedX(switchHandle->beat.min) : timeline->beatToNormalizedX(switchHandle->beat.max));
			}
			else{
				//timeline->setPercentComplete(screenXtoNormalizedX(args.x, zoomBounds));
			}
		}
	}
#endif
	// Drag Note Range
	if(changingRangeMin) {
		noteRange.min += floor((changingRangeAnchor - args.y) * 0.1f);
		noteRange.min = ofClamp(noteRange.min, 0, noteRange.max-5); // -5 because we don't want to zoom to close
		drawRectChanged();
	} else if(changingRangeMax) {
		noteRange.max += floor((changingRangeAnchor - args.y) * 0.1f);
		noteRange.max = ofClamp(noteRange.max, noteRange.min+5, 126); // -5 because we don't want to zoom to close
		drawRectChanged();
	}
}

void ofxTLAntescofoNote::mouseReleased(ofMouseEventArgs& args, long millis){
	if (!bounds.inside(args.x, args.y)) return;
	// don't look for changing ranges after mouse button is released
	changingRangeMin = false;
	changingRangeMax = false;

#if 0
	if(draggingSelectionRange){
		for(int i = 0; i < switches.size(); i++){
			//TIME if(dragSelection.contains( normalizedXtoScreenX(switches[i]->time.min, zoomBounds)))
			if(dragSelection.contains( normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds)) ){
				switches[i]->startSelected = true;
			}

			if(dragSelection.contains( normalizedXtoScreenX(timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds)) ){
				switches[i]->endSelected = true;
			}
		}
	} else {
		if(shouldCreateNewSwitch){
			ofxTLAntescofoNoteOn* newNote = new ofxTLAntescofoNoteOn();
			int pitch = pitchForScreenY(args.y);
			int velocity = 50;
			//TIME float startTime = screenXtoNormalizedX(args.x, zoomBounds);
			float startBeat = screenXtoNormalizedX(timeline->normalizedXToBeat(args.x), zoomBounds);
			addNote(startBeat, pitch, velocity);
			ostringstream str;
			str << "Adding new note : pitch: " << pitch << endl;
			console->addln(str.str());

			//ofLogVerbose("starTime=", startTime);
			updateDragOffsets(args.x); //TODO: what does this do? Do we still need it?
		}
	}


	// If we've dragged a switch such that it overlaps other notes, delete the other notes
	for (int i = 0; i < switches.size(); i++) {
		if(switches[i]->endSelected || switches[i]->startSelected) {
			for (int j = switches.size()-1; j >= 0; j--) {
				//TIME if(switches[j]->pitch == switches[i]->pitch && switches[j]->time.intersects(switches[i]->time) && switches[j] != switches[i]) 
				if(switches[j]->pitch == switches[i]->pitch && switches[j]->beat.intersects(switches[i]->beat) && switches[j] != switches[i]) {
					// we have overlapping switches - delete the old one if we overlap the start, or trim it if we overlap the end
					//TIME if(switches[i]->time.contains(switches[j]->time.min)) {
					if(switches[i]->beat.contains(switches[j]->beat.min)) {
						delete switches[j];
						switches.erase(switches.begin()+j);
					} else {
						//switches[j]->time.max = switches[i]->time.min - 0.001;
						switches[j]->beat.min = switches[i]->beat.min - 1; // XXX ?
					}
				}
				}
			}
		}

#endif
		for(int i = switches.size()-1; i >= 0; i--){
			// deselect all switches if we're not dragging and we haven't released over a note
			//TIME if(!draggingSelectionRange && !switches[i]->time.contains(screenXtoNormalizedX(args.x)))
			if(!draggingSelectionRange &&
					!switches[i]->beat.contains(screenXtoNormalizedX(timeline->normalizedXToBeat(args.x)))){
				switches[i]->startSelected = false;
				switches[i]->endSelected = false;
			}
			// delete all switches that have been collapsed to nothing
			//if(switches[i]->time.min == switches[i]->time.max)
			//	delete switches[i];
			//	switches.erase(switches.begin()+i);
		}	
		draggingSelectionRange = false;
		//	if(autosave) save();
}

void ofxTLAntescofoNote::keyPressed(ofKeyEventArgs& args){

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
				//TIME switches[i-1]->time.max = switches[i]->time.max;
				switches[i-1]->beat.max = switches[i]->beat.max;
				delete switches[i];
				switches.erase(switches.begin()+i);
			}
		}
		//		if(autosave) save();
	}
}

void ofxTLAntescofoNote::nudgeBy(ofVec2f nudgePercent){
	cerr << "TODO implement ofxTLAntescofoNote::nudgeBy()" << endl;
#if 0
	for(int i = 0; i < switches.size(); i++)
		if(switches[i]->startSelected){
			switches[i]->time.min += nudgePercent.x; 
		}
	if(switches[i]->endSelected){
		switches[i]->time.max += nudgePercent.x; 
	}		
}

//	if(autosave) save();
#endif

}

ofxTLAntescofoNoteOn* ofxTLAntescofoNote::switchForScreenXY(float screenPos, int y){
	return switchForPoint(screenXtoNormalizedX(screenPos, zoomBounds), y);
}

ofxTLAntescofoNoteOn* ofxTLAntescofoNote::switchForPoint(float percent, int y){
	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->beat.contains(timeline->normalizedXToBeat(percent))
				&& (switches[i]->pitch == pitchForScreenY(y) || switches[i]->pitch == 0)){ // handle null pitch (rest notes)
					return switches[i];
				}
	}
	return NULL;	
}

void ofxTLAntescofoNote::save(){
	ofxXmlSettings settings;
	string xmlRep = getXMLStringForSwitches(false);
	settings.loadFromBuffer(xmlRep);
	settings.saveFile(xmlFileName);
}


int ofxTLAntescofoNote::loadscoreMusicXML(string filename, string outfilename){
	clear();
	switches = switchesFromMusicXML(filename, outfilename);
	timeline->setBPM(atof(AntescofoWriter->fBPM.c_str()));
	trimRange();
	return switches.size();
}

extern string str_error;

string ofxTLAntescofoNote::get_error()
{
	return str_error;
}


void ofxTLAntescofoNote::clear_error()
{
	str_error.clear();
}

void ofxTLAntescofoNote::set_error(string e) {
	str_error = e;
}


void ofxTLAntescofoNote::clear_actions()
{
	if (ofxAntescofoAction)
		ofxAntescofoAction->clear_actions();
}


int ofxTLAntescofoNote::getNoteType(Event *e)
{       
	if (e) {
		ostringstream str;
		str << "getNoteType: isMArkov:"<< e->isMarkov << endl;
		console->addln(str.str()); str.str("");
		switch (e->isMarkov)
		{
			case 2: // TRILL
				{
					if (0 != e->multi_event)
					{
						if(e->multi_event>0)
						{

							return ANTESCOFO_MULTI; // MULTI TRILL XXX
						}
						return ANTESCOFO_TRILL;
					} else return ANTESCOFO_TRILL; // MULTI wtf?;
				}
			case 4: // CHORD
				return ANTESCOFO_CHORD;

			case 0:  // NOTE
			default:
				{
					if (1 == e->pitch_list.size() && 0.0 == e->pitch_list[0])
						return ANTESCOFO_REST;
					else
					{
						if (0 != e->multi_event)
						{
							if(e->multi_event>0)
							{
								return ANTESCOFO_MULTI;
							}
						} else {
							if (1 == e->pitch_list.size())
								return ANTESCOFO_NOTE;
							else
								return ANTESCOFO_CHORD;
						}
					}
				}

			case 1:  // DummySilence
				return -1;
		}
	}
	cerr << "ofxTLAntescofoNote::getNoteType: unknown type" << endl;
	return -1;
}



int ofxTLAntescofoNote::loadscoreAntescofo(string filename){
	clear();
	//str_error.erase();
	Score *score;
	mParseDriver->setVerbosity(0);
	if (NULL == (score = mParseDriver->parse(filename))) {
		pre_antescofo::error("Parse error: %s\nCheck the syntax\nAbort loading score\n");
		return 0;
	}

	ostringstream str;
	str << "Duration ------------------ score size : " << score->size() << "."<< endl;
	//for (vector<Event *>::iterator i = score->begin(); i != score->end(); i++)
	//	    cout << "Duration ------------------ beatcum: " << (*i)->beatcum << "."<< endl;
	vector<Event *>::iterator i = score->end();
	i--; // duration is the last element index
	float dur_in_beats = (*i)->beatcum + (*i)->beat_duration;
	str << "Duration ------------------ last beat : " << dur_in_beats << "."<< endl;
	if (score->tempo < 0)
		score->tempo = - score->tempo;
	timeline->setBPM(60/score->tempo); // bps
	mDur_in_secs = 60 / timeline->getBPM() * dur_in_beats;
	str << "Duration ------------------ " << dur_in_beats << " beats."<< endl;
	str << "Duration ------------------ " << mDur_in_secs << " seconds.";
	console->addln(str.str()); str.str("");
	mDur_in_secs++; // add one beat for better display
	timeline->setDurationInSeconds(mDur_in_secs);

	bool bGot_Action = true;
	ostringstream oss;
	string actstr;

	for (vector<Event *>::iterator i = score->begin(); i != score->end(); i++)
	{
		Event *e = *i;
		if (e->gfwd) {
			Gfwd *g = e->gfwd;
			oss.str(""); oss.clear();
			list<Action*> l = e->gfwd->bloc();
			for (list<Action*>::iterator a = l.begin(); a != l.end(); a++) {
				if (*a)
					oss << *a;
			}
			actstr = oss.str();
			if (actstr.size())
				bGot_Action = true;
		}

		int newtype = getNoteType(e);
		//str << ">>>>>>>> got newtype " << newtype << " isMarkov:"<< e->isMarkov << " pitchsize:" << e->pitch_list.size(); console->addln(str.str()); str.str("");
		if ((e->pitch_list.size() == 1 && e->beat_duration) || (e->pitch_list.size() && e->pitch_list[0] && !e->beat_duration)) { // NOTE, TRILL, MULTI
			//if (newtype == -1) continue;
			if (newtype == ANTESCOFO_MULTI) {
				double beatmulti = e->beatcum;
				double durmulti = e->multi_dur;
				for (vector<float>::iterator m = e->multi_source.begin(); m != e->multi_source.end(); m++) { // MULTI sources
					ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
					newSwitch->type = ANTESCOFO_MULTI;
					newSwitch->startSelected = newSwitch->endSelected = false;
					newSwitch->beat.min = beatmulti;
					newSwitch->beat.max = newSwitch->beat.min + durmulti*2/3;
					newSwitch->pitch = abs(*m) > 1000 ? *m / 100 : *m;
					newSwitch->velocity = 127;
					newSwitch->channel = 1;
					if (bGot_Action && m == e->multi_source.begin())  { // associate action with first MULTI switch
						newSwitch->action = actstr;
						add_action(e->beatcum, actstr, e);
					}
					// get location in text score
					assert(e->scloc);
					newSwitch->lineNum_begin = e->scloc->begin.line;
					newSwitch->colNum_begin = e->scloc->begin.column;
					newSwitch->lineNum_end = e->scloc->end.line;
					newSwitch->colNum_end = e->scloc->end.column;
					if (m == e->multi_source.begin()) { 
						newSwitch->label = e->cuename;
						if (bGot_Action)  {
							newSwitch->action = actstr;
							add_action(e->beatcum, actstr, e);
						}
					}
					switches.push_back(newSwitch);
					str << "added new switch for MULTI source: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch;
					console->addln(str.str()); str.str("");
					line2note[e->scloc->begin.line] = switches.size() - 1;
					bGot_Action = false;
				}
				for (vector<float>::iterator m = e->multi_target.begin(); m != e->multi_target.end(); m++) { // MULTI destinations
					ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
					newSwitch->type = ANTESCOFO_MULTI_STOP;
					newSwitch->startSelected = newSwitch->endSelected = false;
					newSwitch->beat.min = beatmulti + durmulti/3;
					newSwitch->beat.max = newSwitch->beat.min + durmulti*2/3;
					newSwitch->pitch = abs(*m) > 1000 ? *m / 100 : *m;
					newSwitch->velocity = 127;
					newSwitch->channel = 1;
					// get location in text score
					assert(e->scloc);
					newSwitch->lineNum_begin = e->scloc->begin.line;
					newSwitch->colNum_begin = e->scloc->begin.column;
					newSwitch->lineNum_end = e->scloc->end.line;
					newSwitch->colNum_end = e->scloc->end.column;
					switches.push_back(newSwitch);
					line2note[e->scloc->begin.line] = switches.size() - 1;
					str << "added new switch for MULTI target: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch;
					console->addln(str.str()); str.str("");
					bGot_Action = false;
				}
				if (e->multi_source.size() && e->multi_target.size())
					continue;
			} 

			str << "got pitch:"<< e->pitch_list[0] << " type:"<< newtype << " markov:"<<e->isMarkov;
			console->addln(str.str()); str.str("");
			ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
			newSwitch->type = newtype;
			newSwitch->startSelected = newSwitch->endSelected = false;
			newSwitch->beat.min = e->beatcum;
			newSwitch->beat.max = newSwitch->beat.min + e->beat_duration;
			newSwitch->pitch = abs(e->pitch_list[0]) > 1000 ? e->pitch_list[0] / 100 : e->pitch_list[0];
			newSwitch->velocity = 127;
			newSwitch->channel = 1;
			if (bGot_Action)  {
				newSwitch->action = actstr;
				add_action(e->beatcum, actstr, e);
			}
			// get location in text score
			assert(e->scloc);
			newSwitch->lineNum_begin = e->scloc->begin.line;
			newSwitch->colNum_begin = e->scloc->begin.column;
			newSwitch->lineNum_end = e->scloc->end.line;
			newSwitch->colNum_end = e->scloc->end.column;
			newSwitch->label = e->cuename;
			//if (newSwitch->label.compare(0, 7, "measure") == 0)                 newSwitch->measure = newSwitch->label.substr(7, newSwitch->label.size());
			switches.push_back(newSwitch);
			line2note[e->scloc->begin.line] = switches.size() - 1;
			str << "added new switch: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch;
			console->addln(str.str()); str.str("");
			bGot_Action = false;
		}
		else if (e->pitch_list.size() > 1 && e->beat_duration) {
			// sort
			std::sort(e->pitch_list.begin(), e->pitch_list.end());

			// remove duplicates pitches
			vector<float>::iterator f = std::unique(e->pitch_list.begin(), e->pitch_list.end());
			e->pitch_list.resize(f - e->pitch_list.begin());

			int p = 0;
			for (vector<float>::const_iterator j = e->pitch_list.begin(); j != e->pitch_list.end(); j++)
			{
				ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
				newSwitch->startSelected = newSwitch->endSelected = false;
				newSwitch->beat.min = e->beatcum;
				newSwitch->beat.max = newSwitch->beat.min + e->beat_duration;
				newSwitch->pitch = abs(e->pitch_list[p]) > 1000 ? e->pitch_list[p] / 100 : e->pitch_list[p];
				newSwitch->velocity = 127;
				newSwitch->channel = 1;
				newSwitch->lineNum_begin = e->scloc->begin.line;
				newSwitch->colNum_begin = e->scloc->begin.column;
				newSwitch->lineNum_end = e->scloc->end.line;
				newSwitch->colNum_end = e->scloc->end.column;
				if (bGot_Action)  {
					newSwitch->action = actstr;
					add_action(e->beatcum, actstr, e);
				}
				p++;
				bGot_Action = false;
				if (j == e->pitch_list.begin()) newSwitch->label = e->cuename;
				newSwitch->type = getNoteType(e);
				switches.push_back(newSwitch);
				line2note[e->scloc->begin.line] = switches.size() - 1;
				str << "added new switch: CHORD:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch;
				console->addln(str.str()); str.str("");
			}
			bGot_Action = false;
		} else if (newtype == ANTESCOFO_TRILL) {
			int pitch = 0;

			for (vector< vector<float> >::iterator t = e->TrillMap.begin(); t != e->TrillMap.end(); t++) {
				for (vector<float>::iterator r = t->begin(); r != t->end(); r++) {
					ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
					newSwitch->type = ANTESCOFO_TRILL;
					newSwitch->startSelected = newSwitch->endSelected = false;
					newSwitch->beat.min = e->beatcum ;//+ e->beat_duration/3;
					newSwitch->beat.max = newSwitch->beat.min + e->beat_duration; //*2/3;
					newSwitch->pitch = abs(*r) > 1000 ? *r / 100 : *r;
					newSwitch->velocity = 127;
					newSwitch->channel = 1;
					if (bGot_Action)  {
						newSwitch->action = actstr;
						add_action(e->beatcum, actstr, e);
					}
					// get location in text score
					assert(e->scloc);
					newSwitch->lineNum_begin = e->scloc->begin.line;
					newSwitch->colNum_begin = e->scloc->begin.column;
					newSwitch->lineNum_end = e->scloc->end.line;
					newSwitch->colNum_end = e->scloc->end.column;
					if (r == t->begin()) newSwitch->label = e->cuename;
					switches.push_back(newSwitch);
					line2note[e->scloc->begin.line] = switches.size() - 1;
					str << "added new switch for TRILL: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch;
					console->addln(str.str()); str.str("");
					bGot_Action = false;

				}
			}
		}

	}
	if (ofxAntescofoAction)
		ofxAntescofoAction->setScore(score);
	str << "Score tempo : " << 60/score->tempo;
	console->addln(str.str()); str.str("");
	trimRange();
	unselectAll();
	sort(switches.begin(), switches.end(), switchsort);

	update_duration();
	return switches.size();
}

void ofxTLAntescofoNote::update_duration() {
	int maxdur = ofxAntescofoAction->get_max_note_beat();

	cout << "Maximum note beat calculated : " << maxdur << " beats." << endl;
	float dur_in_beats = mDur_in_secs * timeline->getBPM() / 60;
	if (maxdur > dur_in_beats) {
		dur_in_beats = maxdur + 1;
		mDur_in_secs = 60 / timeline->getBPM() * dur_in_beats;
		cout << "Duration ------------------ " << dur_in_beats << " beats."<< endl;
		cout << "Duration ------------------ " << mDur_in_secs << " seconds.";
		mDur_in_secs++; // add one beat for better display
		timeline->setDurationInSeconds(mDur_in_secs);
	}
}


void ofxTLAntescofoNote::load() {}

void ofxTLAntescofoNote::add_action(float beatnum, string action, Event *e) {
	if (!ofxAntescofoAction)
		createActionTrack();

	ofxAntescofoAction->add_action(beatnum, action, e);
}

void ofxTLAntescofoNote::setScore(Score* s) {
	if (ofxAntescofoAction) 
		ofxAntescofoAction->setScore(s);
}


float ofxTLAntescofoNote::convertAntescofoOutputToTime(float mOsc_beat, float mOsc_tempo, float mOsc_pitch) {

	if (mOsc_tempo == 0) cerr << "Error null tempo returned by Antescofo, skipping a division by zero..." << endl;
	if (mOsc_beat == 0) return 0;

	/*
	//cout << "converting: beat:"<<mOsc_beat << " tempo:"<<mOsc_tempo << " "<<endl;
	double oneMeasure = 60 / mOsc_tempo; //timeline->getBPM();
	float r = (mOsc_beat ) * oneMeasure;// * mDur_in_secs / 1000;
	*/
	float r = timeline->beatToMillisec(mOsc_beat) / 1000;
	timeline->setCurrentTimeSeconds(r);


	// display followed event in editor
	ofxTLAntescofoNoteOn* switchA = 0;
	for (int i = 0; i < switches.size(); i++) {
		if (switches[i]->beat.contains(mOsc_beat))
			switchA = switches[i];
	}

	if(switchA != NULL)
		mAntescofog->editorShowLine(switchA->lineNum_begin, switchA->lineNum_end);

	// TODO : save detectedPitch = mOsc_pitch; in order to display it in a purple color in draw()
	return r;
}


// called from ofxTLAntescofoAction with a modified action string from the text editor
// -> save the action
// -> save a backup of current score
// -> reload modified file, if parse error : restore backup score
bool ofxTLAntescofoNote::change_action(float beatnum, string newaction)
{
	ostringstream str;
	str << "ofxTLAntescofoNote::change_action : " << beatnum << ", new action: "<< newaction;
	console->addln(str.str()); str.str("");
	ofxAntescofoAction->clear_actions();
	string out;

	for (int i = 0; i < switches.size(); i++) {
		if (beatnum == switches[i]->beat.min) {
			cout << "ofxTLAntescofoNote::change_action: replacing " << switches[i]->action << endl << " by :"<<endl << newaction;
			console->addln(str.str()); str.str("");
			out += newaction;
		}
		else if (switches[i]->action.size())
			out += switches[i]->action;
	}

	// save a copy of current score
	vector<ofxTLAntescofoNoteOn*> bkpswitches = switches;

	// try to parse resulting new score
	ofstream f;
	f.open(TEXT_CONSTANT_TEMP_ACTION_FILENAME);
	f << out;
	f.close();
	//if (!loadscoreAntescofo(out)) {
	str_error.erase();
	Score *score;
	if (NULL == (score = mParseDriver->parse(TEXT_CONSTANT_TEMP_ACTION_FILENAME))) {
		pre_antescofo::error("Parse error: %s\nCheck the syntax\nAbort loading score\n");
		return 0;

		str << "ofxTLAntescofoNote::change_action: display dialog box saying [Error action syntax error]" << endl;
		str << "ofxTLAntescofoNote::change_action: display dialog box asking [keep current action ? or cancel change]";
		console->addln(str.str()); str.str("");
		switches = bkpswitches;
	} else return loadscoreAntescofo(TEXT_CONSTANT_TEMP_ACTION_FILENAME);
}

void ofxTLAntescofoNote::createActionTrack() {
	ofxAntescofoAction = new ofxTLAntescofoAction(mAntescofog);
	getTimeline()->addTrack("Antescofo Actions", ofxAntescofoAction);
	ofxAntescofoAction->setNoteTrack(this);
}


string ofxTLAntescofoNote::getXMLStringForSwitches(bool selectedOnly){
	ofxXmlSettings settings;
	settings.addTag("notes");
	settings.pushTag("notes");
	int notesAdded = 0;
	for(int i = 0; i < switches.size(); i++){
		if(!selectedOnly || (switches[i]->startSelected && switches[i]->endSelected)){
			settings.addTag("note");
			settings.pushTag("note",notesAdded);
			//TIME settings.addValue("startTime", switches[i]->time.min);
			//TIME settings.addValue("endTime", switches[i]->time.max);
			settings.addValue("startBeat", switches[i]->beat.min);
			settings.addValue("endBeat", switches[i]->beat.max);
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


// highlight note on doubclick event in the editor
void ofxTLAntescofoNote::showNote(int line)
{
	// find note to highlight
	map<int, int>::iterator b;

	unselectAll();
	if ((b = line2note.find(line)) != line2note.end()) {

		// select corresponding note
		int n = b->second;
		cout << "found note " << n << " for line " << line << endl;
		if (n < switches.size() && switches[n]) {
			switches[n]->startSelected = true;
			switches[n]->endSelected = true;

			int m = n-1;
			cout << "Testing: note " << m << " with beat:" << switches[m]->beat.min  << " is " << switches[m]->beat.contains(switches[n]->beat.center()) << " for line " << line << endl;
			while (m < switches.size() && switches[m] && switches[m]->beat.contains(switches[n]->beat.center())) {
				switches[m]->startSelected = true;
				switches[m]->endSelected = true;
				m--;
			}

			// move zoom to show note
			float pos = timeline->beatToNormalizedX(switches[n]->beat.center());
			ofxTLZoomer2D *zoom = (ofxTLZoomer2D*)timeline->getZoomer();
			ofRange z = zoom->getViewRange();
			ofRange oldz = z;
			//cout << endl << "pos:"<< pos <<" got zoomrange: "<< z.min << "->"<< z.max;
			float c = z.center(); 
			float d = pos - c;

			z.min = ofClamp(z.min + d, 0, 1); z.max = ofClamp(z.max + d, 0, 1);
			if (z.min == .0 && z.span() < oldz.span())
				z.max = oldz.max - oldz.min;
			if (z.max == 1. && z.span() < oldz.span())
				z.min = z.max - oldz.max + oldz.min;

			//cout <<" to zoomrange: "<< z.min << "->"<< z.max<<endl;
			zoom->setViewRange(z);

		}
				/*
		for(int i = 0; i < switches.size(); i++) {
			if (switches[i]->beat.min >= b->second && switches[i]->beat.max <= b->second ) {
				cout << "found note: "<< b->second << endl;
				switches[i]->startSelected = true;
				switches[i]->endSelected = true;
			}
		}
		*/
	} else 
		cerr << "Note not found for line " << line << endl;
	

}

vector<ofxTLAntescofoNoteOn*> ofxTLAntescofoNote::switchesFromXML(ofxXmlSettings xmlStore){
	vector<ofxTLAntescofoNoteOn*> newSwitches;

	int numSwitchStores = xmlStore.getNumTags("notes");
	for(int s = 0; s < numSwitchStores; s++){
		xmlStore.pushTag("notes", s);
		int numSwitches = xmlStore.getNumTags("note");

		for(int i = 0; i < numSwitches; i++){
			ofxTLAntescofoNoteOn* newSwitch = new ofxTLAntescofoNoteOn();
			newSwitch->startSelected = newSwitch->endSelected = false;
			xmlStore.pushTag("note", i);
			//TIME newSwitch->time.min = xmlStore.getValue("startTime",0.0);
			newSwitch->beat.min = xmlStore.getValue("startBeat",0.0);
			newSwitch->beat.max = xmlStore.getValue("endBeat",0.0);
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


vector<ofxTLAntescofoNoteOn*> ofxTLAntescofoNote::switchesFromMusicXML(string filename, string outfilename){
	vector<ofxTLAntescofoNoteOn*> newSwitches;

	// parse musicxml files using libmusicxml2
	using namespace MusicXML2;

	xmlreader r;
	SXMLFile xmlfile;
	//xmlfile = r.read("/Volumes/Data/Coffy/ofx/of_v0072_osx_release/apps/myApps/Antescofog/libmusicxml-2.00-src/files/samples/musicxml/BeetAnGeSample.xml");
	xmlfile = r.read(filename.c_str());
	if (xmlfile) {
		Sxmlelement st = xmlfile->elements();
		if (st) {
			//AntescofoWriter = xml2antescofo(xmlfile, false, std::cout, NULL);
			xml2antescofovisitor v(*AntescofoWriter, true, true, false);
			Santescofoelement as = v.convert(st);

			ostringstream str;
			str << ";  Antescofo code converted using libmusicxml v." << musicxmllibVersionStr() << std::endl;
			str << "; and the embedded xml2antescofo converter v." << musicxml2antescofoVersionStr();
			console->addln(str.str()); str.str("");

			AntescofoWriter->print();
			AntescofoWriter->write(outfilename.c_str());

			int r = loadscoreAntescofo(outfilename);
		}
	}
	return switches;
}


string ofxTLAntescofoNote::copyRequest(){
	//	cout << "copy request" << endl;
	return getXMLStringForSwitches(true);
}

string ofxTLAntescofoNote::cutRequest(){
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

void ofxTLAntescofoNote::pasteSent(string pasteboard){
	cerr << "ofxTLAntescofoNote::pasteSent: TODO" << endl;
#if 0
	//	cout << "pasting " << pasteboard << endl;
	ofxXmlSettings pasted;
	pasted.loadFromBuffer(pasteboard);
	vector<ofxTLAntescofoNoteOn*> newSwitches = switchesFromXML(pasted);
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
#endif
}

void ofxTLAntescofoNote::selectAll(){
	for(int i = 0; i < switches.size(); i++){
		switches[i]->startSelected = switches[i]->endSelected = true;
	}
}

//returns the number of selected items
//this used to determine two things:
//1 Should an incoming click create a new item or remove a multiple selection?
//If the timeline has more than 1 item selected incoming clicks that don't hit anything will deselect all
//otherwise they'll create a new value on that track
//2 Can this track be modified by the current event? If there are selected items
//the this track's state will be stored in the undo buffer.
int ofxTLAntescofoNote::getSelectedItemCount()
{
	int r = 0;

	for(int i = 0; i < switches.size(); i++){
		if (switches[i]->startSelected || switches[i]->endSelected)
			r++;
	}
	return r;
};

void ofxTLAntescofoNote::clear(){
	for(int i = 0; i < switches.size(); i++){
		delete switches[i];
	}
	switches.clear();
	delete AntescofoWriter;
	AntescofoWriter = new MusicXML2::antescofowriter();
}

void ofxTLAntescofoNote::playbackStarted(ofxTLPlaybackEventArgs& args){
	lastTimelinePoint = timeline->getPercentComplete();
}
void ofxTLAntescofoNote::playbackLooped(ofxTLPlaybackEventArgs& args){
	cerr << "ofxTLAntescofoNote::playbackLooped: TODO" << endl;
#if 0
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
#endif
}
void ofxTLAntescofoNote::playbackEnded(ofxTLPlaybackEventArgs& args){
	endActiveNotes();
}

void ofxTLAntescofoNote::endActiveNotes() {
	cerr << "ofxTLAntescofoNote::endActiveNotes: TODO" << endl;
#if 0

	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->growing) {
			switches[i]->growing = false;
			switches[i]->time.max = timeline->getPercentComplete();
		}
	}
#endif

}

bool ofxTLAntescofoNote::isSwitchInBounds(ofxTLAntescofoNoteOn* s){
	ofRange r( timeline->beatToNormalizedX(s->beat.min), timeline->beatToNormalizedX(s->beat.max));
	return zoomBounds.intersects(r);
}

// XXX
void ofxTLAntescofoNote::updateDragOffsets(float clickX){
	for(int i = 0; i < switches.size(); i++){
		//TIME switches[i]->dragOffsets = ofRange(clickX - normalizedXtoScreenX(switches[i]->time.min, zoomBounds),
		//TIME								   clickX - normalizedXtoScreenX(switches[i]->time.max, zoomBounds));
		switches[i]->dragOffsets = ofRange(clickX - normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds),
				clickX - normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds));
	}
}

int ofxTLAntescofoNote::howManySwitchesAreSelected(){
	int numSelected = 0;
	for(int i = 0; i < switches.size(); i++){
		if(switches[i]->startSelected || switches[i]->endSelected){
			numSelected++;
		}
	}
	return numSelected;
}

void ofxTLAntescofoNote::unselectAll(){
	//	if(ofGetModifierKeyShift()){
	//		return;
	//	}

	for(int i = 0; i < switches.size(); i++){
		switches[i]->startSelected = false;
		switches[i]->endSelected = false;
	}
}

ofxTLAntescofoNoteOn* ofxTLAntescofoNote::switchHandleForScreenPoint(ofPoint screenPos, bool& startTimeSelected){
	for(int i = 0; i < switches.size(); i++){
		float xPos = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds);
		if(abs(xPos - screenPos.x) < 7 && (switches[i]->pitch == pitchForScreenY(screenPos.y) || switches[i]->pitch == 0)){
			startTimeSelected = true;
			return switches[i];
		}
		xPos = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds);
		if(abs(xPos - screenPos.x) < 7 && (switches[i]->pitch == pitchForScreenY(screenPos.y) || switches[i]->pitch == 0)){
			startTimeSelected = false;
			return switches[i];
		}
	}
	return NULL;
}


//Finds the closest switch behind a given point
//if the point is inside a switch, it will return NULL.
ofxTLAntescofoNoteOn* ofxTLAntescofoNote::nearestGrowingNoteBeforePointWithPitch(float percent, int pitch){
	float nearest = 1.0;
	ofxTLAntescofoNoteOn* nearestSwitch = NULL;
	for(int i = 0; i < switches.size(); i++){
		//		if (switches[i]->time.max > percent) {
		//			break;
		//		}
		float percent_x = timeline->beatToNormalizedX(switches[i]->beat.min);
		//if(percent - switches[i]->time.min < nearest && switches[i]->pitch == pitch && switches[i]->growing){
		if(percent - percent_x < nearest && switches[i]->pitch == pitch && switches[i]->growing){
			nearest = percent - percent_x;
			nearestSwitch = switches[i];
		}
	}
	return nearestSwitch;
}

bool ofxTLAntescofoNote::hoveringOn(float hoverY){
		return hoverY < bounds.y+bounds.height/2;
}


void ofxTLAntescofoNote::addNote(float beat, int pitch, int velocity, bool growing, int channel){
	ofxTLAntescofoNoteOn* newNote = new ofxTLAntescofoNoteOn();
	newNote->pitch = pitch;
	newNote->velocity = velocity;
	newNote->beat.min = beat;
	newNote->beat.max = beat + 1;
#if 0
	if(!growing) {
		double oneMeasure = 60 / timeline->getBPM();
		double halfMeasure = oneMeasure/2;
		double quarterMeasure = halfMeasure/2;
		newNote->time.max = time + ofMap(quarterMeasure, 0, timeline->getDurationInSeconds(), 0, 1);
	}
#endif
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

void ofxTLAntescofoNote::trimRange() {
	if(switches.size() > 0) {
		ofRange newRange = ofRange(58, 74); //switches[0]->pitch, switches[0]->pitch);
		for (int i = 0; i < switches.size(); i++) {
			if (switches[i]->pitch == 0) continue; // handle rest note
			if (switches[i]->pitch > newRange.max) {
				newRange.max = switches[i]->pitch;
			} else if(switches[i]->pitch < newRange.min) {
				newRange.min = switches[i]->pitch;
			}
		}
		setNoteRange(newRange);
	}
}

void ofxTLAntescofoNote::drawRectChanged(){
	guiHeaderHeight = 13;
	//ofRectangle startRect = ofRectangle(bounds.x + 100 - guiXPadding, 0, 0, guiHeaderHeight);
	ofRectangle startRect = ofRectangle(bounds.x + bounds.width - 220 - guiXPadding, 0, 0, guiHeaderHeight);
	//portInButtonBounds = getBoundsEastOf(startRect, "In: " + midiIn.getName());
	//armInButtonBounds = getBoundsEastOf(portInButtonBounds, "Arm");
	rangeLabelBounds = getBoundsEastOf(startRect, "Range");
	rangeMinSliderBounds = getBoundsEastOf(rangeLabelBounds, ofToString(noteRange.min));
	rangeMaxSliderBounds = getBoundsEastOf(rangeMinSliderBounds, ofToString(noteRange.max));
	rangeTrimButtonBounds = getBoundsEastOf(rangeMaxSliderBounds, "Trim");
	//portOutButtonBounds = getBoundsEastOf(rangeTrimButtonBounds, "Out: " + midiOut.getName());
	//armOutButtonBounds = getBoundsEastOf(portOutButtonBounds, "Arm");
}

ofRectangle ofxTLAntescofoNote::getBoundsEastOf(ofRectangle anchor, string label){
	return ofRectangle(anchor.x + anchor.width + guiXPadding, bounds.y - guiHeaderHeight, label.size() * bitmapFontSize, guiHeaderHeight);
}


