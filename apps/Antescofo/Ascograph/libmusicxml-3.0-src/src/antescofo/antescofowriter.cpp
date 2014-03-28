/*

  Copyright (C) 2003-2013 Thomas Coffy

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

  thomas.coffy@ircam.fr
  http://repmus.ircam.fr/antescofo
*/

#include "antescofowriter.h"

bool debug_ = false;

namespace MusicXML2
{

const rational RATIONAL_INVALID(-1);

void antescofowriter::round_rational(rational &r) {
	if (r.getDenominator() > 50000)
		r.set(r.toFloat() * 10000, 10000);
}

void antescofowriter::v_Notesfind(rational& atbeat, vector<measure_elt>::iterator& i) {
	for (i = v_Notes.begin(); i != v_Notes.end(); i++) {
		if (i->m_pos == atbeat)
			break;
	}
}

bool antescofowriter::v_Noteserase(rational& atbeat) {
	for (vector<measure_elt>::iterator i = v_Notes.begin(); i != v_Notes.end(); i++) {
		if (i->m_pos == atbeat) {
			v_Notes.erase(i);
			return true;
		}
	}
	return false;
}


bool v_Notesless(const measure_elt& a, const measure_elt& b) {
	return a.m_pos.toFloat() < b.m_pos.toFloat();
}

void antescofowriter::findNoteInVector(rational atbeat, rational& ret, rational dur) 
{
	if (debug_) cout << "----------> findNoteInVector: atbeat: " << atbeat.toFloat() << endl;
	vector<measure_elt>::iterator endi = v_Notes.end(); endi--;
	vector<measure_elt>::iterator i;
	v_Notesfind(atbeat, i);
	if (i != v_Notes.end()) {
		ret = i->m_pos;
		return;
	}
	else { // if exact note doesn't exist, maybe a note, began before
		for (i = v_Notes.begin(); i != v_Notes.end(); i++) {
			// note could be before
			i->m_pos.rationalise();
			rational ra(i->m_pos + i->duration);
			ra.rationalise();
			atbeat.rationalise();
			// use toFloat() to compare because there seem to be some bugs without ....
			if ((i->m_pos.toFloat() <= atbeat.toFloat()) && (ra.toFloat() > atbeat.toFloat())) {
				if (endi == i)
				{ 	
					//ret = ra;
				} //else
				ret = i->m_pos;
				return;
			}
		}
	}
	ret = RATIONAL_INVALID;
	return;
}


// search for beat&measure in measure2beat map, 
// if does not exist
//   //if prevmeasure has beat
//   add
// else return real beat.
rational antescofowriter::AddBeat(rational beat, int nmeasure) {
	map<int, rational>::iterator i;
	if ((i = measure2beat.find(nmeasure)) == measure2beat.end()) { // not found
		/*
		//|| measure2beat[nmeasure + 1] > measure2beat[]) 
		map<int, float>::iterator p;
		if ((i = measure2beat.find(nmeasure-1)) != measure2beat.end()) { // not found
		*/
		measure2beat[nmeasure] = beat;
		return beat;

	} else { // found
		//cout << "AddBeat: got beat associated to measure: beat " << i->second.toFloat() << " and measure " << i->first << endl;
		map<int, rational>::iterator n, p;
		// if beatprev < beat < beatnext return beat
		if ((n = measure2beat.find(nmeasure+1)) != measure2beat.end()) {
			if (n->second > beat) {// && beat >= i->second) // if between current measure and next measure's beat

				if ((p = measure2beat.find(nmeasure-1)) != measure2beat.end()) {
					if (p->second < beat)//|| beat >= i->second)
						return beat;
				} 
				// return beat;
			}
		}
		if (beat >= i->second)
			return beat;
		return i->second;
	}
}

/*
   insert a note between existing notes: existBeat > curBeat
   -> create new note at curBeat which is a copy of note at thebeat with tiedNotes
*/
void antescofowriter::insertNote(int type, float pitch, rational dur, float nmeasure, rational &existBeat, rational curBeat, int flag_, string rehearsal) {
	if (type == ANTESCOFO_REST) return;
	if (dur.toFloat() < 0.00001) return;

	if (debug_) cout << "; insertNote(beat:" << curBeat.getNumerator() << "/" << curBeat.getDenominator() << " existBeat:" << existBeat.getNumerator() << "/" << existBeat.getDenominator() << " meas:" << nmeasure << " pitch:" << pitch << " dur:" << dur.getNumerator() 
		<< "/" << dur.getDenominator() << " type:" << type << endl;
	vector<measure_elt>::iterator existi, curi, nexti;
	v_Notesfind(existBeat, existi); // get an iterator for existing note

	rational existed_dur = existi->duration;
	// 1. cut existing note duration to existBeat - curBeat
	existi->duration = curBeat - existBeat;
	existi->duration.rationalise();
	rational cut_dur = existi->duration;
	cut_dur.rationalise();
	rational remain_dur = dur;
	if (debug_) cout << "insertNote(1): cutting existing dur at beat:" << existi->m_pos.toFloat() << " dur:" << existi->duration.toFloat() << " remain:" << remain_dur.toFloat() << endl;

	// 2. if (dur < existing dur): insert note at curBeat with dur duration
	//    else 
	// new note dur is > existing dur
	// while remain_dur > 0: merge notes with new one
	rational tmpbeat = existi->m_pos + existi->duration;
	bool tobetied = false;
	bool created_head = false;
	while (remain_dur.toFloat() > 0.001) {
		if (debug_) cout << "existi mpos:" << existi->m_pos.toFloat() << " remain_dur:" << remain_dur.toFloat()<< " dur:" << existi->duration.toFloat() << endl; 
		if (existi == v_Notes.end()) { // last elt
			measure_elt *e = new measure_elt();
			e->nMeasure = nmeasure;
			e->type = type;
			dur.rationalise();
			remain_dur.rationalise();
			e->duration = remain_dur;
			e->m_pos = tmpbeat;
			e->flags = flag_;
			if (flag_ == ANTESCOFO_FLAG_TIED_END) e->tiednotes.push_back(pitch);
			if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
			if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
			if (tobetied) {
				e->flags = ANTESCOFO_FLAG_TIED_END;
				e->tiednotes.push_back(pitch);
			}
			e->pitches.push_back(pitch);
			if (debug_) cout << "insertNote(1.4): adding at end: dur:"<< e->duration.toFloat() << endl;
			v_Notes.insert(v_Notes.end(), *e);
			break;
		}
		nexti = existi; nexti++;
		measure_elt *e;
		rational durdiff = remain_dur - existi->duration; durdiff.rationalise();
		if (debug_) cout << "durdiff= " << durdiff.toFloat() << endl;
		if (rational(-1, 1000) < durdiff && durdiff < rational(1, 1000) ) {
			if (!created_head) {
				e = new measure_elt();
				e->duration = existed_dur - cut_dur; //tmpbeat - existi->m_pos - remain_dur + existi->duration;
				e->duration.rationalise();
				if (debug_) cout << "M M M M M M M M  : e->duration="<< e->duration.toFloat() << " existed_dur="<< existed_dur.toFloat() << " cut_dur="<< cut_dur.toFloat() << endl;
				remain_dur -= e->duration;
				e->m_pos = existi->m_pos; 
				e->m_pos += cut_dur; e->m_pos.rationalise();
				e->flags = flag_;
				e->nMeasure = nmeasure;
				if (flag_ == ANTESCOFO_FLAG_TIED_END) e->tiednotes.push_back(pitch);
				if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
				if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
				e->pitches.push_back(pitch);
				for (vector<int>::iterator c = existi->pitches.begin(); c != existi->pitches.end() && *c != pitch; c++) {
					e->flags = ANTESCOFO_FLAG_TIED_END;
					e->tiednotes.push_back(*c);
					e->pitches.push_back(*c);
				}
				e->type = existi->type;
				if (e->pitches.size() > 1) e->type = ANTESCOFO_CHORD;
				if (debug_) cout << "insertNote(1.6): inserting note: beat:"<< e->m_pos.toFloat() <<" dur:"<< e->duration.toFloat() << endl;
				curi = v_Notes.insert(nexti, *e);
				created_head = true;
				existi = nexti;
			} else {
				e = &(*existi);
				e->pitches.push_back(pitch);
				if (e->pitches.size() > 1) e->type = ANTESCOFO_CHORD;
				else e->type = type;
				if (debug_) cout << "insertNote(1.6): adding at equal"<< endl;
				if (tobetied || flag_ == ANTESCOFO_FLAG_TIED_END) {
					e->flags = ANTESCOFO_FLAG_TIED_END;
					e->tiednotes.push_back(pitch);
				}
			}
			break;
		} else {
			if (remain_dur > existi->duration) {
				if (!created_head) {
					e = new measure_elt();
					e->duration = existed_dur - cut_dur; //tmpbeat - existi->m_pos - remain_dur + existi->duration;
					e->duration.rationalise();
					if (debug_) cout << "P P P P P P P P P P P : e->duration="<< e->duration.toFloat() << " existed_dur="<< existed_dur.toFloat() << " cut_dur="<< cut_dur.toFloat() << endl;
					remain_dur -= e->duration;
					e->m_pos = existi->m_pos; 
					e->m_pos += cut_dur; e->m_pos.rationalise();
					e->flags = flag_;
					e->nMeasure = nmeasure;
					if (flag_ == ANTESCOFO_FLAG_TIED_END) e->tiednotes.push_back(pitch);
					if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
					if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
					e->pitches.push_back(pitch);
					for (vector<int>::iterator c = existi->pitches.begin(); c != existi->pitches.end() && *c != pitch; c++) {
						e->flags = ANTESCOFO_FLAG_TIED_END;
						e->tiednotes.push_back(*c);
						e->pitches.push_back(*c);
					}
					e->type = existi->type;
					if (e->pitches.size() > 1) e->type = ANTESCOFO_CHORD;
					if (debug_) cout << "insertNote(1.62): inserting note: beat:"<< e->m_pos.toFloat() <<" dur:"<< e->duration.toFloat() << endl;
					curi = v_Notes.insert(nexti, *e);
					created_head = true;
					existi = nexti;
				} else {
					e = &(*existi);
					remain_dur -= existi->duration;
					e->pitches.push_back(pitch);
					if (e->type == ANTESCOFO_REST && e->pitches.size() == 1)
						e->type = ANTESCOFO_NOTE;
					if (e->type == ANTESCOFO_NOTE && e->pitches.size() > 1)
						e->type = ANTESCOFO_CHORD;
					if (tobetied) {
						e->flags = ANTESCOFO_FLAG_TIED_END;
						e->tiednotes.push_back(pitch);
					}
				}
				if (debug_) cout << "insertNote(1.8): adding at >"<< endl;
				//cout << "insertNote(1.8): printing NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOTES" << endl; print(false);
				tobetied = true;
			} else { // remain_dur < existi->duration:
				// add a measure_elt with pitch and existing pitch
				if (debug_) cout << "insertNote(1.9): adding at <"<< endl;
				e = new measure_elt();
				dur.rationalise();
				//existed_dur = existi->duration;
				remain_dur.rationalise();
				e->duration = remain_dur;//- cut_dur -  //tmpbeat - existi->m_pos - remain_dur + existi->duration;
				e->flags = flag_;
				existi->m_pos.rationalise();
				cout << "cut_dur = " << cut_dur.toString() << " = " << cut_dur.toFloat() << endl;
				round_rational(cut_dur);
				cout << "after round: cut_dur = " << cut_dur.toString() << " = " << cut_dur.toFloat() << endl;
				e->m_pos = existi->m_pos; e->m_pos += cut_dur; e->m_pos.rationalise();
				cout << "existi->m_pos = " << existi->m_pos.toString() << " = "<< existi->m_pos.toFloat() << endl;
				//if (cut_dur.toFloat() > - fabs(cut_dur.toFloat()) == fabs(cut_dur.toFloat())
				cout << "e->m_pos = " << e->m_pos.toFloat() << endl;
				e->nMeasure = nmeasure;
				if (flag_ == ANTESCOFO_FLAG_TIED_END) e->tiednotes.push_back(pitch);
				if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
				if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
				e->pitches.push_back(pitch);
				for (vector<int>::iterator c = existi->pitches.begin(); c != existi->pitches.end() && *c != pitch; c++) {
					e->flags = ANTESCOFO_FLAG_TIED_END;
					e->tiednotes.push_back(*c);
					e->pitches.push_back(*c);
				}
				if (tobetied) {
					e->flags = ANTESCOFO_FLAG_TIED_END;
					e->tiednotes.push_back(pitch);
				}
				e->type = existi->type;
				if (debug_) cout << "insertNote(2): inserting note: beat:" << e->m_pos.toFloat() << " dur:" << e->duration.toFloat() << endl;
				curi = v_Notes.insert(nexti, *e);
				//cout << "insertNote(2.1): printing NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOTES" << endl; print(false);
				if (e->m_pos.toFloat() < 0) { cout << "ERROR: got negative beat: " << e->m_pos.toFloat() << endl; break;} //abort();

				// insert remaining existing note 
				rational r = existed_dur - remain_dur - cut_dur;
				if (r.toFloat() > 0.) {
					e = new measure_elt();
					e->nMeasure = nmeasure;
					dur.rationalise();
					existi->m_pos.rationalise(); cut_dur.rationalise(); remain_dur.rationalise();
					//if (debug_) cout << "m_pos========> adding: " << existi->m_pos.toFloat() << " + " << cut_dur.toFloat() << " + " << remain_dur.toFloat() << endl;
					//e->m_pos = existi->m_pos + cut_dur + remain_dur;//existi->m_pos + existi->duration; e->m_pos.rationalise();
					e->m_pos = existi->m_pos; e->m_pos += cut_dur; e->m_pos.rationalise(); e->m_pos += remain_dur; e->m_pos.rationalise();
					e->duration = r; e->duration.rationalise();
					e->flags = flag_;
					if (flag_ == ANTESCOFO_FLAG_TIED_END) e->tiednotes.push_back(pitch);
					if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
					if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
					if (type == ANTESCOFO_NOTE || type == ANTESCOFO_CHORD || type == ANTESCOFO_REST) {
						for (vector<int>::iterator c = existi->pitches.begin(); c != existi->pitches.end() && *c != pitch; c++) {
							e->flags = ANTESCOFO_FLAG_TIED_END;
							e->tiednotes.push_back(*c);
							e->pitches.push_back(*c);
						}
					}
					e->type = existi->type;
					curi++;
					if (debug_) cout << "insertNote(3_2): beat:" << e->m_pos.toFloat() << " dur:" << e->duration.toFloat() << endl;
					v_Notes.insert(curi, *e);
				}
				if (e->m_pos.toFloat() < 0) abort();
				nexti = existi; nexti++;
				break;
			}
			//remain_dur -= e->duration;
			if (debug_) cout << "insertNote(loop): remain_dur = " << remain_dur.toFloat() << endl;
			//cout << "insertNote(loop): printing NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOTES" << endl; print(false);
			if (remain_dur.toFloat() < 0.) {
				remain_dur = 0.;
			}
		}
		existi++; 
	}
}


// curBeat is relative to the measure
// so if nmeasure>1 we suppose notes were added before,
// and the curBeat in absolute beats can be found.
void antescofowriter::AddNote(int type, float pitch, rational dur, float nmeasure, rational &curBeat, int flag_, string rehearsal) {
	vector<measure_elt>::iterator i;
	if (debug_) cout << "; Addnote(beat:"<<curBeat.getNumerator() << "/" << curBeat.getDenominator() << ", meas:" << nmeasure <<" pitch:"<<pitch << " dur:"<< dur.getNumerator()<<"/"<<dur.getDenominator() << " type:"<<  type << " bpm:"<<fBPM<<") ";
	if (dur.toFloat() < 0.00001) return;

	//cout << "AddBeat: beat:" << curBeat.toFloat() << " measure " << nmeasure << endl;
	//curBeat = AddBeat(curBeat, nmeasure);
	//cout << "AddBeat: ==> curBeat:" << curBeat.toFloat() << " measure " << nmeasure << endl;
	if (curBeat.getNumerator() < 0) {
		if (pitch == 0)
			return;
		abort();
		return;
	}
#if 1
	rational abs_curBeat(0);
	if (nmeasure > 1 && curBeat.toFloat() == 1.) {
		if (pitch == 0.)
			return;
		cerr << "AntescofoWriter: something went wrong with note beats, trying to figure out current beat from measure number." << endl;
		for (vector<measure_elt>::const_iterator i = v_Notes.begin(); i != v_Notes.end(); i++) {
			if (i->nMeasure == nmeasure) {
				abs_curBeat = i->m_pos + curBeat - rational(1);
				break;
			}
		}
		if (abs_curBeat.getNumerator()) {
			curBeat = abs_curBeat;
			if (debug_) cout << "; Addnote(beat changed to :"<<curBeat.toFloat()<<endl;
		} else {
			// try to find nearest measure, and add beats...
			rational diffb = 0;
			for (vector<measure_elt>::const_iterator i = v_Notes.begin(); !abs_curBeat.toFloat() && i != v_Notes.end(); i++) {
				for (int diffm = 1; diffm != 4; diffm++) {
					if (i->nMeasure - diffm == nmeasure) {
						diffb += i->duration;
						for (int d = 0; d != diffm && i != v_Notes.end(); d++, i--) {
							diffb += i->duration;
						}
						abs_curBeat = i->m_pos + diffb;
						break;
					}
				}
			}
			if (abs_curBeat.getNumerator()) {
				curBeat = abs_curBeat;
				if (debug_) cout << "; Addnote(beat interpolated changed to :"<<curBeat.toFloat()<<endl;
			}
			else {
				cerr << "AntescofoWriter: something went wrong with note beats, check your MusicXML file, sorry." << endl;
				if (debug_) print(false);
				antescofo_abort();
			}
		}
	}
#endif
	if (curBeat.getNumerator() == 1. && nmeasure > 1)  { antescofo_abort(); }
	if (debug_) {
		if (flag_ == ANTESCOFO_FLAG_TIED_START) cout << "tie=Start";
		if (flag_ == ANTESCOFO_FLAG_TIED_END) cout << "tie=End";
		if (flag_ == ANTESCOFO_FLAG_TREMOLO_START) cout << "TRILL=Start";
		if (flag_ == ANTESCOFO_FLAG_TREMOLO_STOP) cout << "TRILL=End";
	}
	// find note in vector
	rational existBeat;
	findNoteInVector(curBeat, existBeat, dur);
	if (existBeat == RATIONAL_INVALID) {
		if (debug_) cout << "!!!!!! RATIONAL_INVALID" << endl;
		if (v_Notes.size() && (curBeat.toFloat() < (--v_Notes.end())->m_pos.toFloat()) && type == ANTESCOFO_REST)
			return;
		i = v_Notes.end();
	} else {
		if (curBeat > existBeat) {
			insertNote(type, pitch, dur, nmeasure, existBeat, curBeat, flag_, rehearsal);
			return;
		} else if (curBeat < existBeat) {
			insertNote(type, pitch, dur, nmeasure, existBeat, curBeat, flag_, rehearsal);
		}
		v_Notesfind(curBeat, i);
		if (type == ANTESCOFO_REST) return;
	}
	if (i == v_Notes.end()) { // new note
		measure_elt *e = new measure_elt();
		e->nMeasure = nmeasure;
		e->type = type;
		dur.rationalise();
		e->duration = dur;
		e->m_pos = curBeat;
		e->flags = flag_;
		if (flag_ == ANTESCOFO_FLAG_TIED_END) e->tiednotes.push_back(pitch);
		if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
		if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
		if (type == ANTESCOFO_NOTE || type == ANTESCOFO_CHORD || type == ANTESCOFO_REST) {
			if (type != ANTESCOFO_REST) {
				if (dur.toFloat() == .0) // grace
					e->grace_pitches.push_back(pitch);
				else e->pitches.push_back(pitch);
			}   
			if (debug_) cout << "; AddNote: adding single note: " << curBeat.toFloat() <<endl;
		} else if (type == ANTESCOFO_TRILL) {
			e->pitches.push_back(pitch);
			if (debug_) cout << "; AddNote: TRILL note: " << curBeat.toFloat() <<endl;
		} else if (type == ANTESCOFO_MULTI) {
			e->pitches.push_back(pitch);
			if (debug_) cout << "; AddNote: MULTI note: " << curBeat.toFloat() <<endl;
		}
		//v_Notes[curBeat] = *e;
		if (debug_) cout << "Addnote: inserting note: beat:" << e->m_pos.toFloat() << " dur:" << e->duration.toFloat() << endl;
		v_Notes.push_back(*e);
		if (debug_) cout << endl;
	} else { // note already exists at this position
		if (type == ANTESCOFO_REST) return;

		if (i->m_pos != curBeat) { // if note was there, but not on the exact same beat
			if (debug_) cout << "!!!!!!!!!!!! Inserting note in the middle of a previous one. !!!!!!!!!!!!!!" << endl;
			if (debug_) cout << "<<<<<<<< WTF: prevBeat:" << i->m_pos.getNumerator() << "/" << i->m_pos.getDenominator() << " curBeat:" << curBeat.getNumerator() << "/" << curBeat.getDenominator() << " prevdur:"<< i->duration.toFloat() << " dur:" << dur.toFloat() << endl;
			if (i->m_pos != curBeat)
				if (debug_) cout << "i->m_pos=" << i->m_pos.toFloat() << " != " << curBeat.toFloat() << " ### mpos:"<< i->m_pos.toFloat() <<" ###############################################################"; 
			//print(true); abort();
			if (i->type == ANTESCOFO_REST) { // XXX only handle REST for now
				rational d = i->duration;
				if (debug_) cout << "!!!!!!!!!!!! Inserting note: previous note: beat:"<< i->m_pos.toFloat() << " dur:"<< i->duration.toFloat() << endl;
				if (i->m_pos.getNumerator() == 0 && i->duration.getNumerator() == 0) return;
				i->duration = curBeat - i->m_pos;//rational(curBeat*1000, 1000) - rational(i->m_pos * 1000, 1000);// shorten previous note duration
				if (debug_) cout << "!!!!!!!!!!!! Inserting note: reducing first one dur:"<< i->duration.toFloat() << endl;
				if (i->duration + dur < d) { // if prev dur < dur add note
					rational tmpcurbeat(curBeat); tmpcurbeat += dur;//.toFloat();
					rational bkpbeat = curBeat;
					if (debug_) cout << "!!!!!!!!!!!!  Inserting note: dur:"<< (d - i->duration - dur).toFloat() << " atbeat:"<< tmpcurbeat.toFloat() << " !!!!!!!!!!!!!!" << endl;
					if (((d - i->duration - dur).toFloat()) <= 0 ) {
						abort(); return; 
					}
					AddNote(ANTESCOFO_REST, 0, d - i->duration - dur, nmeasure, tmpcurbeat);
					curBeat = bkpbeat;
				} // XXX > ?
				dur.rationalise();
				AddNote(type, pitch, dur, nmeasure, curBeat);
				return;
			}

		}

		if (i->duration == dur || dur.toFloat() == .0) { // if already present note's duration is same than new one, merge it
			if (i->type == ANTESCOFO_REST) { // replace REST by NOTE
				if (dur.toFloat() == .0) { // grace
					if (debug_) cout << "Replace rest by grace note" << endl;
					i->grace_pitches.push_back(pitch);
					i->type = type;
				} else {
					if (debug_) cout << "Replace rest by note" << endl;
					i->pitches.push_back(pitch);
					i->type = type;
					dur.rationalise();
					i->duration = dur;
				}
				i->nMeasure = nmeasure;
				i->m_pos = curBeat;
				i->flags = flag_;
				if (flag_ == ANTESCOFO_FLAG_FERMATA) i->bFermata = true;
				return;
			} else if (type != ANTESCOFO_REST && (i->type == ANTESCOFO_NOTE || i->type == ANTESCOFO_CHORD)) { // change NOTE to CHORD
				if (i->duration.toFloat() == .0) // previous note was a grace note, so just add note.
				{
					if (dur.toFloat() == .0) {
						i->grace_pitches.push_back(pitch);
						if (i->grace_pitches.size() > 1)
							i->type = ANTESCOFO_CHORD;
					} else 
						i->pitches.push_back(pitch);

					if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
					dur.rationalise();
					i->duration = dur;
					i->nMeasure = nmeasure;
					i->m_pos = curBeat;
					i->flags = flag_;
					if (flag_ == ANTESCOFO_FLAG_FERMATA) i->bFermata = true;
					if (debug_) cout << "; Addnote : handling grace note." << endl;
					return;
				}
			} // else :
			i->type = ANTESCOFO_CHORD;
			if (dur.toFloat() == .0) // handle grace notes
				i->grace_pitches.push_back(pitch);
			else i->pitches.push_back(pitch);
			i->flags = flag_;
			if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
			i->nMeasure = nmeasure;
			i->m_pos = curBeat;
		} else { // if already present note's duration is different of new one, create it.
			/* i->second.type = ANTESCOFO_CHORD;
				 i->second.pitches.push_back(pitch);
				 i->second.nMeasure = nmeasure;
				 i->second.m_pos = curBeat; */
			rational new_beat = curBeat + i->duration;
			new_beat.rationalise();
			int new_measure = nmeasure;
			rational new_dur = i->duration - dur;
			new_dur.rationalise();
			if (new_dur.toFloat() > 1/1000) { // note (or chord) already in the list is longer than new note: new note is shorter
				if (debug_) cout << "; Addnote : expanding note("<<pitch<<") new_dur:" << new_dur.toFloat() << " curBeat="<<curBeat.toFloat();
				curBeat.rationalise();
				bool wasrest = false;
				if (i->type == ANTESCOFO_REST) {
					i->type = type;
					wasrest = true;
				} else
					i->type = ANTESCOFO_CHORD;
				i->pitches.push_back(pitch);
				if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
				i->flags = flag_;
				i->nMeasure = nmeasure;
				i->m_pos = curBeat;

				// on the beat: old + new
				dur.rationalise();
				i->duration = dur;
				if (debug_) cout << " shorten note: beat "<< i->m_pos.toFloat() << ", dur: "<< i->duration.toFloat() << ", newdur:"<<new_dur.toFloat()<< endl;
				/* cout << endl << endl << "newBeat = " << new_beat.toFloat() << endl;
				cout << endl << endl << "curBeat = " << curBeat.toFloat() << endl;
				cout << endl << endl << "newdur = " << new_dur.toFloat() << endl;
				cout << endl << endl << "i->duration = " << i->duration.toFloat() << endl;*/

				if (wasrest) {
					rational tmpcurbeat = new_beat; tmpcurbeat -= new_dur;
					tmpcurbeat.rationalise();
					measure_elt* e = new measure_elt();
					e->duration = new_dur;
					e->duration.rationalise();
					e->m_pos = tmpcurbeat;
					e->nMeasure = new_measure;
					e->type = ANTESCOFO_REST;
					vector<measure_elt>::iterator ni = i; ni++;
					if (debug_) cout << "Addnote: inserting rest: beat:" << e->m_pos.toFloat() << " dur:" << e->duration.toFloat() << endl;
					v_Notes.insert(ni, *e);
				} else {
					rational tmpcurbeat(curBeat); tmpcurbeat += dur;
					for (vector<int>::iterator c = i->pitches.begin(); c != i->pitches.end() && *c != pitch; c++) {
						if (tmpcurbeat.toFloat() < 0.) {
							if (debug_) cout << "Continuing because tmpcurbeat=" << tmpcurbeat.toFloat() << endl;
							if (debug_) cout << "dur=" << dur.toFloat() << " curBeat:" << curBeat.toFloat() << endl;
							if (debug_ && dur.toFloat() < 0.) cout << "dur=" << dur.toFloat() << endl;
							continue;
						}
						if (debug_) cout << "=============== ADDING NOTE: "<< *c << " b:"<< tmpcurbeat.toFloat()<< endl;
						new_dur.rationalise();
						tmpcurbeat.rationalise();
						//XXX ??? if (*c < 0) break;
						if (*c)
							AddNote(ANTESCOFO_CHORD, -(*c), new_dur, new_measure, tmpcurbeat);//, flag_);
						else break;
					}
				}
			} else if (new_dur.toFloat() < -1/1000) { // new note is longer than already present one, so copy present pitches into new note with new_dur duration
				new_dur = rational(0) - new_dur;
				if (debug_) cout << "; Addnote : expanding new longer note: new_dur:" << new_dur.toFloat() << endl;
				if (i->type == ANTESCOFO_REST) { // replace REST by NOTE, then add REST with diff duration
					if (debug_) cout << "replace REST by NOTE, then add REST with diff duration"<<endl;
					i->pitches.push_back(pitch);
					if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
					i->type = type;
					vector<measure_elt>::iterator n = i; n++;
					i->duration = n->m_pos - curBeat; // dur
					if (i->duration > dur) i->duration = dur;
					i->duration.rationalise();
					rational newbeat = curBeat; newbeat += dur; newbeat.rationalise();
					// check if next note(?s) needs to be shortened
					if (n != v_Notes.end()) {
						new_dur.rationalise();
						if (n->type == ANTESCOFO_REST) {
							if (debug_) cout << "erasing " << newbeat.toFloat() << " SILENCE " << endl;
							//v_Notes.erase(n->first);
							v_Noteserase(n->m_pos);
							AddNote(ANTESCOFO_REST, 0, new_dur, nmeasure, newbeat);
						} else { // if existing note was rest, and next note isn't rest
							//insertNote(type, pitch, new_dur, nmeasure, i->m_pos, newbeat, ANTESCOFO_FLAG_TIED_END, rehearsal);
							newbeat = curBeat; i->duration.rationalise();
							newbeat += i->duration;
							newbeat.rationalise();
							if (debug_) cout << "insert " << newbeat.toFloat() << " new note: type=" << type << endl;
							AddNote(type, pitch, new_dur, nmeasure, newbeat, ANTESCOFO_FLAG_TIED_END);
							//AddNote(ANTESCOFO_CHORD, pitch, new_dur, nmeasure, newbeat);
						}
					}
				} else if (i->duration.toFloat() == 0) { // handle grace notes
					if (i->pitches.size())
						i->grace_pitches.push_back(i->pitches[0]);
					i->type = type;
					i->flags = flag_;
					i->pitches.push_back(pitch);
					//if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
					i->duration = dur;
					i->duration.rationalise();
				} else {
					// on the beat: old + new
					//if (i->duration = ANTESCOFO_REST) i->duration = dur; else
					i->duration = MIN(i->duration, dur);
					i->type = ANTESCOFO_CHORD;
					i->pitches.push_back(pitch);
					if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
					i->flags = flag_;
					i->nMeasure = nmeasure;
					i->m_pos = curBeat;

					// if no notes exists after 
					vector<measure_elt>::iterator j = i; j++;
					if (j == v_Notes.end()) {
						// on the beat+prevdur: only new
						measure_elt *e = new measure_elt();
						// get measure_elt to know its type
						//e->type = (v_Notes[curBeat].pitches.size() + 1 > 1 ? ANTESCOFO_CHORD : ANTESCOFO_NOTE);
						v_Notesfind(curBeat, j);
						if (debug_) { cout << "v_Notesfind returned: beat:" << j->m_pos.toFloat() << " dur=" << j->duration.toFloat() << endl; }
						e->type = (j->pitches.size() + 1 > 1 ? ANTESCOFO_CHORD : ANTESCOFO_NOTE);
						new_dur.rationalise();
						e->duration = new_dur;
						rational tmpbeat = curBeat + i->duration; tmpbeat.rationalise();
						v_Notesfind(tmpbeat, j);
						//for (vector<int>::iterator c = v_Notes[tmpbeat].pitches.begin(); c != v_Notes[tmpbeat].pitches.end(); c++)
						if (!j->pitches.empty()) {
							for (vector<int>::iterator c = j->pitches.begin(); c != j->pitches.end(); c++)
								e->pitches.push_back(-(*c));
						}
						e->pitches.push_back(-pitch);
						if (flag_ == ANTESCOFO_FLAG_TIED_END) i->tiednotes.push_back(pitch);
						e->nMeasure = nmeasure;
						e->m_pos = tmpbeat;
						if (debug_) cout << "Addnote: adding note: beat:" << e->m_pos.toFloat() << endl;
						e->flags = flag_;
						if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
						if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
						v_Notes.push_back(*e);// XXX que nenni si la note existe deja a cette position
					} else {
						// insert note in notes
						/* i->duration.rationalise();
						rational tmbeat(curBeat); tmbeat += i->duration;
						tmbeat.rationalise();
						new_dur.rationalise();
						if (debug_) cout << "; Addnote : extend note recursively: curBeat=" << curBeat.toFloat() << " duration=" << i->duration.toFloat() << " tmpbeat=" << tmbeat.toFloat() << endl;
						AddNote(type, pitch, new_dur, nmeasure, tmbeat, ANTESCOFO_FLAG_TIED_END);
						*/
						rational tmp(0);
						curBeat.rationalise(); i->duration.rationalise();
						tmp = curBeat; tmp = tmp + i->duration; ;
						/*
						cout << "======> curBeat = " << curBeat.toString() << endl;
						cout << "======> i->duration = " << i->duration.toString() << endl;
						cout << "======> tmp = " << tmp.toString() << endl;
						*/
						tmp.rationalise();
						//cout << "======> tmp = " << tmp.toString() << endl;
						if (debug_) cout << "; Addnote : extend note recursively: curBeat=" << curBeat.toFloat() << " duration=" << i->duration.toFloat() << " (curBeat + i->duration)=" << tmp.toFloat() << endl;
						AddNote(type, pitch, new_dur, nmeasure, tmp, ANTESCOFO_FLAG_TIED_END);
					}
				}
			}
		}

	}
	//v_Notes[curBeat].rehearsal = rehearsal;
	v_Notesfind(curBeat, i);
	i->rehearsal = rehearsal;
	std::sort(v_Notes.begin(), v_Notes.end(), v_Notesless);
	final_compress();

	if (debug_) { cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl; print(false); }
}

void antescofowriter::print(bool with_header) {
	writestream(cout, with_header);
}


void antescofowriter::write(const char *outfilename) {
	ofstream outfile;

	outfile.open(outfilename);
	writestream(outfile, true);
	outfile.close();
}



void antescofowriter::setSelectedParts(vector<string> parts)
{
	write_parts = parts;
}

void antescofowriter::setSelectedStaves(vector<string> staves)
{
	for (vector<string>::iterator v = staves.begin(); v != staves.end(); v++) cout << "Not converting staff : "<<*v<< endl;
	write_staves = staves;
}

void antescofowriter::setSelectedVoices(vector<string> voices)
{
	write_voices = voices;
	//for (vector<string>::iterator v = voices.begin(); v != voices.end(); v++) cout << "$$$$$$$$$$$$$ voices: "<<*v<< endl; antescofo_abort();
}

void antescofowriter::setSelectedMeasures(vector<int> measures)
{
	write_measures = measures;
}



// final processing :
// - compress for each note, if the following note is the same pitch, and if they belong to the same measure
//   add durations and suppress next
// - compress tied notes
void antescofowriter::final_compress()
{
	// - compress
	if (v_Notes.size() < 2) return;
	vector<measure_elt>::iterator next = v_Notes.begin();
	vector<measure_elt>::iterator i = next;
	next++;
	for (; next != v_Notes.end(); ) {
		bool compressed = false;
		if (next->type == i->type) {
			if (next->type == ANTESCOFO_REST) {
				merge_notes(i, next);
				compressed = true;
			}
		}
		if (!compressed) // stay on the same first note if compressed
			i++;
		next = i;
		next++;
	}

	// - compress tied notes
	if (v_Notes.size() < 2) return;
	i = next = v_Notes.begin();
	if (next != v_Notes.end())
		next++;
	for (; next != v_Notes.end(); next++) {
		if (next->type == i->type) {
			if (next->pitches == i->pitches && i->flags == ANTESCOFO_FLAG_TIED_START && next->flags == ANTESCOFO_FLAG_TIED_END
					&& (next->flags != ANTESCOFO_FLAG_TREMOLO_START && next->flags != ANTESCOFO_FLAG_TREMOLO_STOP )) { // merge tied notes but not with tremolo
				if (debug_) cout << "Got tied notes (pos:"<<i->m_pos.toFloat()<<")... merging note." <<endl;
				merge_notes(i, next);
			} else if (i->type == ANTESCOFO_TRILL && (i->flags == ANTESCOFO_FLAG_TREMOLO_START && next->flags == ANTESCOFO_FLAG_TREMOLO_STOP )) {
				if (debug_) cout << "Got trill notes (pos:"<<i->m_pos.toFloat()<< " note:" << i->pitches[0] << ")... merging note." <<endl;
				merge_notes(i, next);
				i->flags = ANTESCOFO_FLAG_NULL;
				//i++, next++;
			}
		}
		if (i == v_Notes.end())
			break;
		i = next;
	}

}


// merge 2 notes a and b (add durations) in a, and delete b
void antescofowriter::merge_notes(vector<measure_elt>::iterator a, vector<measure_elt>::iterator b) {
	if (debug_) cout << "merge_notes " << a->m_pos.toFloat() << " and " << b->m_pos.toFloat() << endl;
	if (a->bpm.size()) {
		if (b->bpm.size())
			return; // do nothing if two note are separated by different tempos
	} else if (b->bpm.size())
		a->bpm = b->bpm;
	if (a->nMeasure != b->nMeasure) { //tied note between different measures
		//cout << "-------- jumping next measure: "<< a->second.nMeasure << "  ----------" << endl; 
		//a->second.nMeasure++;
	}
	a->duration += b->duration;
	a->duration.rationalise();
	if (a->pitches != b->pitches)
		a->pitches.insert(a->pitches.end(), b->pitches.begin(), b->pitches.end());
	v_Notes.erase(b);
}



void antescofowriter::antescofo_abort() {
	cout << "Antescofo abort: -------------- An error occured, leaving. ----------------" << endl;
	print(false);
	abort();
}

void antescofowriter::print_duration(ostream &out, rational &du) {
	if (du.toFloat() < 0.) abort();
	if (du.getNumerator() == 0) {
		out << "0";
	} else {
		out << du.getNumerator();
		if (du.getDenominator() != 1) {
			out << "/";
			out << du.getDenominator();
		}
	}
}

void antescofowriter::writenote(ostream &out, int pitch, measure_elt& e) {
	if (pitch > 0// && (e.flags == ANTESCOFO_FLAG_TIED_END) 
			&& std::find(e.tiednotes.begin(), e.tiednotes.end(), pitch) != e.tiednotes.end())
		pitch = -pitch;
	if (!print_notes_names) {// || pitch % 100 != 0) {
		out << pitch << "00";
	} else {
		string prefix = "";
		if (pitch < 0) {
			prefix = "-";
			pitch = -pitch;
		}
		if (pitch > 1000) pitch /= 100;
		int p = pitch % 12;
		int o = 0;
		if (pitch >= 12 && pitch <= 23) o = 0;
		if (pitch >= 24 && pitch <= 35) o = 1;
		if (pitch >= 36 && pitch <= 47) o = 2;
		if (pitch >= 48 && pitch <= 59) o = 3;
		if (pitch >= 60 && pitch <= 71) o = 4;
		if (pitch >= 72 && pitch <= 83) o = 5;
		if (pitch >= 84 && pitch <= 95) o = 6;
		if (pitch >= 96 && pitch <= 107) o = 7;
		if (pitch >= 108 && pitch <= 120) o = 8;
		string c = "";
		if (p == 0) c = "C";
		if (p == 1) c = "C#";
		if (p == 2) c = "D";
		if (p == 3) c = "D#";
		if (p == 4) c = "E";
		if (p == 5) c = "F";
		if (p == 6) c = "F#";
		if (p == 7) c = "G";
		if (p == 8) c = "G#";
		if (p == 9) c = "A";
		if (p == 10) c = "A#";
		if (p == 11) c = "B";
		out << prefix << c << o;
	}
	}



	void antescofowriter::writestream(ostream &out, bool with_header) {
		//expand();
		//std::sort(v_Notes.begin(), v_Notes.end(), rationalless);
		//cout << "notes sz:" << v_Notes.size() << endl;
		bool bNewMeasure = false;
		bool bPendingFermata = false;
		if (with_header) {
			final_compress();
			out << "; Antescofo score generated using libmusicxml and embedded xml2antescofo converter." <<std::endl;
			out << "; Copyright (c) Thomas Coffy - IRCAM 2013" <<std::endl;
		}
		if (v_Notes.size()) fBPM = v_Notes.begin()->bpm;
		if (!fBPM.size()) fBPM = string("120");
		fLastBPM = fBPM;
		out << "BPM " << fBPM << endl;
		int cur_meas = 0;
		rational fCurrBeat = rational(1);

		int last_glissando_pitch = 0;
		for (vector<measure_elt>::iterator i = v_Notes.begin(); i != v_Notes.end(); i++) { 
			if (i->nMeasure == 0 && i->m_pos.getNumerator() == 0 && i->duration.getNumerator() == 0)
				continue;

			// check we want to write this measure
			bool bad = write_measures.empty() ? false : true;
			for (vector<int>::const_iterator v = write_measures.begin(); v != write_measures.end(); v++) {
				if (*v == i->nMeasure)
					bad = false;
			}
			if (bad) continue;

			// display measure comment
			if (cur_meas != i->nMeasure) {
				bNewMeasure = true;
				cur_meas = i->nMeasure;
				out << endl << "; ----------- measure " << cur_meas << " --- beat " << i->m_pos.getNumerator();
				if (i->m_pos.getDenominator() != 1)
					out << "/" << i->m_pos.getDenominator()<< " ------";
				out << endl;
				if (i->m_pos.getNumerator() < 0 || i->m_pos.getDenominator() < 0)
					abort();
			}
			if (i->bFermata) {
				out << "TEMPO OFF" << endl;
				bPendingFermata = true;
			} else if (bPendingFermata) {
				out << "TEMPO ON" << endl;
				bPendingFermata = false;
			}
			if (i->bpm.size() && i->bpm != fBPM) {
				fBPM = i->bpm ;
				out << "BPM " << fBPM << " @modulate" << endl; 
			}
			if (i->duration.getNumerator()) i->duration.rationalise();

		// grace notes
		int grace_notes = i->grace_pitches.size();
		if (grace_notes) {
			//if (grace_notes == 1) { out << "NOTE "; writenote(out, i->grace_pitches[0]); out << " 0"; }
			//else if (grace_notes > 1) {
			//	out << "CHORD ( ";
			for (vector<int>::const_iterator j = i->grace_pitches.begin(); j != i->grace_pitches.end(); j++) {
				out << "NOTE "; writenote(out, *j, *i); out << " 0" << endl;
				//writenote(out, *j);//out << *j << "00 "; out << ") 0";// << endl;
			}
			//}
			//if (i->second.pitches.size()) out << endl; // don't print new measure label on grace notes
		} 
		if (i->type == ANTESCOFO_NOTE && i->pitches.size() && i->pitches[0]) {
			out << "NOTE "; writenote(out, i->pitches[0], *i); out << " ";
			print_duration(out, i->duration);
		} else if (i->type == ANTESCOFO_REST) {
			out << "NOTE 0 ";
			print_duration(out, i->duration);
		}
		else if (i->pitches.size() && i->pitches[0]) {
			// sort
			std::sort(i->pitches.begin(), i->pitches.end());
			// remove duplicates pitches
			vector<int>::iterator u = std::unique(i->pitches.begin(), i->pitches.end());
			i->pitches.resize(u - i->pitches.begin());

			if (i->type != ANTESCOFO_MULTI) {
				if (i->type == ANTESCOFO_CHORD) {
					if (i->pitches.size() == 1 && i->tiednotes.empty()) 
						out << "NOTE";
					else 
						out << "CHORD (";
				} else if (i->type == ANTESCOFO_TRILL || i->type == ANTESCOFO_FLAG_TREMOLO_START) {
					out << "TRILL (";
					if (i->pitches.size() == 1) {
						int p = *(i->pitches.begin());
						int s = 1;
						if (p > 1000) s = 500;
						i->pitches.push_back(p + s);
					}
				}
				last_glissando_pitch = 0;
			} else { // glissando flags may be: start, stop, stop
				if (last_glissando_pitch && i->flags == ANTESCOFO_FLAG_GLISSANDO_STOP) {
					out << "MULTI ( "; writenote(out, last_glissando_pitch, *i);
					out << " ->";
				}
				if (i->flags == ANTESCOFO_FLAG_GLISSANDO_START || i->flags == ANTESCOFO_FLAG_GLISSANDO_STOP)
					last_glissando_pitch = i->pitches[0];
			}
			for (vector<int>::const_iterator j = i->pitches.begin(); j != i->pitches.end(); j++) {
				if (i->flags != ANTESCOFO_FLAG_GLISSANDO_START) {
					out << " "; writenote(out, *j, *i);// << "00";
				}
				//if (i->second.type == ANTESCOFO_MULTI && i->second.flags == ANTESCOFO_FLAG_GLISSANDO_START) out << " ->";
			}

			if (i->flags != ANTESCOFO_FLAG_GLISSANDO_START) {
				if (i->pitches.size() > 1 || i->type == ANTESCOFO_TRILL || i->flags == ANTESCOFO_FLAG_GLISSANDO_STOP || i->flags == ANTESCOFO_FLAG_TREMOLO_STOP)
					out << " )";
				out << " ";
				print_duration(out, i->duration);// << i->second.duration.getNumerator() << "/" << i->second.duration.getDenominator();
			}
		}
		if (bNewMeasure && i->flags != ANTESCOFO_FLAG_GLISSANDO_START) {
			out << " measure"<< cur_meas;;
			bNewMeasure = false;
		}
		if (i->rehearsal.size()) 
			out << " ; marker: " << i->rehearsal;

		if (i->flags != ANTESCOFO_FLAG_GLISSANDO_START) {
			if (!debug_)
				out << endl;
			else out << " ; m_pos:" << i->m_pos.toFloat() << endl;
		}
	}
}

} // namespace MusicXML2 

