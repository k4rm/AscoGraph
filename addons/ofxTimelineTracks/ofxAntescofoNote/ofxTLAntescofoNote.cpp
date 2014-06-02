/**
 * ofxTLAntescofoNote : pianoroll/staffview for Antescofo score langage
 * Copyright (c) 2012-2013 Thomas Coffy - thomas.coffy@ircam.fr
 *
 * derived from ofxTimeline
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
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef USE_MUSICXML
// libmusicxcml/guido includes
#include "libmusicxml.h"
#include "xml.h"
#include "xmlfile.h"
#include "xmlreader.h"
#include "xml2antescofovisitor.h"
#include "antescofowriter.h"
#endif
#ifdef USE_GUIDO
# include "GuidoComponent.h"
# include "GUIDOScoreMap.h"
# include "GuidoMapCollector.h"
#endif

#include "ofMain.h"
#ifdef ASCOGRAPH_IOS
# include "iOSAscoGraph.h"
# include "ofxConsole.h"
# include "ofxTLZoomer2D.h"
#else
# include <ofxAntescofog.h>
#endif
#include "ofxTLAntescofoNote.h"
#include "ofxModifierKeys.h"

#define RT_TEMPO_VAR_OK

#include "Expression.h"
#include "Action.h"
#include "ActionCompound.h"
#include "ActionAtomic.h"
#include "Track.h"
#include <Antescofo_AscoGraph.h>
#include <Score.h>
#include <location.hh>
#include <position.hh>

#define ofGetModifierKeyShift()   ofGetModifierPressed(OF_KEY_SHIFT)

bool debug_loadscore = false;
#define DEBUG_GUIDO_ASCOGRAPH 0
#define USE_GUIDO_IMAGE_STORAGE 1
//#define USE_GUIDO_SVG 1
int bitmapFontSize = 8;
int guiXPadding = 15;

extern ofxConsole* console;
//extern unsigned int error_counter;


string str_error; // filled by our error()

bool switchsort(ofxTLAntescofoNoteOn* a, ofxTLAntescofoNoteOn* b){
	return a->beat.min < b->beat.min;
}

#ifdef ASCOGRAPH_IOS
ofxTLAntescofoNote::ofxTLAntescofoNote(iOSAscoGraph* g)
#else
ofxTLAntescofoNote::ofxTLAntescofoNote(ofxAntescofog* g)
#endif
{
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
	noteRange = ofRange(58, 88);
	color_gui_bg = ofColor(30, 110, 110);

	bounds.height = 275;
	ofxAntescofoAction = 0;
#ifdef USE_MUSICXML
	AntescofoWriter = new MusicXML2::antescofowriter();
#endif
	mDur_in_secs = 0;
	mCurBeat = -1;
	mCurSecs = 0.;
	bAutoScroll = true;
	bShowPianoRoll = true;
	bLockNotes = true;
	bMousePressed = false;
	
	mAntescofo = new antescofo_ascograph_offline();
	//cout << "================= This Name : " << mAntescofo->thisName()<< endl;
	mNetscore = 0;

#ifdef USE_GUIDO
	GuidoLayoutSettings layoutSettings;
	layoutSettings.systemsDistance = 75;
	layoutSettings.spring = 110; //1.1;
	layoutSettings.force = 750;
	layoutSettings.systemsDistribLimit = 0.25;
	layoutSettings.systemsDistribution = kAutoDistrib;
	layoutSettings.resizePage2Music = true;
	layoutSettings.neighborhoodSpacing = 0;
	layoutSettings.optimalPageFill = 0;
	oguido = new ofxGuido(layoutSettings);
	oguido->guido->setResizePageToMusic(true);
	mCurGuidoId = -1, mCurSwitchId = -1;
	guido_y = bounds.y;
	guido_x = scrolled_guido_x = 0;
#endif
}

ofxTLAntescofoNote::~ofxTLAntescofoNote(){
}

void ofxTLAntescofoNote::setup(){

	enable();
	load();	

	//ofxTLRegisterPlaybackEvents(this);

	ofAddListener(ofEvents().update, this, &ofxTLAntescofoNote::update);
	//ofAddListener(events().zoomEnded, this, &ofxTLAntescofoNote::zoomEnded);

#ifndef ASCOGRAPH_IOS
	string fontfile = ofFilePath::getCurrentExeDir() + "../Resources/DroidSansMono.ttf";
    //fontsize = 13;
    fontsize = 10;
#else
	string fontfile = "DroidSansMono.ttf";
#endif
    cout << "ofxTLAntescofoNote::setup: fontsize="<< fontsize << " fontfile = " << fontfile << endl;
    mFont.loadFont (fontfile, fontsize);

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

#ifdef USE_GUIDO
	if (!bShowPianoRoll) {
		update_guido_render();
	}
#endif
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
		case ANTESCOFO_MULTI_DUMMY:
			ofSetColor(color_note_multi, 67);
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

void ofxTLAntescofoNote::roundedRect(float x, float y, float w, float h, float r) {
	ofBeginShape();
	ofVertex(x+r, y);
	ofVertex(x+w-r, y);
	quadraticBezierVertex(x+w, y, x+w, y+r, x+w-r, y);
	ofVertex(x+w, y+h-r);
	quadraticBezierVertex(x+w, y+h, x+w-r, y+h, x+w, y+h-r);
	ofVertex(x+r, y+h);
	quadraticBezierVertex(x, y+h, x, y+h-r, x+r, y+h);
	ofVertex(x, y+r);
	quadraticBezierVertex(x, y, x+r, y, x, y+r);
	ofEndShape();
}

void ofxTLAntescofoNote::quadraticBezierVertex(float cpx, float cpy, float x, float y, float prevX, float prevY) {
	float cp1x = prevX + 2.0/3.0*(cpx - prevX);
	float cp1y = prevY + 2.0/3.0*(cpy - prevY);
	float cp2x = cp1x + (x - prevX)/3.0;
	float cp2y = cp1y + (y - prevY)/3.0;
	// finally call cubic Bezier curve function
	ofBezierVertex(cp1x, cp1y, cp2x, cp2y, x, y);
}


void ofxTLAntescofoNote::draw_showPianoRoll() {
	//cout << "ofxTLAntescofoNote::draw_showPianoRoll: "  << bounds.x <<"," << bounds.y << " " << bounds.width << "x" << bounds.height << endl;
	//************************ Range Rows
	float rowHeight = bounds.height / (noteRange.span()+1);

	ofFill();
	// draw piano roll
	int piano_width = 20;
	for (int i = noteRange.max; i >= noteRange.min; i--) {
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

	// draw notes
	if(switches.size() > 0) {
		float zoomMinX = bounds.x;
		float zoomMaxX = bounds.x + bounds.width;
		for(int i = 0; i < switches.size(); i++){
			if(isSwitchInBounds(switches[i])){
				float startX =  max(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds), zoomMinX);
				float endX = min(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds), zoomMaxX);

				// manage rest notes
				if (switches[i]->pitch != 0) {
					if (abs(switches[i]->pitch) > noteRange.max || abs(switches[i]->pitch) < noteRange.min) continue; // don't draw note outside range
					ofFill();

					setNoteColor(i);
					// set color to transparent when selected
					if(switches[i]->startSelected && switches[i]->endSelected) ofSetColor(color_note_selected);
					if (abs(switches[i]->pitch) && switches[i]->beat.span() == 0) ofSetColor(color_note, 100); // grace note
					int whichRow = ofMap(abs(switches[i]->pitch), noteRange.max+1, noteRange.min-1, 0, noteRange.span()+1);
					//cout << "min:"<< noteRange.min<< " max:"<< noteRange.max << " span:" << noteRange.span() << " pitch= " << abs(switches[i]->pitch) << " row:" << whichRow << endl;
					ofRectangle noteBounds = ofRectangle(startX, bounds.y + whichRow * rowHeight, endX - startX, rowHeight);

					// note heads
					float y = bounds.y + whichRow * rowHeight;
					float w = endX - startX;
					float h = rowHeight;
					// rounded rect note heads
					int rr = 3;
					if (endX - startX >= rr) { 
						roundedRect(startX, y, endX - startX, rowHeight, rr);
						ofNoFill();
						ofSetColor(0, 0, 0, 200);
						roundedRect(startX, y, endX - startX, rowHeight, rr); // black border around note
					} else { // when note no small, just display rect
						ofRectangle r(startX, y, endX - startX, rowHeight);
						ofRect(r);// et un rect entre les deux
						ofNoFill();
						ofSetColor(0, 0, 0, 200);
						ofRectangle rb(startX, y, endX - startX, rowHeight); // black border around note
						ofRect(rb);
					}
#if 0 // not yet missed events
					// missed events
					if (mLastBeat >= switches[i]->beat.min && switches[i]->missed) {
						cout << "Displaying missed event !" << endl;
						ofSetColor(25, 40, 40, 100);
						ofSphere(50,50,-10,40);
						ofSetColor(255, 0, 0, 200);
					}
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
#if 0
					if (switches[i]->type == ANTESCOFO_MULTI && i+1 < switches.size()
							&& (switches[i+1]->type == ANTESCOFO_MULTI || switches[i+1]->type == ANTESCOFO_MULTI_STOP)) {
						cout << "draw multi line" << endl;
						setNoteColor(i);
						// next note:
						float startX2 =  normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.min), zoomBounds);
						float endX2 = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.max), zoomBounds);
						int whichRow2 = ofMap(abs(switches[i+1]->pitch), noteRange.max, noteRange.min, 0, noteRange.span());
						/*
						ofRectangle noteBounds2 = ofRectangle(startX2, bounds.y + whichRow2 * rowHeight, endX2 - startX2, rowHeight);

						ofLine(noteBounds.x + noteBounds.width/2, noteBounds.y + noteBounds.height/2,
								noteBounds2.x + noteBounds2.width/2, noteBounds2.y + noteBounds2.height/2);
								*/
					}
					if (switches[i]->type == ANTESCOFO_MULTI || switches[i]->type == ANTESCOFO_MULTI_STOP) {
						setNoteColor(i);
					}
#endif
				} else if (switches[i]->type == ANTESCOFO_REST) { // draw a rest note
#if 1
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
#endif
				}
				if (switches[i]->action.size()) {
					// ?
				}
			}
		} // end switch loop

		// draw markers (separate loop used for calculate width)
		float lastX = bounds.x + bounds.width;
		int sizec = mFont.stringWidth(string("_"));
		for(int i = switches.size() - 1; i >= 0; i--){
			if(isSwitchInBounds(switches[i])){
				float startX =  max(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds), zoomMinX);
				float endX = min(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds), zoomMaxX);

				// measure bar
				if (!switches[i]->label.empty()) {
					ofSetColor(0, 0, 0, 85);
					ofSetLineWidth(1);
					ofLine(startX, bounds.y, startX, bounds.y + bounds.height);
					float w = lastX - startX;
					int l = floor( (w+1) / (sizec ));
					int s = switches[i]->label.size();
					if (l > s) l = s;
					string str = switches[i]->label.substr(s-l, l);
					ofSetColor(0, 0, 0, 255);
					//ofDrawBitmapString( str, startX, bounds.y-5);
					mFont.drawString( str, startX, bounds.y-5);
					lastX = startX;
				}
			}
		} 
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

#ifdef USE_GUIDO
class MapGuidoObject : public MapCollector {
public:
	virtual ~MapGuidoObject() {}
	void Graph2TimeMap (const FloatRect& box, const TimeSegment& dates, const GuidoElementInfos& infos )
	{
#ifdef DEBUG_GUIDO_ASCOGRAPH
		//cout    << "map collection: " << box << " dates: " << dates << " type: " << infos.type << ", voice=" << infos.voiceNum << ", staff=" << infos.staffNum  << endl;
#endif
		Guido_notes n;
		n.min = 4. * dates.first.num / dates.first.denom;
		n.max = 4. * dates.second.num / dates.second.denom;
		n.infos = infos;
		notes.push_back(n);
	}
	typedef struct {
		float min, max;
		GuidoElementInfos infos;
	} Guido_notes;
	vector< Guido_notes > notes;

};


// fix switchId2guidoId
void ofxTLAntescofoNote::checkSwitchId2GuidoId(Time2GraphicMap& outmap)/*, MapGuidoObject& mapobj)*/ {
	// for every switch find its beat matching guidoId
	//for (int switchid = 0; switchid < switches.size() && guidoid < mapobj.notes.size(); switchid++, guidoid++) {
	//}
	for (int i = 0; i < switchId2guidoId.size(); i++) {
		int g = switchId2guidoId[i];
	}

	cout << "------------------------- Displaying switchId2GuidoId map:" << endl;
	for (int switchid = 0, guidoid = 0;  switchid < switches.size() && guidoid < outmap.size(); switchid++) {
		cout << "switchId2guidoId[ " << switchid << " ]= " << guidoid << endl;
		// if switch beat matches outmap[guidoid].first time segment that's fine,
		float b_inf = 4. * float(outmap[guidoid].first.first.num)/float(outmap[guidoid].first.first.denom);
		float b_sup = 4. * float(outmap[guidoid].first.second.num)/float(outmap[guidoid].first.second.denom);
		if (switches[switchid]->beat.min <= b_inf + BEAT_EPSILON && switches[switchid]->beat.max >= b_sup - BEAT_EPSILON) {
			cout << "OK beat corresponds to Time2GraphicMap: note #"<< switchid << " and Guido: #" << guidoid << " while comparing switch=[ "
				<< switches[switchid]->beat.min << "-" << switches[switchid]->beat.max << " ] with Guido:[ " << b_inf << "-" << b_sup << " ]"<< endl;
			switchId2guidoId[switchid] = guidoid;
			continue;
		} else {
			cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: Error at note #"<< switchid << " and Guido: #" << guidoid << " while comparing switch=[ "
				<< switches[switchid]->beat.min << "-" << switches[switchid]->beat.max << " ] with Guido:[ " << b_inf << "-" << b_sup << " ]"<< endl;
			if (switchid) { 
				if (switches[switchid]->beat.max < b_inf + BEAT_EPSILON) {
					cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: too late, skipping." << endl;
					if (guidoid > 0) guidoid--;
					continue;
				} else if (switches[switchid]->beat.min >= b_sup - BEAT_EPSILON)
					guidoid++;
				else {
					cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: fainting OK." << endl;
					switchId2guidoId[switchid] = guidoid;
					continue;
				}
				switchid--;
			}
		}
	}
#if 0
	int guidoid = 0;
	for (int switchid = 0; switchid < switches.size() && guidoid < mapobj.notes.size(); switchid++, guidoid++) {
		bool doit = false;
		if (switches[switchid]->beat.min == mapobj.notes[guidoid].min && switches[switchid]->beat.max == mapobj.notes[guidoid].max) {
			doit = true;
		} else {
			if (switches[switchid]->beat.min < mapobj.notes[guidoid].min && switches[switchid]->beat.max <= mapobj.notes[guidoid].max) {
				guidoid--;
				switchId2guidoId[switchid] = -1;
				continue;
			} else if (switches[switchid]->beat.min > mapobj.notes[guidoid].min && switches[switchid]->beat.max > mapobj.notes[guidoid].max) {
			}

			if (switchid) switchid--;
			cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: Error at note #"<< switchid << " and Guido: #" << guidoid << " while comparing "
				<< switches[switchid]->beat.min << " with " << mapobj.notes[guidoid].min << "    and " << switches[switchid]->beat.max << " with " << mapobj.notes[guidoid].max << endl;
		}

		if (doit) {
			cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: setting " << switchid << " <=> Guido: " << guidoid << endl;
			switchId2guidoId[switchid] = guidoid;
		}
	}

#endif
#if 0
	int deltai = 0, lastdeltai = 0;
	int nberror = 0;
	for (int i = 0; i < outmap.size() && i < switches.size(); i++) {
		if (switches[i]->beat.min == mapobj.notes[i+deltai].min && switches[i]->beat.max == mapobj.notes[i+deltai].max) {
			nberror = 0;
			lastdeltai = deltai;
			continue;
		} else {
			cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: Error at note #"<< i << " " << switches[i]->beat.min << " with " << mapobj.notes[i+deltai].min << "    and " << switches[i]->beat.max << " and " << mapobj.notes[i+deltai].max << endl;
			i--;
			deltai++;
			cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: trying with deltai=" << deltai << endl;
			nberror++;

			if (nberror > 10) {
				cout << "ofxTLAntescofoNote::checkSwitchId2GuidoId: Giving up finding mapping, deltai=" << deltai << endl;
				nberror = 0;
				i++; deltai = lastdeltai;
				continue;
			}
		}
	}
#endif
}



void ofxTLAntescofoNote::guido_store_notes() {
	// retrieve time2timemap
	//guido::GuidoMapCollector mapcol(oguido->guido->getGRHandler(), kGuidoEvent);
	MapGuidoObject mapobj;
	GuidoGetMap(oguido->guido->getGRHandler(), 1, oguido->getWidth(), oguido->getHeight(), kGuidoEvent, mapobj);

	// retrieve graphic2time mapping
	Time2GraphicMap outmap;
	int err = GuidoGetSystemMap(oguido->guido->getGRHandler(), 1, oguido->getWidth(), oguido->getHeight(), outmap);

	if (outmap.empty())
		return;

	for (int i = 0; i < outmap.size(); i++) {
		cout << "guido_store_notes i=" << i << " dates: " << outmap[i].first.first.num << "/" << outmap[i].first.first.denom << " - " << outmap[i].first.second.num << "/" << outmap[i].first.second.denom << endl;
	}

	checkSwitchId2GuidoId(outmap);

	int v = 0;
	for (int i = 0; i < switches.size(); i++) {
		/*while (v != switches[i]->notenum) {
			//cout << "skipping v = " << v << " notenum= " << switches[i]->notenum << endl;
			v++;
		}*/
		v = switchId2guidoId[i];

		float x = outmap[v].second.left;
		float y = outmap[v].second.top;
		float w = outmap[v].second.right - x;
		float h = outmap[v].second.bottom - y;

#ifdef DEBUG_GUIDO_ASCOGRAPH
		//cout << "GuidoGetSystemMap: storing notenum=" << v << " i=" << i << " x=" << x << " y=" << y << " w=" << w << " h=" << h << endl;
		//cout << "GuidoGetSystemMap: storing notenum=" << v << " i=" << " r=" << outmap[v].second.right << " b=" << outmap[v].second.bottom << endl;
#endif
		if (w < 0 || outmap[v].second.right < 0) w = 70;
		switches[i]->guidoCoords.x = x;
		switches[i]->guidoCoords.y = y;
		switches[i]->guidoCoords.width = w;
		switches[i]->guidoCoords.height = h;

		//if (v == outmap.size()) v = 0;
	}
}


void ofxTLAntescofoNote::update_guido_render() {
	render_guido(1.);

	//mDur_in_beats = timeline->normalizedXToBeat( timeline->screenXtoNormalizedX(guido_w, ofRange(0., 1.)));
	mDur_in_secs = 60 / timeline->getBPM() * mDur_in_beats;
	cout << "Update guido duration ------------------ " << mDur_in_beats << " beats."<< endl;
	cout << "Update guido duration ------------------ " << mDur_in_secs << " seconds.";
	mDur_in_secs++; // add one beat for better display
	timeline->setDurationInSeconds(mDur_in_secs);

	// set zoom:
	float r = bounds.width / guido_w;
	ofxTLZoomer2D *zoom = (ofxTLZoomer2D*)timeline->getZoomer();
	r = ofClamp(r, 0., 1.);
	cout << "Update guido : setting new zoom range: 0 - " << r << endl;

	ofRange z(0., r);
	zoom->setViewRange(z);
}
bool ofxTLAntescofoNote::render_guido(float xfactor)
{
	if (oguido) {
		delete oguido;
		GuidoLayoutSettings layoutSettings;
		layoutSettings.systemsDistance = 75;
		layoutSettings.spring = 110; //1.1;
		layoutSettings.force = 750;
		layoutSettings.systemsDistribLimit = 0.25;
		layoutSettings.systemsDistribution = kAutoDistrib;
		layoutSettings.resizePage2Music = true;
		layoutSettings.neighborhoodSpacing = 0;
		layoutSettings.optimalPageFill = 0;
		oguido = new ofxGuido(layoutSettings);
		oguido->guido->setResizePageToMusic(true);
	}
	int backingWidth, backingHeight;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
	cout << endl << "==================================> RENDER BUFFER dimension: " << backingWidth << " x "<< backingHeight << endl<<endl;

	float startX = 0.;
	float endX = bounds.x + bounds.width;
	int starti = 0, endi = switches.size() - 1;

	guido_string = getGuidoString(startX, starti, endX, endi);
#ifdef DEBUG_GUIDO_ASCOGRAPH
	cout << "Converted score to Guido code: \"" << guido_string<< "\"" << endl;
#endif
	GuidoPageFormat format;
	oguido->getPageFormat(format);
	format.marginleft = 0.;
	format.marginright = 0.;
	format.margintop = 0.;
	format.marginbottom = 0.;

	GuidoSetDefaultPageFormat(&format);

	int err = oguido->compile_string(guido_string.c_str());
#ifdef DEBUG_GUIDO_ASCOGRAPH
	cout << "Guido returned err:" << err << endl;
#endif
	if (!err) {
		cerr << "Guido returned err:" << err << endl;
		return false;
	}
	guido_h = bounds.height;
	guido_span = zoomBounds.span();

	//oguido->setWidth(bounds.width); oguido->setHeight(bounds.height);


	oguido->getPageFormat(format);
	mRatioGuido = (format.width / format.height);

	ofPushStyle();
	ofSetColor(0,0,0, 255);

#ifdef DEBUG_GUIDO_ASCOGRAPH
	cout << "Margin : " << format.marginleft << " <-> " << format.marginright << endl;
	cout << "guido page rect: " << format.width << " x "<< format.height << " ratio=" << mRatioGuido << endl;
	cout << "bounds rect: " << bounds.width << " x "<< bounds.height << endl;
#endif
	float render_x = 0., render_y = 0.;
	float render_h = bounds.height;
	float render_w = render_h * mRatioGuido;
	cout << "Setting Guido Size: " << render_w << " x " << render_h << endl;

	oguido->setSize(render_w, render_h);

	int fbow = oguido->guido->getDevice()->drawCache.getWidth();
	int fboh = oguido->guido->getDevice()->drawCache.getHeight();
	//oguido->setWidth(fbow); oguido->setHeight(fboh);
#ifdef DEBUG_GUIDO_ASCOGRAPH
	cout << "Rendering Guido: " << render_x << ", " << render_y << " : " << render_w << " x " << render_h << endl;
	cout << "Guido surface dimension: " << fbow << " x " << fboh << endl;
#endif
#ifndef USE_GUIDO_IMAGE_STORAGE
	oguido->draw(render_x, render_y, render_w, render_h);
#endif
	ofPopStyle();

	guido_w = render_w;
	guido_h = render_h;

#ifdef USE_GUIDO_IMAGE_STORAGE
	oguido->setScale(1., 1.);
	for (int i = 0; i < guido_images.size(); i++) {
		if (guido_images[i].isAllocated())
			guido_images[i].clear();
	}
	guido_images.clear();
	// copy into an ofImage:   Internal vs screen
	//   				format.width -> render_w
	// 				x -> 1024
	int stepx = (fbow*format.width)/render_w;
	int stepy = (fboh*format.height)/render_h;
	for (int i = 0; i*stepx < format.width; i++) {
		ofImage img;
		img.allocate(fbow, fboh, OF_IMAGE_COLOR_ALPHA);

		oguido->setScale(1., 1.);
		oguido->guido->getDevice()->SetOrigin(i*stepx, 0);
		oguido->draw(0, render_y, render_w, render_h);

		oguido->guido->getDevice()->drawCache.getTextureReference().readToPixels( img.getPixelsRef() );
		img.update();
		//img.saveImage(string("/tmp/img") + ofToString(i) + ".png");
		guido_images.push_back(img);
		//if (debug_guido) cout << "COPIED I=" << i << " fbow= " << fbow << " imgw=" << img.getWidth() << endl;
	}
#endif

	// once drawn to FBO, build a map of beat to switchId: beat2switchId
	beat2switchId.clear();
	for (int i = 0; i < switches.size(); i++)
		beat2switchId[switches[i]->beat.min] = i;

	// store note mapping
	guido_store_notes();
	return true;
}


void ofxTLAntescofoNote::draw_guido() {
	if (oguido) {
		if (bounds.height != guido_h) 
			render_guido(1.);
		ofPushStyle();

		ofSetColor(255, 255, 255, 255);
		ofFill();
		ofRect(bounds);

		//WAS guido_x = bounds.x - (zoomBounds.min * guido_w);
		guido_y = bounds.y;
#ifdef DEBUG_GUIDO_ASCOGRAPH
		cout << "draw_guido: guido_x = " << guido_x << " guido_y = " << guido_y << endl;
#endif
		ofSetColor(0,0,0, 255);
		GuidoPageFormat format;
		oguido->getPageFormat(format);
		mRatioGuido = (format.width / format.height);
		float render_h = bounds.height;
		float render_w = render_h * mRatioGuido;
#if 0
		oguido->draw(guido_x, guido_y, render_w, render_h);
		oguido->draw_cache(bounds.x, bounds.y);
#endif
#ifdef USE_GUIDO_IMAGE_STORAGE
		if (guido_images.empty()) return;
		//WAS guido_image.drawSubsection(bounds.x, bounds.y, render_w, render_h, guido_x, 0);
		int fbow = guido_images[0].getWidth();
		int fboh = guido_images[0].getHeight();
		int nbimages = guido_images.size();
		int totalw = fbow * nbimages;
		// find right guido images to display:
#ifdef DEBUG_GUIDO_ASCOGRAPH
		//cout << "nbimages=" << nbimages << " totalw=" << totalw << " fbow=" << fbow << endl;
#endif
		if (guido_x > 0) {
			int firstn = guido_x * nbimages / totalw;
			int secondn = firstn+1;
			int firstx = guido_x % fbow;
			int firstw = fbow - firstx;
			int secondx = bounds.x + firstw;
			int secondw = fbow - firstw;
#ifdef DEBUG_GUIDO_ASCOGRAPH
			//cout << "Draw guido_images: firstn=" << firstn << " 1stx="<< firstx << " firstw=" << firstw << endl;
#endif
			guido_images[firstn].drawSubsection(bounds.x, bounds.y, firstw, render_h, firstx, 0);
			if (firstx > 0 || firstw <= bounds.width) {
#ifdef DEBUG_GUIDO_ASCOGRAPH
				//cout << "-- 2ndn=" << secondn << " 2ndx="<< secondx << " 2ndw=" << secondw << " switches.size=" <<switches.size()<< endl;
#endif
				if (secondn < nbimages) {
					guido_images[secondn].drawSubsection(secondx, bounds.y, secondw, bounds.height, 0, 0);
				} else cout << "!!!!!!!!!!!!! draw_guido: secondn=" << secondn << " >= nbimages=" << nbimages << endl;
			}
		} else {
			guido_images[0].drawSubsection(bounds.x, bounds.y, render_w, render_h, 0, 0);
		}
#endif

#if 0
		ofFill();
		for (int i = 0; i < beat2switchId.size(); i++) {
			if (i % 2)
				ofSetColor(200, 0,0, 40);
			else
				ofSetColor(200, 200,0,40);
			ofRectangle r = switches[switchId2guidoId[beat2switchId[i]]]->guidoCoords;
			//r.x -= (guido_x + bounds.x);
			r.y += bounds.y;
			ofRect(r);
		}
#endif

		ofPopStyle();
	}
}

string ofxTLAntescofoNote::getGuidoStringNoteName(int pitch) {
	if (!pitch) return string("_");
	pitch = abs(pitch);
	if (pitch > 1000) pitch /= 100;
	int p = pitch % 12;
	int o = 0;
	if (pitch >= 12 && pitch <= 23) o = 0;
	else if (pitch >= 24 && pitch <= 35) o = 1;
	else if (pitch >= 36 && pitch <= 47) o = 2;
	else if (pitch >= 48 && pitch <= 59) o = 3;
	else if (pitch >= 60 && pitch <= 71) o = 4;
	else if (pitch >= 72 && pitch <= 83) o = 5;
	else if (pitch >= 84 && pitch <= 95) o = 6;
	else if (pitch >= 96 && pitch <= 107) o = 7;
	else if (pitch >= 108 && pitch <= 120) o = 8;
	o -= 3;
	string c;
	if (p == 0) c = "c";
	else if (p == 1) c = "c#";
	else if (p == 2) c = "d";
	else if (p == 3) c = "d#";
	else if (p == 4) c = "e";
	else if (p == 5) c = "f";
	else if (p == 6) c = "f#";
	else if (p == 7) c = "g";
	else if (p == 8) c = "g#";
	else if (p == 9) c = "a";
	else if (p == 10) c = "a#";
	else if (p == 11) c = "b";
	c += ofToString(o);
	return c ;
}

string ofxTLAntescofoNote::getGuidoStringNote(int switchnb) {
	static bool bInChord = false;
	string ret;
	if (switches[switchnb]->type == ANTESCOFO_NOTE || switches[switchnb]->type == ANTESCOFO_CHORD
	    || switches[switchnb]->type == ANTESCOFO_REST || switches[switchnb]->type == ANTESCOFO_MULTI
	    || switches[switchnb]->type == ANTESCOFO_MULTI_STOP || switches[switchnb]->type == ANTESCOFO_MULTI_DUMMY
	    || (switches[switchnb]->type == ANTESCOFO_TRILL && switches[switchnb]->pitch)) {
		double dur = switches[switchnb]->duration;
		if (dur < 0.01) return "";
		//cout << "converting duration: " << dur << endl;
		rational rdur(0, 1);
		for (int i = 1; i < 1000; i++)
		{
			double d2 = i * dur;
			double dnear = rint(d2);
			double err = d2 - dnear;
			if (fabs(err) < BEAT_EPSILON)
			{
				if (1 == i) 
					//return out << (int)(dnear);
					rdur = rational((int)(dnear), 1);
				else
					//return out << (int)(dnear) << "/" << i;
					rdur = rational((int)(dnear), i);
			}
			//cout << "looping i=" << i << " rdur = " << rdur.toFloat() << " err="<< err << " dnear=" << dnear << endl;
		}
		rdur /= 4; // because of guido...
		rdur.rationalise();
		string sdur = rdur.toString();

		ret += getGuidoStringNoteName(switches[switchnb]->pitch) + "*" + rdur.toString();
		//if (debug_guido) cout << "getGuidoStringNoteName( " << switches[switchnb]->pitch << ", dur=" << switches[switchnb]->duration << ") = '" << ret <<"'"<<endl;
	}
	return ret;
}

// return event if curEvent+1 switch is tied to curPitch
int ofxTLAntescofoNote::has_next_event_tied_pitch(int curEvent, int curPitch) {
	if (curEvent + 1 >= switches.size()) return 0;
	bool res = false;
	// check following events, until nextBeat is different
	float nextBeat = switches[curEvent]->beat.max;
	int i = 1;

	while (!res) {
		if (curEvent + i >= switches.size()) return 0;
		if (switches[curEvent+i]->beat.min <= nextBeat) { // while on next event
			if (switches[curEvent+i]->pitch == curPitch && switches[curEvent+i]->is_tied) {
				//cout << "has_next_event_tied_pitch: " << curEvent+i << endl;
				return curEvent+i;
			} else {
				i++;
				continue;
			}
		} else break;
	}
	return 0;
}

void ofxTLAntescofoNote::add_string_staff(string &str1, string &str2, int staff, string str) {
	if (staff == 1)
		str1 += str;
	else if (staff == 2)
		str2 += str;
}

string ofxTLAntescofoNote::getGuidoString(int fromx, int fromi, int tox, int toi) {
	int pitch_limit_between_staves = 58;
#ifdef DEBUG_GUIDO_ASCOGRAPH
	cout << "Getting Guido string from note " << fromi << " to " << toi << endl;
#endif
	string ret1 = "[ ", ret2 = ret1;
	int pageFormat_w = mDur_in_beats * 1.3;// MIN(switches.size() / 8, 20); // TODO utiliser le maxbeat dur ?
	if (!pageFormat_w) pageFormat_w = 20;
	ret1 +=	"\\pageFormat<" + ofToString(pageFormat_w) + "cm, 15cm, 0cm, 1cm, 0cm, 1cm>";
	if (fromi == 0) {
		ret1 += "\\clef<\"g\">\n";
	} else 
		ret1 += "\\clef<\"g\">\n";
	int d = tox - fromx;
	int maxx = ( tox - fromx > bounds.width ? bounds.width : d);
	int curx = fromx;
	bool twostaves = false;
	bool backtostaff1 = false;
	bool isChord1 = false, isChord2 = false, isGrace = false;
	ofRange staffbeat1(0, 0), staffbeat2(0, 0);
	bool was_tied = false;
	bool was_trill1 = false, was_trill2 = false, was_multi = false;
	int curStaff = 1, guidoId = 0;
	for (int i = fromi; i <= toi; i++) {
		if (switches[i]->pitch && abs(switches[i]->pitch) < pitch_limit_between_staves) {
			curStaff = 2;
			if (!twostaves) {
				add_string_staff(ret1, ret2, curStaff, "\\staff<2>\\clef<\"f\">\\stemsDown\n");
				twostaves = true;
			}
		} else curStaff = 1;

		if (switches[i]->type == ANTESCOFO_NOTE && switches[i]->duration == 0.) { // grace notes
			isGrace = true;
			add_string_staff(ret1, ret2, curStaff, "\\grace(");
		} else if (switches[i]->type == ANTESCOFO_CHORD) {
			if (curStaff == 1) {
				if (!isChord1) {
					add_string_staff(ret1, ret2, 1, " {");
					isChord1 = true; 
					if (!isChord2) guidoId++;
				} else add_string_staff(ret1, ret2, curStaff, ", ");
			} else if (twostaves && curStaff == 2) {
				if (!isChord2) {
					add_string_staff(ret1, ret2, 2, " {");
					isChord2 = true;
					if (!isChord1) guidoId++;
				} else add_string_staff(ret1, ret2, curStaff, ", ");
			}
		}
		else if (switches[i]->type == ANTESCOFO_TRILL && switches[i]->pitch) {
			if (curStaff == 1 && !was_trill1) { add_string_staff(ret1, ret2, curStaff, "\\trill({"); was_trill1 = true;}
			else if (curStaff == 2 && !was_trill2) { add_string_staff(ret1, ret2, curStaff, "\\trill({"); was_trill2 = true;}
			else if (i && switches[i-1]->type == ANTESCOFO_TRILL && switches[i-1]->pitch && switches[i-1]->beat.min == switches[i]->beat.min)
				add_string_staff(ret1, ret2, curStaff, ",");
			else add_string_staff(ret1, ret2, curStaff, "{");
		} else if (switches[i]->type == ANTESCOFO_MULTI && switches[i]->pitch) {
			if (!was_multi) {
				add_string_staff(ret1, ret2, curStaff, "\\glissando<fill=\"true\">(");
				was_multi = true;
				/*if (switches[i+1]->type == ANTESCOFO_MULTI && switches[i+1]->beat.min == switches[i]->beat.min) { // handle multi of chords
					add_string_staff(ret1, ret2, curStaff, "{");
					was_multi_chord = true;
				}*/
			} 
			//if (i && switches[i-1]->type == ANTESCOFO_MULTI && switches[i-1]->pitch && switches[i-1]->beat.min == switches[i]->beat.min)
				//add_string_staff(ret1, ret2, curStaff, ",");
		} else if (switches[i]->type == ANTESCOFO_MULTI_STOP) {
			if (was_multi) { i++; while (i < toi && switches[i]->type == ANTESCOFO_MULTI_DUMMY) i++; } // skip dummies 
		}
		int istied = 0;
		if ((istied = has_next_event_tied_pitch(i, switches[i]->pitch)))
			add_string_staff(ret1, ret2, curStaff, " \\tieBegin ");

		//XXX cout << "i= " << i  << " type= " << switches[i]->type << endl;
		// get Guido string for current note
		if (switches[i]->type != ANTESCOFO_MULTI_DUMMY) {
			if (isGrace)
				add_string_staff(ret1, ret2, curStaff, getGuidoStringNoteName(switches[i]->pitch));
			else {
				add_string_staff(ret1, ret2, curStaff, getGuidoStringNote(i));
				if (curStaff == 1) staffbeat1 = switches[i]->beat.max;
				else staffbeat2 = switches[i]->beat.max;
			}
			if (switches[i]->type != ANTESCOFO_CHORD) guidoId++;
		}

		if (isGrace) { add_string_staff(ret1, ret2, curStaff, ")"); isGrace = false; }
		
		if (!istied && switches[i]->is_tied) {
			add_string_staff(ret1, ret2, curStaff, " \\tieEnd ");
		}
		if (switches[i]->pitch >= pitch_limit_between_staves)
			backtostaff1 = true;

		/*XX cout << "FUUUUUUUCK: " << endl;
		XXX cout << "\twas_multi = " << was_multi << endl;
		if (was_multi && i != switches.size()-1) {
			cout << "\tswitches[i+1]->beat.min = " << switches[i+1]->beat.min << "  switches[i]->beat.min = " << switches[i]->beat.min << endl;
			cout << "\tswitches[i+1]->type = " << switches[i+1]->type << endl;
		}*/
		if (switches[i]->type == ANTESCOFO_TRILL && switches[i]->pitch && (i == switches.size()-1 || (switches[i+1]->type != ANTESCOFO_TRILL || (switches[i+1]->beat.min != switches[i]->beat.min)))) {
			if (was_trill1) { add_string_staff(ret1, ret2, 1, "})"); was_trill1 = false; }
			if (was_trill2) { add_string_staff(ret1, ret2, 2, "})"); was_trill2 = false; }
		} else if (was_multi && (i == switches.size()-1 || (switches[i+1]->beat.min != switches[i]->beat.min && switches[i+1]->type != ANTESCOFO_MULTI_STOP && switches[i+1]->type != ANTESCOFO_MULTI_DUMMY))) {
			add_string_staff(ret1, ret2, curStaff, ")");
			was_multi = false;
		} else {

			if (i+1 == switches.size() || (switches[i]->type == ANTESCOFO_CHORD && switches[i+1]->beat.min != switches[i]->beat.min)) { // last note of a CHORD
				if (isChord1) {
					add_string_staff(ret1, ret2, 1, " }");
					//cout << "--------------> closing CHORD 1: " << ret1 << endl;
					isChord1 = false;
				}
				if (twostaves && isChord2) {
					add_string_staff(ret1, ret2, 2, " }");
					//cout << "--------------> closing CHORD 2: " << ret2 << endl;
					isChord2 = false;
				}
			}
		}
		if (i != toi) add_string_staff(ret1, ret2, curStaff, " ");
		//if (backtostaff1 && curStaff != 1) { add_string_staff(ret1, ret2, curStaff, "\\staff<1> "); backtostaff1 = false; curStaff = 1;}

		switchId2guidoId[i] = guidoId-1;
#ifdef DEBUG_GUIDO_ASCOGRAPH
		//cout << "################ Set switchId2guido["<<i<<"] = " << guidoId-1 << endl;
#endif

		// advance the other staffbeat position
		if (i+1 == switches.size()) continue; // last elm, we're done

		int nextnotestaf;
		if (switches[i+1]->pitch && abs(switches[i+1]->pitch) < pitch_limit_between_staves)
			nextnotestaf = 2;
		else nextnotestaf = 1;

		float bmax = switches[i]->beat.max;
		float bmin = switches[i]->beat.min;
#if 0
		if (debug_guido) cout << "DEBUG DE LA MUERTA: pitch=" << switches[i]->pitch << "---> nextnotestaff=" << nextnotestaf << " i=" << i << " switchessize=" 
		   << switches.size() <<" bmax=" << bmax.toFloat() << " 1st=" << ((i < switches.size() && (switches[i+1]->beat.min > switches[i]->beat.min || (switches[i]->type==ANTESCOFO_CHORD && switches[i+1]->type==ANTESCOFO_CHORD))))
		   << " 2nd=" << ((nextnotestaf == 1 && bmax != staffbeat1) || (nextnotestaf == 2 && bmax != staffbeat2)) << endl; 
		   if (debug_guido)
		   if (!((nextnotestaf == 1 && bmax != staffbeat1) || (nextnotestaf == 2 && bmax != staffbeat2))) cout  << " staffbeat1=" << staffbeat1.toFloat() << " staffbeat2="
		   << staffbeat2.toFloat() << endl << endl;
		   /*if (debug_guido) cout << "------> bmax=" << bmax.toFloat() << " bmin=" << bmin.toFloat() << " curStaff=" << curStaff << " staffbeat1=" 
		     << staffbeat1.toFloat() << " staffbeat2=" << staffbeat2.toFloat() << " dif=" << dur << endl;*/
		if (((i < switches.size() 
			&& (switches[i+1]->beat.min > switches[i]->beat.min || (switches[i]->type==ANTESCOFO_CHORD && nextnotestaf==2)))) // next event beat is another beat or last event
			&& ((nextnotestaf == 1 && bmax != staffbeat1) || (nextnotestaf == 2 && bmax != staffbeat2))) {
#endif		
		// next note is on different beat, but silence is needed to be added on nextnotestaff
		bool shouldAddMissingForNextBeat = (switches[i+1]->beat.min > switches[i]->beat.min && ((nextnotestaf == 1 && bmax != staffbeat1.max) || (nextnotestaf == 2 && bmax != staffbeat2.max)));
		// next note is on same beat, but silence is needed to be added on nextnotestaff
		bool shouldAddMissingForSameBeat = (switches[i+1]->beat.min == switches[i]->beat.min && ((nextnotestaf == 1 && bmin > staffbeat1.min) || (nextnotestaf == 2 && bmin > staffbeat2.min)));
#ifdef DEBUG_GUIDO_ASCOGRAPH
		if (shouldAddMissingForNextBeat) cout << "---------> shouldAddMissingForNextBeat" << endl;
		if (shouldAddMissingForSameBeat) cout << "-------> shouldAddMissingForSameBeat" << endl;
#endif

		if (i < switches.size() && (shouldAddMissingForNextBeat || shouldAddMissingForSameBeat)) {

			double dur;
			if (shouldAddMissingForNextBeat)
				dur = bmax - (nextnotestaf == 1 ? staffbeat1.max : staffbeat2.max);
			else if (shouldAddMissingForSameBeat)
				dur = bmin - (nextnotestaf == 1 ? staffbeat1.min : staffbeat2.min);
			rational rdur(0, 1);
			for (int j = 1; j < 64; j++)
			{
				double d2 = j * dur;
				double dnear = rint(d2);
				double err = d2 - dnear;
				if (fabs(err) < BEAT_EPSILON)
				{
					rdur = rational((int)(dnear), j);
				}
			}
#ifdef DEBUG_GUIDO_ASCOGRAPH
			cout << "------> Adding rest: " << rdur.toString() << endl;
#endif
			rdur /= 4; // because of guido...
			rdur.rationalise();

			if (rdur.toFloat() < 0) continue;
			guidoId++;

			if (staffbeat2.max == 0.)
			if (!twostaves) {
				add_string_staff(ret1, ret2, nextnotestaf, "\\staff<2>\\clef<\"f\">\n");
				twostaves = true;
			}
			add_string_staff(ret1, ret2, nextnotestaf, "_*" + rdur.toString() + " ");
			staffbeat1 = staffbeat2 = bmax;
		}
	}

	curStaff = 1;
	add_string_staff(ret1, ret2, curStaff, " ]");
	if (twostaves)
		ret1 = "{" + ret1 + ",\n" + ret2 + "]}";

	cout << "------------------------- After get_Guido_string: Displaying switchId2GuidoId map:" << endl;
	for (int switchid = 0; switchid < switches.size(); switchid++) {
		cout << "switchId2guidoId[ " << switchid << " ]= " << switchId2guidoId[switchid] << endl;
	}
	return ret1;
}
#endif

#if 0
void ofxTLAntescofoNote::zoomStarted(ofxTLZoomEventArgs& zoomEvent)
{
	if (debug_guido) cout << "ofxTLAntescofoNote::zoomStarted : " << zoomEvent.currentZoom.min << " - " << zoomEvent.currentZoom.max << endl;
}

void ofxTLAntescofoNote::zoomDragged(ofxTLZoomEventArgs& zoomEvent)
{
	if (debug_guido) cout << "ofxTLAntescofoNote::zoomDragged : " << zoomEvent.currentZoom.min << " - " << zoomEvent.currentZoom.max << endl;
}

void ofxTLAntescofoNote::zoomEnded(ofxTLZoomEventArgs& zoomEvent)
{
	if (debug_guido) cout << "ofxTLAntescofoNote::zoomEnded : " << zoomEvent.currentZoom.min << " - " << zoomEvent.currentZoom.max << endl;
}
#endif

int ofxTLAntescofoNote::note_height() {
	return 2 * bounds.height / (noteRange.span()+1);
}

int ofxTLAntescofoNote::note_pitch2y(int p) {
	int y = 0;
	int rowHeight = note_height();
	int centerY = bounds.height / 2 + bounds.y;
	int whichRow = 0;

	// whichRow en absolu depuis 
	// C4 = 60 => 1
	if (p == 18 || p == 19) whichRow = 23;
	else if (p == 21 || p == 20) whichRow = 22;
	else if (p == 23) whichRow = 22;
	else if (p == 24 || p == 25) whichRow = 21;
	else if (p == 26 || p == 27) whichRow = 20;
	else if (p == 28) whichRow = 19;
	else if (p == 29 || p == 30) whichRow = 18;
	else if (p == 31 || p == 32) whichRow = 17;
	else if (p == 33 || p == 34) whichRow = 16;
	else if (p == 35) whichRow = 15;
	else if (p == 36 || p == 37) whichRow = 14;
	else if (p == 39 || p == 38) whichRow = 13;
	else if (p == 40) whichRow = 12;
	else if (p == 41 || p == 42) whichRow = 11;
	else if (p == 43 || p == 44) whichRow = 10;
	else if (p == 45 || p == 46) whichRow = 9;
	else if (p == 47) whichRow = 8;
	else if (p == 48 || p == 49) whichRow = 7;
	else if (p == 50 || p == 51) whichRow = 6;
	else if (p == 52) whichRow = 5;
	else if (p == 53 || p == 54) whichRow = 4;
	else if (p == 55 || p == 56) whichRow = 3;
	else if (p == 57 || p == 58) whichRow = 2;
	else if (p == 59) whichRow = 1;
	else if (p == 60 || p == 61) whichRow = 0;
	else if (p == 62 || p == 63) whichRow = -1; 
	else if (p == 64) whichRow = -2; // E
	else if (p == 65 || p == 66) whichRow = -3;
	else if (p == 67 || p == 68) whichRow = -4;
	else if (p == 69 || p == 70) whichRow = -5;
	else if (p == 71) whichRow = -6;
	else if (p == 72 || p == 73) whichRow = -7;
	else if (p == 74 || p == 75) whichRow = -8;
	else if (p == 76) whichRow = -9;
	else if (p == 77 || p == 78) whichRow = -10;
	else if (p == 79 || p == 80) whichRow = -11;
	else if (p == 81 || p == 82) whichRow = -12;
	else if (p == 83) whichRow = -13;
	else if (p == 84 || p == 85) whichRow = -14;
	else if (p == 86 || p == 87) whichRow = -15;
	else if (p == 88) whichRow = -16;
	else if (p == 89 || p == 90) whichRow = -17;
	else if (p == 91 || p == 92) whichRow = -18;
	else if (p == 93 || p == 94) whichRow = -19;
	else if (p == 95) whichRow = -20;
	else if (p == 96 || p == 97) whichRow = -21;
	else if (p == 98 || p == 99) whichRow = -22;
	else if (p == 100) whichRow = -23;
	else if (p == 101 || p == 102) whichRow = -23;
	else if (p == 103 || p == 104) whichRow = -24;
	else if (p == 105 || p == 106) whichRow = -25;

	y = centerY + whichRow * rowHeight;
	return y;
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

	float rowHeight = note_height();//bounds.height / (noteRange.span()+1);

	ofSetColor(color_staves_fg, 180);
	// draw staves :
	ofSetColor(0, 0, 0, 255);//195);
	int staves[10] = { 77, 74, 71, 67, 64, 57, 53, 50, 47, 43 };
	for (int i = 0; i < 10; i++) {
		if (staves[i] > noteRange.max || staves[i] < noteRange.min) continue; // don't draw note outside range
		//int whichRow = ofMap(staves[i], noteRange.max, noteRange.min, 0, noteRange.span());
		//int whichRow = ofMap(staves[i], noteRange.max+1, noteRange.min-1, 0, noteRange.span()+1);
		//cout << "whichRow: " << whichRow << endl;
		//if (i >= 5) // let a space between two staves
		//whichRow += 2;
		//float y = bounds.y + whichRow * rowHeight;
		float y = note_pitch2y(staves[i]); //bounds.y + whichRow * rowHeight;

		//cout << "line " << i << " whichRow:" << whichRow << " y:" << y << endl;
		ofRect(bounds.x, y, score_line_w, score_line_h);
	}
	bool bShowGraceNote = false;
	ofSetColor(0, 0, 0, 255);
	if(switches.size() > 0) {
		float zoomMinX = bounds.x;
		float zoomMaxX = bounds.x + bounds.width;
		for(int i = 0; i < switches.size(); i++){
			if(isSwitchInBounds(switches[i])){
				float startX =  normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds);
				float endX = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds);

				// measure bar
#if 0
				if (switches[i]->label.size()) {
					ofSetColor(0, 0, 0, 105);
					int yy = bounds.y + rowHeight* ofMap(staves[0], noteRange.max, noteRange.min, 0, noteRange.span());
					int hh = rowHeight* ofMap(staves[9], noteRange.max, noteRange.min, 0, noteRange.span());
					if (switches[i]->label.size()) {
						ofLine(startX, yy, startX, bounds.y + hh);
					}
					ofSetColor(0, 0, 0, 255);
					mFont.drawString(switches[i]->label, startX, yy - 4);
					ofSetColor(0, 0, 0, 255);
				}
#endif
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
					//int whichRow = ofMap(p, noteRange.max, noteRange.min, 0, noteRange.span());
					//if (!(p % 2)) p--;
					int whichRow = ofMap(p, noteRange.max+1, noteRange.min-1, 0, noteRange.span()+1);

					//ofRectangle noteBounds = ofRectangle(startX, bounds.y + whichRow * rowHeight, endX - startX, rowHeight);
					int y = note_pitch2y(p);
					ofRectangle noteBounds = ofRectangle(startX, y, endX - startX, rowHeight);

					// draw note, if grace note: halfsize gray note
					int notehead_w = 10, notehead_h = 9;
					if (switches[i]->beat.span() == 0 && p) {
						bShowGraceNote = true; notehead_w /= 1.3; notehead_h /= 1.3;
						ofSetColor(0, 0, 0, 100);
					} else { bShowGraceNote = false; ofSetColor(0, 0, 0, 255); }

					// draw note image
					ofSetColor(0, 0, 0, 255);
					int h = note_height();
					int factorh = h * 2;// / noteImage->getHeight();
					int factorw = h * 2;// / noteImage->getWidth();

					//if (bShowGraceNote) sp /= 2;
					//noteImage->draw(startX, y - sp/2, sp, sp);//noteImage->getWidth(), noteImage->getHeight());
					noteImage->draw(startX/*-factorw/2*/, y-factorh/2, factorw, factorh);//noteImage->getWidth(), noteImage->getHeight());

					if (accident)
						mFont.drawString("#", startX-factorw, y+factorh/2); //startX - 10, y + 2*rowHeight);

					// draw little rect displaying duration after the note if not grace note
					ofSetColor(0, 0, 0, 255);
					if (!bShowGraceNote) ofRect(startX+2, y, endX-startX-2, 1);

					ofSetColor(0, 0, 0, 255);
					// draw little lines if note if out of staves
					if (p == 60) {
						//whichRow = ofMap(p, noteRange.max, noteRange.min, 0, noteRange.span());
						ofRect(startX - notehead_w/2, y, notehead_w * 3/2, score_line_h);
					}
					if (p >= 81) {
						for (int k = p; k >= 81 ; ) {
							//whichRow = ofMap(k, noteRange.max, noteRange.min, 0, noteRange.span());
							y = note_pitch2y(k); //bounds.y + whichRow*rowHeight;
							ofRect(startX - notehead_w/2, y, notehead_w*3/2, score_line_h);
							int t = k % 12;
							if (t == 1 || t == 3 || t == 6 || t == 8 || t == 10)
								k -= 2;
							else k -= 1;
						}
					} else if (p <= 41) {
						for (int k = p; k <= 41 ;) {
							//whichRow = ofMap(k, noteRange.max, noteRange.min, 0, noteRange.span());
							y = note_pitch2y(k);// bounds.y + whichRow*rowHeight;
							ofRect(startX - notehead_w/2, y, notehead_w*3/2, score_line_h);
							int t = k % 12;
							if (t == 1 || t == 3 || t == 6 || t == 8 || t == 10)
								k += 2;
							else k += 1;
						}
					}

					// glissando : draw line
					/*if (switches[i]->type == ANTESCOFO_MULTI && i+1 < switches.size()
					  && (switches[i+1]->type == ANTESCOFO_MULTI || switches[i+1]->type == ANTESCOFO_MULTI_STOP)) {
					//setNoteColor(i);
					// next note:
					float startX2 =  normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.min), zoomBounds);
					float endX2 = normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i+1]->beat.max), zoomBounds);
					int whichRow2 = ofMap(abs(switches[i+1]->pitch), noteRange.max, noteRange.min, 0, noteRange.span());
					ofRectangle noteBounds2 = ofRectangle(startX2, bounds.y + whichRow2 * rowHeight, endX2 - startX2, rowHeight);

					ofLine(startX+notehead_w, noteBounds.y + noteBounds.height/2,
					startX2, noteBounds2.y + noteBounds2.height/2);
					}*/

				} else { // rest
					int y = note_pitch2y(71);
					int h = 7;
					ofSetColor(0, 0, 0, 255);
					ofRect(startX, y - note_height()/2, endX-startX, note_height());
				}
			}
		}
		// draw markers (separate loop used for calculate width)
		float lastX = bounds.x + bounds.width;
		int sizec = mFont.stringWidth(string("_"));
		for(int i = switches.size() - 1; i >= 0; i--){
			if(isSwitchInBounds(switches[i])){
				int startX = max(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.min), zoomBounds), zoomMinX);
				int endX = min(normalizedXtoScreenX( timeline->beatToNormalizedX(switches[i]->beat.max), zoomBounds), zoomMaxX);

				if (!switches[i]->label.empty()) {
					ofSetLineWidth(1);
					//ofLine(startX, bounds.y, startX, bounds.y + bounds.height);
					ofSetColor(0, 0, 0, 250);//105);
					int yy = note_pitch2y(staves[0]); //bounds.y + rowHeight* ofMap(staves[0], noteRange.max, noteRange.min, 0, noteRange.span());
					int yy2 = note_pitch2y(staves[9]);//rowHeight* ofMap(staves[9], noteRange.max, noteRange.min, 0, noteRange.span());
					if (switches[i]->label.size()) {
						ofLine(startX, yy, startX, yy2);
					}
					float w = lastX - startX;
					int l = floor( (w+1) / (sizec ));
					int s = switches[i]->label.size();
					if (l > s) l = s;
					string str = switches[i]->label.substr(s-l, l);
					ofSetColor(0, 0, 0, 255);
					mFont.drawString(str, startX, yy - 4);
					lastX = startX;
				}
			}
		}
	}
}


void ofxTLAntescofoNote::autoscroll() {
	static float lastpos = 0;
	// was : float pos = timeline->getPercentComplete();
	float pos = mCurSecs / mDur_in_secs;

	if (bShowPianoRoll) { // piano roll autoscroll
		if (bAutoScroll && mDur_in_secs && lastpos != pos) {
			ofxTLZoomer2D *zoom = (ofxTLZoomer2D*)timeline->getZoomer();

			ofRange z = zoom->getViewRange();
			ofRange oldz = z;
			//cout << endl << "pos:"<< pos <<" got zoomrange: "<< z.min << "->"<< z.max;
			// continuous scrolling : keep playhead on center
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
			//zoom->setViewRange(z);

			lastpos = pos;
			mAntescofog->shouldRedraw();

			// TODO page by page scrolling : when the playhead gets close to the end of the page, move zoom to next page and playhead to beginning
		}
	}
#ifdef USE_GUIDO
	else { // guido autoscroll
		if (mCurGuidoId == -1) {
			guido_y = bounds.y;
			scrolled_guido_x = bounds.x;
			scrolled_guido_w = 0;
			return;
		}
		pos = switches[mCurSwitchId]->guidoCoords.x / guido_w;
		//cout << "==============autoscroll==============> mCurGuidoId = " << mCurGuidoId << " pos= " << pos << " lastpos= " << lastpos << endl;

		if (bAutoScroll && mDur_in_secs && lastpos != pos) {
			bMousePressed = false;
			ofxTLZoomer2D *zoom = (ofxTLZoomer2D*)timeline->getZoomer();

			ofRange z = zoom->getViewRange();
			ofRange oldz = z;
			z.max = z.min + bounds.width / guido_w;
#ifdef DEBUG_GUIDO_ASCOGRAPH
			cout << endl << "pos:"<< pos << " mCurBeat=" << mCurBeat << " mDur_in_beats=" << mDur_in_beats <<" got zoomrange: "<< z.min << "->"<< z.max;
#endif
			// continuous scrolling : keep playhead on center
			float c = z.center(); 
			float d = pos - c;

			z.min = ofClamp(z.min + d, 0, 1); z.max = ofClamp(z.max + d, 0, 1);
			if (z.min == .0 && z.span() < oldz.span())
				z.max = oldz.max - oldz.min;
			if (z.max == 1. && z.span() < oldz.span())
				z.min = z.max - oldz.max + oldz.min;

#ifdef DEBUG_GUIDO_ASCOGRAPH
			cout <<" to zoomrange: "<< z.min << "->"<< z.max<<endl;
#endif
			zoom->setViewRange(z);
#if 0
			zoom->notifyZoomStarted();
			zoom->notifyZoomEnded();
#endif

			lastpos = pos;
#if 0
			cout << "switches[" << mCurSwitchId << "]->guidoCoords.x = " << switches[mCurSwitchId]->guidoCoords.x << endl;
			cout << "switches[" << mCurSwitchId << "]->guidoCoords.y = " << switches[mCurSwitchId]->guidoCoords.y << endl;
			cout << "switches[" << mCurSwitchId << "]->guidoCoords.w = " << switches[mCurSwitchId]->guidoCoords.width << endl;
			cout << "switches[" << mCurSwitchId << "]->guidoCoords.h = " << switches[mCurSwitchId]->guidoCoords.height << endl;
#endif

			//guido_x = guido_w * z.center();
			//WAS guido_x = bounds.x + (zoomBounds.min * guido_w);
			scrolled_guido_w = switches[mCurSwitchId]->guidoCoords.width;
			if (guido_w <= bounds.width || switches[mCurSwitchId]->guidoCoords.x <= bounds.width*2/3) { // before scrolling
#ifdef DEBUG_GUIDO_ASCOGRAPH
				cout << "BEGIN SCROLL" << endl;
#endif
				scrolled_guido_x = switches[mCurSwitchId]->guidoCoords.x + bounds.x - scrolled_guido_w/2;
				guido_x = 0;
			} /*else if (switches[mCurSwitchId]->guidoCoords.x > guido_w - bounds.width/3) { // stop scrolling at the end
				cout << "END SCROLL" << endl;
				//scrolled_guido_x = bounds.width/2 + ((int)switches[mCurGuidoId]->guidoCoords.x % (int)bounds.width) - bounds.x;
				scrolled_guido_x = bounds.width - guido_w + switches[mCurSwitchId]->guidoCoords.x - scrolled_guido_w - bounds.x;
				guido_x = guido_w - bounds.width;
			} */else { // normal scrolling
#ifdef DEBUG_GUIDO_ASCOGRAPH
				cout << "MID SCROLL" << endl;
#endif
				scrolled_guido_x = bounds.width / 2 - scrolled_guido_w/2 + bounds.x;
				int switchid = mCurSwitchId;// >= switches.size()-1 ? switches.size()-1 : mCurSwitchId+1;
				guido_x = switches[switchid]->guidoCoords.x - bounds.width/2;// - scrolled_guido_w;
				cout << "mCurSwitchId=" << mCurSwitchId << " switches.size()=" << switches.size() << endl;
			}
#ifdef DEBUG_GUIDO_ASCOGRAPH
			 cout << "mCurGuidoId = " << mCurGuidoId << "  scrolled_guido_x = " << scrolled_guido_x << "  scrolled_guido_w = " << scrolled_guido_w << " guido_x=" << guido_x <<  endl ;
#endif
			if (scrolled_guido_w < 0) {
				cout << "!!!!!!!!!!!!!!!!!!!!!!!" << "!!!!!!!!!!!!!!!!!!!!!!!!" << "!!!!!!!!!!!!!!!!!!!!!!!  ====================> " << endl; 
				cout << "scrolled_guido_w = " << scrolled_guido_w << " mCurGuidoId = " << mCurGuidoId << endl ;
				cout << "!!!!!!!!!!!!!!!!!!!!!!!" << "!!!!!!!!!!!!!!!!!!!!!!!!" << "!!!!!!!!!!!!!!!!!!!!!!!  ====================> " << endl; 
			}


			mAntescofog->shouldRedraw();

			// TODO page by page scrolling : when the playhead gets close to the end of the page, move zoom to next page and playhead to beginning
		}
	}
#endif
}


#ifdef USE_GUIDO
// refresh mCurGuidoId
void ofxTLAntescofoNote::update_guido() {
	if (bMousePressed) {
		guido_x = (zoomBounds.min * guido_w);
		guido_y = bounds.y;
	}

	for (map<float, int>::iterator i = beat2switchId.begin(); i != beat2switchId.end(); i++) {
		//cout << "i first=" << i->first << " mCurBeat="<< mCurBeat<< endl;
		if (i->first == mCurBeat) {
			//mCurGuidoId = i->second;
			mCurSwitchId = i->second;
			mCurGuidoId = switchId2guidoId[i->second];
			cout << "Setting mCurGuidoId=" << mCurGuidoId << " with switchId = " << i->second << endl;

			// draw at scrolled_guido_x

			//guido_x = bounds.x + scrolled_guido_x - bounds.width / 2;
			//guido_y = bounds.y;
			// WAS guido_x = bounds.x - (zoomBounds.min * guido_w);

			break;
		}
	}

	//cout << "update_guido: setting mCurGuidoId=" << mCurGuidoId << endl;
}
#endif

void ofxTLAntescofoNote::draw_playhead() {
	if (bShowPianoRoll) { // pianoroll playhead
		float x = normalizedXtoScreenX( mCurSecs/mDur_in_secs, zoomBounds);
		if (x < bounds.x && mCurBeat && mCurSecs) return;
		if (!mCurBeat) autoscroll();
		ofPushStyle();
		ofSetColor(0, 0, 0, 255);
		ofLine(x, bounds.y, x, bounds.y + bounds.height);
		ofSetLineWidth(3);
		ofPopStyle();
	} else { // guido playhead
#ifdef USE_GUIDO
		if (mCurBeat == -1) return;

		//cout << endl << "draw_playhead: mCurBeat=" << mCurBeat << endl;
		// use map : beat2switchId
		if (mCurGuidoId != -1 && !bMousePressed) {
			ofPushStyle();
			ofFill();
			ofSetColor(51, 51, 255, 75);
			ofSetLineWidth(1);

			int x = scrolled_guido_x;
			int y = guido_y + 6;
			int w = scrolled_guido_w;
			int h = bounds.height - 6*2;
			//cout << "ofxTLAntescofoNote::draw_playhead: " << mCurBeat << "= switch #" << mCurGuidoId <<" drawing rect: " << x << ", " << y << " : " << w << " x " << h<< endl;
			roundedRect(x, y, w, h, 5);
			ofSetColor(51, 51, 255, 175);
			ofNoFill();
			//ofRect(x, y, w, h);
			roundedRect(x, y, w, h, 5);
			ofPopStyle();
		}
#endif
	}
}

void ofxTLAntescofoNote::draw() {
	ofPushStyle();
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
#ifdef USE_GUIDO
	update_guido();

#endif

	if (bAutoScroll) autoscroll();
	if (bShowPianoRoll) {
		draw_showPianoRoll();
	} else
#ifdef USE_GUIDO
		draw_guido();
#else
		draw_showStaves();
#endif

	if (mCurSecs || mCurBeat != -1) draw_playhead();
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
#ifndef ASCOGRAPH_IOS
	cout << "ofxTLAntescofoNote::mousePressed " << endl;
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

	bMousePressed = true;
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

	// deja selected
	if(clickedSwitchA != NULL){
		mAntescofog->editorShowLine(clickedSwitchA->lineNum_begin, clickedSwitchA->lineNum_end, clickedSwitchA->colNum_begin, clickedSwitchA->colNum_end);
		if(!ofGetModifierKeyShift()){
			timeline->unselectAll();
		}
		bool startAlreadySelected = clickedSwitchA->startSelected;
		bool endAlreadySelected = clickedSwitchA->endSelected;
		clickedSwitchA->startSelected = didSelectedStartTime || (ofGetModifierKeyShift() && startAlreadySelected);
		clickedSwitchA->endSelected   = !didSelectedStartTime || (ofGetModifierKeyShift() && endAlreadySelected);
		/*
		if(didSelectedStartTime){
			timeline->setDragTimeOffset( clickedSwitchA->dragOffsets.min );
		}
		else{
			timeline->setDragTimeOffset( clickedSwitchA->dragOffsets.max );
		}
		*/
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
			mAntescofog->editorShowLine(clickedSwitchA->lineNum_begin, clickedSwitchA->lineNum_end, clickedSwitchA->colNum_begin, clickedSwitchA->colNum_end);
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
#endif
	return true;
}

void ofxTLAntescofoNote::mouseMoved(ofMouseEventArgs& args, long millis){
}

void ofxTLAntescofoNote::mouseDragged(ofMouseEventArgs& args, long millis) { //bool snapped){
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

#ifdef USE_MUSICXML
int ofxTLAntescofoNote::loadscoreMusicXML(string filename, string outfilename){
	clear();
	switches = switchesFromMusicXML(filename, outfilename);
	timeline->setBPM(atof(AntescofoWriter->fBPM.c_str()));
	trimRange();
	return switches.size();
}
#endif


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


int ofxTLAntescofoNote::getNoteType(Event *e)
{
	int ret = -1;
	if (e) {
		ostringstream str;

		switch (e->isMarkov)
		{
			case 2: // TRILL
				{
					if (0 != e->multi_event)
					{
						if(e->multi_event>0)
						{
							ret = ANTESCOFO_TRILL; // MULTI TRILL XXX
						}
						ret = ANTESCOFO_TRILL;
					} else ret = ANTESCOFO_TRILL; // MULTI wtf?;
					break;
				}
			case 4: // CHORD
				ret = ANTESCOFO_CHORD;
				break;

			case 0:  // NOTE
			case 1:
			default:
				{
					if (1 == e->pitch_list.size() && 0.0 == e->pitch_list[0] && e->isSilence()) {
						ret = ANTESCOFO_REST;
						break;
					}
					else
					{
						if (0. != e->multi_event)
						{
							if(e->multi_event > 0)
							{
								ret = ANTESCOFO_MULTI;
								break;
							} else {
								ret = ANTESCOFO_MULTI_DUMMY;
								break;
							}
						} else {
							if (1 == e->pitch_list.size())
								ret = ANTESCOFO_NOTE;
							else
								ret = ANTESCOFO_MULTI;
							break;
						}
					}
				}

			//case 1:  // DummySilence
			//	return -1;
		}
		if (ret == -1) cerr << "ofxTLAntescofoNote::getNoteType: unknown type: multi_event:"<< e->multi_event << " pitches:" << e->pitch_list.size()<< endl;
		if (debug_loadscore) {
			cout << "ofxTLAntescofoNote::getNoteType: returning ";
			switch (ret) {
				case 0:
					cout << "ANTESCOFO_REST";
					break;
				case 1:
					cout << "ANTESCOFO_CHORD";
					break;
				case 2:
					cout << "ANTESCOFO_NOTE";
					break;
				case 3:
					cout << "ANTESCOFO_TRILL";
					break;
				case 4:
					cout << "ANTESCOFO_MULTI";
					break;
				case 5:
					cout << "ANTESCOFO_MULTI_STOP";
					break;
				case 6:
					cout << "ANTESCOFO_MULTI_DUMMY";
					break;
				default:
					break;
			}
			cout << endl;
		}
	}

	//if (debug_loadscore) { str << "getNoteType: isMArkov:"<< e->isMarkov << " multi_event:"<< e->multi_event 
		//<< " pitches:" << e->pitch_list.size() << endl; console->addln(str.str()); str.str(""); }
	return ret;
}


bool ofxTLAntescofoNote::getAccompanimentMarkers_rec_group(Gfwd *g, vector<float>& map_index, vector<float>& map_markers) {
	bool res = false;
	bool debug = false;

	vector<Action*>::const_iterator i;
	for (i = g->actions().begin(); i != g->actions().end(); i++) {
		if (debug) cout << "getAccompanimentMarkers in event in action" << endl;
		Action *a = *i;
		AssignmentAction *assign = dynamic_cast<AssignmentAction*>(a);
		if (assign) {
			if (debug) cout << "getAccompanimentMarkers in event in action in assign: name:"  << assign->var().name() << endl;
			if (assign->var().name().size() && assign->var().name() == string("$map_rel")) {
				if (debug) cout << "getAccompanimentMarkers in event in action in assign: name:"  << assign->value() << " OK" << endl;
				const Expression* rhs = assign->value();
				const MapDefinition* md = dynamic_cast<const MapDefinition*>(rhs);
				if (md && md->map_list) {
					const MapList *ml = md->map_list;
					for(MapList::const_iterator it = ml->begin(); it!=ml->end(); it++) {
						if (debug) cout << "map: rel: " << (*it)->first << ", " << (*it)->second << endl;
						const TaggedValue *k = (*it)->first->is_value();
						const TaggedValue *n = (*it)->second->is_value();
						float l, o;
						if (k && k->is_numeric() && n && n->is_numeric()) {
							l = eval_double(k);
							o = eval_double(n);
							map_index.push_back(o);
						}
					}
				}
			}
			if (assign->var().name().size() && assign->var().name() == string("$map_abs")) {
				if (debug) cout << "getAccompanimentMarkers in event in action in assign: name:"  << assign->value() << " OK" << endl;
				const Expression* rhs = assign->value();
				const MapDefinition* md = dynamic_cast<const MapDefinition*>(rhs);
				if (md && md->map_list) {
					const MapList *ml = md->map_list;
					for(MapList::const_iterator it = ml->begin(); it!=ml->end(); it++) {
						if (debug) cout << "map: abs: " << (*it)->first << ", " << (*it)->second << endl;
						const TaggedValue *k = (*it)->first->is_value();
						const TaggedValue *n = (*it)->second->is_value();
						float l, o;
						if (k && k->is_numeric() && n && n->is_numeric()) {
							l = eval_double(k);
							o = eval_double(n);
							map_markers.push_back(o);
						}
					}
				}
			}


		}
	}
	return res;

}

// look for a variable containing a map with marker in the score
bool ofxTLAntescofoNote::getAccompanimentMarkers(vector<float>& map_index, vector<float>& map_markers) {
	bool res = false;
	if (!mNetscore) return res;
	for (vector<Event *>::iterator e = mNetscore->begin(); e != mNetscore->end(); e++) {
		if ((*e)->gfwd) {
			Gfwd *g = dynamic_cast<Gfwd *>((*e)->gfwd);
			if (g) {
				res = getAccompanimentMarkers_rec_group(g, map_index, map_markers);
				if (res)
					break;
			}
		}
	}
	return res;
}

int ofxTLAntescofoNote::loadscoreAntescofo(string filename){
	clear();
	//str_error.erase();
	Score *score;

	mAntescofo->set_verbosity_level(5);
	// check if filename is really a filename
	cerr << "check if filename is really a filename" << endl;
	struct stat st;
	if (lstat(filename.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		cerr << "loadScore: Input is a directory, you stupid... :)" << endl;
		return 0;
	}
	reset_global_error_counter();

	mAntescofo->reset_error_count();
#if 0
	if (NULL == (score = mAntescofo->Parse(filename))) {
		//::Error("%s\n", filename.c_str());
        cerr << "PARSING ERROR: " << filename.c_str() << endl;
		return 0;
	}
#endif
	score = mAntescofo->Parse(filename);
	if (score == NULL) {
		//::Error("%s\n", filename.c_str());
        cerr << "PARSING ERROR: " << filename.c_str() << endl;
		return 0;
	}
	mNetscore = score;
	ostringstream str;
	ofLog() << "Duration ------------------ score size : " << score->size() << "."<< endl;
	str << "Duration ------------------ score size : " << score->size() << "."<< endl;
	vector<Event *>::iterator i = score->end();
	i--; // duration is the last element index
	mDur_in_beats = (*i)->beatcum + (*i)->beat_duration;
	str << "Duration ------------------ last beat : " << mDur_in_beats << "."<< endl;
	if (score->tempo < 0)
		score->tempo = - score->tempo;
	timeline->setBPM(60/score->tempo); // bps
	mDur_in_secs = 60 / timeline->getBPM() * mDur_in_beats;
	str << "Duration ------------------ " << mDur_in_beats << " beats."<< endl;
	str << "Duration ------------------ " << mDur_in_secs << " seconds.";
	console->addln(str.str()); str.str("");
	mDur_in_secs++; // add one beat for better display
	timeline->setDurationInSeconds(mDur_in_secs);

	bool bGot_Action = true;
	ostringstream oss;
	string actstr;
	score->show(cout);
	int evt_nb = 0;

	for (vector<Event *>::iterator i = score->begin(); i != score->end(); i++, evt_nb++)
	{
		Event *e = *i;
		//if (!e->isEvent() /*&& i != score->begin()*/) continue; // TODO store and display actions on first dummy silence
		if (e->gfwd) {
			Gfwd *g = dynamic_cast<Gfwd*>(e->gfwd); //TODO may be useful to display other things than Gfwd
			if (g) {
				oss.str(""); oss.clear();
				list<Action*> l = g->bloc();
				for (list<Action*>::iterator a = l.begin(); a != l.end(); a++) {
					if (*a)
						oss << *a;
				}
				actstr = oss.str();
				if (actstr.size()) bGot_Action = true;
			} else {
				oss.str(""); oss.clear();
				oss << *(e->gfwd);
				actstr = oss.str();
				if (actstr.size()) bGot_Action = true;
			}
		}

		int newtype = getNoteType(e);
		//TODO if (i == score->begin() && !e->beat_duration) { add_action(e->beatcum, actstr, e); }
		if ((e->pitch_list.size() == 1 && e->beat_duration) || (e->pitch_list.size() && e->pitch_list[0] && !e->beat_duration)) { // NOTE, TRILL, MULTI
			//if (newtype == -1) continue;
			if (newtype == ANTESCOFO_MULTI) {
				double beatmulti = e->beatcum;
				double durmulti = e->multi_dur;
				for (vector<float>::iterator m = e->multi_source.begin(); m != e->multi_source.end(); m++) { // MULTI sources
					ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
					newSwitch->type = ANTESCOFO_MULTI;
					newSwitch->startSelected = newSwitch->endSelected = false;
					newSwitch->duration = e->multi_dur;
					newSwitch->beat.min = beatmulti;
					newSwitch->beat.max = newSwitch->beat.min + durmulti*2/3;
					newSwitch->pitch = abs(*m) > 1000 ? abs(*m) / 100 : abs(*m);
					newSwitch->is_tied = (*m < 0);
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
					newSwitch->isLast = false;
					newSwitch->notenum = e->notenum;
					newSwitch->label = e->cuename;
					/*if (m == e->multi_source.begin()) { 
						if (bGot_Action)  {
							newSwitch->action = actstr;
							add_action(e->beatcum, actstr, e);
						}
					}*/
					switches.push_back(newSwitch);
					if (debug_loadscore) { str << "added new switch for MULTI source: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch; console->addln(str.str()); str.str(""); }
					line2note[e->scloc->begin.line] = switches.size() - 1;
					bGot_Action = false;
				}
				for (vector<float>::iterator m = e->multi_target.begin(); m != e->multi_target.end(); m++) { // MULTI destinations
					ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
					newSwitch->type = ANTESCOFO_MULTI_STOP;
					newSwitch->startSelected = newSwitch->endSelected = false;
					newSwitch->duration = e->multi_dur;
					newSwitch->beat.min = beatmulti + durmulti/3;
					newSwitch->beat.max = newSwitch->beat.min + durmulti*2/3;
					newSwitch->pitch = abs(*m) > 1000 ? abs(*m) / 100 : abs(*m);
					newSwitch->is_tied = (*m < 0);
					newSwitch->velocity = 127;
					newSwitch->channel = 1;
					newSwitch->isLast = true;
					newSwitch->notenum = e->notenum;
					// get location in text score
					assert(e->scloc);
					newSwitch->lineNum_begin = e->scloc->begin.line;
					newSwitch->colNum_begin = e->scloc->begin.column;
					newSwitch->lineNum_end = e->scloc->end.line;
					newSwitch->colNum_end = e->scloc->end.column;
					switches.push_back(newSwitch);
					line2note[e->scloc->begin.line] = switches.size() - 1;
					if (debug_loadscore) { str << "added new switch for MULTI_STOP target: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch; console->addln(str.str()); str.str(""); }
					bGot_Action = false;
				}
				if (e->multi_source.size() && e->multi_target.size())
					continue;
			} 

			if (debug_loadscore) { str << "got pitch:"<< e->pitch_list[0] << " type:"<< newtype << " markov:"<<e->isMarkov; console->addln(str.str()); str.str(""); }
			ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
			newSwitch->type = newtype;
			newSwitch->startSelected = newSwitch->endSelected = false;
			newSwitch->duration = e->beat_duration;
			newSwitch->beat.min = e->beatcum;
			newSwitch->beat.max = newSwitch->beat.min + e->beat_duration;
			newSwitch->pitch = abs(e->pitch_list[0]) > 1000 ? abs(e->pitch_list[0]) / 100 : abs(e->pitch_list[0]);
			newSwitch->is_tied = (e->pitch_list[0] < 0);
			newSwitch->velocity = 127;
			newSwitch->channel = 1;
			newSwitch->isLast = false;
			newSwitch->notenum = e->notenum;
			if (bGot_Action)  {
				newSwitch->action = actstr;
				add_action(e->beatcum, actstr, e);
			}
			// get location in text score
			if (!e->scloc) cout << *e;
			else {
				assert(e->scloc); // XXX useless assert
				newSwitch->lineNum_begin = e->scloc->begin.line;
				newSwitch->colNum_begin = e->scloc->begin.column;
				newSwitch->lineNum_end = e->scloc->end.line;
				newSwitch->colNum_end = e->scloc->end.column;
				newSwitch->label = e->cuename;
				newSwitch->isLast = false;
				newSwitch->notenum = e->notenum;
				switches.push_back(newSwitch);
				line2note[e->scloc->begin.line] = switches.size() - 1;
			}
			if (debug_loadscore) { str << "added new switch: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch; console->addln(str.str()); str.str(""); }
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
				newSwitch->duration = e->beat_duration;
				newSwitch->beat.min = e->beatcum;
				newSwitch->beat.max = newSwitch->beat.min + e->beat_duration;
				newSwitch->pitch = abs(e->pitch_list[p]) > 1000 ? abs(e->pitch_list[p]) / 100 : abs(e->pitch_list[p]);
				newSwitch->is_tied = (e->pitch_list[p] < 0);
				newSwitch->velocity = 127;
				newSwitch->channel = 1;
				newSwitch->isLast = false;
				newSwitch->notenum = e->notenum;
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
				if (debug_loadscore) { str << "added new switch: CHORD:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch; console->addln(str.str()); str.str(""); }
			}
			switches.back()->isLast = true;
			bGot_Action = false;
		} else if (newtype == ANTESCOFO_TRILL) {
			int pitch = 0;

			for (vector< vector<float> >::iterator t = e->TrillMap.begin(); t != e->TrillMap.end(); t++) {
				for (vector<float>::iterator r = t->begin(); r != t->end(); r++) {
					if (!*r) continue;
					ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
					newSwitch->type = ANTESCOFO_TRILL;
					newSwitch->startSelected = newSwitch->endSelected = false;
					newSwitch->duration = e->beat_duration;
					newSwitch->beat.min = e->beatcum ;//+ e->beat_duration/3;
					newSwitch->beat.max = newSwitch->beat.min + e->beat_duration; //*2/3;
					newSwitch->pitch = abs(*r) > 1000 ? abs(*r) / 100 : abs(*r);
					newSwitch->is_tied = (*r < 0);
					newSwitch->velocity = 127;
					newSwitch->channel = 1;
					if (bGot_Action)  {
						newSwitch->action = actstr;
						add_action(e->beatcum, actstr, e);
					}
					newSwitch->notenum = e->notenum;
					// get location in text score
					assert(e->scloc);
					newSwitch->lineNum_begin = e->scloc->begin.line;
					newSwitch->colNum_begin = e->scloc->begin.column;
					newSwitch->lineNum_end = e->scloc->end.line;
					newSwitch->colNum_end = e->scloc->end.column;
					if (r == t->begin()) newSwitch->label = e->cuename;
					switches.push_back(newSwitch);
					line2note[e->scloc->begin.line] = switches.size() - 1;
					if (debug_loadscore) { str << "added new switch for TRILL: beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max << "] pitch:"<<  newSwitch->pitch; console->addln(str.str()); str.str(""); }
					bGot_Action = false;

				}
			}
			switches.back()->isLast = true;
		} else if (e->isSilence()) { // rest
			if (debug_loadscore) { str << "GOT REST"; console->addln(str.str()); str.str(""); }
			ofxTLAntescofoNoteOn *newSwitch = new ofxTLAntescofoNoteOn();
			newSwitch->type = ANTESCOFO_REST;
			newSwitch->startSelected = newSwitch->endSelected = false;
			newSwitch->duration = e->beat_duration;
			newSwitch->beat.min = e->beatcum;
			newSwitch->beat.max = newSwitch->beat.min + e->beat_duration;
			newSwitch->pitch = 0;
			newSwitch->is_tied = false;
			newSwitch->velocity = 127;
			newSwitch->channel = 1;
			newSwitch->isLast = false;
			newSwitch->notenum = e->notenum;
			if (bGot_Action)  {
				newSwitch->action = actstr;
				add_action(e->beatcum, actstr, e);
			}
			// get location in text score
			if (!e->scloc) cout << *e;
			newSwitch->label = e->cuename;
#if 0 // XXX
			assert(e->scloc);
			newSwitch->lineNum_begin = e->scloc->begin.line;
			newSwitch->colNum_begin = e->scloc->begin.column;
			newSwitch->lineNum_end = e->scloc->end.line;
			newSwitch->colNum_end = e->scloc->end.column;
#endif
			switches.push_back(newSwitch);
			line2note[e->scloc->begin.line] = switches.size() - 1;
			if (debug_loadscore) { str << "added new switch: REST beat:[" << newSwitch->beat.min << ":" << newSwitch->beat.max; console->addln(str.str()); str.str(""); }
			bGot_Action = false;
		} else {
			if (debug_loadscore) { cerr << endl << "ERROR: unhandled event!!!!"<< endl; }
		}

	}
	map<int, std::vector<std::string> >::iterator ito;
	for (ito = mNetscore->jump_labels.begin(); ito != mNetscore->jump_labels.end(); ito++) {
		vector< int> jindz;
		for (uint i=0; i< ito->second.size(); i++)
		{
			int index = mNetscore->labelPosition(ito->second.at(i));
			//int index = mNetscore->labelPosition(ito->second.at(ito->first));
			float pos = (*mNetscore)[index]->beatcum;
			float srcpos = (*mNetscore)[ito->first]->beatcum;
			cout << "Jump at " << ito->first << " (pos:"<< srcpos<< ") :" << ito->second.at(i).c_str() << " index:"<< index << endl;
			ofxTLAntescofoNoteOn* switche = get_switch_for_beatnum(srcpos);
			assert(switche != NULL);
			switche->jump_dests.push_back(pos);
		}
	}
	setScore(score);
	str << "Score tempo : " << 60/score->tempo;
	console->addln(str.str()); str.str("");
	trimRange();
	missedAll();
	unselectAll();
	sort(switches.begin(), switches.end(), switchsort);

	update_duration();
	getcues();

#ifdef USE_GUIDO
	if (!bShowPianoRoll)
		update_guido_render();
#endif

	return switches.size();
}

ofxTLAntescofoNoteOn* ofxTLAntescofoNote::get_switch_for_beatnum(float beatnum) {
	for (vector<ofxTLAntescofoNoteOn*>::iterator i = switches.begin(); i != switches.end(); i++) {
		if ((*i)->beat.min == beatnum)
			return *i;
	}
	cout << "get_switch_for_beatnum: ERROR : note not found: "<< beatnum << endl;
	return NULL;
}

void ofxTLAntescofoNote::getcues() {

	if (switches.size())
	{
		cuepoints.clear();
		cout << "Getting cue points:";
		for (uint iter=1; iter < mNetscore->size(); ++iter)
		{
			if (!(mNetscore->at(iter)->cuename.empty()))
			{
				cout << " " << mNetscore->at(iter)->cuename << endl;
				cuepoints.push_back(mNetscore->at(iter)->cuename);
			}
		}
		cout << endl;
	}

}

void ofxTLAntescofoNote::update_duration() {
	if (!ofxAntescofoAction)
		createActionTrack();

	int maxdur = ofxAntescofoAction->get_max_note_beat();
	if (maxdur) cout << "Maximum note beat calculated from actions: " << maxdur << " beats." << endl;
	float dur_in_beats = mDur_in_secs * timeline->getBPM() / 60;
	if (maxdur > dur_in_beats) {
		dur_in_beats = maxdur + 1;
		mDur_in_secs = 60 / timeline->getBPM() * dur_in_beats;
		mDur_in_secs++; // add one beat for better display
		timeline->setDurationInSeconds(mDur_in_secs);
	}

	cout << "Duration ------------------ " << dur_in_beats << " beats."<< endl;
	cout << "Duration ------------------ " << mDur_in_secs << " seconds.";
}


void ofxTLAntescofoNote::load() {}

void ofxTLAntescofoNote::add_action(float beatnum, string action, Event *e) {
	if (!ofxAntescofoAction)
		createActionTrack();
	ofxAntescofoAction->add_action(beatnum, action, e);
}

void ofxTLAntescofoNote::clear_actions()
{
	if (ofxAntescofoAction)
		ofxAntescofoAction->clear_actions();
}

ofxTLAntescofoAction* ofxTLAntescofoNote::createActionTrack() {
	if (ofxAntescofoAction) return ofxAntescofoAction;
	ofxAntescofoAction = new ofxTLAntescofoAction(mAntescofog);
	getTimeline()->addTrack("Actions", ofxAntescofoAction);
	ofxAntescofoAction->setNoteTrack(this);
	return ofxAntescofoAction;
}

void ofxTLAntescofoNote::deleteActionTrack() {
	if (ofxAntescofoAction) {
		getTimeline()->removeTrack(ofxAntescofoAction);
		if (ofxAntescofoAction) {
			delete ofxAntescofoAction;
		}
		ofxAntescofoAction = 0;
	}
}

void ofxTLAntescofoNote::setScore(Score* s) {
	mNetscore = s;
	if (ofxAntescofoAction)
		ofxAntescofoAction->setScore(s);
}


float ofxTLAntescofoNote::convertAntescofoOutputToTime(float mOsc_beat, float mOsc_tempo, float mOsc_pitch) {
	cout << "convertAntescofoOutputToTime: " << mOsc_beat << endl;
	//if (mOsc_tempo == 0) cerr << "Error null tempo returned by Antescofo, skipping a division by zero..." << endl;
	//if (mOsc_beat == 0) return 0;

	float r = timeline->beatToMillisec(mOsc_beat) / 1000;
	//timeline->setCurrentTimeSeconds(r);
	mCurSecs = r;
	mCurBeat = mOsc_beat;

	// display followed event in editor
	ofxTLAntescofoNoteOn* switchA = 0;
	for (int i = 0; i < switches.size(); i++) {
		if (switches[i]->beat.contains(mOsc_beat)) {
			switchA = switches[i];
			switchA->missed = false;
		}
	}
#ifndef ASCOGRAPH_IOS
	if(switchA != NULL && bAutoScroll)
		mAntescofog->editorShowLine(switchA->lineNum_begin, switchA->lineNum_end, switchA->colNum_begin, switchA->colNum_end);
#endif
	// TODO : save detectedPitch = mOsc_pitch; in order to display it in a purple color in draw()
	return r;
}


ofRectangle ofxTLAntescofoNote::getBounds()
{
	return bounds;
}



// called from ofxTLAntescofoAction with a modified action string from the text editor
// -> save the action
// -> save a backup of current score
// -> reload modified file, if parse error : restore backup score
bool ofxTLAntescofoNote::change_action(float beatnum, string newaction)
{
#ifndef ASCOGRAPH_IOS
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
	if (NULL == (score = mAntescofo->Parse(TEXT_CONSTANT_TEMP_ACTION_FILENAME))) {
		::error("%s\n", TEXT_CONSTANT_TEMP_ACTION_FILENAME);
		return 0;

		str << "ofxTLAntescofoNote::change_action: display dialog box saying [Error action syntax error]" << endl;
		str << "ofxTLAntescofoNote::change_action: display dialog box asking [keep current action ? or cancel change]";
		console->addln(str.str()); str.str("");
		switches = bkpswitches;
	} else return loadscoreAntescofo(TEXT_CONSTANT_TEMP_ACTION_FILENAME);
#else
    return false;
#endif
}

// called from ofxTLAntescofoAction with a score not saved to parse from the text editor
// -> save the text editor content to a tmp file
// -> save a backup of current score
// -> parse tmp file, if parse error : restore backup score
bool ofxTLAntescofoNote::loadscoreAntescofo_fromString(string newscore, string filepath)
{
	// save a copy of current score
	vector<ofxTLAntescofoNoteOn*> bkpswitches = switches;

	// try to parse resulting new score
    remove(filepath.c_str());
	ofstream f;
	f.open(filepath.c_str());
	f << newscore;
	f.close();
	str_error.erase();
	Score *score;
	reset_global_error_counter();
	if (NULL == (score = mAntescofo->Parse(filepath))) {
		::error("%s\n", filepath.c_str());
		mAntescofog->display_error();
		return 0;
	}
    if (ofxAntescofoAction)
        ofxAntescofoAction->clear_actions();
	return loadscoreAntescofo(filepath);
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

			if (n == 0)
				return;
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

#ifdef USE_MUSICXML
vector<ofxTLAntescofoNoteOn*> ofxTLAntescofoNote::switchesFromMusicXML(string filename, string outfilename){
	vector<ofxTLAntescofoNoteOn*> newSwitches;

	// parse musicxml files using libmusicxml2
	using namespace MusicXML2;
	// check if filename is really a filename
	cerr << "check if filename is really a filename" << endl;
	struct stat st;
	if (lstat(filename.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		cerr << "loadScore: Input is a directory, you stupid... :)" << endl;
		return newSwitches;
	}

	xmlreader r;
	SXMLFile xmlfile;
	xmlfile = r.read(filename.c_str());
	if (xmlfile) {
		Sxmlelement st = xmlfile->elements();
		if (st) {
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
#endif

string ofxTLAntescofoNote::copyRequest(){
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
	return switchString;
}

void ofxTLAntescofoNote::pasteSent(string pasteboard){
	cerr << "ofxTLAntescofoNote::pasteSent: TODO" << endl;
}

void ofxTLAntescofoNote::missedAll(){
	for(int i = 0; i < switches.size(); i++){
		switches[i]->missed = true;
	}
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
	if (mNetscore) {
#ifdef USE_GUIDO
		guido_string.clear();
		mCurGuidoId = -1;
#endif
		deleteActionTrack();
		//mAntescofo // TODO rajouter l'appel au ~Score() !!!
		delete mNetscore;
		mNetscore = 0;
		mCurSecs = 0.;
		mCurBeat = -1;
		if (switches.size()) {
			for (vector<ofxTLAntescofoNoteOn*>::iterator i = switches.begin(); i != switches.end(); i++) {
				ofxTLAntescofoNoteOn* s = *i;
				delete s;
			}
			switches.clear();

		}
	}
#ifdef USE_MUSICXML
	delete AntescofoWriter;
	AntescofoWriter = new MusicXML2::antescofowriter();
#endif
}

void ofxTLAntescofoNote::playbackStarted(ofxTLPlaybackEventArgs& args){
	lastTimelinePoint = timeline->getPercentComplete();
}
void ofxTLAntescofoNote::playbackLooped(ofxTLPlaybackEventArgs& args){
	cerr << "ofxTLAntescofoNote::playbackLooped: TODO" << endl;
}
void ofxTLAntescofoNote::playbackEnded(ofxTLPlaybackEventArgs& args){
	endActiveNotes();
}

void ofxTLAntescofoNote::endActiveNotes() {
	cerr << "ofxTLAntescofoNote::endActiveNotes: TODO" << endl;
}

bool ofxTLAntescofoNote::isSwitchInBounds(ofxTLAntescofoNoteOn* s){
	ofRange r( timeline->beatToNormalizedX(s->beat.min), timeline->beatToNormalizedX(s->beat.max));
	return zoomBounds.intersects(r);
}

void ofxTLAntescofoNote::updateDragOffsets(float clickX){
	for(int i = 0; i < switches.size(); i++){
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
		float percent_x = timeline->beatToNormalizedX(switches[i]->beat.min);
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
	newNote->growing = growing;
	newNote->channel = channel;
	newNote->startHovered = false;
	newNote->endHovered = false;
	newNote->triggeredOn = false;
	newNote->triggeredOff = false;
	switches.push_back(newNote);
	sort(switches.begin(), switches.end(), switchsort);
	drawRectChanged();
}

void ofxTLAntescofoNote::trimRange() {
	if(switches.size() > 0) {
		ofRange newRange = ofRange(58, 74);
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
	ofRectangle startRect = ofRectangle(bounds.x + bounds.width - 165 - guiXPadding, 0, 0, guiHeaderHeight);
	rangeLabelBounds = getBoundsEastOf(startRect, "Range");
	rangeMinSliderBounds = getBoundsEastOf(rangeLabelBounds, ofToString(noteRange.min));
	rangeMaxSliderBounds = getBoundsEastOf(rangeMinSliderBounds, ofToString(noteRange.max));
	rangeTrimButtonBounds = getBoundsEastOf(rangeMaxSliderBounds, "Trim");
}

ofRectangle ofxTLAntescofoNote::getBoundsEastOf(ofRectangle anchor, string label){
	return ofRectangle(anchor.x + anchor.width + guiXPadding, bounds.y - guiHeaderHeight - 4, label.size() * bitmapFontSize, guiHeaderHeight);
}



float ofxTLAntescofoNote::get_max_note_beat()
{
	float maxbeat = 0.;
	if (switches.size() > 1) {
		vector<ofxTLAntescofoNoteOn*>::iterator i = switches.end();
		i--;
		maxbeat = (*i)->beat.max;
	}
	return maxbeat;
}

