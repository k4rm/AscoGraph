#pragma once


#include <GL/glew.h>
#include <ofMain.h>
#import <Cocoa/Cocoa.h>
#import "ScintillaView.h"

#define EDITOR_TAB_HEIGHT	20

@class ofxCodeEditor;
@protocol ScintillaViewMyDelegate <ScintillaNotificationProtocol>
//- (void) myClassDelegateMethod: (ofxCodeEditor *) sender;
- (void)notification: (Scintilla::SCNotification*)notification;
@end

@interface ofxCodeEditor : NSObject <ScintillaViewMyDelegate>{
    NSSplitView*	splitView;
    NSScrollView*	scrollview;
    NSTextView*		textView;
    NSTextStorage*	textStorage;
    NSLayoutManager* 	layoutManager;
    NSTextContainer* 	textContainer;
    NSView*		mGLview;
    NSWindow*		mWindow;
    NSString*		editorContent;
    NSView*		tabsView;
    ScintillaView*	mEditor; // current tab
    vector<ScintillaView*> mEditorsList; // tabs list
    vector<NSString*> mEditorsFilenames;
    const char *normal_keywords, *major_keywords, *procedure_keywords, *system_keywords, *client_keywords, *user_keywords;
    vector<string> action_keywords;
    vector<string> dic_keywords, curr_dic_keywords;
    char *dic_char_list;
    char *curr_dic_char_list;
    id<ScintillaViewMyDelegate> mDelegate;
    bool bMatchCase, bWrapMode;
};

//@property (nonatomic, assign) id <ScintillaViewMyDelegate> delegate;

@property (nonatomic, assign) id<ScintillaViewMyDelegate> delegate;
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
- (void) undo;
- (void) redo;
- (void) setEditable: (bool) ed; 
- (int) getNbLines;
- (int) getLenght;
- (string) getSelection;
- (void) braceMatch;
- (void) autocomplete;
//- (void) showAutocompletion;
- (NSSplitView*) get_splitview;
- (void) pushback_keywords: (const char*)keyw;
- (void) loadFile: (string) filename;
- (void) getEditorContent: (string&) content;
- (void) searchText: (string) str backwards:(bool)pBackWards;
- (int) searchNreplaceText:(string)str str2:(string)str2 doAll:(bool)pDoAll;
- (void) searchFinish;
- (void) setWrapMode:(bool)mode;
- (void) setMatchCase:(bool)mode;
- (void) replaceString:(int)linea lineb:(int)lineb str:(const char *)str;
- (void) replaceString:(int)linea lineb:(int)lineb cola:(int)cola colb:(int)colb str:(const char*)str;
- (void) insertStringAtPos:(int)posa posb:(int)posb str:(const char*)str;
- (void) set_normal_keywords: (const char*)normal_keywords_;
- (void) set_major_keywords: (const char*)major_keywords_;
- (void) set_procedure_keywords: (const char*)procedure_keywords_;
- (void) set_system_keywords: (const char*)system_keywords_;
- (void) set_action_keywords: (vector<string>&)system_keywords_;
- (void) setAutoCompleteOn;
- (int) tabsSize;
- (int) tabsHeight;
//- (void) notify(intptr_t windowid, unsigned int iMessage, uintptr_t wParam, uintptr_t lParam);
//- (void)notification: (Scintilla::SCNotification*)notification;

@end

@interface SplitViewDelegate : NSObject <NSSplitViewDelegate, ScintillaViewMyDelegate>
{
}
- (void)notification: (Scintilla::SCNotification*)notification;
@end
