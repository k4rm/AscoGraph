/*

  MusicXML Library
  Copyright (C) 2008  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "conversions.h"
#include "partsummary.h"
#include "rational.h"
#include "xml2antescofovisitor.h"
#include "xmlpart2antescofo.h"
#include "xml_tree_browser.h"

using namespace std;

namespace MusicXML2 
{

//______________________________________________________________________________
xmlpart2antescofo::xmlpart2antescofo(antescofowriter& _w, bool generateComments, bool
		generateStem, bool generateBar) : w(_w),
	fGenerateComments(generateComments), fGenerateStem(generateStem), 
	fGenerateBars(generateBar),
	fNotesOnly(false), fCurrentStaffIndex(0), fCurrentStaff(0),
	fTargetStaff(0), fTargetVoice(0)
{
	fGeneratePositions = true;
    fRepeatForward = fRepeatBackward = false;
	xmlpart2antescofo::reset();
}

//______________________________________________________________________________
void xmlpart2antescofo::reset ()
{
	antescofonotestatus::resetall();
	fCurrentBeamNumber = 0;
	fInCue = fInGrace = fInhibitNextBar = fPendingBar
		   = fBeamOpened = fCrescPending = fSkipDirection = false;
	fCurrentStemDirection = kStemUndefined;
	fCurrentDivision = 1;
	fCurrentOffset = 0;
	fPendingPops = 0;
	fMeasNum = 0;
	fCurBeat = 1;
	fLastDur = 0;
    fTrill = fGlissandoStart = fGlissandoStop = fInBackup = fInForward = false;
}

//______________________________________________________________________________
void xmlpart2antescofo::initialize (Santescofoelement seq, int staff, int antescofostaff, int voice, 
		bool notesonly, rational defaultTimeSign) 
{
	fCurrentStaff = fTargetStaff = staff;	// the current and target staff
	fTargetVoice = voice;					// the target voice
	fNotesOnly = notesonly;					// prevent multiple output for keys, clefs etc... 
	fCurrentTimeSign = defaultTimeSign;		// a default time signature
	fCurrentStaffIndex = antescofostaff;		// the current antescofo staff index
	start (seq);
}

//________________________________________________________________________
// some code for the delayed elements management
// delayed elements are elements enclosed in a <direction> element that
// contains a non-null <offset> element. This offset postpone the graphic
// appearance of the element in 'offset' time units in the futur.
// Time units are <division> time units 
//________________________________________________________________________
// add an element to the list of delayed elements
void xmlpart2antescofo::addDelayed (Santescofoelement elt, long offset) 
{
	add(elt);
	return;
	
	if (offset > 0) {
		delayedElement de;
		de.delay = offset;
		de.element = elt;
		fDelayed.push_back(de);
	}
	else add (elt);
}

//________________________________________________________________________
// checks ready elements in the list of delayed elements
// 'time' is the time elapsed since the last check, it is expressed in
// <division> time units
void xmlpart2antescofo::checkDelayed (long time)
{
	vector<delayedElement>::iterator it = fDelayed.begin();
	while (it!=fDelayed.end()) {
		it->delay -= time;
		if (it->delay < 0) {
			add (it->element);
			it = fDelayed.erase(it);
		}
		else it++;
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::stackClean () 
{
	if (fInCue) {
		pop();			
		fInCue = false;
	}
	if (fInGrace) {
		pop();			
		fInGrace = false;
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::checkStaff (int staff) {
    if (staff != fCurrentStaff) {
        Santescofoelement tag = antescofotag::create("staff");
		int offset = staff - fCurrentStaff;
//cout << "move from staff " << fCurrentStaffIndex << " to " << (fCurrentStaffIndex + offset) << endl;
		fCurrentStaff = staff;
        fCurrentStaffIndex += offset;
		tag->add (antescofoparam::create(fCurrentStaffIndex, false));
        add (tag);
    }
}

//______________________________________________________________________________
void xmlpart2antescofo::moveMeasureTime (int duration, bool moveVoiceToo)
{
    //cout << "moveMeasureTime: fCurBeat:"<<fCurBeat<<" dur:"<< duration << " moveVoiceToo:"<<moveVoiceToo<<endl;
	rational r(duration, fCurrentDivision*4);
	r.rationalise();
	fCurrentMeasurePosition += r;
	fCurrentMeasurePosition.rationalise();
	if (fCurrentMeasurePosition > fCurrentMeasureLength)
		fCurrentMeasureLength = fCurrentMeasurePosition;
        if (moveVoiceToo) {
            fCurrentVoicePosition += r;
            fCurrentVoicePosition.rationalise();
            // advance fCurBeat
            rational d(duration, fCurrentDivision);
            d.rationalise();
            cout << "moveMeasureTime: add fCurBeat:" << fCurBeat << " : " << d.toFloat() << endl;
            fCurBeat += d.toFloat();
            fLastDur = d;
        }
}

//______________________________________________________________________________
// check the current position in the current voice:  when it lags behind 
// the current measure position, it creates the corresponding empty element
//______________________________________________________________________________
void xmlpart2antescofo::checkVoiceTime ( const rational& currTime, const rational& voiceTime)
{
	rational diff = currTime - voiceTime;
	diff.rationalise();
	if (diff.getNumerator() > 0) {
		/*antescofonoteduration dur (diff.getNumerator(), diff.getDenominator());
		Santescofoelement note = antescofonote::create(fTargetVoice, "empty", 0, dur, "");
		add (note);*/
                //cout << "checkVoiceTime: should add rest note dur:"<<diff.getNumerator() << "/"<<diff.getDenominator() << endl;
		w.AddNote(ANTESCOFO_REST, 0, diff, fMeasNum, fCurBeat, 0);
		fCurrentVoicePosition += diff;
		fCurrentVoicePosition.rationalise();
    }
	else if (diff.getNumerator() < 0)
		cerr << "warning! checkVoiceTime: measure time behind voice time " << string(diff) << endl;
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_backup& elt )
{
    fInBackup = true;
	stackClean();	// closes pending chords, cue and grace
	int duration = elt->getIntValue(k_duration, 0);
    cout << "BACKUP----------------< " << duration <<  " fCurBeat:" << fCurBeat << endl;

	if (duration) {
		// backup is supposed to be used only for moving between voices
		// thus we don't move the voice time (which is supposed to be 0)
		moveMeasureTime (-duration, false);
    }
    cout << "BACKUP----------------< " << duration <<  " fCurBeat:" << fCurBeat << endl;
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_forward& elt )
{
    fInForward = true;
    bool scanElement = (elt->getIntValue(k_voice, 0) == fTargetVoice)
        && (elt->getIntValue(k_staff, 0) == fTargetStaff);
    int duration = elt->getIntValue(k_duration, 0);
    cout << "FORWARD("<<scanElement<<") ----------------> " << duration << endl;
    if (fCurBeat == 1 && fMeasNum > 1)
        moveMeasureTime(duration, true);
    else 
        moveMeasureTime(duration,/*false*/ scanElement);
    if (!scanElement) return;


    if (duration) {		
        rational r(duration, fCurrentDivision*4);
        r.rationalise();
        //antescofonoteduration dur (r.getNumerator(), r.getDenominator());
        //Santescofoelement note = antescofonote::create(fTargetVoice, "empty", 0, dur, "");
        w.AddNote(ANTESCOFO_REST, 0, r, fMeasNum, fCurBeat, 0);
        //add (note);
        fMeasureEmpty = false;
    }
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_part& elt ) 
{
    cout << endl << "--------------------------------- visit start S_part " << endl;
	reset();
	if (!current()) {
		Santescofoelement seq = antescofoseq::create();
		start (seq);
	}
}

bool is_number(const std::string& s)
{
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_measure& elt ) 
{
        fRehearsals = "";
	const string& implicit = elt->getAttributeValue ("implicit");
	if (implicit == "yes") fPendingBar = false;
	if (fPendingBar) {
		// before adding a bar, we need to check that there are no repeat begin at this location
		ctree<xmlelement>::iterator repeat = elt->find(k_repeat);
		if ((repeat == elt->end()) || (repeat->getAttributeValue("direction") != "forward")) {
			checkStaff (fTargetStaff);
			Santescofoelement tag = antescofotag::create("bar");
			add (tag);
		}
	}
	fCurrentMeasure = elt;
    fMeasNum++;
    
    int number_attr = 0;
    string n = elt->getAttributeValue("number");
    
    if (!n.empty() && is_number(n)) number_attr = atoi(n.c_str());

    if (number_attr && number_attr != fMeasNum) fMeasNum = number_attr;
	fCurrentMeasureLength.set  (0, 1);
	fCurrentMeasurePosition.set(0, 1);
	fCurrentVoicePosition.set  (0, 1);
	fInhibitNextBar = false; // fNotesOnly;
	fPendingBar = false;
	fPendingPops = 0;
	fMeasureEmpty = true;

        //if (fMeasNum == 1) fCurBeat = 1;

        cout << "--------------------------------- visit start S_measure: fMeasNum:"<<fMeasNum << " fCurBeat:"<< fCurBeat << endl;
/*
        float abs_curBeat = .0;
        if (fMeasNum > 1 && fCurBeat == 1) {
            cout << "AntescofoWriter: fMeasure and fCurBeat desync, trying to fix it."<< endl;
            for (map<float, measure_elt>::const_iterator i = w.v_Notes.begin(); i != w.v_Notes.end(); i++) {
                if (i->second.nMeasure == fMeasNum) {
                    abs_curBeat = i->second.m_pos + fCurBeat - 1;
                    break;
                }
            }
            if (abs_curBeat) {
                fCurBeat = abs_curBeat;
                cout << "AntescofoWriter: fCurBeat changed to :"<< fCurBeat<<endl;
            }
            else {
                cerr << "AntescofoWriter: something went wrong with note beats, check your MusicXML file, sorry." << endl;
                //w.antescofo_abort();
            }
        }
        */
}
        

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_measure& elt ) 
{
    cout << "--------------------------------- visit end S_measure: fCurBeat:"<<fCurBeat << endl;

    cout << "fCurrentMeasureLength: "<<fCurrentMeasureLength.toFloat() << " fCurBeat:"<< fCurBeat << " "<< endl;// << " d:"  << fCurrentMeasureLength.toFloat() - fCurBeat << endl;
    checkVoiceTime (fCurrentMeasureLength, fCurrentVoicePosition);

    /*
    if (fCurrentMeasureLength.toFloat()) {
    //float diff = fCurrentMeasureLength.toFloat() - fCurBeat;
        fCurBeat += fCurrentMeasureLength.toFloat();
    }
    */

    if (!fInhibitNextBar) {
    if (fGenerateBars) fPendingBar = true;
    else if (!fMeasureEmpty) {
        if (fCurrentVoicePosition < fCurrentMeasureLength)
            fPendingBar = true;
      }
    }

    bool scanElement = (elt->getIntValue(k_voice, 0) == fTargetVoice) && (elt->getIntValue(k_staff, 0) == fTargetStaff);
    if (!scanElement) {
        rational d(fCurrentMeasureLength.toFloat(), fCurrentDivision);
        d.rationalise();
        cout << "visitEnd S_measure: add fCurBeat:" << fCurBeat << " : " << d.toFloat() << endl;
        fCurBeat += d.toFloat();
        fLastDur = d;
    }
}


//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_rehearsal& elt ) 
{
    cout << "rehearsal:"<< elt->getValue() << endl;
    fRehearsals = elt->getValue();
    //abort();
    /*
	if (fNotesOnly || (elt->getIntValue(k_staff, 0) != fTargetStaff)) {
		fSkipDirection = true;
	}
	else {
		fCurrentOffset = elt->getLongValue(k_offset, 0);
	}
        */
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_direction& elt ) 
{
	if (fNotesOnly || (elt->getIntValue(k_staff, 0) != fTargetStaff)) {
		fSkipDirection = true;
	}
	else {
		fCurrentOffset = elt->getLongValue(k_offset, 0);
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_direction& elt ) 
{
	fSkipDirection = false;
	fCurrentOffset = 0;
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_key& elt ) 
{
	if (fNotesOnly) return;
	//Santescofoelement tag = antescofotag::create("key");
	// tag->add (antescofoparam::create(keysignvisitor::fFifths, false)); add (tag);
	fFifths = keysignvisitor::fFifths;
	//sMode = keysignvisitor::sMode;

}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_coda& elt )
{
	if (fSkipDirection) return;
	Santescofoelement tag = antescofotag::create("coda");
	add(tag);
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_segno& elt )
{
	if (fSkipDirection) return;
	Santescofoelement tag = antescofotag::create("segno");
	add(tag);
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_wedge& elt )
{
	if (fSkipDirection) return;

	string type = elt->getAttributeValue("type");
	Santescofoelement tag;
	if (type == "crescendo") {
		tag = antescofotag::create("crescBegin");
		fCrescPending = true;
	}
	else if (type == "diminuendo") {
		tag = antescofotag::create("dimBegin");
		fCrescPending = false;
	}
	else if (type == "stop") {
		tag = antescofotag::create(fCrescPending ? "crescEnd" : "dimEnd");
	}
	if (tag) {
		if (fCurrentOffset) addDelayed(tag, fCurrentOffset);
		else add (tag);
	}

}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_metronome& elt ) 
{
	if (fSkipDirection) return;

	metronomevisitor::visitEnd (elt);
	if (fBeats.size() != 1) return;					// support per minute tempo only (for now)
	if (!metronomevisitor::fPerMinute) return;		// support per minute tempo only (for now)

	Santescofoelement tag = antescofotag::create("tempo");
	beat b = fBeats[0];
	rational r = NoteType::type2rational(NoteType::xml(b.fUnit)), rdot(3,2);
	while (b.fDots-- > 0) {
		r *= rdot;
	}
	r.rationalise();

	stringstream s;
	s << metronomevisitor::fPerMinute;
	//tag->add (antescofoparam::create("tempo=\""+s.str()+"\"", false));
	//tag->add (antescofoparam::create("BPM "+s.str(), false));
	string str;
    s >> str;
    w.setBPM(str);
	cout << "xmlpart2antescofo : got metronome : BPM: "<< metronomevisitor::fPerMinute<< endl;
    if (fCurrentOffset) addDelayed(tag, fCurrentOffset);
	add (tag);
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart( S_dynamics& elt)
{
	if (fSkipDirection) return;

	ctree<xmlelement>::literator iter;
	for (iter = elt->lbegin(); iter != elt->lend(); iter++) {
		if ((*iter)->getType() != k_other_dynamics) {
			Santescofoelement tag = antescofotag::create("intens");
			tag->add (antescofoparam::create((*iter)->getName()));
			if (fGeneratePositions) xml2antescofovisitor::addPosition(elt, tag, 12);
			if (fCurrentOffset) addDelayed(tag, fCurrentOffset);
			else add (tag);
		}
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart( S_octave_shift& elt)
{
	if (fSkipDirection) return;

	const string& type = elt->getAttributeValue("type");
	int size = elt->getAttributeIntValue("size", 0);

	switch (size) {
		case 8:		size = 1; break;
		case 15:	size = 2; break;
		default:	return;
	}

	if (type == "up")
		size = -size;
	else if (type == "stop")
		size = 0;
	else if (type != "down") return;

	Santescofoelement tag = antescofotag::create("oct");
	if (tag) {
		tag->add (antescofoparam::create(size, false));
//		add (tag);			// todo: handling of octave offset with notes
// in addition, there is actually a poor support for the oct tag in antescofo
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_note& elt ) 
{
	notevisitor::visitStart ( elt );
}

//______________________________________________________________________________
string xmlpart2antescofo::alter2accident ( float alter ) 
{
	stringstream s;
	while (alter > 0.5) {
		s << "#";
		alter -= 1;
	}
	while (alter < -0.5) {
		s << "&";
		alter += 1;
	}
	
	string accident;
	s >> accident;
	return accident;
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_sound& elt )
{
	if (fNotesOnly) return;

	Santescofoelement tag = 0;
	Sxmlattribute attribute;
	
	if ((attribute = elt->getAttribute("dacapo")))
		tag = antescofotag::create("daCapo");
	else {
		if ((attribute = elt->getAttribute("dalsegno"))) {
			tag = antescofotag::create("dalSegno");
		}
		else if ((attribute = elt->getAttribute("tocoda"))) {
			tag = antescofotag::create("daCoda");
		}
		else if ((attribute = elt->getAttribute("fine"))) {
			tag = antescofotag::create("fine");
		} else if ((attribute = elt->getAttribute("tempo"))) {
            cout << "xmlpart2antescofo : got sound tempo : BPM: "<< attribute->getValue() << endl;
            w.setBPM(attribute->getValue());
        }
//		if (tag) tag->add(antescofoparam::create("id="+attribute->getValue(), false));
	}
	if (tag) add (tag);
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_ending& elt )
{
	string type = elt->getAttributeValue("type");
	if (type == "start") {
		Santescofoelement tag = antescofotag::create("volta");
		string num = elt->getAttributeValue ("number");
		tag->add(antescofoparam::create(num, true));
		tag->add(antescofoparam::create(num + ".", true));
		push(tag);
	}
	else {
		if (type == "discontinue")
			current()->add(antescofoparam::create("format=\"|-\"", false));
		pop();
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_repeat& elt ) 
{
	string direction = elt->getAttributeValue("direction");
	if (direction == "forward") 
		fRepeatForward = true;
	else if (direction == "backward") {
		fRepeatBackward = true;
		//fInhibitNextBar = true;
	}
    cout << "visitEnd: direction: repeat: "<< direction << endl;
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_barline& elt ) 
{
	const string& location = elt->getAttributeValue("location");
	if (location == "middle") {
		// todo: handling bar-style (not yet supported in antescofo)
		Santescofoelement tag = antescofotag::create("bar");
		add(tag);
	}
	// todo: support for left and right bars
	// currently automatically handled at measure boundaries
	else if (location == "right") {
	}
	else if (location == "left") {
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_time& elt ) 
{
	string timesign;
	if (!timesignvisitor::fSenzaMisura) {
    	if (timesignvisitor::fSymbol == "common") {
			rational ts = timesignvisitor::timesign(0);
			if ((ts.getDenominator() == 2) && (ts.getNumerator() == 2))
				timesign = "C/";
			else if ((ts.getDenominator() == 4) && (ts.getNumerator() == 4))
				timesign = "C";
			else 
				timesign = string(ts);
			fCurrentTimeSign = ts;
		}
    	else if (timesignvisitor::fSymbol == "cut") {
            timesign = "C/";
			fCurrentTimeSign = rational(2,2);
		}
		else {
			stringstream s; string sep ="";
			fCurrentTimeSign.set(0,1);
			for (unsigned int i = 0; i < timesignvisitor::fTimeSign.size(); i++) {
				s << sep << timesignvisitor::fTimeSign[i].first << "/" << timesignvisitor::fTimeSign[i].second;
				sep = "+";
				rational ts = timesignvisitor::timesign(i);
				fCurrentTimeSign += timesignvisitor::timesign(i);
			}
			s >> timesign;
		}

    }
	if (fNotesOnly) return;

	Santescofoelement tag = antescofotag::create("meter");
    tag->add (antescofoparam::create(timesign));

	if (fGenerateBars) tag->add (antescofoparam::create("autoBarlines=\"off\"", false));
	add(tag);
}

//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_clef& elt ) 
{
	int staffnum = elt->getAttributeIntValue("number", 0);
	if ((staffnum != fTargetStaff) || fNotesOnly) return;

	stringstream s; 
	if ( clefvisitor::fSign == "G")			s << "g";
	else if ( clefvisitor::fSign == "F")	s << "f";
	else if ( clefvisitor::fSign == "C")	s << "c";
	else if ( clefvisitor::fSign == "percussion")	s << "perc";
	else if ( clefvisitor::fSign == "TAB")	s << "TAB";
	else if ( clefvisitor::fSign == "none")	s << "none";
	else {													// unknown clef sign !!
		cerr << "warning: unknown clef sign \"" << clefvisitor::fSign << "\"" << endl;
		return;	
	}
	
	string param;
	if (clefvisitor::fLine != clefvisitor::kStandardLine) 
		s << clefvisitor::fLine;
    s >> param;
	if (clefvisitor::fOctaveChange == 1)
		param += "+8";
	else if (clefvisitor::fOctaveChange == -1)
		param += "-8";
	Santescofoelement tag = antescofotag::create("clef");
	checkStaff (staffnum);
    tag->add (antescofoparam::create(param));
    add(tag);
}

//______________________________________________________________________________
// tools and methods for converting notes
//______________________________________________________________________________
vector<S_slur>::const_iterator xmlpart2antescofo::findTypeValue ( const std::vector<S_slur>& slurs, const string& val ) const 
{
	std::vector<S_slur>::const_iterator i;
	for (i = slurs.begin(); i != slurs.end(); i++) {
		if ((*i)->getAttributeValue("type") == val) break;
	}
	return i;
}

//______________________________________________________________________________
vector<S_tied>::const_iterator xmlpart2antescofo::findTypeValue ( const std::vector<S_tied>& tied, const string& val ) const 
{
	std::vector<S_tied>::const_iterator i;
	for (i = tied.begin(); i != tied.end(); i++) {
		if ((*i)->getAttributeValue("type") == val) break;
	}
	return i;
}

//______________________________________________________________________________
vector<S_beam>::const_iterator xmlpart2antescofo::findValue ( const std::vector<S_beam>& beams, const string& val ) const 
{
	std::vector<S_beam>::const_iterator i;
	for (i = beams.begin(); i != beams.end(); i++) {
		if ((*i)->getValue() == val) break;
	}
	return i;
}

//______________________________________________________________________________
bool xmlpart2antescofo::checkTiedBegin ( const std::vector<S_tied>& tied )
{
	std::vector<S_tied>::const_iterator i = findTypeValue(tied, "start");
	bool r = false;
    if (i != tied.end()) {
        cout << "got start Tied"<<endl;

		Santescofoelement tag = antescofotag::create("tieBegin");
/*		string num = (*i)->getAttributeValue ("number");
        cout << "got start Tied number:" << num<< endl;
        if (num.size())
            tag->add (antescofoparam::create(num, false));
		string placement = (*i)->getAttributeValue("placement");
        if (placement == "below")
            tag->add (antescofoparam::create("curve=\"down\"", false));*/
		add(tag);
        r = true;
	}
    return r;
}

bool xmlpart2antescofo::checkTiedEnd ( const std::vector<S_tied>& tied )
{
	std::vector<S_tied>::const_iterator i = findTypeValue(tied, "stop");
    bool r = false;

	if (i != tied.end()) {
        cout << "got end Tied"<<endl;

		Santescofoelement tag = antescofotag::create("tieEnd");
/*		string num = (*i)->getAttributeValue ("number");
        if (num.size())
            tag->add (antescofoparam::create(num, false));
*/		add(tag);
        r = true;
	}
    return r;
}

//______________________________________________________________________________
void xmlpart2antescofo::checkSlurBegin ( const std::vector<S_slur>& slurs ) 
{
	std::vector<S_slur>::const_iterator i = findTypeValue(slurs, "start");
	if (i != slurs.end()) {
		string tagName = "slurBegin";
		string num = (*i)->getAttributeValue("number");
		if (num.size()) tagName += ":" + num;
		Santescofoelement tag = antescofotag::create(tagName);
		string placement = (*i)->getAttributeValue("placement");
        if (placement == "below")
            tag->add (antescofoparam::create("curve=\"down\"", false));
		add(tag);
	}
}

void xmlpart2antescofo::checkSlurEnd ( const std::vector<S_slur>& slurs ) 
{
	std::vector<S_slur>::const_iterator i = findTypeValue(slurs, "stop");
	if (i != slurs.end()) {
		string tagName = "slurEnd";
		string num = (*i)->getAttributeValue("number");
		if (num.size()) tagName += ":" + num;
		Santescofoelement tag = antescofotag::create (tagName);
		add(tag);
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::checkBeamBegin ( const std::vector<S_beam>& beams ) 
{
	std::vector<S_beam>::const_iterator i = findValue(beams, "begin");
	if (i != beams.end()) {
		if (!fBeamOpened ) {
			fCurrentBeamNumber = (*i)->getAttributeIntValue("number", 1);
//			Santescofoelement tag = antescofotag::create("beamBegin");	// poor support of the begin end form in antescofo
//			add (tag);
			Santescofoelement tag = antescofotag::create("beam");
			push (tag);
			fBeamOpened = true;
		}
	}
}

void xmlpart2antescofo::checkBeamEnd ( const std::vector<S_beam>& beams ) 
{
	std::vector<S_beam>::const_iterator i;
	for (i = beams.begin(); (i != beams.end()) && fBeamOpened; i++) {
		if (((*i)->getValue() == "end") && ((*i)->getAttributeIntValue("number", 1) == fCurrentBeamNumber)) {
			fCurrentBeamNumber = 0;
			pop();
			fBeamOpened = false;
		}
	}
/*
	std::vector<S_beam>::const_iterator i = findValue(beams, "end");
	if (i != beams.end()) {
		if (fCurrentBeamNumber == (*i)->getAttributeIntValue("number", 1)) {
			fCurrentBeamNumber = 0;
//			Santescofoelement tag = antescofotag::create("beamEnd");	// poor support of the begin end form in antescofo
//			add (tag);
			pop();
			fBeamOpened = false;
		}
	}
*/
}

//______________________________________________________________________________
void xmlpart2antescofo::checkStem ( const S_stem& stem ) 
{
	Santescofoelement tag;
	if (stem) {
		if (stem->getValue() == "down") {
			if (fCurrentStemDirection != kStemDown) {
				tag = antescofotag::create("stemsDown");
				fCurrentStemDirection = kStemDown;
			}
		}
		else if (stem->getValue() == "up") {
			if (fCurrentStemDirection != kStemUp) {
				tag = antescofotag::create("stemsUp");
				fCurrentStemDirection = kStemUp;
			}
		}
		else if (stem->getValue() == "none") {
			if (fCurrentStemDirection != kStemNone) {
				tag = antescofotag::create("stemsOff");
				fCurrentStemDirection = kStemNone;
			}
		}
		else if (stem->getValue() == "double") {
		}
	}
	else if (fCurrentStemDirection != kStemUndefined) {
		tag = antescofotag::create("stemsAuto");
		fCurrentStemDirection = kStemUndefined;
	}
	if (tag) add(tag);
}

//______________________________________________________________________________
int xmlpart2antescofo::checkArticulation ( const notevisitor& note ) 
{
	int n = 0;
	Santescofoelement tag;
	if (note.fAccent) {
		tag = antescofotag::create("accent");
		push(tag);
		n++;
	}
	if (note.fStrongAccent) {
		tag = antescofotag::create("marcato");
		push(tag);
		n++;
	}
	if (note.fStaccato) {
		tag = antescofotag::create("stacc");
		push(tag);
		n++;
	}
	if (note.fTenuto) {
		tag = antescofotag::create("ten");
		push(tag);
		n++;
	}
	return n;
}

//______________________________________________________________________________
vector<Sxmlelement> xmlpart2antescofo::getChord ( const S_note& elt ) 
{
	vector<Sxmlelement> v;
	ctree<xmlelement>::iterator nextnote = find(fCurrentMeasure->begin(), fCurrentMeasure->end(), elt);
	if (nextnote != fCurrentMeasure->end()) nextnote++;	// advance one step
	while (nextnote != fCurrentMeasure->end()) {
		// looking for the next note on the target voice
		if ((nextnote->getType() == k_note) && (nextnote->getIntValue(k_voice,0) == fTargetVoice)) { 
			ctree<xmlelement>::iterator iter;			// and when there is one
			iter = nextnote->find(k_chord);
			if (iter != nextnote->end())
				v.push_back(*nextnote);
			else break;
		}
		nextnote++;
	}
	return v;
}

//______________________________________________________________________________
void xmlpart2antescofo::checkCue (const notevisitor& nv) 
{
	if (nv.isCue()) {
		if (!fInCue) {
			fInCue = true;
			Santescofoelement tag = antescofotag::create("cue");
			push(tag);
		}
	}
	else if (fInCue) {
		fInCue = false;
		pop();			
	}
}

//______________________________________________________________________________
void xmlpart2antescofo::checkGrace (const notevisitor& nv) 
{
	if (nv.isGrace()) {
		if (!fInGrace) {
			fInGrace = true;
			Santescofoelement tag = antescofotag::create("grace");
			push(tag);
		}
	}
	else if (fInGrace) {
		fInGrace = false;
		pop();			
	}
}

//______________________________________________________________________________
int xmlpart2antescofo::checkFermata (const notevisitor& nv) 
{
	if (nv.inFermata()) {
		return 1;
	}
	return 0;
}


#if 0
//________________________________________________________________________
string xmlpart2antescofo::i2step(int i)
{
	switch (i) {
		case antescofocontextvisitor::A:	return "A";
		case antescofocontextvisitor::B:	return "B";
		case antescofocontextvisitor::C:	return "C";
		case antescofocontextvisitor::D:	return "D";
		case antescofocontextvisitor::E:	return "E";
		case antescofocontextvisitor::F:	return "F";
		case antescofocontextvisitor::G:	return "G";
	}
	return "";
}
#endif

//________________________________________________________________________
int xmlpart2antescofo::step2i(const std::string& step) const
{
	if (step.size() != 1) return -1;
	switch (step[0]) {
		case 'A':	return xmlpart2antescofo::A;
		case 'B':	return xmlpart2antescofo::B;
		case 'C':	return xmlpart2antescofo::C;
		case 'D':	return xmlpart2antescofo::D;
		case 'E':	return xmlpart2antescofo::E;
		case 'F':	return xmlpart2antescofo::F;
		case 'G':	return xmlpart2antescofo::G;
	}
	return -1;
}

//________________________________________________________________________
float xmlpart2antescofo::getMidiPitch(const notevisitor& nv) const
{
    //if (fType == kPitched) {
		int step = step2i(nv.getStep());
		if (step >= 0) {
			short step2pitch [] = { 0, 2, 4, 5, 7, 9, 11 };
			float pitch = ((nv.getOctave() + 1) * 12.f) + step2pitch[step];
			//cout << "============= getMidiPitch (pitch:" << pitch << " + alter:"  << nv.getAlter() << ")="<< pitch + nv.getAlter() <<endl;
			return pitch + nv.getAlter();
		}
	//}
    return -1;
}



//______________________________________________________________________________
string xmlpart2antescofo::noteName ( const notevisitor& nv )
{
	cout << "================== getalter: " << nv.getAlter() << endl;;
	string accident = alter2accident(nv.getAlter());
	string name;
	if (nv.getType() == notevisitor::kRest)
		name="_";
	else {
		name = nv.getStep();
		if (!name.empty()) name[0]=tolower(name[0]);
		else cerr << "warning: empty note name" << endl;
	}

	cout << "================== getalter name:: " << name << endl;;
	return name;
}

//______________________________________________________________________________
antescofonoteduration xmlpart2antescofo::noteDuration ( const notevisitor& nv )
{
	antescofonoteduration dur(0,0);
	if (nv.getType() == kRest) {
		rational r(nv.getDuration(), fCurrentDivision*4);
		r.rationalise();
		dur.set (r.getNumerator(), r.getDenominator());
	}
	else {
		rational r = NoteType::type2rational(NoteType::xml(nv.getGraphicType()));
		if (r.getNumerator() == 0) // graphic type missing or unknown
			r.set (nv.getDuration(), fCurrentDivision*4);
		r.rationalise();
		rational tm = nv.getTimeModification();
		r *= tm;
		r.rationalise();
		dur.set (r.getNumerator(), r.getDenominator(), nv.getDots());
	}

	return dur;
}
    
    
bool xmlpart2antescofo::checkNotation( S_note& elt )
{
    fTrill = fGlissandoStart = fGlissandoStop = false;
    ctree<xmlelement>::iterator next;
	for (ctree<xmlelement>::iterator i = elt->begin(); i != elt->end(); ) {
		next = i;
		next++;
		switch (i->getType()) {                
			case k_notations:
			//case k_lyric:
                for (ctree<xmlelement>::iterator j = elt->begin(); j != elt->end(); j++) {
                    switch (j->getType()) {
                        case k_ornaments:
                            for (ctree<xmlelement>::iterator k = elt->begin(); k != elt->end(); k++) {
                                switch (k->getType()) {
                                    case k_trill_mark:
                                        //Santescofoelement tag = antescofotag::create("trill");
                                        fTrill = true;
                                        return true;
                                }
                            }
                            //break;
                        case k_glissando:
                            if (j->getAttributeValue("type") == "start") fGlissandoStart = true;
                            if (j->getAttributeValue("type") == "stop") fGlissandoStop = true;
                            return true;
                        case k_slide:
                            if (j->getAttributeValue("type") == "start") fGlissandoStart = true;
                            if (j->getAttributeValue("type") == "stop") fGlissandoStop = true;
                            return true;
                    }
                }
                break;
		}
		i = next;
	}
    return false;
}

//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_duration& elt )
{
    notevisitor::visitStart( elt );
    
	bool scanElement = (elt->getIntValue(k_voice, 0) == fTargetVoice) && (elt->getIntValue(k_staff, 0) == fTargetStaff);	
	if (!scanElement) return;

    //int duration = (int)(*elt);
        stringstream s;
	s << elt->getValue();
        int duration;
        s >> duration;
    if (duration /*&& notevisitor::getVoice() == fTargetVoice*/) {
        rational r(duration, fCurrentDivision);//*4);
        r.rationalise();

        if (fInBackup) {
            cout << "Backup duration : removing "<< r.toFloat()<<" to fCurBeat:"<<fCurBeat << endl;
            fCurBeat -= r.toFloat();
            cout << "Backup duration : new fCurBeat:"<<fCurBeat << endl;
        }
        else if (fInForward) {
            cout << "Backup duration : adding "<< r.toFloat()<<" to fCurBeat:"<< fCurBeat << endl;
            fCurBeat += r.toFloat();
            cout << "Backup duration : new fCurBeat:"<<fCurBeat << endl;
        }
    }
    if (fCurBeat <= 0) fCurBeat = 1;
}

// check we want to convert this measure
bool xmlpart2antescofo::checkWriteMeasure()
{
		bool bad = w.write_measures.empty() ? false : true;
		for (vector<int>::const_iterator v = w.write_measures.begin(); v != w.write_measures.end(); v++) {
			if (*v == fMeasNum)
				bad = false;
		}
		return !bad;
}

//______________________________________________________________________________
void xmlpart2antescofo::newNote ( const notevisitor& nv,  S_note& elt  )
{
    checkNotation(elt);

    bool tiedStart = checkTiedBegin (nv.getTied());
    bool tiedEnd = checkTiedEnd (nv.getTied());

    rational d(nv.getDuration(), fCurrentDivision);
    d.rationalise();

    int flag = ANTESCOFO_FLAG_NULL;
    if (tiedStart)          flag = ANTESCOFO_FLAG_TIED_START;
    if (tiedEnd)            flag = ANTESCOFO_FLAG_TIED_END;
    if (fGlissandoStart)    flag = ANTESCOFO_FLAG_GLISSANDO_START;
    if (fGlissandoStop)     flag = ANTESCOFO_FLAG_GLISSANDO_STOP;
    if (checkFermata(nv))   flag = ANTESCOFO_FLAG_FERMATA;
    if (fRepeatForward)     flag = ANTESCOFO_FLAG_REPEAT_FORWARD;
    if (fRepeatBackward)    flag = ANTESCOFO_FLAG_REPEAT_BACKWARD;

    if (!checkWriteMeasure())
        return;

    // check we want to convert this staff
    bool badstaff = w.write_staves.empty() ? false : true;
    for (vector<string>::const_iterator v = w.write_staves.begin(); v != w.write_staves.end(); v++) {
        stringstream ss;
        ss << fCurrentStaff;
        // cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ COMPARE $$$$$$ "<< *v << " $$$$$$ and $$$$$$ " << ss.str() << endl;
        if (*v == ss.str()) {
            badstaff = false;
            cout << " ------------------------------------------ WARNING not converting this staff: " << *v << endl;
        }
    }
    if (badstaff) {
        return;
    }

    if (nv.inChord() && !fTrill && !fGlissandoStart && !fGlissandoStop) {
        w.AddNote(ANTESCOFO_CHORD, getMidiPitch(nv), d, fMeasNum, fCurBeat, flag, fRehearsals);
    }
    else if (nv.getType() == notevisitor::kRest)
        w.AddNote(ANTESCOFO_REST, 0, d, fMeasNum, fCurBeat, flag, fRehearsals);
    else if (fGlissandoStart || fGlissandoStop)
        w.AddNote(ANTESCOFO_MULTI, getMidiPitch(nv), d, fMeasNum, fCurBeat, flag, fRehearsals);
    else if (fTrill)
        w.AddNote(ANTESCOFO_TRILL, getMidiPitch(nv), d, fMeasNum, fCurBeat, flag, fRehearsals);
    else {
        if (nv.isGrace()) d = 0;
        w.AddNote(ANTESCOFO_NOTE, getMidiPitch(nv), d, fMeasNum, fCurBeat, flag, fRehearsals);
    }
}

void xmlpart2antescofo::advance_beat_duration()
{
    //cout << "advance_beat_duration you wanna "<<endl;

    // advance fCurBeat and fLastDur
	float d = ((float)(getDuration())) / fCurrentDivision;
	if (!inChord()) { // XXX ???  && ! isCue() && !inGrace())
        //cout << "advance_beat_duration in "<<endl;
        fCurBeat += d;
		fLastDur = d;
	}
}
    
//______________________________________________________________________________
void xmlpart2antescofo::visitEnd ( S_note& elt )
{
    //cout << "+++++++++++++++++++++++++++ note visit end: fCurBeat:"<< fCurBeat << " inChord()="<<inChord() << " !scanvoice:"<< !(notevisitor::getVoice() == fTargetVoice) << endl;
	notevisitor::visitEnd ( elt );

	if (inChord()) return;					// chord notes have already been handled

	bool scanVoice = (notevisitor::getVoice() == fTargetVoice);
#if 0
	if (!isGrace()) {
		moveMeasureTime (getDuration(), scanVoice);
		checkDelayed (getDuration());		// check for delayed elements (directions with offset)
    }
#endif

	if (!scanVoice) {
            return; 
        }
            checkStaff(notevisitor::getStaff());

            checkVoiceTime (fCurrentMeasurePosition, fCurrentVoicePosition);

            //	if (notevisitor::getType() != notevisitor::kRest)
            //		checkStem (notevisitor::fStem);
            //	checkCue(*this);    // inhibited due to poor support in guido (including crashes)
            checkGrace(*this);
            //checkSlurBegin (notevisitor::getSlur());
            //checkBeamBegin (notevisitor::getBeam());

            //if (checkFermata(*this)) { }
            //pendingPops += checkArticulation(*this);

            vector<Sxmlelement> chord = getChord(elt);

            newNote (*this, elt);
            for (vector<Sxmlelement>::const_iterator iter = chord.begin(); iter != chord.end(); iter++) {
                notevisitor nv;
                xml_tree_browser browser(&nv);
                Sxmlelement note = *iter;
                browser.browse(*note);
                checkStaff(nv.getStaff());
                newNote (nv, elt);
            } 

            //checkBeamEnd (notevisitor::getBeam());
            //checkSlurEnd (notevisitor::getSlur());

            fMeasureEmpty = false;

        
#if 1
            if (!isGrace()) {
                moveMeasureTime (getDuration(), scanVoice);
                checkDelayed (getDuration());		// check for delayed elements (directions with offset)
            }
            //advance_beat_duration();
#endif
}

//______________________________________________________________________________
// time management
//______________________________________________________________________________
void xmlpart2antescofo::visitStart ( S_divisions& elt ) 
{
	fCurrentDivision = (long)(*elt);
}

}

