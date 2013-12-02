#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
using namespace std;

#include "ofxMidiparser.h"

///////////////////////////////////
//// MIDI conversion
//////////////////////////////////

NSString* convertMidiFileToNotes(string& midifile) {
	NSString *midifileNS = [NSString stringWithCString:midifile.c_str() encoding:[NSString defaultCStringEncoding]];
	NSData* midicontent = [NSData dataWithContentsOfFile:midifileNS ];
	ofxMidiParser* midiParser = [[ofxMidiParser alloc] init];
	
	int num = -1;
	return [midiParser getAntescofoNotesForTrack:midicontent trackWanted:num];
}

NSString* convertMidiFileToActions(string& midifile) {
	NSString *midifileNS = [NSString stringWithCString:midifile.c_str() encoding:[NSString defaultCStringEncoding]];
	NSData* midicontent = [NSData dataWithContentsOfFile:midifileNS ];
	ofxMidiParser* midiParser = [[ofxMidiParser alloc] init];
	
	int num = -1;
	return [midiParser getAntescofoActionsForTrack:midicontent trackWanted:num];
}



int main(int argc, char* argv[])
{
	bool do_actions = false, do_notes = false;
	int c = 0;

	while ((c = getopt (argc, argv, "an:")) != -1) {
		switch (c)
		{
			case 'a':
				do_actions = true;
				break;
			case 'n':
				do_notes = true;
			default:
				break;
		}
	}
	c = argc - 1;

	if (argv[c] && (do_actions || do_notes)) {
		string outstr;
		string filename = string(argv[c]);
		if (do_actions && !do_notes) {
			cout << "Trying to convert a MIDI file to Antescofo actions: " << filename << endl;
			outstr = [convertMidiFileToActions(filename) UTF8String];
		} else if (do_notes && !do_actions) {
			cout << "Trying to convert a MIDI file to Antescofo notes: " << filename << endl;
			outstr = [convertMidiFileToNotes(filename) UTF8String];
		}
		cerr << "Converted score is:" << endl << outstr << endl;

		ofstream outfile;
		string outfilename = filename + ".asco.txt";

		outfile.open(outfilename.c_str());
		outfile << outstr;
		outfile.close();
		return 0;
	} else {
		cerr << endl << "Usage:" << endl;
		cerr << argv[0] << " [-a|-n] inputfile.mid" << endl;
		cerr << "\t-n\tconvert input MIDI file to Antescofo notes" << endl;
		cerr << "\t-a\tconvert input MIDI file to Antescofo actions" << endl;
		cerr << "Output filename will be: inputfile.mid.asco.txt" << endl << endl;
	}
	return 1;
}





