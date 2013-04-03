//
//  ofxAntescofoAction.cpp
//  ofxAntescofog
//
//  Created by Thomas Coffy on 06/12/12.
//
//
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
}


ofxTLAntescofoAction::~ofxTLAntescofoAction()
{
}

void ofxTLAntescofoAction::setup()
{
	_timeline = getTimeline();

	load();

	disable();
}

void ofxTLAntescofoAction::draw()
{
	if (mActionGroups.empty()) {
		//bounds.height = 0;
		disable();
	}
	if (mScore && bounds.height > 6) {
		update_groups();
		for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
			if (zoomBounds.max >= timeline->beatToNormalizedX((*i)->beatnum)
					|| zoomBounds.min <= timeline->beatToNormalizedX((*i)->beatnum + (*i)->duration)) {
				ofSetColor(0, 0, 0, 125);

				ActionGroupHeader *act = *i;
				//act->rect.x =  bounds.x + normalizedXtoScreenX( timeline->beatToNormalizedX(act->beatnum), zoomBounds);
				//act->rect.y = bounds.y + 3;
				//act->rect.width = normalizedXtoScreenX( timeline->beatToNormalizedX(act->duration), zoomBounds);
				//cout << "ofxTLAntescofoAction: beat:" << act->beatnum <<" duration:" << act->duration<< " w:"<< act->rect.width << endl;
				act->draw(this);
				//act->print();
			}
		}
	}

}

//--------------------------------------------------
void ofxTLAntescofoAction::drawBitmapStringHighlight(string text, int x, int y, const ofColor& background, const ofColor& foreground) {
}

void ofxTLAntescofoAction::add_action_curves(float beatnum, ActionGroup *ar, Cfwd *c) 
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
#if 0
		ActionRect *ar = new ActionRect("", beatnum, e->gfwd, e, rect);

		for (vector<Action*>::const_iterator i = e->gfwd->actions().begin(); i != e->gfwd->actions().end(); i++)
		{
			cout << "ofxTLAntescofoAction::add_action: adding : " << endl << action<< endl;
			Cfwd* c = dynamic_cast<Cfwd*>(*i);
			if (c) 
				add_action_curves(beatnum, ar, c);
		}

		// store for future use
		mActionRects.push_back(ar);
#else
		ActionGroupHeader *header = new ActionGroupHeader(beatnum, e->gfwd, e);
		/*
		for (vector<Action*>::const_iterator i = e->gfwd->actions().begin(); i != e->gfwd->actions().end(); i++) {
			Cfwd* c = dynamic_cast<Cfwd*>(*i);
			if (c) 
				add_action_curves(beatnum, ar, c);
		}
		*/

		// store for future use
		mActionGroups.push_back(header);


#endif
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


// for updating subgroups
int ofxTLAntescofoAction::update_sub(ActionGroup *ag)
{
	int debugsub = 0;
	if (debugsub) cout << "update_sub: " << ag->header->title << endl;
	int toth = 0, curh = 0; // find max h
	int cury = ag->header->rect.y;
	//if (! ag->header->top_level_group) cury += ag->header->HEADER_HEIGHT;

	list<ActionGroup*>::const_iterator g;
	for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
		ActionMessage *m;
		if ((m = dynamic_cast<ActionMessage*>(*g))) {
			m->x = ag->header->rect.x;
			if (m->delay) {
				m->x += normalizedXtoScreenX( timeline->beatToNormalizedX(m->delay), zoomBounds);
			}
			cury += ag->header->LINE_HEIGHT;
			m->y = cury;
			curh += ag->header->LINE_HEIGHT;
			toth += curh;
			//cout << "________________ x:"<< m->x << " y:" << m->y <<  "cury:" << cury << endl;
			if (debugsub) 	cout << "update_sub: in msg: y=" << m->y << endl;
		}
		if (!(*g)->header->top_level_group) {
				(*g)->header->rect.x = ag->header->rect.x;
				if ((*g)->header->delay)
					(*g)->header->rect.x += normalizedXtoScreenX( timeline->beatToNormalizedX((*g)->header->delay), zoomBounds);
				if (debugsub) cout << "update_sub: cury:" << cury << " recty:" << ag->header->rect.y << endl;
				(*g)->header->rect.y = ag->header->rect.y; //cury;
				//cury += ag->header->LINE_HEIGHT;
				//if (!ag->header->top_level_group) { // if parent group is not top level group add HEADER HEIGHT
					//gr->header->rect.y += gr->header->HEADER_HEIGHT;
				//}
		}
		curh = update_sub(*g);
		ag->header->rect.height = curh;
		if (curh > toth)
			toth += curh;
	}
	//cout << "update_sub: return " << toth << endl;
	if (ag->header->hidden) return ag->header->HEADER_HEIGHT;
	if (!toth) { // leaf elmt -> message
		//if (!ag->header->top_level_group)
			return ag->header->HEADER_HEIGHT; //ag->header->LINE_HEIGHT;
		//else return 0;
	}
	
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
			if (curw > maxw)
				maxw += curw;
		}
	}
	return maxw;
}

// for updating subgroups width
// return width in beats
int ofxTLAntescofoAction::update_sub_width(ActionGroup *ag)
{
	int maxw = 0, curw = 0;
	ag->header->duration = 0;

	ActionMessage *m;
	if ((m = dynamic_cast<ActionMessage*>(ag))) {
		int sizec = mFont.stringWidth(string("a"));
		int len = sizec * m->action.size();
		if (m->delay) 
			len += get_x(m->delay);
		maxw = len;
		//cout << "update_sub_width: ismsg: " << maxw << endl;
	} else {
		list<ActionGroup*>::const_iterator g;
		for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
			//if ((*g)->header->delay) ag->header->duration += (*g)->header->delay;
			(*g)->header->rect.width = curw = update_sub_width(*g);
			if (curw > maxw)
				maxw += curw;
		}
	}
	cout << "update_sub_width: " << maxw << endl;
	return maxw;
}
// avoid x overlapping
void ofxTLAntescofoAction::update_avoid_overlap()
{
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		list<ActionGroupHeader*>::const_iterator j = i;
		if (++j != mActionGroups.end() && zoomBounds.contains(timeline->beatToNormalizedX((*j)->beatnum))) {
			if ( ((*i)->rect.x + (*i)->rect.width) + 1 >= (*j)->rect.x) {
				(*i)->rect.width = (*j)->rect.x - (*i)->rect.x - 3;
			}
		}

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
}

// for updating subgroups width
// return width in beats
int ofxTLAntescofoAction::update_sub_y(ActionGroup *ag)
{

	// go deep and assign each header rect.y
	list<ActionGroup*>::const_iterator g;
	for (g = ag->sons.begin(); g != ag->sons.end(); g++) {
		if (! ag->header->top_level_group) {
			(*g)->header->rect.y += ag->header->HEADER_HEIGHT;
			break;
		}
	}
	// loop rec ?
}



void ofxTLAntescofoAction::update_groups()
{
	// update actions' rect
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		(*i)->rect.x = bounds.x + normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->beatnum), zoomBounds);
		(*i)->rect.y = bounds.y + 3;
		//update_sub_y((*i)->group);
		update_sub_duration((*i)->group);
		(*i)->rect.width = update_sub_width((*i)->group);
		
		//if ((*i)->duration == 0) (*i)->rect.width = 200; // TODO else
		//(*i)->rect.width = normalizedXtoScreenX( timeline->beatToNormalizedX((*i)->duration), zoomBounds );

		if (!(*i)->hidden && (*i)->group) {
			(*i)->rect.height = update_sub((*i)->group);
		}
	}

	update_avoid_overlap();

#ifdef OLDWIDTH_CALCULATION
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
#endif


}

//--------------------------------------------------------------
void ofxTLAntescofoAction::windowResized(int w, int h){

}

bool ofxTLAntescofoAction::mousePressed_In_Arrow(ofMouseEventArgs& args, ActionGroupHeader* header) {
	bool res = false;
	cout << "mousePressed: rect: x:" << header->rect.x << " y:" << header->rect.y << " w:"<< header->rect.width << " h:" << header->rect.height << endl; 
	if (header->rect.inside(args.x, args.y)) {
		if (header->hidden) {
			header->hidden = false;
			cout << "mousePressed: setting action '"<< header->title <<"' hidden:"<< header->hidden << endl;
			if (header->group) {
				Event *e = header->group->event;
				mAntescofog->editorShowLine(e->scloc->begin.line, e->scloc->end.line);
				res = true;
			}
		} else if (header->is_in_arrow(args.x, args.y)) {
			header->hidden = true;
		} else {
			if (header->group) {
				Event *e = header->group->event;
				mAntescofog->editorShowLine(e->scloc->begin.line, e->scloc->end.line);
				res = true;
			}
		}
	}
	return res;
}

bool ofxTLAntescofoAction::mousePressed(ofMouseEventArgs& args, long millis)
{
	bool res = false;
	for (list<ActionGroupHeader*>::const_iterator i = mActionGroups.begin(); i != mActionGroups.end(); i++) {
		//if ((*i)->header) {
			if (!(*i)->top_level_group && mousePressed_In_Arrow(args, *i)) {
				res = true;
				return res;
			} else if ((*i)->group) { // look for subgroups

				ActionGroup *a = (*i)->group;
				if (a->sons.size()) {
					for (list<ActionGroup*>::const_iterator j = a->sons.begin(); j != a->sons.end(); j++) {
						if ((*j)->header &&  mousePressed_In_Arrow(args, (*j)->header)) {
							res = true;
							return res;
						}
					}
				}
			}
		//}
	}
	return res;
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

////////////////////////////////////////////////////

ActionGroup::ActionGroup(Gfwd* g, Event *e, ActionGroupHeader* header_) 
			: header(header_), gfwd(g), event(e)
{
	if (g) {
		vector<Action*>::const_iterator i;
		for (i = g->actions().begin(); i != g->actions().end(); i++) {
			Action *tmpa = *i;

			// can be group
			Gfwd *gg = dynamic_cast<Gfwd*>(tmpa);
			if (gg) {
				ActionGroupHeader* nh = new ActionGroupHeader(header->beatnum + get_delay(gg), gg, event);//, false);
				//ActionGroup *ag = new ActionGroup(gg, event, nh); sons.push_back(ag);
				if (nh->group)
					sons.push_back(nh->group);
			}

			// can be message
			Message *m = dynamic_cast<Message*>(tmpa);
			if (m) {
				ActionMessage *am = new ActionMessage(m, event, header); 
				sons.push_back((ActionGroup*)am);
			}

			// can be a curve
			Cfwd* c = dynamic_cast<Cfwd*>(tmpa);
			if (c) {
				ActionCurve *cu = new ActionCurve(c, event, header); 
				sons.push_back((ActionGroup*)cu);
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

double ActionGroup::get_delay(Action* tmpa) 
{
	double d = 0.;
	if (tmpa && tmpa->delay() && tmpa->delay()->value()) {
		d = (double)tmpa->delay()->eval();
	}
	return d;
}

void ActionGroup::print() {
	if (header && !header->top_level_group)
		header->print();
	for (list<ActionGroup*>::iterator i = sons.begin(); i != sons.end(); i++) {
		(*i)->print();
	}
}

void ActionGroup::draw(ofxTLAntescofoAction *tlAction)
{
	if (header)
		header->draw(tlAction);
}


/* 
   Curve
 */
ActionCurve::ActionCurve(Cfwd* c, Event *e, ActionGroupHeader* header_)
{
	header = header_;
	if (c) {
		cout << "got Cfwd: " << c->label() << endl; 
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
		trackName = string("CFWD ") + label;
		string uniqueName = _timeline->confirmedUniqueName(trackName);

		int howmany = (*(values.begin())).size();
		ofxTLMultiCurves* curves = new ofxTLMultiCurves();
		curves->clear();
		curves->disable();
		_timeline->addTrack(trackName, curves);
		curves->setHowmany(howmany);

		// set values ranges
		int j = 0; 
		for (int j = 0; j < howmany; j++) {
			float min = 0, max = 0;
			for (vector< vector<double> >::iterator i = values.begin(); i != values.end(); i++) {
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
			for (int k = 0; k < delays.size(); k++) {
				//cout << " ------- delays["<<n<<"]: " << ar->cfwd->delays[n] << endl;
				dcumul += delays[k];
				//for (vector< vector<double> >::iterator i = ar->cfwd->values.begin(); i != ar->cfwd->values.end(); i++) {
				cout << "ofxTLAntescofoAction::add_action: CFWD add keyframe[" << n << "] msec=" << _timeline->beatToMillisec(header->beatnum + dcumul)
					<< " val=" <<  values[k][n] << endl;

				//c->addKeyframeAtMillis(ar->cfwd->values[i], ofxAntescofoNote->beatToMillisec(ar->beatnum + dcumul));
				curves->addKeyframeAtBeatAtCurveId(n, values[k][n], header->beatnum + dcumul);
				// }
			}

			curves->enable();
		}
	}

}


ActionCurve::~ActionCurve()
{
}


/*
	Message
 */

ActionMessage::ActionMessage(Message* m, Event *e, ActionGroupHeader* header_) 
{
	header = header_;

	ostringstream oss;
	oss << *m;
	action = oss.str();
	x = header->rect.x;
	y = header->rect.y;
	//header->hidden = false;
	//ofRectangle tmprect = rect; tmprect.y += HEADER_HEIGHT;
	//ActionRect *newact = new ActionRect(act, header->beatnum + get_delay(m), NULL, event);
	//ActionRects.push_back(newact);

	delay = get_delay(m);
	cout << "Action: adding message with delay: " << delay << " : " << action << endl;
}

void ActionMessage::print() {
	cout << "**** Action Message: " << action << " x:" << x << " y:" << y<< endl;
}

void ActionMessage::draw(ofxTLAntescofoAction* tlAction) {
	int sizec = tlAction->mFont.stringWidth(string("a"));
	string str = action;
	if (header->rect.width < action.size() * sizec) {
		int nc = header->rect.width / sizec - 2;
		str = action.substr(0, nc);
	}
	cout << "Action message: DRAWING: " << str << " x:" << x << " y:" << y << endl;
	//rect.width = sizec * c.size(); // TODO limiter la largeur en fct du suivant
	tlAction->mFont.drawString(str, x, y + header->LINE_HEIGHT);
	//ofRect(rect);
}


ActionGroupHeader::ActionGroupHeader(float beatnum_, Action* a_, Event *e_)
	: beatnum(beatnum_), action(a_), event(e_), top_level_group(false), duration(0),
	hidden(true), delay(0),
	HEADER_HEIGHT(16), ARROW_LEN(15), LINE_HEIGHT(15), LINE_SPACE(12)
{
	if (action) {
		string lab = action->label();
		cout << "ActionGroupHeader: adding : " << lab << endl;
		if (lab.size() && strncmp(lab.c_str(), "top_gfwd_", 9) == 0) {
			realtitle = action->label();
			top_level_group = true;
			hidden = false;
		}
		title = "Group " + action->label();
		rect.height = HEADER_HEIGHT;


		Gfwd *g = dynamic_cast<Gfwd*>(action);
		if (g) {
			group = new ActionGroup(g, event, this);
		}
	}
}

ActionGroupHeader::~ActionGroupHeader()
{
	if (group)
		delete group;
}
				
void ActionGroupHeader::draw(ofxTLAntescofoAction *tlAction) 
{
	ofNoFill(); // border
	ofSetColor(0, 0, 0, 255);

	//cout << "ActionRects.draw: label:"<< realtitle<< " x:"<<rect.x << " y:" << rect.y << " " << rect.width <<  "x"<< rect.height << endl;
	if (top_level_group || !hidden) {
		int sizec = tlAction->mFont.stringWidth(string("a"));
		if (group) {
			ofRect(rect);

			if (!top_level_group) {
				ofFill(); // rect color filled
				ofSetColor(headerColor);
				/*
				if (duration == 0) // groups with no delay
					rect.width = 200; // TODO
				else rect.width = tlAction->get_x(duration);
				*/
				ofRectangle recthead = rect;
				recthead.height = HEADER_HEIGHT;
				ofRect(recthead);

				ofNoFill();
				ofSetColor(0, 0, 0, 255);
				drawArrow(); // arrow
				tlAction->mFont.drawString(title, rect.x, rect.y + LINE_HEIGHT - 2);
			}
			//cout << "ActionGroupHeader.draw !hidden: ("<<rect.x<<","<<rect.y<<", "<< rect.width << "x"<< rect.height << ") : " << title << endl;
			list<ActionGroup*>::const_iterator i;
			for (i = group->sons.begin(); i != group->sons.end(); i++) {
					(*i)->draw(tlAction);
			}
		}
	} else { // hidden, just draw header and arrow
		ofFill(); // rect color filled
		ofSetColor(headerColor);
		int savedh = rect.height;
		rect.height = HEADER_HEIGHT;
		/*
		if (duration == 0) // groups with no delay
			rect.width = 200; // TODO
		else rect.width = tlAction->get_x(beatnum) - tlAction->get_x(duration);
		*/
		ofRect(rect);
		rect.height = savedh;
		ofNoFill();
		ofSetColor(0, 0, 0, 255);
		drawArrow(); // arrow
		tlAction->mFont.drawString(title, rect.x, rect.y + LINE_HEIGHT - 2);
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
	cout << "***** ActionGroup Header:" << realtitle << " beat:" << beatnum<<" dur:" << duration << " hidden:"<< hidden 
	     << " x:" << rect.x << " y:" << rect.y << " w:" << rect.width << " h:" << rect.height << endl; //" height:" << get_height(tlAction);
	if (top_level_group && group)
		group->print();
	/*
	for (list<string>::const_iterator i = actions.begin(); i != actions.end(); i++) {
		cout << " action:\""<< *i << "\"";
	}
	cout << " x:" << rect.x << " y:" << rect.y << " " << rect.width << "x" << rect.height << endl;
	list<ActionRect*>::const_iterator i;
	for (i = ActionRects.begin(); i != ActionRects.end(); i++) {
		(*i)->print(tlAction);
	}
	*/
}

bool ActionGroupHeader::is_in_arrow(int x, int y)
{
	//cout << "is in arrow: " << x << " " << y << endl;
	bool res = false;
	if (x > rect.x + rect.width - ARROW_LEN && y < rect.y + HEADER_HEIGHT)
		res = true;
	return res;
}

