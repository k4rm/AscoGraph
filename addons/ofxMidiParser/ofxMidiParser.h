//  Created by Thomas Coffy on 6/19/13.

#import <Foundation/Foundation.h>

typedef enum tagMidiTimeFormat
{
    MidiTimeFormatTicksPerBeat,
    MidiTimeFormatFramesPerSecond
} MidiTimeFormat;


class Notes {
	public:
		Notes(vector<int>& pitchList_, float dur_, float bpm, float timestamp) 
			: pitchList(pitchList_), dur(dur_), tempo(bpm), offset(timestamp) {}
		Notes(vector<int>& pitchList_, vector<int>& linkedPitchList_, float dur_, float bpm, float timestamp)
			: pitchList(pitchList_), linkedPitchList(linkedPitchList_), dur(dur_), tempo(bpm), offset(timestamp) {}
		vector<int> pitchList;
		vector<int> linkedPitchList;
		float dur;
		float tempo, tempo32;
		float vel;
		float offset;
};

@interface ofxMidiParser : NSObject
{
    NSMutableString *log;
    NSData *data;
    NSUInteger offset;
    
    UInt16 format;
    UInt16 trackCount;
    MidiTimeFormat timeFormat;
    
    UInt16 ticksPerBeat;
    UInt16 framesPerSecond;
    UInt16 ticksPerFrame;
    UInt32 bpm;
    UInt8 currentChannel;
}

@property (nonatomic, retain) NSMutableString *log;

@property (readonly) UInt16 format;
@property (readonly) UInt16 trackCount;
@property (readonly) MidiTimeFormat timeFormat;

- (BOOL) parseData: (NSData *) midiData;
- (NSString*) getAntescofoActionsForTrack:(NSData*)midiData trackWanted:(int)trackWanted;
- (NSString*) getAntescofoNotesForTrack:(NSData*)midiData trackWanted:(int)trackWanted;
- (void) print_notes: (vector<Notes*>&)antescofo_notes;

@end
