#ifndef __antescofowriter__
#define __antescofowriter__


#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>
#include "xml.h"
#include "xmlfile.h"
#include "xmlreader.h"
#include "rational.h"

using namespace std;

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

namespace MusicXML2
{



// ----------
// measure_elt_types
#   define ANTESCOFO_REST           0
#   define ANTESCOFO_CHORD          1
#   define ANTESCOFO_NOTE           2
#   define ANTESCOFO_TRILL          3
#   define ANTESCOFO_MULTI          4
#   define ANTESCOFO_MULTI_STOP     5

#   define ANTESCOFO_FLAG_NULL              0
#   define ANTESCOFO_FLAG_TIED_START        1
#   define ANTESCOFO_FLAG_TIED_END          2
#   define ANTESCOFO_FLAG_GLISSANDO_START   3
#   define ANTESCOFO_FLAG_GLISSANDO_STOP    4
#   define ANTESCOFO_FLAG_FERMATA			5
#   define ANTESCOFO_FLAG_REPEAT_BACKWARD   6
#   define ANTESCOFO_FLAG_REPEAT_FORWARD    7


class measure_elt {
	public:
		measure_elt() : bFermata(false), bpm(""), nMeasure(0), m_pos(0), type(0) { pitches.reserve(12); }
		int 				type;
		rational 			duration;
		vector<int>         pitches;
		vector<int>         grace_pitches;
		int					nMeasure; // measure number
		float				m_pos; // position in part, in beats unit
		int                 flags; // handles tied notes..
		bool operator<(const measure_elt& rhs) const { return (m_pos < rhs.m_pos); }
		bool operator==(const measure_elt& rhs) { return (m_pos == rhs.m_pos); }
		bool operator==(float pos) { return (m_pos == pos); }
		string              bpm;
        string  rehearsal;
		bool bFermata;
        string jump_dests;
};


class antescofowriter {
	public:
		enum pedalType { kDamperPedal, kSoftpedal, kSostenutoPedal };

		antescofowriter() : fLastBPM("0"), fBPM("120"), nBeats(4), print_notes_names(true) { }
		~antescofowriter() {}

		map<float, measure_elt> v_Notes;
		map<int, float> measure2beat;
		int nBeats, nBeat_type; // nBeats is the number of beats per measure
		string fBPM, fLastBPM;
		vector<string> write_voices;
		vector<string> write_staves;
		vector<string> write_parts;
		vector<int> write_measures;
		bool print_notes_names;

		void setBPM(string _bpm) { fBPM = _bpm; }

                map<float, measure_elt>::iterator findNoteInVector(float atbeat, rational dur) {
                    map<float, measure_elt>::iterator i;
                    if (((i = v_Notes.find(atbeat))) != v_Notes.end())
                        return i;
                    else { // if exact note doesn't exist, maybe a note, began before
			for (i = v_Notes.begin(); i != v_Notes.end(); i++) {
                            if (i->second.m_pos <= atbeat // note could be before
                                && i->second.m_pos + i->second.duration.toFloat() > atbeat)
                                    return i;
                        }
                    }
                    return v_Notes.end();
                }

                // search for beat&measure in measure2beat map, 
                // if does not exist
                //   //if prevmeasure has beat
                //   add
                // else return real beat.
                float AddBeat(float beat, int nmeasure) {
                    map<int, float>::iterator i;
                    if ((i = measure2beat.find(nmeasure)) == measure2beat.end()) { // not found
                        /*
                        //|| measure2beat[nmeasure + 1] > measure2beat[]) 
                        map<int, float>::iterator p;
                        if ((i = measure2beat.find(nmeasure-1)) != measure2beat.end()) { // not found
                        */
                        measure2beat[nmeasure] = beat;
                        return beat;

                    } else { // found
                        cout << "AddBeat: got beat associated to measure: beat " << i->second << " and measure " << i->first << endl;
                        map<int, float>::iterator n, p;
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
                
		// curBeat is relative to the measure
		// so if nmeasure>1 we suppose notes were added before,
		// and the curBeat in absolute beats can be found.
		void AddNote(int type, float pitch, rational dur, float nmeasure, float &curBeat, int flag_ = ANTESCOFO_FLAG_NULL, string rehearsal = "") {
			map<float, measure_elt>::iterator i;
			std::cout << "; Addnote(beat:"<<curBeat<< ", meas:" << nmeasure <<" pitch:"<<pitch << " dur:"<< dur.getNumerator()<<"/"<<dur.getDenominator() << " type:"<<  type << " bpm:"<<fBPM<<") ";
			//cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl; print(false);

			/*if (nmeasure > 1 && curBeat == 1) {
			  cout << "problem: replace " << curBeat << " by " << fCurBeat_internal << endl;
			  curBeat = fCurBeat_internal;
			  }*/

                        cout << "AddBeat: beat:" << curBeat << " measure " << nmeasure << endl;
                        curBeat = AddBeat(curBeat, nmeasure);
                        cout << "AddBeat: ==> curBeat:" << curBeat << " measure " << nmeasure << endl;
#if 1
			float abs_curBeat = .0;
			if (nmeasure > 1 && curBeat == 1) {
				cerr << "AntescofoWriter: something went wrong with note beats, trying to figure out current beat from measure number." << endl;
				for (map<float, measure_elt>::const_iterator i = v_Notes.begin(); i != v_Notes.end(); i++) {
					if (i->second.nMeasure == nmeasure) {
						abs_curBeat = i->second.m_pos + curBeat - 1;
						break;
					}
				}
				if (abs_curBeat) {
					curBeat = abs_curBeat;
					cout << "; Addnote(beat changed to :"<<curBeat<<endl;
				} else {
                                    // try to find nearest measure, and add beats...
                                    float diffb = 0;
                                    for (map<float, measure_elt>::const_iterator i = v_Notes.begin(); !abs_curBeat && i != v_Notes.end(); i++) {
                                        for (int diffm = 1; diffm != 4; diffm++) {
                                            if (i->second.nMeasure - diffm == nmeasure) {
                                                diffb += i->second.duration.toFloat();
                                                for (int d = 0; d != diffm && i != v_Notes.end(); d++, i--) {
                                                    diffb += i->second.duration.toFloat();
                                                }
                                                abs_curBeat = i->second.m_pos + diffb;
                                                break;
                                            }
                                        }
                                    }
                                    if (abs_curBeat) {
                                        curBeat = abs_curBeat;
                                        cout << "; Addnote(beat interpolated changed to :"<<curBeat<<endl;
                                    }
                                    else {
                                        cerr << "AntescofoWriter: something went wrong with note beats, check your MusicXML file, sorry." << endl;
                                        print(false);
                                        antescofo_abort();
                                    }
                                }
			}
#endif
			if (curBeat == 1 && nmeasure > 1)  { antescofo_abort(); }
			if (flag_ == ANTESCOFO_FLAG_TIED_START) cout << "tie=Start";
			if (flag_ == ANTESCOFO_FLAG_TIED_END) cout << "tie=End";
			// find note in vector
			if (( i = findNoteInVector(curBeat, dur)) == v_Notes.end()) { // new note
                                measure_elt *e = new measure_elt();
                                e->nMeasure = nmeasure;
                                e->type = type;
                                dur.rationalise();
                                e->duration = dur;
                                e->m_pos = curBeat;
                                e->flags = flag_;
                                if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
                                if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
                                if (type == ANTESCOFO_NOTE || type == ANTESCOFO_CHORD || type == ANTESCOFO_REST) {
                                    if (type != ANTESCOFO_REST) {
                                        if (dur.toFloat() == .0) // grace
                                            e->grace_pitches.push_back(pitch);
                                        else e->pitches.push_back(pitch);
                                    }   
                                    cout << "; AddNote: adding single note: " << curBeat <<endl;
                                } else if (type == ANTESCOFO_TRILL) {
                                    e->pitches.push_back(pitch);
                                    cout << "; AddNote: TRILL note: " << curBeat <<endl;
                                } else if (type == ANTESCOFO_MULTI) {
                                    e->pitches.push_back(pitch);
                                    cout << "; AddNote: MULTI note: " << curBeat <<endl;
                                }
                                v_Notes[curBeat] = *e;
                                cout << endl;
                            } else { // note already exists at this position
                                if (type == ANTESCOFO_REST) return;

                                if (i->second.m_pos != curBeat) { // if note was there, but not on the exact same beat
                                    cout << "!!!!!!!!!!!! Inserting note in the middle of a previous one. !!!!!!!!!!!!!!" << endl;
                                    if (i->second.type == ANTESCOFO_REST) { // XXX only handle REST for now
                                        rational d = i->second.duration;
                                        cout << "!!!!!!!!!!!! Inserting note: previous note: beat:"<< i->second.m_pos << " dur:"<< i->second.duration.toFloat() << endl;
                                        if (i->second.m_pos == 0 && i->second.duration.toFloat() == 0) return;
                                        i->second.duration = rational(curBeat*1000, 1000) - rational(i->second.m_pos * 1000, 1000);// shorten previous note duration
                                        cout << "!!!!!!!!!!!! Inserting note: reducing first one dur:"<< i->second.duration.toFloat() << endl;
                                        if (i->second.duration + dur < d) { // if prev dur < dur add note
                                            float tmpcurbeat = curBeat + dur.toFloat();
                                            float bkpbeat = curBeat;
                                            cout << "!!!!!!!!!!!! Inserting note: dur:"<< (d - i->second.duration - dur).toFloat() << " atbeat:"<< tmpcurbeat << " !!!!!!!!!!!!!!" << endl;
                                            if (((d - i->second.duration - dur).toFloat()) <= 0 ) return; 
                                            AddNote(ANTESCOFO_REST, 0, d - i->second.duration - dur, nmeasure, tmpcurbeat);
                                            curBeat = bkpbeat;
                                        } // XXX > ?
                                        dur.rationalise();
                                        AddNote(type, pitch, dur, nmeasure, curBeat);
                                        return;
                                    }

                                }

                                if (i->second.duration == dur || dur.toFloat() == .0) { // if already present note's duration is same than new one, merge it
                                if (i->second.type == ANTESCOFO_REST) { // replace REST by NOTE
                                    if (dur.toFloat() == .0) { // grace
                                        cout << "Replace rest by grace note" << endl;
                                        i->second.grace_pitches.push_back(pitch);
                                            i->second.type = type;
                                        } else {
                                            cout << "Replace rest by note" << endl;
                                            i->second.pitches.push_back(pitch);
                                            i->second.type = type;
                                            dur.rationalise();
                                            i->second.duration = dur;
                                        }
                                        i->second.nMeasure = nmeasure;
                                        i->second.m_pos = curBeat;
                                        i->second.flags = flag_;
                                        if (flag_ == ANTESCOFO_FLAG_FERMATA) i->second.bFermata = true;
                                        return;
                                    } else if (type != ANTESCOFO_REST && (i->second.type == ANTESCOFO_NOTE || i->second.type == ANTESCOFO_CHORD)) { // change NOTE to CHORD
                                        if (i->second.duration.toFloat() == .0) // previous note was a grace note, so just add note.
                                        {
                                            if (dur.toFloat() == .0) {
                                                i->second.grace_pitches.push_back(pitch);
                                                if (i->second.grace_pitches.size() > 1)
                                                    i->second.type = ANTESCOFO_CHORD;
                                            } else 
                                                i->second.pitches.push_back(pitch);
                                            dur.rationalise();
                                            i->second.duration = dur;
                                            i->second.nMeasure = nmeasure;
                                            i->second.m_pos = curBeat;
                                            i->second.flags = flag_;
                                            if (flag_ == ANTESCOFO_FLAG_FERMATA) i->second.bFermata = true;
                                            cout << "; Addnote : handling grace note." << endl;
                                            return;
                                        }
                                    }
                                    // else :
                                    i->second.type = ANTESCOFO_CHORD;
                                    i->second.pitches.push_back(pitch);
                                    i->second.nMeasure = nmeasure;
                                    i->second.m_pos = curBeat;
                                } else { // if already present note's duration is different of new one, create it.
					/*
						i->second.type = ANTESCOFO_CHORD;
						i->second.pitches.push_back(pitch);
						i->second.nMeasure = nmeasure;
						i->second.m_pos = curBeat;
						*/

						float new_beat = curBeat + i->second.duration.toFloat();
						int new_measure = nmeasure;
						rational new_dur = i->second.duration - dur;
						if (new_dur > rational(0)) { // note (or chord) already in the list is longer than new note: new note is shorter
							cout << "; Addnote : expanding note: new_dur:" << new_dur.toFloat();
							i->second.type = ANTESCOFO_CHORD;
							i->second.pitches.push_back(pitch);
							i->second.nMeasure = nmeasure;
							i->second.m_pos = curBeat;

							// on the beat: old + new
							i->second.duration = dur;
							cout << " shorten note: beat "<< i->second.m_pos << ", dur: "<< i->second.duration.toFloat() << " and new beat:"<< curBeat+i->second.duration.toFloat()<<  ", dur:"<<new_dur.toFloat()<< endl;

							for (vector<int>::iterator c = i->second.pitches.begin(); c != i->second.pitches.end() && *c != pitch; c++) {
								cout << "=============== ADDING NOTE: "<< *c << " b:"<< new_beat - new_dur.toFloat()<< endl;
								float tmpcurbeat = new_beat - new_dur.toFloat();
                                                                new_dur.rationalise();
								AddNote(ANTESCOFO_CHORD, *c, new_dur, new_measure, tmpcurbeat);
							}
						} else { // new note is longer than already present one, so copy present pitches into new note with new_dur duration
							new_dur = rational(0) - new_dur;
							cout << "; Addnote : expanding new longer note: new_dur:" << new_dur.toFloat();

							if (i->second.type == ANTESCOFO_REST) { // replace REST by NOTE, then add REST with diff duration
								cout << "replace REST by NOTE, then add REST with diff duration"<<endl;
								i->second.pitches.push_back(pitch);
								i->second.type = type;
								i->second.duration = dur;
								float newbeat = curBeat + dur.toFloat();
								// check if next note(?s) needs to be shortened
								map<float, measure_elt>::iterator n = i;
								n++;
								if (n != v_Notes.end() && n->second.type == ANTESCOFO_REST) {
									cout << "erasing " << newbeat << " SILENCE " << endl;
									v_Notes.erase(n->first);
                                                                        new_dur.rationalise();
									AddNote(ANTESCOFO_REST, 0, new_dur, nmeasure, newbeat);
								}
							} else {
								// on the beat: old + new
								//if (i->second.duration = ANTESCOFO_REST) i->second.duration = dur; else
								i->second.duration = MIN(i->second.duration, dur);
								i->second.type = ANTESCOFO_CHORD;
								i->second.pitches.push_back(pitch);
								i->second.nMeasure = nmeasure;
								i->second.m_pos = curBeat;

								// on the beat+prevdur: only new
								measure_elt *e = new measure_elt();
								e->type = (v_Notes[curBeat].pitches.size() + 1 > 1 ? ANTESCOFO_CHORD : ANTESCOFO_NOTE);
                                                                new_dur.rationalise();
								e->duration = new_dur;
								for (vector<int>::iterator c = v_Notes[curBeat + i->second.duration.toFloat()].pitches.begin(); c != v_Notes[curBeat + i->second.duration.toFloat()].pitches.end(); c++)
									e->pitches.push_back(*c);
								e->pitches.push_back(pitch);
								e->nMeasure = nmeasure;
								e->m_pos = curBeat + i->second.duration.toFloat();
								e->flags = flag_;
								if (flag_ == ANTESCOFO_FLAG_FERMATA) e->bFermata = true;
								if (fLastBPM != fBPM) { fLastBPM = fBPM; e->bpm = fBPM; }
								v_Notes[curBeat + i->second.duration.toFloat()] = *e;// XXX que nenni si la note existe deja a cette position
							}
						}
					}

					//cout << "; Adding CHORD" << endl;
					/*cout << "CHORD pitches sz:" << i->second.pitches.size() << " : ";// << <<endl;
					for (vector<int>::const_iterator j = i->second.pitches.begin(); j != i->second.pitches.end(); j++)
						cout << *j << "00 ";
					cout << endl;*/

				//}
			}
                        v_Notes[curBeat].rehearsal = rehearsal;
		}

		void print(bool with_header=true) {
			writestream(cout, with_header);
		}


		void write(const char *outfilename) {
			ofstream outfile;

			outfile.open(outfilename);
			writestream(outfile, true);
			outfile.close();
		}



		void setSelectedParts(vector<string> parts)
		{
			write_parts = parts;
		}
		void setSelectedStaves(vector<string> staves)
		{
			for (vector<string>::iterator v = staves.begin(); v != staves.end(); v++) cout << "Not converting staff : "<<*v<< endl;
			write_staves = staves;
		}

		void setSelectedVoices(vector<string> voices)
		{
			write_voices = voices;
			//for (vector<string>::iterator v = voices.begin(); v != voices.end(); v++) cout << "$$$$$$$$$$$$$ voices: "<<*v<< endl; antescofo_abort();
		}

		void setSelectedMeasures(vector<int> measures)
		{
			write_measures = measures;
		}




	protected:

		// final processing :
		// - compress for each note, if the following note is the same pitch, and if they belong to the same measure
		//   add durations and suppress next
		// - compress tied notes
		void final_compress()
		{
			// - compress
			if (v_Notes.size() < 2) return;
			map<float, measure_elt>::iterator next = v_Notes.begin();
			map<float, measure_elt>::iterator i = next;
			next++;
			for (; next != v_Notes.end(); next++) {
				if (next->second.nMeasure == i->second.nMeasure && next->second.type == i->second.type) {
					if ((/*next->second.type == ANTESCOFO_NOTE ||*/ next->second.type == ANTESCOFO_REST)
							&& next->second.pitches.size() && i->second.pitches.size() && next->second.pitches[0] == i->second.pitches[0]) {
						merge_notes(i, next);
					} else if ((next->second.type == ANTESCOFO_TRILL || next->second.type == ANTESCOFO_MULTI || next->second.type == ANTESCOFO_CHORD)
							&& i->second.pitches == next->second.pitches) {
						merge_notes(i, next);
					}
				}
				i = next;
			}

			// - compress tied notes
			if (v_Notes.size() < 2) return;
			i = next = v_Notes.begin();
			next++;
			for (; next != v_Notes.end(); next++) {
				if (next->second.type == i->second.type && next->second.pitches == i->second.pitches
						&& i->second.flags == ANTESCOFO_FLAG_TIED_START && next->second.flags == ANTESCOFO_FLAG_TIED_END) {
					cout << "Got tied notes (pos:"<<i->first<<")... merging note." <<endl;
					merge_notes(i, next);
				}
				i = next;
			}

		}


		// merge 2 notes a and b (add durations) in a, and delete b
		void merge_notes(map<float, measure_elt>::iterator a, map<float, measure_elt>::iterator b) {
			cout << "merge_notes " << a->first << " and " << b->first << endl;
			if (a->second.bpm.size()) {
				if (b->second.bpm.size())
					return; // do nothing if two note are separated by different tempos
			} else if (b->second.bpm.size())
				a->second.bpm = b->second.bpm;
			if (a->second.nMeasure != b->second.nMeasure) { //tied note between different measures
				//cout << "-------- jumping next measure: "<< a->second.nMeasure << "  ----------" << endl; 
				//a->second.nMeasure++;
			}
			a->second.duration += b->second.duration;
			v_Notes.erase(b);
		}



		void antescofo_abort() {
                        cout << "Antescofo abort: -------------- An error occured, leaving. ----------------" << endl;
			print(false);
			abort();
		}

		void print_duration(ostream &out, rational &du) {
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

		void writenote(ostream &out, int pitch) {
			if (!print_notes_names) {// || pitch % 100 != 0) {
				out << pitch << "00";
			} else {
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
				out << c << o;
			}
		}

		void writestream(ostream &out, bool with_header) {

			//expand();
			//std::sort(v_Notes.begin(), v_Notes.end());
			//cout << "notes sz:" << v_Notes.size() << endl;
			bool bNewMeasure = false;
			bool bPendingFermata = false;
			if (with_header) {
				final_compress();
				out << "; Antescofo score generated using libmusicxml and embedded xml2antescofo converter." <<std::endl;
				out << "; Copyright (c) Thomas Coffy - IRCAM 2012" <<std::endl;
			}
			if (v_Notes.size()) fBPM = v_Notes.begin()->second.bpm;
			if (!fBPM.size()) fBPM = string("120");
			fLastBPM = fBPM;
			out << "BPM " << fBPM << endl;
			int cur_meas = 0;
			float fCurrBeat = 1;

			int last_glissando_pitch = 0;
			for (map<float, measure_elt>::iterator i = v_Notes.begin(); i != v_Notes.end(); i++) { 
				if (i->second.nMeasure == 0 && i->second.m_pos == 0 && i->second.duration == rational(0))
					continue;
				// display measure comment
				if (cur_meas != i->second.nMeasure) {
					bNewMeasure = true;
					cur_meas = i->second.nMeasure;

					// check we want to write this measure
					bool bad = write_measures.empty() ? false : true;
					for (vector<int>::const_iterator v = write_measures.begin(); v != write_measures.end(); v++) {
						if (*v == cur_meas)
							bad = false;
					}
					if (bad) continue;

					out << endl << "; ----------- measure " << cur_meas << " --- beat " << i->first << " ------" << endl;
				}
				if (i->second.bFermata) {
					out << "TEMPO OFF" << endl;
					bPendingFermata = true;
				} else if (bPendingFermata) {
					out << "TEMPO ON" << endl;
					bPendingFermata = false;
				}
				if (i->second.bpm.size() && i->second.bpm != fBPM) {
					fBPM = i->second.bpm ;
					out << "BPM " << fBPM << " @modulate" << endl; 
				}
				if (i->second.duration.toFloat()) i->second.duration.rationalise();

				// grace notes
				int grace_notes = i->second.grace_pitches.size();
				if (grace_notes) {
					if (grace_notes == 1) { out << "NOTE "; writenote(out, i->second.grace_pitches[0]); out << " 0"; }
					else if (grace_notes > 1) {
						out << "CHORD ( ";
						for (vector<int>::const_iterator j = i->second.grace_pitches.begin(); j != i->second.grace_pitches.end(); j++)
							writenote(out, *j);//out << *j << "00 ";
						out << ") 0";// << endl;
					}
					if (i->second.pitches.size()) out << endl; // don't print new measure label on grace notes
				} 
				if (i->second.type == ANTESCOFO_NOTE && i->second.pitches.size() && i->second.pitches[0]) {
					out << "NOTE "; writenote(out, i->second.pitches[0]); out << " ";
					print_duration(out, i->second.duration);
				} else if (i->second.type == ANTESCOFO_REST) {
					out << "NOTE 0 ";
					print_duration(out, i->second.duration);
				}
				else if (i->second.pitches.size() && i->second.pitches[0]) {
					// sort
					std::sort(i->second.pitches.begin(), i->second.pitches.end());
					// remove duplicates pitches
					vector<int>::iterator u = std::unique(i->second.pitches.begin(), i->second.pitches.end());
					i->second.pitches.resize(u - i->second.pitches.begin());

					if (i->second.type != ANTESCOFO_MULTI) {
						if (i->second.type == ANTESCOFO_CHORD) {
							if (i->second.pitches.size() == 1) 
								out << "NOTE";
							else 
								out << "CHORD (";
						} else if (i->second.type == ANTESCOFO_TRILL)
							out << "TRILL (";
						last_glissando_pitch = 0;
					} else { // glissando flags may be: start, stop, stop
						if (last_glissando_pitch && i->second.flags == ANTESCOFO_FLAG_GLISSANDO_STOP) {
							out << "MULTI ( "; writenote(out, last_glissando_pitch);
							out << " ->";
						}
						if (i->second.flags == ANTESCOFO_FLAG_GLISSANDO_START || i->second.flags == ANTESCOFO_FLAG_GLISSANDO_STOP)
							last_glissando_pitch = i->second.pitches[0];
					}
					for (vector<int>::const_iterator j = i->second.pitches.begin(); j != i->second.pitches.end(); j++) {
						if (i->second.flags != ANTESCOFO_FLAG_GLISSANDO_START) {
							out << " "; writenote(out, *j);// << "00";
						}
						//if (i->second.type == ANTESCOFO_MULTI && i->second.flags == ANTESCOFO_FLAG_GLISSANDO_START) out << " ->";
					}

					if (i->second.flags != ANTESCOFO_FLAG_GLISSANDO_START) {
						if (i->second.pitches.size() > 1 || i->second.type == ANTESCOFO_TRILL || i->second.flags == ANTESCOFO_FLAG_GLISSANDO_STOP) 
							out << " )";
						out << " ";
						print_duration(out, i->second.duration);// << i->second.duration.getNumerator() << "/" << i->second.duration.getDenominator();
					}
				}
				if (bNewMeasure && i->second.flags != ANTESCOFO_FLAG_GLISSANDO_START) {
					out << " measure"<< cur_meas;;
					bNewMeasure = false;
				}
				if (i->second.rehearsal.size()) 
					out << " ; marker: " << i->second.rehearsal;

				if (i->second.flags != ANTESCOFO_FLAG_GLISSANDO_START) {
					out << endl;
				}
			}
		}
}; // class antescofowriter
} // namespace MusicXML2 



#endif
