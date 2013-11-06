//  Created by Thomas Coffy on 6/19/13.

#import <Foundation/Foundation.h>

typedef enum tagMidiTimeFormat
{
    MidiTimeFormatTicksPerBeat,
    MidiTimeFormatFramesPerSecond
} MidiTimeFormat;

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

@end


