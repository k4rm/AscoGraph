#pragma once


#include <GL/glew.h>
#include <ofMain.h>
#import <Cocoa/Cocoa.h>
#import "ScintillaView.h"

@interface ofxCodeEditor : NSObject {
    NSSplitView*	splitView;
    NSScrollView*	scrollview;
    NSTextView*		textView;
    NSTextStorage*	textStorage;
    NSLayoutManager* 	layoutManager;
    NSTextContainer* 	textContainer;
    NSView*		mGLview;
    NSWindow*		mWindow;
    ScintillaView*	mEditor;
		const char **normal_keywords, **major_keywords;
};

//- (void) setColorProperty: (int) property parameter: (long) parameter fromHTML: (NSString*) fromHTML;


- (void) setup: (NSWindow*) window glview: (NSView*) glview rect: (ofRectangle&) rect;
- (void) die;
- (void) setupEditor;
- (void) showLine: (int) linea lineb: (int) lineb;
- (void) scrollLine: (int) line;
- (int) getNbLines;
- (int) getLenght;
- (void) showAutocompletion;
- (void) loadFile: (string) filename;
- (void) getEditorContent: (string&) content;
//- (void) crossFadeWithOld(NSWindow* window, NSView *oldView, NSView* newView);
- (void) searchText:(string) str;
- (void) set_normal_keywords: (const char*[])normal_keywords_;
- (void) set_major_keywords: (const char*[])major_keywords_;


@end

@interface SplitViewDelegate : NSObject <NSSplitViewDelegate>
{
}
@end
/*
@property (retain) NSTextStorage *textStorage;
@property (retain) NSLayoutManager *layoutManager;
@property (retain) NSTextContainer *textContainer;



- (void) setup;
- (void) die;*/

