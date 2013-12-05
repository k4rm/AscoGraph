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
    const char *normal_keywords, *major_keywords, *procedure_keywords, *system_keywords, *client_keywords, *user_keywords;
};


- (void) setup: (NSWindow*) window glview: (NSView*) glview rect: (ofRectangle&) rect;
- (void) die;
- (void) setupEditor;
- (void) showLine: (int) linea lineb: (int) lineb;
- (void) showLine: (int) linea lineb:(int)lineb cola:(int)cola colb:(int)colb;
- (void) setCurrentPos:(int) pos;
- (int) getCurrentLine;
- (int) getCurrentPos;
- (int) getMaxLines;
- (void) scrollLine: (int) line;
- (void) gotoPos:(int) pos;
- (int) modified;
- (void) clear;
- (int) getNbLines;
- (int) getLenght;
- (string) getSelection;
- (void) braceMatch;
//- (void) showAutocompletion;
- (void) loadFile: (string) filename;
- (void) getEditorContent: (string&) content;
- (void) searchText:(string) str;
- (int) searchNreplaceText:(string)str str2:(string)str2;
- (void) searchNext: (string) str;
- (void) setWrapMode:(bool)mode;
- (void) replaceString:(int)linea lineb:(int)lineb str:(const char *)str;
- (void) replaceString:(int)linea lineb:(int)lineb cola:(int)cola colb:(int)colb str:(const char*)str;
- (void) insertStringAtPos:(int)posa posb:(int)posb str:(const char*)str;
- (void) set_normal_keywords: (const char*)normal_keywords_;
- (void) set_major_keywords: (const char*)major_keywords_;
- (void) set_procedure_keywords: (const char*)procedure_keywords_;
- (void) set_system_keywords: (const char*)system_keywords_;

@end

@interface SplitViewDelegate : NSObject <NSSplitViewDelegate>
{
}
@end
