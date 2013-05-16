//
//  ofxAntescofoAction.cpp
//  ofxAntescofog
//
//  Created by Thomas Coffy on 06/12/12.
//
//
#include <sstream>
#include <algorithm>
#include <string>
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

ofxTimeline *_timeline;

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
	movingAction = false;
	movingActionRect = ofRectangle(0, 0, 40, 20);
}


ofxTLAntescofoAction::~ofxTLAntescofoAction()
{
}

void ofxTLAntescofoAction::setup()
{
	_timeline = getTimeline();

	load();

	update();
	disable();
}

void ofxTLAntescofoAction::draw()
{
	if (mActionGroups.empty()) {
		//bounds.height = 0;
		disable();
	}
	if (mScore && bounds.height > 1) {
		update_groups();
		for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
			if (zoomBounds.max >= timeline->beatToNormalizedX((*i)->beatnum)
					|| zoomBounds.min <= timeline->beatToNormalizedX((*i)->beatnum + (*i)->duration)) {
				ofSetColor(0, 0, 0, 125);

				ActionGroupHeader *act = *i;
				act->draw(this);
				
				//act->print();
				
				if (0 && movingAction) {
					ofPushStyle();
					ofFill();
					//ofSetColor(123, 13, 13, 140);
					ofSetColor(200, 200, 200, 255);
					ofRect(movingActionRect);
					ofPopStyle();
				}
					
			}
		}

#if 0
		// draw selection
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
#endif
	}

}

//--------------------------------------------------
void ofxTLAntescofoAction::drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground) {
}

void ofxTLAntescofoAction::add_action_curves(float beatnum, ActionGroup *ar, Curve *c) 
{ 
#if 0
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
#endif
}

int ofxTLAntescofoAction::get_x(float beat) {
	return normalizedXtoScreenX( timeline->beatToNormalizedX(beat), zoomBounds);
}


ofColor ofxTLAntescofoAction::get_random_color() {
	static ofColor color(205, 1, 1, 200);

	color += ofColor(10, 255, 255, 200);
	return color;
}

void ofxTLAntescofoAction::attribute_header_colors(list<ActionGroupHeader*> headers) {
	for (list<ActionGroupHeader*>::const_iterator i = headers.begin(); i != headers.end(); i++)
	{
		(*i)->headerColor = get_random_color();
		//attribute_header_colors((*i)->ActionRects);
		if ((*i)->group) { // for subgroups
			ActionGroup *a = (*i)->group;
			if (a->sons.size()) {
				for (list<ActionGroup*>::const_iterator j = a->sons.begin(); j != a->sons.end(); j++) {
					if ((*j)->header)
						(*j)->header->headerColor = get_random_color();
				}
			}
		}
	}
}

void ofxTLAntescofoAction::add_action(float beatnum, string action, Event *e)
{
	// clean tabs
	string tab("\t");
	string doublespace("  ");
	findAndReplace(action, tab, doublespace);

	ofRectangle rect(bounds.x, bounds.y, 0, 0);
	// extract data
	if (e->gfwd) {
		double delay = 0;
		if (e->gfwd && e->gfwd->delay() && e->gfwd->delay()->value() && e->gfwd->delay()->value()->is_value())
			delay = (double)e->gfwd->delay()->eval();
		ActionGroupHeader *header = new ActionGroupHeader(beatnum, delay, e->gfwd, e);
		/*
		for (vector<Action*>::const_iterator i = e->gfwd->actions().begin(); i != e->gfwd->actions().end(); i++) {
			Cfwd* c = dynamic_cast<Cfwd*>(*i);
			if (c) 
				add_action_curves(beatnum, ar, c);
		}
		*/

		// store for future use
		mActionGroups.push_back(header);

	}

	// TODO add nested event
	if (mActionGroups.size()) {
		enable();
		// ensure zoom track is at the bottom
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
		getTimeline()->bringTrackToTop(getTimeline()->getZoomer());
	} else
		disable();
	attribute_header_colors(mActionGroups);
}


void ofxTLAntescofoAction::clear_actions()
{
	for (list<ActionGroupHeader*>::iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++)
		delete *i;
	mActionGroups.clear();
}



void ofxTLAntescofoAction::save()
{}

void ofxTLAntescofoAction::load()
{
	mFont.loadFont ("DroidSansMono.ttf", 8);

}

void ofxTLAntescofoAction::update()
{
}


int ofxTLAntescofoAction::get_max_note_beat()
{
	update_groups();

	int maxbeat = 0;
	if (!mActionGroups.empty()) {
		list<ActionGroupHeader*>::const_iterator i = mActionGroups.end();
		i--;
		maxbeat = (*i)->beatnum + (*i)->delay;
	}
	return maxbeat;
}


// for updating subgroups
// return height of ActionGroup in px
int ofxTLAntescofoAction::update_sub(ActionGroup *ag)
{
	int debugsub = 0;
	if (debugsub) cout << "update_sub: " << ag->header->title << endl;
	int toth = 0, curh = 0; // find max h
	int cury = ag->header->rect.y;
	//if (! ag->header->top_level_group) cury += ag->header->HEADER_HEIGHT;

	if (ag->header->hidden) {
		return ag->header->HEADER_HEIGHT;
	} else if (!ag->header->top_level_group) {
		cury += ag->header->HEADER_HEIGHT;
		curh += ag->header->HEADER_HEIGHT;
		toth += curh;
	}
	
	/*
	ActionGroup* sg;
	if (!m && (sg = dynamic_cast<ActionGroup*>(*g))) {
		curh += HEADER_HEIGHT;
		sg->header->rect.x = get_x(sg->header->beatnum + sg->header->delay); // normalizedXtoScreenX( timeline->beatToNormalizedX(sg->header->beatnum), zoomBounds);
		if (!sg->header->top_level_group)// && g != ag->sons.begin()) { // increment y and h 
			cury += sg->header->LINE_HEIGHT;
		}
		sg->header->rect.y = cury;
		cury += sg->header->rect.height;
		toth += curh + 2;

	}*/

	list<ActionGroup*>::const_iterator g;
	for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
		ActionMessage *m = 0;
		if ((m = dynamic_cast<ActionMessage*>(*g))) {
			if (debugsub) cout << "ag->header-> " << ag->header->realtitle << endl;
			m->x = ag->header->rect.x;
			if (m->delay)
				m->x = get_x(ag->header->beatnum + ag->header->delay + m->delay);
			m->y = cury;
			if (debugsub) cout << m->action << "m->x=" << m->x << " m->y=" << m->y << " curh:" << curh << " toth:" << toth << endl;
		}
		if (!m && !(*g)->header->top_level_group) {
			(*g)->header->rect.x = ag->header->rect.x;
			if ((*g)->header->delay)
				(*g)->header->rect.x = get_x((*g)->header->beatnum + (*g)->header->delay);// normalizedXtoScreenX( timeline->beatToNormalizedX((*g)->header->beatnum + (*g)->header->delay), zoomBounds);
			//(*g)->header->rect.x += normalizedXtoScreenX( timeline->beatToNormalizedX((*g)->header->delay), zoomBounds);
			(*g)->header->rect.y = cury;
		}
	 	curh = update_sub(*g);
		if (debugsub) cout << " curh from upadte_sub:" << curh << endl;
		//ag->header->rect.height += curh;
		toth += curh;
		cury += curh;
	}
	//cout << "update_sub: return " << toth << endl;
	if (!toth) // leaf elmt -> message 
		toth = ag->header->LINE_HEIGHT;
	
	ag->header->rect.height = toth;
	return toth;
}

// for updating subgroups durations
// return width in beats
float ofxTLAntescofoAction::update_sub_duration(ActionGroup *ag)
{
	int maxw = 0, curw = 0;
	ag->header->duration = 0;

	ActionMessage *m;
	if ((m = dynamic_cast<ActionMessage*>(ag))) {
		if (m->delay) 
			return m->delay;
	} else {
		list<ActionGroup*>::const_iterator g;
		for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
			if ((*g)->header->delay)
				ag->header->duration += (*g)->header->delay;
			curw = update_sub_duration(*g);
			//if (curw > maxw)
				maxw += curw;
		}
	}
	return maxw;
}

// for updating subgroups width
// return width in px
int ofxTLAntescofoAction::update_sub_width(ActionGroup *ag)
{
	int maxw = 0, curw = 0;
	int del = 0; float beatdel = 0.;
	ag->header->duration = 0;

	//if (!ag->is_in_bounds(this)) { ag->header->rect.width = 0; return 0; }
	int debugsub = 0;
	if (debugsub) cout << "update_width: " << ag->header->title << endl;
	ActionMessage *m;
	if ((m = dynamic_cast<ActionMessage*>(ag))) {
		int sizec = mFont.stringWidth(string("_"));
		int len = sizec * (m->action.size());
		if (m->delay)
			len += get_x(m->header->beatnum + m->delay) - get_x(m->header->beatnum);
		maxw = len;
		if (debugsub) cout << "\tupdate_width: msg maxw:" << maxw << endl;

	} else {
		/*
		ActionGroup* sg;
		if ((sg = dynamic_cast<ActionGroup*>(ag))) {
			if (sg->header->delay)
				maxw += get_x(sg->header->delay);
		}
		*/
		list<ActionGroup*>::const_iterator g;
		for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
			curw = update_sub_width(*g);
			if ((*g)->header->delay) {
				beatdel = (*g)->header->delay;
				del = get_x((*g)->header->beatnum + beatdel) - get_x((*g)->header->beatnum); // get_x((*g)->header->delay);
			}
			if (curw + del > maxw)
				maxw = curw + del;
		}
		if (debugsub) cout << "\tupdate_width: maxw:" << maxw << endl;
		ag->header->rect.width = maxw;
		//maxw = del + maxw;
	}
	return maxw;
}


// avoid x overlapping
void ofxTLAntescofoAction::update_avoid_overlap_rec(ActionGroup* g, int w)
{
	if (g->header->rect.width + g->header->rect.x > w)
		g->header->rect.width = w - g->header->rect.x;
	for (list<ActionGroup*>::const_iterator i = g->sons.begin(); i != g->sons.end(); i++) {
		update_avoid_overlap_rec(*i, w);
	}
}

// avoid x overlapping
void ofxTLAntescofoAction::update_avoid_overlap()
{
	//bounds.height = 0;
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		list<ActionGroupHeader*>::const_iterator j = i;

		//ofRange r(_timeline->screenXtoNormalizedX(header->rect.x, tlAction->getZoomBounds()), _timeline->screenXtoNormalizedX(header->rect.x + header->rect.width, tlAction->getZoomBounds()));
		//if (++j != mActionGroups.end() && (zoomBounds.contains(timeline->beatToNormalizedX((*j)->beatnum)) || zoomBounds.contains(timeline->beatToNormalizedX((*j)->beatnum) + (*j)->duration)) ) {
		//if (++j != mActionGroups.end() && zoomBounds.contains(timeline->beatToNormalizedX((*j)->beatnum))) {
		if (++j != mActionGroups.end() && (*i)->group->is_in_bounds(this) ) {
			if ( ((*i)->rect.x + (*i)->rect.width) + 1 >= (*j)->rect.x) {
				update_avoid_overlap_rec((*i)->group, (*j)->rect.x /*- (*i)->rect.x*/ - 2);
			}
		}
		//cout << "Action hearder height:"  << (*i)->rect.height <<" x:" << bounds.x << endl;
	}
#if 0 //TODO should do 
	list<ActionGroup*>::const_iterator g;
	for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
		if (! ag->header->top_level_group) {
			(*g)->header->rect.y += ag->header->HEADER_HEIGHT;
			break;
		}
	}
	// loop rec ?
#endif
#if 0
	// check maximum height, and set track height
	float maxh = 0;
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++)
		if (maxh < (*i)->rect.height)
			maxh = (*i)->rect.height;

	if (maxh > bounds.height) {
		bounds.height = maxh;
		//cout << "ofxTLAntescofoAction update_avoid_overlap: changing action track size:" << maxh << endl;
	}
#endif
}

// for updating subgroups y
int ofxTLAntescofoAction::update_sub_y(ActionGroup *ag)
{

	/*
	// go deep and assign each header rect.y
	list<ActionGroup*>::const_iterator g;
	for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
		if (! ag->header->top_level_group) {
			(*g)->header->rect.y += ag->header->HEADER_HEIGHT;
			break;
		}
	}
	// loop rec ?
	*/
	return 0;
}



void ofxTLAntescofoAction::update_groups()
{
	// update actions' rect
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		//if ((*i)->group && !(*i)->group->is_in_bounds(this)) continue;
		(*i)->rect.x = normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->beatnum), zoomBounds);
		(*i)->rect.y = bounds.y + 3;
		//update_sub_y((*i)->group);
		update_sub_duration((*i)->group);
		(*i)->rect.width = update_sub_width((*i)->group);
		
		if (!(*i)->hidden && (*i)->group) {
			(*i)->rect.height = update_sub((*i)->group);
		}
	}

	update_avoid_overlap();
}

//--------------------------------------------------------------
void ofxTLAntescofoAction::windowResized(int w, int h){

}

bool ofxTLAntescofoAction::mousePressed_In_Arrow(ofMouseEventArgs& args, ActionGroup* group) {
	bool res = false;
	if (!group) return res;
	ActionGroupHeader *header = group->header;
	//cout << "mousePressed: rect:"<< header->realtitle<< " x:" << header->rect.x << " y:" << header->rect.y << " w:"<< header->rect.width << " h:" << header->rect.height << endl; 
	if (header->rect.inside(args.x, args.y)) {
		if (header->hidden) {
			header->hidden = false;
			//cout << "mousePressed: setting action '"<< header->title <<"' hidden:"<< header->hidden << endl;
			if (group) {
				Event *e = group->event;
				//mAntescofog->editorShowLine(e->scloc->begin.line, e->scloc->end.line);
				res = true;
			}
			return res;
		} else if (header->is_in_arrow(args.x, args.y)) {
			header->hidden = true;
			res = true;
		} else {
			if (group) { // not hidden and click in group
				Event *e = group->event;
				//mAntescofog->editorShowLine(e->scloc->begin.line, e->scloc->end.line);
				if (group->sons.size()) { // rec in subgroups
					ActionGroup *a = group;
					for (list<ActionGroup*>::const_iterator j = a->sons.begin(); j != a->sons.end(); j++) {
						if ((*j)->header && (*j)->header->group && (*j)->header->group != group 
								&&  mousePressed_In_Arrow(args, (*j)->header->group)) {
							return true;
						}
					}
				}
			}
		}
	}
	return res;
}

ActionGroup* ofxTLAntescofoAction::groupFromScreenPoint_rec(ActionGroup* group, int x, int y) {
	ActionGroup* rg = 0;
	for (list<ActionGroup*>::const_iterator i = group->sons.begin(); i != group->sons.end(); i++) {
		if ((*i)->header->rect.inside(x, y) && (*i)->header != group->header) {
			if (!(rg = groupFromScreenPoint_rec(*i, x, y))) //&& *i == *(group->sons.end())) 
				return *i;
			else return rg;
		}
	}
	if (group->header && group->header->rect.inside(x, y)) {
		return group;
	}
	return 0;
}

ActionGroup* ofxTLAntescofoAction::groupFromScreenPoint(int x, int y) {
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		ActionGroup *gr = 0;
		if ((*i)->rect.inside(x, y) && (gr = groupFromScreenPoint_rec((*i)->group, x, y)))
			return gr;
	}
	return 0;
}

void ofxTLAntescofoAction::regionSelected(ofLongRange timeRange, ofRange valueRange) {
	cout << "regionSelected:" << timeRange.min << " " << timeRange.max << " y: " << valueRange.min << " " << valueRange.max << endl;
}

bool ofxTLAntescofoAction::mousePressed(ofMouseEventArgs& args, long millis)
{
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++)
		(*i)->group->selected = false;


	//cout << "mousePressed: x:"<< args.x << " y:" << args.y << endl; 
	bool res = false;
	// selection
	ActionGroup* clickedGroup = groupFromScreenPoint(args.x, args.y);
	if (clickedGroup) {
		cout << "clickedGroup: " << clickedGroup->header->realtitle << endl;
		/*
		clickedGroup->selected = true;
		movingAction = true;
		movingActionRect.x = args.x;
		movingActionRect.y = args.y;
		*/
		if (clickedGroup->header)
			mAntescofog->editorShowLine(clickedGroup->header->lineNum_begin, clickedGroup->header->lineNum_end);
	}
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {

		if (!(*i)->top_level_group && mousePressed_In_Arrow(args, (*i)->group)) {
			if (!clickedGroup) mAntescofog->editorShowLine((*i)->lineNum_begin, (*i)->lineNum_end);
			res = true;
			return res;
		} else if ((*i)->group) { // look for subgroups
			ActionGroup *a = (*i)->group;
			if (a->sons.size()) {
				for (list<ActionGroup*>::const_iterator j = a->sons.begin(); j != a->sons.end(); j++) {
					if ((*j)->header &&  mousePressed_In_Arrow(args, (*j)->header->group)) {
						if (!clickedGroup) mAntescofog->editorShowLine((*i)->lineNum_begin, (*i)->lineNum_end);
						res = true;
						return res;
					}
				}
			}
		}// else if (header->rect.inside(args.x, args.y)) { }
	}

#if 0
	else { // selecting
		draggingSelectionRange = true;
		selectionRangeAnchor.x = args.x;
		selectionRangeAnchor.y = args.y;
		dragSelection.x = selectionRangeAnchor.x;
		dragSelection.y = selectionRangeAnchor.y;
		dragSelection.width = 0;
		dragSelection.height = 0;
	}
#endif
	return res;
}

void ofxTLAntescofoAction::mouseMoved(ofMouseEventArgs& args, long millis)
{
}


void ofxTLAntescofoAction::mouseDragged(ofMouseEventArgs& args, long millis)//bool snapped);
{
	if (movingAction) {
		movingActionRect.x = args.x;
		movingActionRect.y = args.y;
	}

	if(draggingSelectionRange){
		//dragSelection.min = MIN(args.x, selectionRangeAnchor);
		//dragSelection.max = MAX(args.x, selectionRangeAnchor);
		dragSelection.x = MIN(args.x, selectionRangeAnchor.x);
		dragSelection.y = MIN(args.y, selectionRangeAnchor.y);
	}
}

// move action from action->selected beatnum 
// to moveActionRect. x y
void ofxTLAntescofoAction::move_action() {
	list<ActionGroupHeader*>::iterator from;
	for (from = mActionGroups.begin(); from != mActionGroups.end(); from++) {
		if ((*from)->group && (*from)->group->selected) {
			(*from)->group->selected = false;
			break;
		}
	}
	if (from == mActionGroups.end() || !(*from)->group)
		return;
	bool doneMoving = false;
	ActionGroup* dest = groupFromScreenPoint(movingActionRect.x, movingActionRect.y);
	// if dest ActionGroup exists : simply add it
	if (dest) {
		cout << "move_action: dest found" << endl;
		dest->sons.push_back((*from)->group);
		doneMoving = true;
	} else { // else find the event and plug it to it
		cout << "move_action: dest not found" << endl;
		for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
			if ((*i)->rect.x <= movingActionRect.x && (*i)->rect.x + (*i)->rect.width >= movingActionRect.x) {
				cout << "move_action: dest finally found" << endl;
				if (!(*i)->group)
					(*i)->group = (*from)->group;
				else (*i)->group->sons.push_back((*from)->group);
				//(*from)->group->header = *i;
				dest = (*i)->group;
				doneMoving = true;
				break;
			}
		}
	}

	if (doneMoving) {// remove from from
		// assign dest header to from subgroups
		for (list<ActionGroup*>::const_iterator j = (*from)->group->sons.begin(); j != (*from)->group->sons.end(); j++) {
			(*j)->header = dest->header;
		}
		//(*from)->group->header = dest->header;
		(*from)->group = 0;
		mActionGroups.erase(from);
		delete *from;
		//update_groups();
	}


	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		//(*i)->print();

	}
}

void ofxTLAntescofoAction::mouseReleased(ofMouseEventArgs& args, long millis)
{
	if (!bounds.inside(args.x, args.y)) return;
	if (movingAction) {
		//move_action();
	}
	movingAction = false;
	/*
	if(draggingSelectionRange){
		for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
			//normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->beatnum), zoomBounds)) 
			if((*i)->group && (dragSelection.contains( (*i)->rect.x) || dragSelection.contains( (*i)->rect.x + (*i)->rect.width)))
				(*i)->group->selected = true;
		}
		draggingSelectionRange = false;
	}
	*/
}

void ofxTLAntescofoAction::keyPressed(int key){

}

void ofxTLAntescofoAction::keyPressed(ofKeyEventArgs& args)
{}

void ofxTLAntescofoAction::setScore(Score* score)
{
	mScore = score;
}

string ofxTLAntescofoAction::cut_str(int w, string in)
{
	int sizec = mFont.stringWidth(string("_"));
	//cout << "ActionGroup: cur_str: w:"<< w << " in:" << in << " nc=" << in.size() * sizec  << endl;
	if (w < sizec)
		return string("");
	if (w < in.size() * sizec) {
		int nc = w / sizec - 1;
		//cout << "ActionGroup: cur_str: cutting action msg :nc = " << nc << endl;
		return in.substr(0, nc);
	}
	return in;
}


ofRectangle ofxTLAntescofoAction::getBoundedRect(ofRectangle& r)
{
	ofRectangle ret = r;
	if (ret.x < bounds.x && ret.x + ret.width > bounds.x) {
		ret.x = bounds.x;
		ret.width = r.width - ret.x - abs(r.x);
	}
	if (ret.x + ret.width > bounds.x + bounds.width)
		ret.width = bounds.width - ret.x + bounds.x;// + abs(r.x);

	return ret;
}

void ofxTLAntescofoAction::replaceEditorScore(ActionCurve* actioncurve) {
	vector<Action*>::const_iterator i;
	float d = 0;
	string newscore;
	ostringstream oss;
	string actstr;


	if (actioncurve) {
		actioncurve->parentCurve->curve->show(oss);
		actstr = oss.str();
		mAntescofog->replaceEditorScore(actioncurve->header->lineNum_begin, actioncurve->header->lineNum_end, actstr);
	}
	
	/*
	for (list<ActionGroupHeader*>::const_iterator ahi = mActionGroups.begin(); ahi != mActionGroups.end(); ahi++) {
		newscore += (*ahi)->dump();
		
	}
	cout << "===================================================" << endl;
	cout << "   Got new full score : " << endl;
	cout << newscore << endl;
	cout << "===================================================" << endl;
	*/
		/* parser structures way
		if ((*ahi)->group && (*ahi)->group->gfwd) {
			Gfwd *g = (*ahi)->group->gfwd;
			for (i = g->actions().begin(); i != g->actions().end(); i++) {
				Action *tmpa = *i;

				// can be a curve
				Curve* c = dynamic_cast<Curve*>(tmpa);
				if (c && modifiedcurve) {
					ActionMultiCurves *cu = new ActionMultiCurves(c, d, event, header); 
					sons.push_back((ActionGroup*)cu);
					continue;
				} else { // use parser dump to dump 
					oss.str(""); oss.clear();
					list<Action*> l = g->bloc();
					for (list<Action*>::iterator a = l.begin(); a != l.end(); a++) {
						if (*a)
							oss << *a;
					}
					actstr = oss.str();
					newscore += actstr;
					actstr.clear();
				}
			}
		}
	}
		*/


}

////////////////////////////////////////////////////

ActionGroup::ActionGroup(Gfwd* g, Event *e, ActionGroupHeader* header_) 
			: header(header_), gfwd(g), event(e), period(0), selected(false)
{
	if (g) {
		vector<Action*>::const_iterator i;
		float d = 0;
		for (i = g->actions().begin(); i != g->actions().end(); i++) {
			Action *tmpa = *i;

			d += get_delay(tmpa);
			// can be group
			Gfwd *gg = dynamic_cast<Gfwd*>(tmpa);
			if (gg) {
				ActionGroupHeader* nh = new ActionGroupHeader(header->beatnum, d, gg, event);//, false);
				if (nh->group)
					sons.push_back(nh->group);
				continue;
			}

			// can be message
			Message *m = dynamic_cast<Message*>(tmpa);
			if (m) {
				ActionMessage *am = new ActionMessage(m, d, event, header); 
				sons.push_back((ActionGroup*)am);
				continue;
			}

			// can be a curve
			Curve* c = dynamic_cast<Curve*>(tmpa);
			if (c) {
				ActionMultiCurves *cu = new ActionMultiCurves(c, d, event, header); 
				sons.push_back((ActionGroup*)cu);
				continue;
			}

			// can be a loop
			Lfwd* l = dynamic_cast<Lfwd*>(tmpa);
			if (l && l->_group) {
				//ActionLoop *lu = new ActionLoop(l, d, event, header); 
				//sons.push_back((ActionGroup*)lu);
				ActionGroupHeader* nh = new ActionGroupHeader(header->beatnum, d, l->_group, event);//, false);
				if (nh->group) {
					if (l->_period && l->_period->value()->is_value())
						nh->group->period = l->_period->eval();
					nh->realtitle = nh->title = l->label();
					sons.push_back(nh->group);
				}
				continue;
			}

			header->duration += get_delay(tmpa);
		}
	}
}

ActionGroup::~ActionGroup()
{
	if (trackName.size()) {
		cout << "ActionGroup: removing track " << trackName << endl;
		_timeline->removeTrack(trackName);
	}

	list<ActionGroup*>::const_iterator i;
	for (i = sons.begin(); i != sons.end(); i++) {
		delete *i;
	}
}


string ActionGroup::dump()
{
	string groupdump;
	ostringstream oss;

	if (event) {
		oss << *event;
		groupdump += oss.str();
	}

	if (gfwd) {
		string actstr;
		vector<Action*>::const_iterator i;
		for (i = gfwd->actions().begin(); i != gfwd->actions().end(); i++) {
			Action *tmpa = *i;

			oss.str(""); oss.clear();
			//list<Action*> l = ->bloc();
			//for (list<Action*>::iterator a = l.begin(); a != l.end(); a++) {
				if (tmpa)
					oss << *tmpa; //*a;
			//}
			actstr = oss.str();
			groupdump += actstr;
			actstr.clear();
		}

	}
	if (header && !header->top_level_group) {
		list<ActionGroup*>::const_iterator s;
		for (s = sons.begin(); s != sons.end(); s++) {
			groupdump += (*s)->dump();
		}
	}
	return groupdump;
}


double ActionGroup::get_delay(Action* tmpa) 
{
	double d = 0.;
	if (tmpa && tmpa->delay() && tmpa->delay()->value()->is_constant()) { //->value() && tmpa->delay()->value()->is_value()) {
		d = eval_double(tmpa->delay()->value());
	}
	return d;
}

void ActionGroup::print() {
	if (period) cout << "ActionGroup loop period: " << period << endl;
	if (header && !header->top_level_group)
		header->print();
	for (list<ActionGroup*>::iterator i = sons.begin(); i != sons.end(); i++) {
		(*i)->print();
	}
}

bool ActionGroup::is_in_bounds(ofxTLAntescofoAction *tlAction) {
	ofRange r(_timeline->screenXtoNormalizedX(header->rect.x, tlAction->getZoomBounds()), _timeline->screenXtoNormalizedX(header->rect.x + header->rect.width, tlAction->getZoomBounds()));
	return tlAction->getZoomBounds().intersects(r);
}

void ActionGroup::draw(ofxTLAntescofoAction *tlAction)
{
	if (header && header->group && header->group->is_in_bounds(tlAction))
		header->draw(tlAction);
}

/* 
   Curve
 */
ActionMultiCurves::ActionMultiCurves(Curve* c, float delay_, Event *e, ActionGroupHeader* header_)
{
	delay = delay_;
	header = header_;
	curve = c;
	if (curve) {
		cout << "got MultiCurve: " << curve->label() << endl; 
		label = curve->label();
		// TODO get attributes..

		header->lineNum_begin = curve->locate()->begin.line;
		header->lineNum_end = curve->locate()->end.line;

		howmany = curve->seq_vect.size();

		for (uint i = 0; i < howmany; i++)
		{
			SeqContFunction* s = curve->seq_vect[i];
			list<Var*>::iterator it_var = s->var_list->begin();
			for (uint j = 0; /*j < s->s_vect[0][0].size() &&*/ it_var != s->var_list->end(); j++, it_var++)
			{
				string var = (*it_var)->name();
				cout << "ActionMultiCurves: adding sub curve for var:"<< var << endl;

				// TODO grain
				ActionCurve* newc = new ActionCurve(var, &s->s_vect[j], &s->dur_vect, 0, e, header, this);
				sons.push_back(newc);
			}
		}
	}
}

ActionMultiCurves::~ActionMultiCurves()
{
}

void ActionMultiCurves::draw(ofxTLAntescofoAction *tlAction) {
}

void ActionMultiCurves::print() {
}


string ActionMultiCurves::dump()
{
	string groupdump;
	if (curve) {
		ostringstream oss;
		string actstr;

		oss.str(""); oss.clear();
		curve->show(oss);
		groupdump = oss.str();
	}

	return groupdump;
}

ActionCurve::ActionCurve(string var, vector<SimpleContFunction>* simple_vect_, vector<AnteDuration*>* dur_vect_, float delay_, Event *e, ActionGroupHeader* header_, ActionMultiCurves* parentCurve_)
	: parentCurve(parentCurve_)
{
	delay = delay_;
	header = header_;
	event = e;
	varname = var;
	simple_vect = simple_vect_;
	dur_vect = dur_vect_;
	if (e) {
		cout << "ofxTLAntescofoAction::add_action: ActionCurve: got "<< dur_vect->size() << " delays" << endl;// << seq->label() << endl; 
		// get values
		vector<double> hvalues;
		for (uint j = 0; j != dur_vect->size() - 1; j++) {
			FloatValue* f = dynamic_cast<FloatValue*>((*simple_vect)[j].y0);
			if (f) {
				double dou = f->get_double();
				cout << "ofxTLAntescofoAction::add_action: got values:" << dou << endl;
				hvalues.push_back(dou);
			} else {
				IntValue* in = dynamic_cast<IntValue*>((*simple_vect)[j].y0);
				if (in) {
					int ii = in->get_int();
					cout << "ofxTLAntescofoAction::add_action: got values:" << ii << endl;
					hvalues.push_back(ii);
				}
			}
			//cout << "ActionCurve : got y0:" << s_vect[j].y0->get_double() << " y1:" << s_vect[j].y1->get_double() << " type:"<< s_vect[j].type << endl;
		} 
		// get last y1
		FloatValue* f = dynamic_cast<FloatValue*>((*simple_vect)[simple_vect->size()-1].y1);
		if (f) {
			double dou = f->get_double();
			cout << "ofxTLAntescofoAction::add_action: got values:" << dou << endl;
			hvalues.push_back(dou);
		} else {
			IntValue* in = dynamic_cast<IntValue*>((*simple_vect)[simple_vect->size()-1].y1);
			if (in) {
				int ii = in->get_int();
				cout << "ofxTLAntescofoAction::add_action: got values:" << ii << endl;
				hvalues.push_back(ii);
			}
		}
		values.push_back(hvalues);
		// get delays
		for (std::vector<AnteDuration*>::const_iterator j = dur_vect->begin(); j != dur_vect->end(); j++) {
			FloatValue* f = dynamic_cast<FloatValue*>((*j)->value());
			if (f) {
				double dou = f->get_double();
				cout << "ofxTLAntescofoAction::add_action: got delay:" << dou << endl;
				delays.push_back(dou);
			} else {
				IntValue* in = dynamic_cast<IntValue*>((*j)->value());
				if (in) {
					int ii = in->get_int();
					cout << "ofxTLAntescofoAction::add_action: got delay:" << ii << endl;
					delays.push_back(ii);
				}
			}
		}
		//cout << " ============ values:"<< values.size() << " delays:"<<  delays.size() << " ========= " << endl;
		// add track
		if (values.size() && delays.size() && (*(values.begin())).size()) {
			trackName = string("CURVE ") + varname;
			string uniqueName = _timeline->confirmedUniqueName(trackName);

			ofxTLBeatCurves* curves = new ofxTLBeatCurves();

			_timeline->addTrack(trackName, curves);
			curves->tlAction = (ofxTLAntescofoAction *)_timeline->getTrack("Actions");
			curves->ref = this;
			// set values ranges
			int j = 0; 
			float min = 0, max = 0;
			for (vector<double>::iterator i = values[0].begin(); i != values[0].end(); i++) {
				if (min > (*i)) min = (*i);
				if (max < (*i)) max = (*i);
			}
			curves->setValueRangeMax(max);
			cout << "ofxTLAntescofoAction::add_action: CFWD value max: "<< max << endl;
			cout << "ofxTLAntescofoAction::add_action: CFWD value min: "<< min << endl;
			curves->setValueRangeMin(min);
			// set keyframes
			double dcumul = 0.;
			for (int k = 0; k < delays.size(); k++) {
				//cout << " ------- delays["<<n<<"]: " << ar->cfwd->delays[n] << endl;
				dcumul += delays[k];
				//for (vector< vector<double> >::iterator i = ar->cfwd->values.begin(); i != ar->cfwd->values.end(); i++) {
				cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << 0 << "] msec=" << _timeline->beatToMillisec(header->beatnum + dcumul)
					<< " val=" <<  values[0][k] << endl;

				//c->addKeyframeAtMillis(ar->cfwd->values[i], ofxAntescofoNote->beatToMillisec(ar->beatnum + dcumul));
				curves->addKeyframeAtBeat(values[0][k], header->beatnum + dcumul);
				// }
				curves->enable();
			}
		}
	}
}

ActionCurve::~ActionCurve()
{
}

bool debug_edit_curve = true;

bool ActionCurve::set_dur_val(double d, AnteDuration* a)
{
	cout << "ActionCurve::set_dur_val: " << d << endl;
	assert(d);
	if (Value* v = (Value*)a->value()->is_value()) {
		*v = FloatValue(d);
		return true;
	} else {
		cerr << "ActionCurve: ERROR : set_dur_val: can not convert to FloatValue" << a->value() << endl; 
		return false;
	}
}

bool ActionCurve::set_y(Expression* y, double val) {
	Value* v = (Value*)y->is_value();
	if (v) {
		*v = FloatValue(val);
		return true;
	} else {
		cerr << "ActionCurve: ERROR : set_y: can not convert to FloatValue" << endl; 
		return false;
	}
}


void ActionCurve::print()
{
	cout << "ActionCurve::print: "<< endl; 
	vector<SimpleContFunction>::iterator s = simple_vect->begin();
	for (vector<AnteDuration*>::iterator k = dur_vect->begin(); k != dur_vect->end(); k++, s++) {
		cout << "\tdelay: " << (*k)->eval();
		if (s != simple_vect->end())
			cout << "  y0: " << s->y0 << " y1: " << s->y1 << endl;
	}
	cout << endl;
}

FloatValue* ActionCurve::get_new_y(Expression* y) {
	if (y && y->is_value() && y->is_value()->is_numeric()) {
		if (debug_edit_curve) cout << "get_new_y: " << eval_double(y->is_value()) << endl;
		return new FloatValue(eval_double(y->is_value()));
	} else return NULL;
}

void ActionCurve::deleteKeyframeAtBeat(float beat) {
	if (beat == 0) {
		//set_dur_val(val, simple_vect->begin());
		abort();
		return;
	}

	double dcumul = 0.;
	int i = 0;
	bool done = false;

	if (dur_vect->size() == 2) {
		cerr << "Can not delete point because curves need at least 2 points" << endl;
		return;
	}

	if (debug_edit_curve) { cout << "Entering deleteKeyframeAtBeat: " << beat << endl; parentCurve->curve->show(cout); print(); }
	vector<SimpleContFunction>::iterator s = simple_vect->begin();
	for (vector<AnteDuration*>::iterator k = dur_vect->begin(); k != dur_vect->end() /* && s != simple_vect->end()*/; k++, i++, s++) {
		if (debug_edit_curve) cout << "ofxTLAntescofoAction:: del keyframe  looping : " << i << " curdur:" << (*k)->eval() <<  " dcumul:" << dcumul<< endl;

		dcumul += (*k)->eval();
		//cout << "------ y0 " << (*s).y0 << endl; cout << " y1:" << (*s).y1 << endl;
		if (dcumul < beat)
			continue;
		else {
			vector<AnteDuration*>::iterator p = k;
			p--;
			// delete k, but change prev duration += k duration
			if (s == simple_vect->end()) { //k == dur_vect->end()) {
				cout << "------------------------------------ last elt ---------------" << endl;
					dur_vect->erase(k);
					simple_vect->erase(s);
			} else {
				vector<AnteDuration*>::iterator n = k;
				n++;
				if (set_dur_val((*n)->eval() + (*k)->eval(), *n)) {
					// add new point to simple_vect[]
					if (debug_edit_curve) cout << "Next point duration : "<< (*n)->eval() << endl;
					dur_vect->erase(k);
					vector<SimpleContFunction>::iterator sp = s;
					sp--;
					sp->y1 = get_new_y(s->y1);
					simple_vect->erase(s);
				} else { cout << "Can not convert to Value." << endl; }
			}
			break;
		}
	}
	if (!done && dcumul < beat) { // delete last point
		cout << "ofxTLAntescofoAction:: del keyframe: delete !done !!!!!!!!!!!!!!!!" << endl;
		/*
		vector<AnteDuration*>::iterator k = dur_vect->end();
		k--;
		vector<SimpleContFunction>::iterator s = simple_vect->end();
		s--;
		AnteDuration *ad = new AnteDuration(beat - (*k)->eval());
		simple_vect->push_back(SimpleContFunction(s->antesc, new StringValue("linear"), ad, get_new_y(s->y1), new FloatValue(val), s->var));
		dur_vect->push_back(ad);
		//set_dur_val(dcumul - beat, *k)
		*/
	}
	parentCurve->curve->show(cout);
	print();


}

// when new breakpoint is created, we should reduce prev breakpoint duration, before adding
void ActionCurve::addKeyframeAtBeat(float beat, float val)
{
	if (debug_edit_curve) { cout << "ofxTLAntescofoAction:: add keyframe at beat: " << beat << " val: " << val << endl; print(); }
	if (beat == 0) {
		//set_dur_val(val, simple_vect->begin());
		return;
	}

	double dcumul = 0.;
	int i = 0;
	bool done = false;

	vector<SimpleContFunction>::iterator s = simple_vect->begin();
	for (vector<AnteDuration*>::iterator k = dur_vect->begin(); k != dur_vect->end() /* && s != simple_vect->end()*/; k++, i++, s++) {
		if (debug_edit_curve) cout << "ofxTLAntescofoAction:: add keyframe at beat: looping : " << i << " curdur:" << (*k)->eval() <<  " dcumul:" << dcumul<< endl;

		dcumul += (*k)->eval();
		//cout << "------ y0 " << (*s).y0 << endl; cout << " y1:" << (*s).y1 << endl;
		if (dcumul < beat)
			continue;
		else {
			vector<AnteDuration*>::iterator p = k;
			p--;
			// insert point between p and k 
			// substract (dcumul - beat) on prev point
			double d = beat - (*p)->eval();
			if (set_dur_val(dcumul - beat, *k)) {
				// add new point to simple_vect[]
				if (debug_edit_curve) cout << "New point duration : beat:"<< beat << " k:"<< (*k)->eval() << " duration:" << d << endl;
				if (i <= simple_vect->size()) {
					vector<SimpleContFunction>::iterator sp = s; sp--;
					AnteDuration* ad = new AnteDuration(d);
					FloatValue* ny1 = get_new_y(sp->y1);
					assert(ny1);
					FloatValue* ny0 = new FloatValue(val);
					s = simple_vect->insert(s, SimpleContFunction(sp->antesc, new StringValue("linear"), ad, ny0, ny1, s->var));
					dur_vect->insert(k, ad);
					// change prev y1
					s--;
					s->y1 = ny0;
					done = true;
					if (debug_edit_curve) cout << "looping stopped, inserted: " << i << endl;
				} 
			} else { cout << "Can not convert to Value." << endl; }
			break;
		} 
	}
	if (!done && dcumul < beat) { // insert beat after last point
		vector<AnteDuration*>::iterator k = dur_vect->end();
		k--;
		vector<SimpleContFunction>::iterator s = simple_vect->end();
		s--;
		AnteDuration *ad = new AnteDuration(beat - (*k)->eval());
		simple_vect->push_back(SimpleContFunction(s->antesc, new StringValue("linear"), ad, get_new_y(s->y1), new FloatValue(val), s->var));
		dur_vect->push_back(ad);
		//set_dur_val(dcumul - beat, *k)
	}
	parentCurve->curve->show(cout);
	print();
}


void ActionCurve::changeKeyframeEasing(float beat, string type) {
	if (debug_edit_curve) cout << "ActionCurve::changeKeyframeEasing: beat:"<< beat << " type:"<< type << endl;
	//if (type == "sine")
	
	float dcumul = 0;
	int i = 0;
	bool done = false;

	vector<SimpleContFunction>::iterator s = simple_vect->begin();
	for (vector<AnteDuration*>::iterator k = dur_vect->begin(); k != dur_vect->end(); k++, i++, s++) {
		if (debug_edit_curve) cout << "ofxTLAntescofoAction:: change keyframe easing at beat: looping : " << i << " curdur:" << (*k)->eval() <<  " dcumul:" << dcumul<< endl;

		dcumul += (*k)->eval();

		if (dcumul < beat)
			continue;
		else {
			if (debug_edit_curve) cout << "change: is value:" << s->type->is_value() << endl;
			if (s->type && s->type->is_value()) {
				if (debug_edit_curve) cout << "ofxTLAntescofoAction:: change keyframe" << endl;
				Value* v = (Value*)s->type->is_value();
				//StringValue *sv = dynamic_cast<StringValue*>(s->type);
				*v = StringValue(type);
				done = true;
				break;
			}
		}
	}
	if (done) {
		parentCurve->curve->show(cout);
		print();
	}
}


// 
void ActionCurve::moveKeyframeAtBeat(float to_beat, float from_beat, float to_val, float from_val)
{
	if (debug_edit_curve) 
	{
		cout << "ofxTLAntescofoAction:: move keyframe from beat: " << from_beat << " (val:" << from_val << ") to beat: " << to_beat << "(val:" << to_val <<")"<< endl;
		print();
	}
	//if (from_beat == 0) { // TODO set val return; }

	double dcumul = 0.;
	double dfrom_beat = from_beat;
	int i = 0;

	vector<SimpleContFunction>::iterator s = simple_vect->begin();
	for (vector<AnteDuration*>::iterator k = dur_vect->begin(); k != dur_vect->end(); k++, i++, s++) {
		if (debug_edit_curve) cout << "ofxTLAntescofoAction:: move keyframe at beat: looping : " << i << endl;

		dcumul += (double)((*k)->eval());

		if (dcumul < dfrom_beat)
			continue;
		else if ( dcumul > dfrom_beat - 0.01 && dcumul < dfrom_beat + 0.01) { //dcumul == dfrom_beat) 
			if (debug_edit_curve) cout << "ofxTLAntescofoAction:: beat found" << endl;
			// prev dur -= (from_beat - to_beat)
			vector<AnteDuration*>::iterator p = k;
			p--;
			AnteDuration *d = *k;
			if (set_dur_val(d->value()->is_value()->get_double() - (from_beat - to_beat), *k)) {
				// change current delay
				//if (debug_edit_curve) cout << "Changed point duration : " << to_beat - (*p)->eval() << endl;
				// change next
				if (k != dur_vect->end()) {
					double nextdur = 0.;
					vector<AnteDuration*>::iterator n = k; n++;
					AnteDuration *an = *n;
					if (an) {
						if (an->value() && an->value()->is_value()->get_double() + from_beat < to_beat) { // point dragged after next point 
							cout << "!!!!!!!!!!!!!!!!!!!!!!! negative delay TODO" << endl;

						} else {
							nextdur = an->value()->is_value()->get_double() + (from_beat - to_beat);
							set_dur_val(nextdur, *n);
						}
					}
				}
				vector<SimpleContFunction>::iterator sp = s;
				if (s != simple_vect->end())
					set_y(s->y0, to_val);
				s--;
				set_y(s->y1, to_val);
			}
			break;
		} else {
			cout << "An error moveKeyframeAtBeat...: dcumul:" << dcumul << " from_beat:"<< dfrom_beat << endl;
			abort();
		}
	}

	parentCurve->curve->show(cout);
	print();
}


/*
	Message
 */
ActionMessage::ActionMessage(Message* m, float delay_, Event *e, ActionGroupHeader* header_) 
{
	header = header_;
	selected = false;
	ostringstream oss;
	oss << *m;
	action = oss.str();
	x = header->rect.x;
	y = header->rect.y;
	//header->hidden = false;
	//ofRectangle tmprect = rect; tmprect.y += HEADER_HEIGHT;
	//ActionRect *newact = new ActionRect(act, header->beatnum + get_delay(m), NULL, event);
	//ActionRects.push_back(newact);

	delay = delay_;
	cout << "Action: adding message with delay: " << delay << " : " << action << endl;
}

void ActionMessage::print() {
	cout << "**** Action Message: " << " x:" << x << " y:" << y << " " <<  action;// << endl;
}


void ActionMessage::draw(ofxTLAntescofoAction* tlAction) {
	//if (!is_in_bounds(tlAction)) return;
	ofNoFill();
	ofSetColor(0, 0, 0, 255);
	tlAction->mFont.drawString(tlAction->cut_str(header->rect.width, action), x + 1, y + header->LINE_HEIGHT - 6);
	if (selected) { ofSetColor(255, 0, 0, 255); ofSetLineWidth(3); }
	ofRect(tlAction->getBoundedRect(header->rect));
	//if (selected) ofPopStyle();
}


ActionGroupHeader::ActionGroupHeader(float beatnum_, float delay_, Action* a_, Event *e_)
	: beatnum(beatnum_), action(a_), event(e_), top_level_group(false), duration(0),
	hidden(true), delay(delay_),
	HEADER_HEIGHT(16), ARROW_LEN(15), LINE_HEIGHT(18), LINE_SPACE(12)
{
	if (action) {
		string lab = action->label();
		cout << "ActionGroupHeader: adding : " << lab << endl;
		realtitle = lab;
		if (lab.size() && strncmp(lab.c_str(), "top_gfwd_", 9) == 0) {
			top_level_group = true;
			hidden = false;
		}
		title = "Group " + action->label();
		rect.height = HEADER_HEIGHT;
		Gfwd *g = dynamic_cast<Gfwd*>(action);
		if (g) {
			lineNum_begin = action->locate()->begin.line;
			lineNum_end = action->locate()->end.line + 1;

			group = new ActionGroup(g, event, this);
		}
	}
}

ActionGroupHeader::~ActionGroupHeader()
{
	if (group)
		delete group;
}

string ActionGroup::get_period()
{
	string ret;
	std::ostringstream oss;
	oss << period;

	ret = oss.str();
	return ret;
}

void ActionGroupHeader::draw(ofxTLAntescofoAction *tlAction) 
{
	ofSetLineWidth(1);
	//cout << "ActionRects.draw: label:"<< realtitle<< " inbounds:" << group->is_in_bounds(tlAction) <<  " x:"<<rect.x << " y:" << rect.y << " " << rect.width <<  "x"<< rect.height << endl;
	if (top_level_group && group && group->is_in_bounds(tlAction)) {
		ofFill();
		ofSetColor(200, 200, 200, 255);
		ofRectangle inrect = rect;
		inrect.x += 1; inrect.y = rect.y + 1;
		inrect.height = inrect.height - 2;
		inrect.width = inrect.width - 2;
		ofRect(tlAction->getBoundedRect(inrect)); // draw white body

		list<ActionGroup*>::const_iterator i;
		for (i = group->sons.begin(); i != group->sons.end(); i++) {
			if ((*i)->is_in_bounds(tlAction))
				(*i)->draw(tlAction);
		}
	} else if (!hidden) {
		int sizec = tlAction->mFont.stringWidth(string("_"));
		if (group && group->is_in_bounds(tlAction)) {
			ofFill();
			ofSetColor(200, 200, 200, 255);
			ofRectangle inrect = rect;
			inrect.x += 1; inrect.y = rect.y + 1;
			inrect.height = inrect.height - 2;
			if (!top_level_group) { inrect.height -= HEADER_HEIGHT; inrect.y += HEADER_HEIGHT; }
			inrect.width = inrect.width - 2;
			ofRect(tlAction->getBoundedRect(inrect));
			if (!top_level_group) {
				ofFill(); // rect color filled
				ofSetColor(headerColor);
				ofRectangle recthead = rect;
				recthead.height = HEADER_HEIGHT;
				ofRect(tlAction->getBoundedRect(recthead));

				ofNoFill();
				ofSetColor(0, 0, 0, 255);
				if (rect.width > ARROW_LEN + 4)
					drawArrow(); // arrow
				string name = title;
				if (group && group->period > 0) { name = "Loop " + realtitle; name += " period:"; name += group->get_period(); }
				tlAction->mFont.drawString(tlAction->cut_str(rect.width, name), rect.x + 1, rect.y + LINE_HEIGHT - 6);
				ofNoFill(); // black border
				if (group->selected) {ofSetColor(255, 0, 0, 255); ofSetLineWidth(3); }
				ofRect(tlAction->getBoundedRect(rect));
			} 
			//cout << "ActionGroupHeader.draw !hidden: ("<<rect.x<<","<<rect.y<<", "<< rect.width << "x"<< rect.height << ") : " << title << endl;
			list<ActionGroup*>::const_iterator i;
			for (i = group->sons.begin(); i != group->sons.end(); i++) {
				if ((*i)->is_in_bounds(tlAction))
					(*i)->draw(tlAction);
			}
		}
	} else { // hidden, just draw header and arrow
		ofFill(); // rect color filled
		ofSetColor(headerColor);
		rect.height = HEADER_HEIGHT;
		ofRect(tlAction->getBoundedRect(rect)); // header filled rect
		ofNoFill();
		ofSetColor(0, 0, 0, 255);
		if (rect.width > ARROW_LEN + 4)
			drawArrow(); // arrow
		string name = title;
		if (group && group->period > 0) {name = "Loop " + realtitle; name += " period:"; name += group->get_period(); }
		tlAction->mFont.drawString(tlAction->cut_str(rect.width, name), rect.x + 1, rect.y + LINE_HEIGHT - 6);
		ofNoFill();
		if (group->selected) {ofSetColor(255, 0, 0, 255); ofSetLineWidth(3); }
		ofRect(tlAction->getBoundedRect(rect)); // black border rect
		//cout << "ActionGroupHeader.draw hidden: ("<<rect.x<<","<<rect.y<<") : " << title << endl;
	}
}

/*
   2
   1
   3
   */
void ActionGroupHeader::drawArrow() {
	//cout << "ActionRects.draw arrow " << title << " ("<<rect.x<<","<<rect.y<<") : hidden:" << hidden << endl; 
	int xlen = ARROW_LEN;
	int space = 3;
	int halfHeight = HEADER_HEIGHT / 2;

	int x1, y1, x2, y2, x3, y3;
	if (hidden) {
		x1 = rect.x + rect.width - xlen - space;
		y1 = rect.y + halfHeight;
		x2 = rect.x + rect.width - space;
		y2 = rect.y + space;
		x3 = x2;
		y3 = rect.y + HEADER_HEIGHT - space;
	} else {
		x1 = rect.x + rect.width - xlen - space;
		y1 = rect.y + space;
		x2 = rect.x + rect.width - space;
		y2 = y1;
		x3 = rect.x + rect.width - space - xlen/2;
		y3 = rect.y + HEADER_HEIGHT - space;
	}

	arrow.setFillColor(ofColor(0, 0, 0, 200));
	arrow.setFilled(true);
	arrow.lineTo(x1, y1);
	arrow.lineTo(x2, y2);
	arrow.lineTo(x3, y3);
	arrow.lineTo(x1, y1);
	arrow.close();
	arrow.draw();
	arrow.clear();
}

void ActionGroupHeader::print() {
	cout << "***** ActionGroup Header:" << realtitle << " line:" << lineNum_begin << ":"<< lineNum_end<< " beat:" << beatnum << " delay:"<< delay <<" dur:" << duration << " hidden:"<< hidden 
	     << " x:" << rect.x << " y:" << rect.y << " w:" << rect.width << " h:" << rect.height << endl; //" height:" << get_height(tlAction);
	if (top_level_group && group)
		group->print();
}


string ActionGroupHeader::dump() {
	string groupdump;

	if (group) {
		groupdump += group->dump();
		
	}
	return groupdump;
}


bool ActionGroupHeader::is_in_arrow(int x, int y)
{
	bool res = false;
	if (x > rect.x + rect.width - ARROW_LEN && y < rect.y + HEADER_HEIGHT)
		res = true;
	cout << "is in arrow: " << x << " " << y << " return " << res << endl;
	return res;
}


/*
ActionLoop::ActionLoop(Lfwd *l, float delay_, Event *e, ActionGroupHeader* header_)
	: lfwd(l)
{
	delay = delay_;
	header = header_;
	if (l) {
		cout << "got Loop: " << l->label() << endl; 
		label = l->label();
		double dou = 0.;

		if (l->_period) {
			period = l->_period->eval();
		}
		if (l->_group)
			ActionGroup *g = new ActionGroup(l->_group, event, header);
	}
}


void ActionLoop::draw(ofxTLAntescofoAction *tlAction)
{


}

void ActionLoop::print()
{
	cout << "**** Action Loop: " << label << " period:" << period<< endl;
}


*/




/* old cfwd:
ActionCurve::ActionCurve(Cfwd* c, float delay_, Event *e, ActionGroupHeader* header_)
{
	delay = delay_;
	header = header_;
	if (c) {
		cout << "got Curve: " << c->label() << endl; 
		label = c->label();
		CfwdStep* s = c->_continuous_step;
		double dou = 0.;

		// TODO get _beta

		// get grain
		if (s->_grain) {
			FloatValue *fgrain = dynamic_cast<FloatValue*>(s->_grain->value());
			if (fgrain) {
				dou = fgrain->get_double();
				cout << "ofxTLAntescofoAction::add_action: got grain:" << dou << endl;
				grain = dou;
			} else {
				IntValue* in = dynamic_cast<IntValue*>(s->_grain->value());
				if (in) {
					dou = fgrain->get_int();
					cout << "ofxTLAntescofoAction::add_action: got grain:" << dou << endl;
					grain = dou;
				}
			}
		}
		// get values
		vector< vector <double> >::iterator vvalues = values.begin();
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
			values.push_back(hvalues);
		}
		// get delays
		for (std::vector<AnteDuration*>::const_iterator j = c->_delays_vector.begin(); j != c->_delays_vector.end(); j++) {
			FloatValue* f = dynamic_cast<FloatValue*>((*j)->value());
			if (f) {
				dou = f->get_double();
				cout << "ofxTLAntescofoAction::add_action: got delay:" << dou << endl;
				delays.push_back(dou);
			} else {
				IntValue* in = dynamic_cast<IntValue*>((*j)->value());
				if (in) {
					dou = in->get_int();
					cout << "ofxTLAntescofoAction::add_action: got delay:" << dou << endl;
					delays.push_back(dou);
				}
			}
		}
		cout << "ofxTLAntescofoAction::add_action: delays size:" << delays.size() << endl;
	}

	//for (int n = 0; n < ar->cfwd->delays.size(); n++) { cout << " ------- delays " << ar->cfwd->delays[n] << endl; }

	// add track
	if (values.size() && delays.size() && (*(values.begin())).size()) {
		trackName = string("CURVE ") + label;
		string uniqueName = _timeline->confirmedUniqueName(trackName);

		int howmany = (*(values.begin())).size();
		if (howmany == 1) { // mono curves
			ofxTLBeatCurves* curves = new ofxTLBeatCurves();
			_timeline->addTrack(trackName, curves);

			// set values ranges
			int j = 0; 
			for (int j = 0; j < howmany; j++) { // useless lazy copy-pasted loop from below
				float min = 0, max = 0;
				for (vector< vector<double> >::iterator i = values.begin(); i != values.end(); i++) {
					if (min > (*i)[j]) min = (*i)[j];
					if (max < (*i)[j]) max = (*i)[j];
				}
				curves->setValueRangeMax(max);
				cout << "ofxTLAntescofoAction::add_action: CFWD value max: "<< max << endl;
				cout << "ofxTLAntescofoAction::add_action: CFWD value min: "<< min << endl;
				curves->setValueRangeMin(min);
			}
			// set keyframes
			for (int n = 0; n < howmany; n++) { // useless lazy copy-pasted loop from below
				double dcumul = 0.;
				for (int k = 0; k < delays.size(); k++) {
					//cout << " ------- delays["<<n<<"]: " << ar->cfwd->delays[n] << endl;
					dcumul += delays[k];
					//for (vector< vector<double> >::iterator i = ar->cfwd->values.begin(); i != ar->cfwd->values.end(); i++) {
					cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << n << "] msec=" << _timeline->beatToMillisec(header->beatnum + dcumul)
						<< " val=" <<  values[k][n] << endl;

					//c->addKeyframeAtMillis(ar->cfwd->values[i], ofxAntescofoNote->beatToMillisec(ar->beatnum + dcumul));
					curves->addKeyframeAtBeat(values[k][n], header->beatnum + dcumul);
					// }
				}
				curves->enable();
			}
		} else { // multicurves
			ofxTLMultiCurves* multicurves = new ofxTLMultiCurves();
			multicurves->clear();
			multicurves->disable();
			_timeline->addTrack(trackName, multicurves);
			multicurves->setHowmany(howmany);

			// set values ranges
			int j = 0; 
			for (int j = 0; j < howmany; j++) {
				float min = 0, max = 0;
				for (vector< vector<double> >::iterator i = values.begin(); i != values.end(); i++) {
					if (min > (*i)[j]) min = (*i)[j];
					if (max < (*i)[j]) max = (*i)[j];
				}
				multicurves->setValueRangeMax(j, max);
				cout << "ofxTLAntescofoAction::add_action: CFWD value max: "<< max << endl;
				cout << "ofxTLAntescofoAction::add_action: CFWD value min: "<< min << endl;
				multicurves->setValueRangeMin(j, min);
			}
			// set keyframes
			for (int n = 0; n < howmany; n++) {
				double dcumul = 0.;
				for (int k = 0; k < delays.size(); k++) {
					//cout << " ------- delays["<<n<<"]: " << ar->cfwd->delays[n] << endl;
					dcumul += delays[k];
					//for (vector< vector<double> >::iterator i = ar->cfwd->values.begin(); i != ar->cfwd->values.end(); i++) {
					cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << n << "] msec=" << _timeline->beatToMillisec(header->beatnum + dcumul)
						<< " val=" <<  values[k][n] << endl;

					//c->addKeyframeAtMillis(ar->cfwd->values[i], ofxAntescofoNote->beatToMillisec(ar->beatnum + dcumul));
					multicurves->addKeyframeAtBeatAtCurveId(n, values[k][n], header->beatnum + dcumul);
					// }
				}
				multicurves->enable();
			}
		} // end multicurves
	}

}*/
