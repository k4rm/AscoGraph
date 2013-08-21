//
//  ofxAntescofoAction.cpp
//  part of AscoGraph : graphical editor for Antescofo musical scores.
//
//  Created by Thomas Coffy on 06/12/12.
//  Licensed under the Apache License : http://www.apache.org/licenses/LICENSE-2.0
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

bool debug_edit_curve = false;
bool debug_actiongroup = false;
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
		// draw close cross
		ofSetColor(0, 0, 0, 255);
		ofRect(mRectCross);
		ofLine(mRectCross.x, mRectCross.y, mRectCross.x+mRectCross.width, mRectCross.y+mRectCross.height);
		ofLine(mRectCross.x+mRectCross.width, mRectCross.y, mRectCross.x, mRectCross.y+mRectCross.height);

		update_groups();
		for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
			if (zoomBounds.max >= timeline->beatToNormalizedX((*i)->beatnum)
					|| zoomBounds.min <= timeline->beatToNormalizedX((*i)->beatnum + (*i)->duration)) {
				ofSetColor(0, 0, 0, 125);

				ActionGroupHeader *act = *i;
				act->draw(this);
				
				act->print();
				
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
		// store for future use
		mActionGroups.push_back(header);

	}

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
	mRectCross.x = bounds.x + bounds.width - 20;
	mRectCross.y = bounds.y - 15;
	mRectCross.width = 14;
	mRectCross.height = 14;
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
	int debugsub = 1;
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
		ActionMultiCurves *c = 0;
		ActionCurve *ac = 0;
		if ((m = dynamic_cast<ActionMessage*>(*g))) {
			if (debugsub) cout << "ag->header-> " << ag->header->realtitle << endl;
			m->x = ag->header->rect.x;
			if (m->delay)
				m->x = get_x(ag->header->beatnum + ag->header->delay + m->delay);
			m->y = cury;
			if (debugsub) cout << m->action << "m->x=" << m->x << " m->y=" << m->y << " curh:" << curh << " toth:" << toth << endl;
		} else if ((c = dynamic_cast<ActionMultiCurves*>(*g))) {
			/*
			c->x = ag->header->rect.x;
			if (c->delay)
				c->x = get_x(ag->header->beatnum + ag->header->delay + c->delay);
			c->y = cury;
			*/
		} else if ((ac = dynamic_cast<ActionCurve*>(*g))) {
			continue;
		}


		if (!m && !(*g)->header->top_level_group) {
			(*g)->header->rect.x = ag->header->rect.x;
			if ((*g)->header->delay)
				(*g)->header->rect.x = get_x((*g)->header->beatnum + (*g)->header->delay);// normalizedXtoScreenX( timeline->beatToNormalizedX((*g)->header->beatnum + (*g)->header->delay), zoomBounds);
			//(*g)->header->rect.x += normalizedXtoScreenX( timeline->beatToNormalizedX((*g)->header->delay), zoomBounds);
			(*g)->header->rect.y = cury;
			cout << "******************* curh:"<< curh << " cury="<< cury << endl;
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
	ActionMultiCurves *c;
	if ((m = dynamic_cast<ActionMessage*>(ag))) {
		int sizec = mFont.stringWidth(string("_"));
		int len = sizec * (m->action.size());
		if (m->delay)
			len += get_x(m->header->beatnum + m->delay) - get_x(m->header->beatnum);
		maxw = len;
		if (debugsub) cout << "\tupdate_width: msg maxw:" << maxw << endl;

	} else if ((c = dynamic_cast<ActionMultiCurves*>(ag))) {
		int sizec = mFont.stringWidth(string("_"));
		int len = sizec * (ag->header->title.size());
		if (c->delay)
			len += get_x(c->header->beatnum + m->delay) - get_x(c->header->beatnum);
		maxw = len;
		if (debugsub) cout << "\tupdate_width: curves msg maxw:" << maxw << endl;

	} 
	{
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

		if (++j != mActionGroups.end() && (*i)->group->is_in_bounds(this) ) {
			if ( ((*i)->rect.x + (*i)->rect.width) + 1 >= (*j)->rect.x) {
				update_avoid_overlap_rec((*i)->group, (*j)->rect.x /*- (*i)->rect.x*/ - 2);
			}
		}
		//cout << "Action hearder height:"  << (*i)->rect.height <<" x:" << bounds.x << endl;
	}
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

				ActionMultiCurves* c = 0;
				if ((c = dynamic_cast<ActionMultiCurves*>(group)))
					c->show();

			}
			return res;
		} else if (header->is_in_arrow(args.x, args.y)) {
			header->hidden = true;
			res = true;

			// hide curve tracks
			ActionMultiCurves* c = 0;
			if ((c = dynamic_cast<ActionMultiCurves*>(group)))
				c->hide();
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
	if (!bounds.inside(args.x, args.y)) return false;
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

	if (mRectCross.inside(args.x, args.y)) {
		cout << "ofxTLAntescofoAction::mouseReleased: should close track" << endl;
		disable();
		timeline->removeTrack(this);
		return;
	}
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
				ActionGroupHeader* nh = new ActionGroupHeader(header->beatnum, d, c, event);//, false);

				//ActionMultiCurves *cu = new ActionMultiCurves(c, d, event, nh); 
				//sons.push_back((ActionGroup*)cu);
				if (nh->group)
					sons.push_back(nh->group);
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
	period = 0.;
	if (curve) {
		cout << "got MultiCurve: " << curve->label() << endl; 
		label = curve->label();

		header->lineNum_begin = curve->locate()->begin.line;
		header->lineNum_end = curve->locate()->end.line;

		nbvects = curve->seq_vect.size();


		cout << "ActionMultiCurves: contains " << nbvects << " vectors." << endl;
		for (uint i = 0; i < nbvects; i++)
		{
			howmany = curve->seq_vect[i]->var_list->size();
			cout << "ActionMultiCurves: contains vect:" << nbvects << " howmany:"<< howmany << endl;
			SeqContFunction* s = curve->seq_vect[i];
			list<Var*>::iterator it_var = s->var_list->begin();
			//for (uint j = 0; /*j < s->s_vect[0][0].size() &&*/ it_var != s->var_list->end(); j++, it_var++)
			//{
				string var = (*it_var)->name();
				cout << "ActionMultiCurves: adding sub curve:" << endl;

				ActionCurve* newc = new ActionCurve(*(s->var_list), s, &s->dur_vect, 0, e, header, this);
				sons.push_back(newc);
			//}
		}
		hide();
#if 0
		cout << "ActionMultiCurves: contains " << nbvects << " vectors:"<< nbvects << endl;
		for (uint i = 0; i < nbvects; i++)
		{
			howmany = curve->seq_vect[i]->var_list->size();
			cout << "ActionMultiCurves: contains vect:" << nbvects << " howmany:"<< howmany << endl;
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
#endif

	}
}

void ActionMultiCurves::hide() {
	list<ActionGroup*>::iterator j;
	for (j = sons.begin(); j != sons.end(); j++) {
		ActionCurve* c = (ActionCurve*)*j;
		if (c->curves)
			_timeline->removeTrack(c->curves);
	}

}


void ActionMultiCurves::show() {
	list<ActionGroup*>::iterator j;
	for (j = sons.begin(); j != sons.end(); j++) {
		ActionCurve* c = (ActionCurve*)*j;

		c->simple_vect = &c->seq->s_vect[0];
		c->createTracks_from_parser_objects(c->vars, c->dur_vect, c->delay, c->event, header, this);
	}

}


/*
 * Split a multi curve into tracks
 * that means copy for each member of seq_vect
 *      for each var: create a curve
 */
void ActionCurve::split()
{
	if (parentCurve->curve) {
		cout << "ActionMultiCurves:: split " << parentCurve->curve->label() << endl; 
		label = parentCurve->curve->label();
		int nbvects = parentCurve->curve->seq_vect.size();

		vector<SeqContFunction*> listseq;
		ofxTLAntescofoAction* actionTrack = (ofxTLAntescofoAction *)_timeline->getTrack("Actions");
		cout << "ActionMultiCurves: contains " << nbvects << " vectors." << endl;
		int nbtracks = 0;
		for (uint i = 0; i < nbvects; i++)
		{
			parentCurve->howmany = parentCurve->curve->seq_vect[i]->var_list->size();
			cout << "ActionMultiCurves: contains vect:" << nbvects << " howmany:"<< parentCurve->howmany << endl;
			SeqContFunction* s = parentCurve->curve->seq_vect[i];
			list<Var*>::iterator it_var = s->var_list->begin();
			// accumulate types
			vector<Expression*> listinterp;

			for (int j = 0; j < s->s_vect[0].size(); j++) {
				listinterp.push_back(s->s_vect[0][j].type);
			}
			for (uint j = 0; /*j < s->s_vect[0][0].size() &&*/ it_var != s->var_list->end(); j++, it_var++)
			{
				nbtracks++;
				vector<Expression*> listexp; 
				vector<SimpleContFunction>* simple_vect = &parentCurve->curve->seq_vect[i]->s_vect[j]; //&(seq->s_vect[j]);
				// get values
				vector<double> hvalues;
				for (uint j = 0; j != parentCurve->curve->seq_vect[i]->dur_vect.size() - 1; j++) {
					listexp.push_back((*simple_vect)[j].y0);
				}
				// get last y1
				listexp.push_back((*simple_vect)[simple_vect->size()-1].y1);
				
				antescofo *ao = (actionTrack)->mAntescofog->ofxAntescofoNote->mAntescofo;
				SeqContFunction* n = new SeqContFunction(ao, *it_var, s->dur_vect, listexp, listinterp, s->grain, *parentCurve->curve);
				listseq.push_back(n);
			}
		}
		// assign new seq_vect
		vector<SeqContFunction*> oldseq = parentCurve->curve->seq_vect;
		parentCurve->curve->seq_vect = listseq;
		parentCurve->curve->show(cout);


		// remove track
		/*
		list<ActionGroup*>::iterator j;
		for (j = sons.begin(); j != sons.end(); ) {
			list<ActionGroup*>::iterator currentg;
			ActionGroup *g = *j;
			j++;
			sons.erase(currentg);
			//delete g;
		}*/
		_timeline->removeTrack(trackName);
		/*
		parentCurve->howmany = 1;
		// add tracks
		vector<SeqContFunction*>::iterator i;
		for (i = listseq.begin(); i != listseq.end(); i++) {
			createTracks_from_parser_objects(*(*i)->var_list, *i, &(*i)->dur_vect, 0, event, header, parentCurve);
		}
		*/

		actionTrack->replaceEditorScore(this);
		string newscore;
		[actionTrack->mAntescofog->editor getEditorContent:newscore];
		((ofxTLAntescofoNote *)_timeline->getTrack("Notes"))->loadscoreAntescofo_fromString(newscore);
	}
}

ActionMultiCurves::~ActionMultiCurves()
{
}


void ActionMultiCurves::draw(ofxTLAntescofoAction *tlAction) {
	//header->rect.width
	ActionGroup::draw(tlAction);
}


void ActionMultiCurves::print() {
	cout << "***** ActionMultiCurves: got " << howmany << " curves." << endl;
	ActionGroup::print();
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

ActionCurve::ActionCurve(list<Var*> &var, SeqContFunction* seq_, vector<AnteDuration*>* dur_vect_, float delay_, Event *e, ActionGroupHeader* header_, ActionMultiCurves* parentCurve_)
	: parentCurve(parentCurve_), vars(var), curves(0), seq(seq_)
{
	delay = delay_;
	header = header_;
	event = e;
	vars = var;
	period = 0.;
	for (list<Var*>::iterator i = var.begin(); i != var.end(); i++) {
		varname += (*i)->name();
		if (i != var.end()) varname += ", ";
	}
	simple_vect = &seq->s_vect[0];
	dur_vect = dur_vect_;

	//createTracks_from_parser_objects(var, seq, dur_vect, delay, e, header, parentCurve_);
}


bool ActionCurve::createTracks_from_parser_objects(list<Var*> &var, vector<AnteDuration*>* dur_vect_, float delay_, Event *e, ActionGroupHeader* header_, ActionMultiCurves* parentCurve_) 
{
	//simple_vect = &seq->s_vect[0];
	dur_vect = dur_vect_;
	parentCurve = parentCurve_;

	cout << "ofxTLAntescofoAction::adding ActionCurve: for var: "<< varname << " : " << dur_vect->size() << " delays" << endl;// << seq->label() << endl; 
	if (values.empty() && delays.empty()) {
		for (int i = 0; i < var.size(); i++) {
			vector<SimpleContFunction>* simple_vect = &(seq->s_vect[i]);
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
		}
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
				else if ((*j)->value()->is_constant()) {
					double val = eval_double((*j)->value());
					cout << "ofxTLAntescofoAction::add_action: got delay:" << val << endl;
					delays.push_back(val);
				}
			}
		}
	}
	//cout << " ============ values:"<< values.size() << " delays:"<<  delays.size() << " ========= " << endl;
	// add track
	if (values.size() && delays.size() && (*(values.begin())).size()) {
		string uniqueName = _timeline->confirmedUniqueName(trackName);

		if (parentCurve->howmany > 1) {
			trackName = string("Curves ") + parentCurve->label + string(" ") +varname;
			ofxTLMultiCurves* curve = new ofxTLMultiCurves();
			_timeline->addTrack(trackName, curve);
			curve->setTimeline(_timeline);
			curve->setHowmany(parentCurve->howmany);

			for (int i = 0; i < parentCurve->howmany; i++) {
				curve->getCurve(i)->tlAction = (ofxTLAntescofoAction *)_timeline->getTrack("Actions");
				curve->getCurve(i)->ref = this;
				// set values ranges
				int j = 0; 
				float min = 0, max = 0;
				for (vector<double>::iterator ii = values[i].begin(); ii != values[i].end(); ii++) {
					if (min > (*ii)) min = (*ii);
					if (max < (*ii)) max = (*ii);
				}
				curve->setValueRangeMax(i, max);
				curve->setValueRangeMin(i, min);
				// set keyframes
				double dcumul = 0.;
				vector<SimpleContFunction>::iterator s = simple_vect->begin();
				for (int k = 0; k < delays.size(); k++, s++) {
					dcumul += delays[k];
					cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << k << "] msec=" << _timeline->beatToMillisec(header->beatnum + dcumul)
						<< " val=" <<  values[i][k] << endl;
					string easetype;
					if (s != simple_vect->end() && s->type && s->type->is_value()) {
						Value* v = (Value*)s->type->is_value();
						//*v = StringValue(type);
						ostringstream oss; 
						oss << *v;
						easetype = oss.str();
					}

					curve->addKeyframeAtBeatAtCurveId(i, values[i][k], header->beatnum + dcumul);
					if (easetype.size() && easetype != "linear" && easetype != "\"linear\"")
						curve->changeKeyframeEasingAtCurveId(i, header->beatnum + dcumul, easetype);
				}

				curve->getCurve(i)->enable();
			}

			curve->enable();
		} else {
			trackName = string("Curves ") + parentCurve->label + string(" ") +varname;
			curves = new ofxTLBeatCurves();
			curves->keyColor = ofColor(255, 0, 0, 255);

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
			vector<SimpleContFunction>::iterator s = simple_vect->begin();
			for (int k = 0; k < delays.size(); k++, s++) {
				dcumul += delays[k];
				cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << 0 << "] msec=" << _timeline->beatToMillisec(header->beatnum + dcumul)
					<< " val=" <<  values[0][k] << endl;
				string easetype;
				if (s != simple_vect->end() && s->type && s->type->is_value()) {
					Value* v = (Value*)s->type->is_value();
					//*v = StringValue(type);
					ostringstream oss; 
					oss << *v;
					easetype = oss.str();
				}

				curves->addKeyframeAtBeat(values[0][k], header->beatnum + dcumul);
				if (easetype.size() && easetype != "linear" && easetype != "\"linear\"")
					curves->changeKeyframeEasing(header->beatnum + dcumul, easetype);
				curves->enable();
			}
		}
	}
	return true;
}


ActionCurve::~ActionCurve()
{
}


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
		//abort();
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
bool ActionCurve::addKeyframeAtBeat(float beat, float val)
{
	if (debug_edit_curve) { cout << "ofxTLAntescofoAction:: add keyframe at beat: " << beat << " val: " << val << endl; print(); }
	if (beat == 0) {
		//set_dur_val(val, simple_vect->begin());
		return false;
	}
	cout << "ofxTLAntescofoAction:: add keyframe at beat: howmany:" << parentCurve->howmany << endl;
	if (parentCurve->howmany > 1) // do not support multicurves editing for now
		return false;

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
					//assert(ny1);
					if (!ny1) cout << "addKeyframeAtBeat:ERROR : not an constant value in curve...." << endl;
					FloatValue* ny0 = new FloatValue(val);
					//s = simple_vect->insert(s, SimpleContFunction(sp->antesc, new StringValue("linear"), ad, ny0, ny1, s->var));
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
	return true;
}


void ActionCurve::changeKeyframeEasing(float beat, string type) {
	//if (debug_edit_curve) 
	cout << "ActionCurve::changeKeyframeEasing: beat:"<< beat << " type:"<< type << endl;
	
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
			if (s != simple_vect->end() && s->type && s->type->is_value()) {
				if (debug_edit_curve) cout << "change: is value:" << s->type->is_value() << endl;
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
	if (debug_actiongroup) 	cout << "Action: adding message with delay: " << delay << " : " << action << endl;
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
		if (debug_actiongroup) cout << "ActionGroupHeader: adding : " << lab << endl;
		realtitle = lab;
		if (lab.size() && strncmp(lab.c_str(), "_topgroup_", 9) == 0) {
			top_level_group = true;
			hidden = false;
		}
		Curve* c = dynamic_cast<Curve*>(action);
		if (c) {
			title = "Curve " + action->label();
			rect.height = HEADER_HEIGHT;
			cout << "ActionGroupHeader : -----create curve ----" << endl;
			lineNum_begin = action->locate()->begin.line;
			lineNum_end = action->locate()->end.line + 1;

			//ActionMultiCurves *cu = new ActionMultiCurves(c, d, event, nh); 
			group = new ActionMultiCurves(c, delay, event, this); 

			//group = new ActionGroup(g, event, this);

		} else {

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
	cout << "ActionRects.draw: label:"<< realtitle<< " inbounds:" << group->is_in_bounds(tlAction) <<  " x:"<<rect.x << " y:" << rect.y << " " << rect.width <<  "x"<< rect.height << endl;
	if (top_level_group && group && group->is_in_bounds(tlAction)) {
		cout << "1" << endl;
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
		cout << "2" << endl;
		int sizec = tlAction->mFont.stringWidth(string("_"));
		if (group && group->is_in_bounds(tlAction)) {
		cout << "3" << endl;
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
		cout << "4" << endl;
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
		cout << "ActionGroupHeader.draw hidden: ("<<rect.x<<","<<rect.y << ", "<< rect.width << "x" << rect.height <<  ") : " << title << endl;
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

