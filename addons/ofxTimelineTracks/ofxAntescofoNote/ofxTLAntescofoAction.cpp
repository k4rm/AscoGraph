//
//  ofxAntescofoAction.cpp
//  ofxAntescofog
//
//  Created by Thomas Coffy on 06/12/12.
//
//
#include <algorithm>
#include <string>
#include "ofxTimeline.h"
#include "ofxTLAntescofoNote.h"
#include "ofxTLAntescofoAction.h"
#include "ofxTLMultiCurves.h"
#include "Score.h"
#include "Values.h"
#include "Action.h"
#include <location.hh>
#include <position.hh>
#include <ofxAntescofog.h>
#include "ofxCodeEditor.h"


template<class T>
int inline findAndReplace(T& source, const T& find, const T& replace)
{
	int num=0;
	int fLen = find.size();
	int rLen = replace.size();
	for (int pos=0; (pos=source.find(find, pos))!=T::npos; pos+=rLen)
	{
		num++;
		source.replace(pos, fLen, replace);
	}
	return num;
}

ofxTLAntescofoAction::ofxTLAntescofoAction(ofxAntescofog *Antescofog)
{
	mAntescofog = Antescofog;
	bEditorShow = false;
}


ofxTLAntescofoAction::~ofxTLAntescofoAction()
{
}

void ofxTLAntescofoAction::setup()
{
	load();

	disable();
}

void ofxTLAntescofoAction::draw()
{
    if (mActionRects.empty()) {
        //bounds.height = 0;
				disable();
		}
	if (mScore && bounds.height > 6) {
		for (list<ActionRect*>::const_iterator i = mActionRects.begin(); i != mActionRects.end(); i++) {
			if (zoomBounds.intersects(timeline->beatToNormalizedX((*i)->beatnum))) {
				ofSetColor(0, 0, 0, 125);
				ofRect((*i)->rect);
				ofSetColor(255, 255, 255, 255);
				mFont.drawString((*i)->drawn_action, (*i)->rect.x, (*i)->rect.y + 15);
			}
		}
	}


}

//--------------------------------------------------
void ofxTLAntescofoAction::drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground) {
}

void ofxTLAntescofoAction::add_action(float beatnum, string action, Event *e)
{

	// clean tabs
	string tab("\t");
	string doublespace("  ");
	findAndReplace(action, tab, doublespace);

	ActionRect *ar = new ActionRect(action, beatnum, e);

	// extract data
	if (e->gfwd) {
		for (vector<Action*>::const_iterator i = e->gfwd->actions().begin(); i != e->gfwd->actions().end(); i++)
		{
			cout << "ofxTLAntescofoAction::add_action: adding : " << endl << action<< endl;
			Cfwd* c = dynamic_cast<Cfwd*>(*i);
			if (c) {
				cout << "got Cfwd: " << c->label() << endl; 
				Display_cfwd* d = new Display_cfwd();
				d->label = c->label();
				CfwdStep* s = c->_continuous_step;
				double dou = 0.;

				// TODO get _beta

				// get grain
				if (s->_grain) {
					FloatValue *fgrain = dynamic_cast<FloatValue*>(s->_grain->value());
					if (fgrain) {
						dou = fgrain->get_double();
						cout << "ofxTLAntescofoAction::add_action: got grain:" << dou << endl;
						d->grain = dou;
					} else {
						IntValue* in = dynamic_cast<IntValue*>(s->_grain->value());
						if (in) {
							dou = fgrain->get_int();
							cout << "ofxTLAntescofoAction::add_action: got grain:" << dou << endl;
							d->grain = dou;
						}
					}
				}
				// get values
				vector< vector <double> >::iterator vvalues = d->values.begin();
				for (std::vector< std::vector<Expression*>* >::const_iterator j = c->_values_vector.begin(); j != c->_values_vector.end(); j++) {
					vector<double> hvalues;// = d->values.begin();
					for (std::vector<Expression* >::const_iterator k = (*j)->begin(); k != (*j)->end(); k++) {
						FloatValue* f = dynamic_cast<FloatValue*>(*k);
						if (f) {
							dou = f->get_double();
							cout << "ofxTLAntescofoAction::add_action: got values:" << dou << endl;
							hvalues.push_back(dou);
						} else {
							IntValue* in = dynamic_cast<IntValue*>(*k);
							if (in) {
								dou = in->get_int();
								cout << "ofxTLAntescofoAction::add_action: got values:" << dou << endl;
								hvalues.push_back(dou);
							}
						}
					}
					//cout << "ofxTLAntescofoAction::add_action: push values:" << endl;
					d->values.push_back(hvalues);
				}
				// get delays
				for (std::vector<AnteDuration*>::const_iterator j = c->_delays_vector.begin(); j != c->_delays_vector.end(); j++) {
					FloatValue* f = dynamic_cast<FloatValue*>((*j)->value());
					if (f) {
						dou = f->get_double();
						cout << "ofxTLAntescofoAction::add_action: got delay:" << dou << endl;
						d->delays.push_back(dou);
					} else {
						IntValue* in = dynamic_cast<IntValue*>((*j)->value());
						if (in) {
							dou = in->get_int();
							cout << "ofxTLAntescofoAction::add_action: got delay:" << dou << endl;
							d->delays.push_back(dou);
						}
					}
				}
				ar->cfwd = d;
				cout << "ofxTLAntescofoAction::add_action: delays size:" << d->delays.size() << endl;
			}
		}

		//for (int n = 0; n < ar->cfwd->delays.size(); n++) { cout << " ------- delays " << ar->cfwd->delays[n] << endl; }
		// add track
		if (ar && ar->cfwd && ar->cfwd->values.size() && ar->cfwd->delays.size() && (*(ar->cfwd->values.begin())).size()) {
			string trackName = string("CFWD ") + ar->cfwd->label;
			string xmlFileName;
			if(xmlFileName == ""){
				string uniqueName = getTimeline()->confirmedUniqueName(trackName);
				xmlFileName = ofToDataPath("GUI/" + uniqueName + "_.xml");
			}

			int howmany = (*(ar->cfwd->values.begin())).size();
			ofxTLMultiCurves* curves = new ofxTLMultiCurves();
			curves->clear();
			curves->disable();
			getTimeline()->addTrack(trackName, curves);
			curves->setHowmany(howmany);
			ar->trackName = trackName;

			// set values ranges
			int j = 0; 
			for (int j = 0; j < howmany; j++) {
				float min = 0, max = 0;
				for (vector< vector<double> >::iterator i = ar->cfwd->values.begin(); i != ar->cfwd->values.end(); i++) {
					if (min > (*i)[j]) min = (*i)[j];
					if (max < (*i)[j]) max = (*i)[j];
				}
				curves->setValueRangeMax(j, max);
				cout << "ofxTLAntescofoAction::add_action: CFWD value max: "<< max << endl;
				cout << "ofxTLAntescofoAction::add_action: CFWD value min: "<< min << endl;
				curves->setValueRangeMin(j, min);
			}

			// set keyframes
			for (int n = 0; n < howmany; n++) {
				double dcumul = 0.;
				for (int k = 0; k < ar->cfwd->delays.size(); k++) {
					//cout << " ------- delays["<<n<<"]: " << ar->cfwd->delays[n] << endl;
					dcumul += ar->cfwd->delays[k];
					//for (vector< vector<double> >::iterator i = ar->cfwd->values.begin(); i != ar->cfwd->values.end(); i++) {
					cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << n << "] msec=" << timeline->beatToMillisec(ar->beatnum + dcumul)
						<< " val=" <<  ar->cfwd->values[k][n] << endl;

					//c->addKeyframeAtMillis(ar->cfwd->values[i], ofxAntescofoNote->beatToMillisec(ar->beatnum + dcumul));
					curves->addKeyframeAtBeatAtCurveId(n, ar->cfwd->values[k][n], ar->beatnum + dcumul);
					// }
				}

				curves->enable();
			}
		}
	}

	// store for future use
	mActionRects.push_back(ar);

	// TODO add nested event
	if (mActionRects.size()) {
		enable();
		// ensure zoom track is at the bottom
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
	} else
		disable();
}


void ofxTLAntescofoAction::clear_actions()
{
	//cout << "ofxTLAntescofoAction::clear_actions";
	for (list<ActionRect*>::iterator i = mActionRects.begin(); i != mActionRects.end(); i++) {
		cout << "ofxTLAntescofoAction::clear_actions: removing track " << (*i)->trackName;
		getTimeline()->removeTrack((*i)->trackName);
	}
	mActionRects.clear();
}



void ofxTLAntescofoAction::save()
{}

void ofxTLAntescofoAction::load()
{
	mFont.loadFont ("DroidSansMono.ttf", 8);

}

void ofxTLAntescofoAction::update()
{

	for (list<ActionRect*>::const_iterator i = mActionRects.begin(); i != mActionRects.end(); i++)
		(*i)->w = 0;
	if (mScore && bounds.height > 6) {
		int sizec = mFont.stringWidth(string("a"));
		// check width limit for action boxes
		// calculate each witdh : has to be limited when next action comes
		for (list<ActionRect*>::const_iterator i = mActionRects.begin(); i != mActionRects.end(); i++)
		{
			if (zoomBounds.contains(timeline->beatToNormalizedX((*i)->beatnum))) {
				vector<string> lines = ofSplitString((*i)->action, "\n");
				int maxw = 0;
				for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); i++) maxw = MAX(i->size(), maxw);
				float width = maxw * (sizec + mFont.getLetterSpacing() + mFont.getSpaceSize());
				list<ActionRect*>::const_iterator j = i;
				if (++j != mActionRects.end() && zoomBounds.contains(timeline->beatToNormalizedX((*j)->beatnum))) {
					int x = normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->beatnum), zoomBounds);
					int nextx = normalizedXtoScreenX( timeline->beatToNormalizedX((*j)->beatnum), zoomBounds);
					float width = maxw * (sizec + mFont.getLetterSpacing() + mFont.getSpaceSize());
					if (x + width > nextx)
						(*i)->w = nextx - x - 3;
				}
			}
		}

		for (list<ActionRect*>::const_iterator i = mActionRects.begin(); i != mActionRects.end(); i++)
		{
			if (zoomBounds.contains(timeline->beatToNormalizedX((*i)->beatnum))) {
				int x = normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->beatnum), zoomBounds);
				int strx = bounds.x + x;
				int stry = bounds.y + 4;
				// cut height 
				string sub = (*i)->action;
				float height = mFont.stringHeight(sub) + mFont.getSpaceSize();
				vector<string> lines = ofSplitString((*i)->action, "\n");
				while (height > bounds.height) {
					lines.erase(lines.end());
					height = 0;
					for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); i++)
						height += mFont.getLineHeight();//mFont.stringHeight(*i);//  mFont.getSpaceSize();
					//cout << "ofxTLAntescofoAction::draw : bounds:" << bounds.height << " : string:" << height << endl;
				}
				if (lines.size() > 2) lines.erase(lines.end());
				// cut width
				int maxw = 0;
				
				for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); i++) {
					maxw = MAX(i->size(), maxw);
				}
				float width = maxw * (sizec + mFont.getLetterSpacing() + mFont.getSpaceSize());
				if ((*i)->w) width = (*i)->w;
				ofRectangle rs(strx, stry, width, height + 13);//mFont.getStringBoundingBox(sub, strx, stry); 
				(*i)->rect = rs;
				//cout << "ofxTLAntescofoAction::draw: bounds:" << bounds.height << " : string:" << rs.width << "x"<< rs.height << endl;
				//cout << "line count: " << lines.size() << endl;
				string renders;
				for (vector<string>::const_iterator l = lines.begin(); l != lines.end(); l++) {
					int k;
					if ((*i)->w) // cut lines if width is shorter
						k = (*i)->w / (sizec + mFont.getLetterSpacing() + mFont.getSpaceSize());
					else k = l->size();
					//cout << "string size:"<< l->size() << " w:"<<(*i)->w << " k:" << k << endl;
					renders += l->substr(0, k) + "\n";
				}
				(*i)->drawn_action = renders;
			}
		}
	}


}

//--------------------------------------------------------------
void ofxTLAntescofoAction::windowResized(int w, int h){

}

bool ofxTLAntescofoAction::mousePressed(ofMouseEventArgs& args, long millis)
{
	for (list<ActionRect*>::const_iterator i = mActionRects.begin(); i != mActionRects.end(); i++)
	{
		int x = normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->beatnum), zoomBounds);
		if ((*i)->rect.inside(args.x, args.y)) {
			string str = (*i)->action.substr(0, 20);
			//if (mAntescofog->bEditorShow)
			Event *e = (*i)->e;
			mAntescofog->editorShowLine(e->scloc->begin.line, e->scloc->end.line);
				//mAntescofog->editor->searchText(str);
			/*ofstream f;
			  f.open(TEXT_CONSTANT_TEMP_ACTION_FILENAME);
			  f << (*i)->action;
			  f.close();
			  bEditorShow = true;
			  mAntescofog->setEditorMode(bEditorShow, (*i)->beatnum);
			  */
		} /*else {
		    if (bEditorShow) {
		    bEditorShow = false;
		    mAntescofog->setEditorMode(bEditorShow, r, (*i)->beatnum);
		    }
		    }*/
	}
	return false;
}

void ofxTLAntescofoAction::mouseMoved(ofMouseEventArgs& args, long millis)
{}


void ofxTLAntescofoAction::mouseDragged(ofMouseEventArgs& args, long millis)//bool snapped);
{}

void ofxTLAntescofoAction::mouseReleased(ofMouseEventArgs& args, long millis)
{

}

void ofxTLAntescofoAction::keyPressed(int key){

}

void ofxTLAntescofoAction::keyPressed(ofKeyEventArgs& args)
{}

void ofxTLAntescofoAction::setScore(Score* score)
{
	mScore = score;
}
