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
#   define ANTESCOFO_FLAG_FERMATA	    5
#   define ANTESCOFO_FLAG_REPEAT_BACKWARD   6
#   define ANTESCOFO_FLAG_REPEAT_FORWARD    7
#   define ANTESCOFO_FLAG_TREMOLO_START	    8
#   define ANTESCOFO_FLAG_TREMOLO_STOP	    9


class measure_elt {
	public:
		measure_elt() : bFermata(false), bpm(""), nMeasure(0), m_pos(rational(0)), type(0) { pitches.reserve(12); }
		int 			type;
		rational 		duration;
		vector<int>		pitches;
		vector<int>		tiednotes;
		vector<int>		grace_pitches;
		int			nMeasure; // measure number
		rational		m_pos; // position in part, in beats unit
		int                 	flags; // handles tied notes..
		string			bpm;
		string  		rehearsal;
		bool			bFermata;
		string			jump_dests;

		bool operator<(const measure_elt& rhs) const { return (m_pos < rhs.m_pos); }
		bool operator==(const measure_elt& rhs) { return (m_pos == rhs.m_pos); }
		bool operator==(rational pos) { return (m_pos == pos); }
};


class antescofowriter {
	public:
		enum pedalType { kDamperPedal, kSoftpedal, kSostenutoPedal };

		antescofowriter() : fLastBPM("0"), fBPM("120"), nBeats(4), print_notes_names(true) { }
		~antescofowriter() {}

		map<rational, measure_elt> v_Notes;
                map<int, rational> measure2beat;
		int nBeats, nBeat_type; // nBeats is the number of beats per measure
		string fBPM, fLastBPM;
		vector<string> write_voices;
		vector<string> write_staves;
		vector<string> write_parts;
		vector<int> write_measures;
		bool print_notes_names;
		void setBPM(string _bpm) { fBPM = _bpm; }

		map<rational, measure_elt>::iterator findNoteInVector(rational atbeat, rational dur);

		// search for beat&measure in measure2beat map, 
		// if does not exist
		//   //if prevmeasure has beat
		//   add
		// else return real beat.
		rational AddBeat(rational beat, int nmeasure);
			         
		// curBeat is relative to the measure
		// so if nmeasure>1 we suppose notes were added before,
		// and the curBeat in absolute beats can be found.
		void AddNote(int type, float pitch, rational dur, float nmeasure, rational &curBeat, int flag_ = ANTESCOFO_FLAG_NULL, string rehearsal = "");

		void print(bool with_header=true);
		void write(const char *outfilename);

		void setSelectedParts(vector<string> parts);
		void setSelectedStaves(vector<string> staves);
		void setSelectedVoices(vector<string> voices);
		void setSelectedMeasures(vector<int> measures);


	protected:

		// final processing :
		// - compress for each note, if the following note is the same pitch, and if they belong to the same measure
		//   add durations and suppress next
		// - compress tied notes
		void final_compress();

		// merge 2 notes a and b (add durations) in a, and delete b
		void merge_notes(map<rational, measure_elt>::iterator a, map<rational, measure_elt>::iterator b);

		void antescofo_abort();
		void print_duration(ostream &out, rational &du);
		void writenote(ostream &out, int pitch, measure_elt& e);
					
		void writestream(ostream &out, bool with_header);

}; // class antescofowriter
} // namespace MusicXML2 



#endif
