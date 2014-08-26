#include <sstream>
#include <string>
#include <sys/time.h>

#include "ofxHotKeys.h"
#include "ofxAntescofog.h"
#include "ofxCocoaWindow.h"
#include "ofxTLBeatTicker.h"
#include "ofxMidiparser.h"
#include "Function.h"
#include "Environment.h"
#include "AntescofoCore.h"
#include <AppKit/NSCursor.h>

bool enable_simulate = true;
bool disable_httpd = true;

bool enable_new_window = false;

int _debug = 0;
static string str_error; // filled by our error()

int fontsize = OFX_UI_FONT_SMALL;

extern ofxConsole* console;

#define CONSTANT_EDITOR_VIEW_WIDTH	400



#ifdef TARGET_OSX
#include "ofxCodeEditor.h"
ofxAntescofog::ofxAntescofog(int argc, char* argv[], ofxCocoaWindow* window) {    
	mouseX = mouseY = 0;
	cocoaWindow = window;
#else
ofxAntescofog::ofxAntescofog(int argc, char* argv[]) {
#endif
	console = new ofxConsole(4, 500, 800, 300, 10);
	bShowColorSetup = false;
	bShowOSCSetup = false;
	bSnapToGrid = true;
	mHasReadMessages = false;
	fAntescofoTimeSeconds = 0;
	bAutoScroll = true;
	bColorSetupInitDone = false;
	bFindTextInitDone = false;
	bOSCSetupInitDone = bLockAscoGraph = false;
	bShowError = false;
	bShowFind = false;
	bErrorInitDone = false;
	bEditorShow = true;
	bFullTextEditorShow = false;
	bSetupDone = false;
	editor = 0;
	bShouldRedraw = true;
	bLineWrapMode = true;
	bIsSimulating = false;
	bShowActions = true;
	bMouseCursorSet = bScoreFromCommandLine = false;
	ofxJumpTrack = 0;
	audioTrack = NULL;
	ofxAntescofoSim = 0;
	mFindWindow = NULL;
	subWindow = NULL;
	mOsc_beat = -1;
	mGotoPos = 0.;

	if (argc > 1 && argv[1][0] != '-') {
		bScoreFromCommandLine = true;
		mScore_filename = argv[1];
	}

	gettimeofday(&last_draw_time, 0);
	guiBottom = 0;
}

@implementation NSFindWindow 
@synthesize fog;
-(void)keyDown:(NSEvent *)theEvent 
{
	switch([theEvent keyCode]) {
		case 53: // esc
			NSLog(@"NSFindWindow: ESC pressed: closing Find window.");
			[self close];
			break;
		default:
			;
	}
}
- (void) windowWillClose:(NSNotification *)notification
{
	NSWindow *windowAboutToClose = [notification object];
	if (fog && fog->mFindWindow == windowAboutToClose) {
		NSLog(@"NSFindWindow: about to close");
		fog->mFindWindow = NULL;
	}
}
/* simple trampolines for signaling click events to the non-ObjC object ofxAntescofog */
-(void)findNextText_pressed
{
	if (fog) fog->findNextText_pressed();
}
-(void)findPrevText_pressed
{
	if (fog) fog->findPrevText_pressed();
}
-(void)replaceText_pressed
{
	if (fog) fog->replaceText_pressed();
}
-(void)replaceAllText_pressed
{
	if (fog) fog->replaceAllText_pressed();
}
@end

@implementation NSFindTextField
-(void)keyDown:(NSEvent *)theEvent 
{
	NSLog(@"NSFindTextField: pressed: %d", [theEvent keyCode]);
	
	switch([theEvent keyCode]) {
		case 53: // esc
			NSLog(@"NSFindTextField: ESC pressed: closing Find window.");
			//[self close];
			[super keyDown:theEvent];
			break;
		default:
			;
	}
}
@end

void ofxAntescofog::findPrevText_pressed() {
	NSString *nsstr = [mFindTextField stringValue]; 
	string str([nsstr UTF8String]);

	if (str.size()) {
		cout << "findText_pressed: going to find string: [ " << str << " ]" << endl;
		[editor setMatchCase:[mBtnFindMatchCase state]];
		[editor setWrapMode:[mBtnFindWrapMode state]];
		[ editor searchText:str backwards:YES ];
	}
}

void ofxAntescofog::findNextText_pressed() {
	NSString *nsstr = [mFindTextField stringValue]; 
	string str([nsstr UTF8String]);

	if (str.size()) {
		cout << "findText_pressed: going to find string: [ " << str << " ]" << endl;
		[editor setMatchCase:[mBtnFindMatchCase state]];
		[editor setWrapMode:[mBtnFindWrapMode state]];
		[ editor searchText:str backwards:NO];
		[ editor searchFinish ];
	}
}

void ofxAntescofog::replaceText_pressed() {
	NSString *nsstr = [mFindTextField stringValue]; 
	string str([nsstr UTF8String]);

	NSString *nsstr2 = [mReplaceTextField stringValue]; 
	string str2([nsstr2 UTF8String]);

	if (str.size()) {
		cout << "replaceText_pressed: going to replace string: [ " << str << " ] with [ " << str2 << " ]" << endl;
		[editor setMatchCase:[mBtnFindMatchCase state]];
		[editor setWrapMode:[mBtnFindWrapMode state]];
		[ editor searchNreplaceText:str str2:str2 doAll:NO ];
	}
}

void ofxAntescofog::replaceAllText_pressed() {
	NSString *nsstr = [mFindTextField stringValue]; 
	string str([nsstr UTF8String]);

	NSString *nsstr2 = [mReplaceTextField stringValue]; 
	string str2([nsstr2 UTF8String]);

	if (str.size()) {
		cout << "replaceText_pressed: going to replace string: [ " << str << " ] with [ " << str2 << " ]" << endl;
		[editor setMatchCase:[mBtnFindMatchCase state]];
		[editor setWrapMode:[mBtnFindWrapMode state]];
		[ editor searchNreplaceText:str str2:str2 doAll:YES ];
	}
}

void ofxAntescofog::create_Find_and_Replace_window() {
	bShowColorSetup = false;
	bShowOSCSetup = false;
	bShowFind = true;
	//if (mFindWindow) [mFindWindow release];
	int mFindWindow_w = 550, mFindWindow_h = 180;
	// window
	mFindWindow = [[NSFindWindow alloc] initWithContentRect:NSMakeRect(0, 0, mFindWindow_w, mFindWindow_h)
		styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];
	//autorelease];
	[mFindWindow setFog:this];
	[mFindWindow setOpaque:NO];
	[mFindWindow cascadeTopLeftFromPoint:NSMakePoint(320, 710)];
	[mFindWindow setTitle:@"Find / Replace"];
	[mFindWindow makeKeyAndOrderFront:[cocoaWindow->delegate getNSWindow]];
	[mFindWindow setBackgroundColor:[NSColor colorWithDeviceRed:0.3 green:0.3 blue:0.3 alpha:0.52]];
	[mFindWindow setHasShadow:YES];
	[mFindWindow useOptimizedDrawing:YES];
	[mFindWindow setMovableByWindowBackground:YES];

	[[NSNotificationCenter defaultCenter] addObserver:mFindWindow
		selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:mFindWindow];
	// label Find:
	NSTextField *textField;
	textField = [[NSTextField alloc] initWithFrame:NSMakeRect(25, mFindWindow_h-50, 40, 17)];
	[textField setStringValue:@"Find:"];
	[textField setBezeled:NO];
	[textField setDrawsBackground:NO];
	[textField setEditable:NO];
	[textField setSelectable:NO];
	[textField setTextColor:[NSColor whiteColor]];
	[[mFindWindow contentView] addSubview:textField];
	[textField release];

	// Textbox find
	NSFindTextField *tf = [[NSFindTextField alloc] initWithFrame: NSMakeRect(66, mFindWindow_h-50, 270, 20)];
	//tf.delegate = (id)mFindWindow;
	[tf setEditable:YES];
	[tf setSelectable:YES];
	[tf setEnabled:YES];
	[tf setBackgroundColor:[NSColor colorWithDeviceRed:0.3 green:0.3 blue:0.3 alpha:0.7]];
	[tf setTextColor:[NSColor whiteColor]];
	[[mFindWindow contentView] addSubview: tf];
	//[mFindWindow setContentView:tf];
	//[mFindWindow makeKeyAndOrderFront:nil];
	[mFindWindow makeFirstResponder:tf];
	mFindTextField = tf;
	[tf release];

	// Find Prev Button
	NSButton *btn = [[NSButton alloc] initWithFrame:NSMakeRect(340, mFindWindow_h-50-5, 100,24)];
	[[btn cell] setControlSize:NSRegularControlSize];
	[btn setButtonType:NSMomentaryPushInButton];
	[btn setBordered:YES];
	[btn setBezelStyle:NSRoundedBezelStyle];
	[btn setImagePosition:NSNoImage];
	[btn setAlignment:NSCenterTextAlignment];
	[[btn cell] setControlTint:NSBlueControlTint];
	[btn setEnabled:YES];
	[btn setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[btn setTitle:@"Find Prev"];
	[btn setTarget:mFindWindow];
	[btn setAction:@selector(findPrevText_pressed)];
	[[mFindWindow contentView] addSubview: btn];
	[btn release];

	// Find Next Button
	btn = [[NSButton alloc] initWithFrame:NSMakeRect(440, mFindWindow_h-50-5, 100,24)];
	[[btn cell] setControlSize:NSRegularControlSize];
	[btn setButtonType:NSMomentaryPushInButton];
	[btn setBordered:YES];
	[btn setBezelStyle:NSRoundedBezelStyle];
	[btn setImagePosition:NSNoImage];
	[btn setAlignment:NSCenterTextAlignment];
	[[btn cell] setControlTint:NSBlueControlTint];
	[btn setEnabled:YES];
	[btn setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[btn setTitle:@"Find Next"];
	[btn setTarget:mFindWindow];
	[btn setAction:@selector(findNextText_pressed)];
	[[mFindWindow contentView] addSubview: btn];
	[btn release];

	// label Replace:
	textField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, mFindWindow_h-50-50, 60, 17)];
	[textField setStringValue:@"Replace:"];
	[textField setBezeled:NO];
	[textField setDrawsBackground:NO];
	[textField setEditable:NO];
	[textField setSelectable:NO];
	[textField setTextColor:[NSColor whiteColor]];
	[[mFindWindow contentView] addSubview:textField];
	[textField release];

	// Textbox replace
	tf = [[NSFindTextField alloc] initWithFrame: NSMakeRect(66, mFindWindow_h-50-50, 270, 20)];
	//tf.delegate = (id)mFindWindow;
	[tf setEditable:YES];
	[tf setSelectable:YES];
	[tf setEnabled:YES];
	[tf setBackgroundColor:[NSColor colorWithDeviceRed:0.3 green:0.3 blue:0.3 alpha:0.7]];
	[tf setTextColor:[NSColor whiteColor]];
	[[mFindWindow contentView] addSubview: tf];
	mReplaceTextField = tf;
	[tf release];

	// Replace Button
	btn = [[NSButton alloc] initWithFrame:NSMakeRect(340, mFindWindow_h-50-5-50, 100,24)];
	[[btn cell] setControlSize:NSRegularControlSize];
	[btn setButtonType:NSMomentaryPushInButton];
	[btn setBordered:YES];
	[btn setBezelStyle:NSRoundedBezelStyle];
	[btn setImagePosition:NSNoImage];
	[btn setAlignment:NSCenterTextAlignment];
	[[btn cell] setControlTint:NSBlueControlTint];
	[btn setEnabled:YES];
	[btn setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[btn setTitle:@"Replace"];
	[btn setTarget:mFindWindow];
	[btn setAction:@selector(replaceText_pressed)];
	[[mFindWindow contentView] addSubview: btn];
	[btn release];

	// Replace All Button
	btn = [[NSButton alloc] initWithFrame:NSMakeRect(440, mFindWindow_h-50-5-50, 100,24)];
	[[btn cell] setControlSize:NSRegularControlSize];
	[btn setButtonType:NSMomentaryPushInButton];
	[btn setBordered:YES];
	[btn setBezelStyle:NSRoundedBezelStyle];
	[btn setImagePosition:NSNoImage];
	[btn setAlignment:NSCenterTextAlignment];
	[[btn cell] setControlTint:NSBlueControlTint];
	[btn setEnabled:YES];
	[btn setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[btn setTitle:@"Replace All"];
	[btn setTarget:mFindWindow];
	[btn setAction:@selector(replaceAllText_pressed)];
	[[mFindWindow contentView] addSubview: btn];
	[btn release];

	// match case button
	mBtnFindMatchCase = [[NSButton alloc] initWithFrame:NSMakeRect (80, mFindWindow_h-50-5-50-50, 140, 25)];
	[mBtnFindMatchCase setButtonType:NSSwitchButton];
	[mBtnFindMatchCase setBezelStyle:0];
	[mBtnFindMatchCase setTitle:@"Case sensitive"];
	[mBtnFindMatchCase setState:NSOffState];
	[[mFindWindow contentView] addSubview:mBtnFindMatchCase];
	[mBtnFindMatchCase release];
	// wrap mode button
	mBtnFindWrapMode = [[NSButton alloc] initWithFrame:NSMakeRect (300, mFindWindow_h-50-5-50-50, 150, 25)];
	[mBtnFindWrapMode setButtonType:NSSwitchButton];
	[mBtnFindWrapMode setBezelStyle:0];
	[mBtnFindWrapMode setTitle:@"Wrap around"];
	[mBtnFindWrapMode setState:NSOnState];
	[[mFindWindow contentView] addSubview:mBtnFindWrapMode];
	[mBtnFindWrapMode release];


	[[cocoaWindow->delegate getNSWindow] addChildWindow:mFindWindow ordered:NSWindowAbove];
}


void ofxAntescofog::menu_item_hit(int n)
{
	//cout << "menu = " << n << endl;
	switch (n) {
		case INT_CONSTANT_BUTTON_UNDO:
			[editor undo];
			break;
		case INT_CONSTANT_BUTTON_REDO:
			[editor redo];
			break;
		case INT_CONSTANT_BUTTON_LOAD:
			{
				ofFileDialogResult openFileResult = ofSystemLoadDialog(TEXT_CONSTANT_TITLE_LOAD_SCORE);
				if (openFileResult.bSuccess){
					string f = openFileResult.filePath;
					ofLogVerbose("Selected file: " + f);
					loadScore(f, true);
				} else {
					ofLogVerbose("Cancel load score hit.");
				}
				break;
			}
		case INT_CONSTANT_BUTTON_RELOAD:
			if (askToSaveScore() != INT_CONSTANT_BUTTON_CANCEL_SAVE) {
				ofxAntescofoNote->clear();
				loadScore(mScore_filename, false);
			}
			break;
		case INT_CONSTANT_BUTTON_NEW:
			if (askToSaveScore() != INT_CONSTANT_BUTTON_CANCEL_SAVE) {
				ofxAntescofoNote->clear();
				newScore();
			}
			break;

		case INT_CONSTANT_BUTTON_SAVE:
			ofLogVerbose("Save score hit");
			saveScore();
			break;
		case INT_CONSTANT_BUTTON_SAVE_AS:
			ofLogVerbose("Save As score hit");
			saveAsScore();
			break;
		case INT_CONSTANT_BUTTON_COLORSETUP:
			if (!bShowColorSetup && !bShowOSCSetup) {
				timeline.disable();
				bShowColorSetup = true;
				bShowOSCSetup = false;
				bShowFind = false;
				guiBottom->disable();
			} 
			break;
		case INT_CONSTANT_BUTTON_OSCSETUP:
			if (!bShowOSCSetup && !bShowColorSetup) {
				timeline.disable();
				bShowColorSetup = false;
				bShowOSCSetup = true;
				guiBottom->disable();
			}
			break;
		case INT_CONSTANT_BUTTON_HIDE:
			[[NSRunningApplication currentApplication] hide];
			break;
		case INT_CONSTANT_BUTTON_CREATE_GROUP:
		case INT_CONSTANT_BUTTON_CREATE_LOOP:
		case INT_CONSTANT_BUTTON_CREATE_CURVE:
		case INT_CONSTANT_BUTTON_CREATE_CURVES:
		case INT_CONSTANT_BUTTON_CREATE_WHENEVER:
		case INT_CONSTANT_BUTTON_CREATE_OSCSEND:
		case INT_CONSTANT_BUTTON_CREATE_OSCRECV:
			ofLogVerbose("Create Menu hit");
			createCodeTemplate(n);
			break;
#ifdef USE_GUIDO
		case INT_CONSTANT_BUTTON_TOGGLEVIEW:
			ofxAntescofoNote->toggleView();
			ofxAntescofoBeatTicker->disabled = ofxAntescofoNote->mode_guido();
			break;
#endif
		case INT_CONSTANT_BUTTON_TOGGLEEDIT:
			setEditorMode(!bEditorShow, 0);
			break;
		case INT_CONSTANT_BUTTON_TOGGLE_FULL_EDITOR:
			bEditorShow = true;
			bFullTextEditorShow = !bFullTextEditorShow;
			if (bFullTextEditorShow)
				setEditorMode(bEditorShow, 0, bFullTextEditorShow);
			else {
				NSView *right = [[ [ editor get_splitview ]subviews] objectAtIndex:1];
				NSRect rightFrame = [right frame];
				rightFrame.size.width = CONSTANT_EDITOR_VIEW_WIDTH;
				float editor_x = ofGetWidth() - CONSTANT_EDITOR_VIEW_WIDTH;
				rightFrame.origin.x = editor_x;
				[right setFrame:rightFrame];
			}
			break;
		case INT_CONSTANT_BUTTON_SHOWHIDE_ACTION:
			if (ofxAntescofoNote->getActionTrack()) {
				ofxAntescofoNote->deleteActionTrack();
				[mShowhideActiontrackMenuItem setState:NSOffState];
			} else {
				ofxAntescofoNote->createActionTrack();
				[mShowhideActiontrackMenuItem setState:NSOnState];
				loadScore(mScore_filename, false);
			}
			break;
		case INT_CONSTANT_BUTTON_SNAP:
			bSnapToGrid = !bSnapToGrid;
			cout << "Setting SnapToGrid to : " << bSnapToGrid << endl;
			timeline.setShowBPMGrid(bSnapToGrid);
			timeline.enableSnapToBPM(bSnapToGrid);
			[mSnapMenuItem setState:(bSnapToGrid ? NSOnState : NSOffState)];
			break;
		case INT_CONSTANT_BUTTON_AUTOSCROLL:
			bAutoScroll = !bAutoScroll;
			cout << "Setting autoscroll mode:" << bAutoScroll << endl; 
			ofxAntescofoNote->setAutoScroll(bAutoScroll);
			[mAutoscrollMenuItem setState:(bAutoScroll ? NSOnState : NSOffState)];
			break;
		case INT_CONSTANT_BUTTON_ACTIONVIEWDEEPMODE:
		{
			cout << "Setting Action View Deep Mode Level." << endl;
			ofxTLAntescofoAction* actiontrck = ofxAntescofoNote->getActionTrack();
			if (actiontrck)
				actiontrck->bViewActionWithDeepLevels = !actiontrck->bViewActionWithDeepLevels;
			break;
		}
		case INT_CONSTANT_BUTTON_LINEWRAP:
			bLineWrapMode = !bLineWrapMode;
			cout << "Setting line wrapping mode:" << bLineWrapMode << endl; 
			[ editor setWrapMode:bLineWrapMode ];
			[mLineWrapModeMenuItem setState:(bLineWrapMode ? NSOnState : NSOffState)];
			break;
		case INT_CONSTANT_BUTTON_AUTOCOMPLETE:
			cout << "Autocompletion key pressed." << endl;
			[ editor autocomplete];
			break;
		case INT_CONSTANT_BUTTON_LOCK:
			{
				cout << "Locking AscoGraph." << endl;
				bLockAscoGraph = !bLockAscoGraph;
				ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
				if (actiontrack) actiontrack->setEditable(!bLockAscoGraph);
				[editor setEditable:!bLockAscoGraph];
				[mLockMenuItem setState:(bLockAscoGraph ? NSOnState : NSOffState)];
				break;
			}
		case INT_CONSTANT_BUTTON_FIND:
#if 1 // Find/Replace window draft
			cout << "Setting find text mode" << endl; 
			if (mFindWindow == NULL) bShowFind = false;
			if (!bShowFind) {
				create_Find_and_Replace_window();
			}
#else
			if (!bShowFind) {
				timeline.disable();
				bShowColorSetup = false;
				bShowOSCSetup = false;
				bShowFind = true;
				guiBottom->disable();
			}
#endif
			break;
		case INT_CONSTANT_BUTTON_SIMULATE:
			{
				if (enable_simulate) {
					bIsSimulating = true;
					simulate();
				}
				break;
			}
		case INT_CONSTANT_BUTTON_EDIT:
			{
				if (enable_simulate) {
					bIsSimulating = false;
					stop_simulate_and_goedit();
				}
				break;
			}

		case INT_CONSTANT_BUTTON_PLAY:
			{
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				m.addStringArg("play");
				mOSCsender.sendMessage(m);
				break;
			}
		case INT_CONSTANT_BUTTON_START:
			{
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				m.addStringArg("start");
				m.addStringArg("");
				mOSCsender.sendMessage(m);
				break;
			}
		case INT_CONSTANT_BUTTON_STOP:
			{
				mGotoPos = 0.;
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				m.addStringArg("stop");
				mOSCsender.sendMessage(m);
				mPlayLabel.clear();
				break;
			}
		case INT_CONSTANT_BUTTON_PLAYSTRING:
			{
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				string msg = [editor getSelection];
				//cout << "<<<<<<<<<<<<<<<<<<<< sending playstring >>>>>>>>>>>>>>>>>>>>>" << endl;
				//cout << msg << endl;
				msg += "\n";
				if (msg.size() < 4000) { // hard coded max lenght of msg in oscpack
					m.addStringArg("playstring");
					m.addStringArg(msg);
					try {
						mOSCsender.sendMessage(m);
					} catch(exception& e) {
						cerr << "OSC error: " << e.what() << endl;
					}
				} else {
					ofxOscBundle bu;
					int b = 0, sz = 4000;
					bool done = false;
					while (sz <= msg.size()) {
						string sub = msg.substr(b, sz);
						ofxOscMessage m;
						m.setAddress("/antescofo/cmd");
						string cmd;
						if (msg.size() - 4000 <= b)
							cmd = "playstring";
						else cmd = "playstring_append";
						cout << "<<<<<<<<<<<<<<<<<<<< adding " << cmd << " : size=" << sz - b << " sz="<< sz << " b=" << b << endl;
						m.addStringArg(cmd);
						m.addStringArg(sub);
						cout << sub << endl;
						try { mOSCsender.sendMessage(m); } catch(exception& e) { cerr << "OSC error: " << e.what() << endl; }
						if (done) break;
						b = sz;
						if (sz + 4000 > msg.size()) {
							sz = msg.size();
							if (!done) done = true;
						} else sz += 4000;
					}
					//try { mOSCsender.sendBundle(bu); } catch(exception& e) { cerr << "OSC error: " << e.what() << endl; }
				}
				break;
			}
		case INT_CONSTANT_BUTTON_GET_PATCH_RECEIVERS:
			{
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				string msg = [editor getSelection];
				cout << "<<<<<<<<<<<<<<<<<<<< sending get_patch_receivers >>>>>>>>>>>>>>>>>>>>>" << endl;
				cout << msg << endl;
				msg += "\n";
				m.addStringArg("get_patch_receivers");
				m.addStringArg(msg);
				try {
					mOSCsender.sendMessage(m);
				} catch(...)
				{}
				break;

			}
		case INT_CONSTANT_BUTTON_PREVEVENT:
			{
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				m.addStringArg("previousevent");
				mOSCsender.sendMessage(m);
				break;
			}
		case INT_CONSTANT_BUTTON_NEXTEVENT:
			{
				ofxOscMessage m;
				m.setAddress("/antescofo/cmd");
				m.addStringArg("nextevent");
				mOSCsender.sendMessage(m);
				break;
			}
		case INT_CONSTANT_BUTTON_ZOOM_IN:
			{
				((ofxTLZoomer2D*)timeline.getZoomer())->zoomin();
				break;
			}
		case INT_CONSTANT_BUTTON_ZOOM_OUT:
			{
				((ofxTLZoomer2D*)timeline.getZoomer())->zoomout();
				break;
			}
		case INT_CONSTANT_BUTTON_OPEN_ALL_CURVES:
			{
				ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
				if (actiontrack) actiontrack->show_all_curves();
				break;
			}
		case INT_CONSTANT_BUTTON_OPEN_ALL_GROUPS:
			{
				ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
				if (actiontrack) actiontrack->show_all_groups();
				break;
			}
		case INT_CONSTANT_BUTTON_CLOSE_ALL_CURVES:
			{
				ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
				if (actiontrack) actiontrack->hide_all_curves();
				break;
			}
	}
	if (n >= INT_CONSTANT_BUTTON_CUES_INDEX) {
		cout << "Cuepoints Drop Down List hit: " << endl; 
		map<int, string>::iterator li = mCuesIndexToString.find(n - INT_CONSTANT_BUTTON_CUES_INDEX);
		if (li != mCuesIndexToString.end()) {
			mPlayLabel = li->second;
			cout << "Sending OSC: gotolabel " << mPlayLabel << endl; 
			ofxOscMessage m;
			m.setAddress("/antescofo/cmd");
			m.addStringArg("gotolabel");
			m.addStringArg(mPlayLabel);
			mOSCsender.sendMessage(m);
		}
	} else if (n >= INT_CONSTANT_BUTTON_OPENRECENT) {
		int m = n - INT_CONSTANT_BUTTON_OPENRECENT;
		string f = mRecentFiles[mRecentFiles.size() - 1 - m];
		cout << "mRecentFiles size: " << mRecentFiles.size() << " m = " << m << endl ;
		cout << "Open Recent Drop down list hit : " << f << endl;
		loadScore(f, true);
	}

	bShouldRedraw = true;
}

void ofxAntescofog::setMouseCursorGoto(bool bState) {
	string speaker_icon_path(ofFilePath::getCurrentExeDir() + "../Resources/speaker_cursor.png");
	static NSImage* speaker_img = [[NSImage alloc] initWithContentsOfFile: [NSString stringWithUTF8String:speaker_icon_path.c_str()]];
	static NSCursor* cur = [[NSCursor alloc] initWithImage:speaker_img hotSpot:[[NSCursor currentCursor] hotSpot]];

	NSView *v = [cocoaWindow->delegate getNSView];

	if (!ofxAntescofoBeatTicker)
		return;
	if (bState) {
		//if (mCursor) { mCursor }
		//[v addCursorRect:ofxAntescofoBeatTicker->bounds cursor:resizeRightCursor];
		//[aCursor setOnMouseEntered:YES];
		//[[NSCursor resizeRightCursor] set];

		NSRect rect = NSMakeRect( timeline.getTrackHeader(ofxAntescofoBeatTicker)->getBounds().x, timeline.getTrackHeader(ofxAntescofoBeatTicker)->getBounds().y,
					ofxAntescofoBeatTicker->getBounds().width, timeline.getTrackHeader(ofxAntescofoBeatTicker)->getBounds().height + ofxAntescofoBeatTicker->getBounds().height);
		[v resetCursorRects];
		[v addCursorRect:rect cursor:cur];
		[cur set];
	} else {
		[v resetCursorRects];
		[v discardCursorRects];
		[[NSCursor arrowCursor] set];
	}

}

void ofxAntescofog::setGotoPos(float pos) {
	cout << "Storing mGotopos " << pos << endl; 
	mGotoPos = pos;
	/*
	ofxOscMessage m;
	m.setAddress("/antescofo/cmd");
	m.addStringArg("gotobeat");
	m.addFloatArg(pos);
	mOSCsender.sendMessage(m);
	*/
}

static ofxAntescofog *fog;

@implementation ofxCocoaDelegate (ofxAntescofogAdditions)
- (void)gonna_terminate:(id)sender
{
	if (fog) {
		if (fog->askToSaveScore() != INT_CONSTANT_BUTTON_CANCEL_SAVE) {
			cout << "gonna_terminate: Going to leave."<<endl;
			exit(0);
		}
	}
}

- (void)menu_item_hit:(id)sender
{

	NSMenuItem *i = (NSMenuItem*)sender;

	//if (i) cout << "menu item button hit" << [i tag] << endl;
	if (fog) {
		fog->menu_item_hit([i tag]);
	}
}

- (void) receiveNotification:(NSNotification *) notification
{
	if ([[notification name] isEqualToString:@"DoubleClick"]) {
		NSDictionary *userInfo = notification.userInfo;
		int line = [[userInfo objectForKey:@"line"] intValue];
		NSLog(@"DoubleClick on line: %d", line);
		if (fog) {
			fog->editorDoubleclicked(line);
		}
		return;
	} else if ([[notification name] isEqualToString:NSTextDidChangeNotification]) {
		if (fog) {
			fog->editorTextDidChange();
		}
	}
	NSLog(@"Notification: %@", [notification name]);
}
@end

string ofxAntescofog::getApplicationSupportSettingFile() {
	//NSString *path = [[NSFileManager defaultManager] applicationSupportDirectory];
	NSString *executableName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleExecutable"];

	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	if ([paths count] == 0)
	{
		cerr << "ERROR: can not get application directory" << endl;
		return "";
	}
	NSString *resolvedPath = [paths objectAtIndex:0];
	resolvedPath = [resolvedPath stringByAppendingPathComponent:executableName];

	string dir([resolvedPath UTF8String]);
	if (ofDirectory::createDirectory(dir, false, true))
		cout << "getApplicationSupports: directory " << dir << " created." << endl;

	string settingsfile(dir + "/" + TEXT_CONSTANT_LOCAL_SETTINGS);

	cout << "getApplicationSupports: settings file = " << settingsfile << endl;
	return settingsfile;
}

void ofxAntescofog::populateOpenRecentMenu() {
	static bool created = false;
	if (mRecentFiles.size()) {
		//for (int i = 0; i < mRecentFiles.size(); i++) cout << "Recent File : " << mRecentFiles[i] << endl;
		NSMenuItem* openRecent = [mFileMenu itemWithTag:INT_CONSTANT_BUTTON_OPENRECENT];
		NSMenu *mysubmenu;
		if (openRecent) {
			mysubmenu = [openRecent submenu];
			[mysubmenu removeAllItems];
		} else {
			openRecent = [[[NSMenuItem alloc] initWithTitle:@"Open Recent" action:@selector(menu_item_hit:) keyEquivalent:@"" ] autorelease];
			mysubmenu = [[NSMenu alloc] init];
		}
		for (int i = mRecentFiles.size()-1, j = 0; j < 10 && i > 0; i--, j++)
		{
			NSString* name = [NSString stringWithUTF8String: mRecentFiles[i].c_str()];
			if (name) {
				cout << "Recent File : adding to menu: " << [name UTF8String] << endl;

				NSMenuItem* newItem = [[NSMenuItem alloc] initWithTitle:name action:@selector(menu_item_hit:) keyEquivalent:@""];
				[newItem setTag:INT_CONSTANT_BUTTON_OPENRECENT+j];
				[newItem setRepresentedObject:[NSString stringWithFormat:@"%d",j]];
				//[newItem setKeyEquivalentModifierMask: NSCommandKeyMask];
				//[newItem setKeyEquivalent:[NSString stringWithFormat:@"%d", j + 1]];

				//[openRecent addItem:newItem];
				[mysubmenu addItem:newItem];
				[newItem release];
			}
		}
		[openRecent setTag:INT_CONSTANT_BUTTON_OPENRECENT];
		[openRecent setSubmenu:mysubmenu];
		//[fileMenu setSubmenu:openRecent];
		if (!created)
			[mFileMenu addItem:openRecent];
		created = true;
	}
}

void ofxAntescofog::setupUI() {
	id menubar = [[NSMenu new] autorelease];
	id appMenu = [[NSMenu new] autorelease];
	id appName = [[NSProcessInfo processInfo] processName];
	[NSApp setMainMenu:menubar];


	cout << "Creating OSX Menus..." << endl;
	//////////////////
	// Application 
	// . about
	id appMenuItem = [[[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""] autorelease];
	NSString* title = [@"About " stringByAppendingString:appName];
	[appMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

	// . color setup
	id colorMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Colors Setup" action:@selector(menu_item_hit:) keyEquivalent:@","] autorelease];
	[colorMenuItem setTag:INT_CONSTANT_BUTTON_COLORSETUP];
	[appMenu addItem:colorMenuItem];

	// . OSC setup
	id oscMenuItem = [[[NSMenuItem alloc] initWithTitle:@"OSC Setup" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[oscMenuItem setTag:INT_CONSTANT_BUTTON_OSCSETUP];
	[appMenu addItem:oscMenuItem];

	// . hide AscoGraph
	id hideMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Hide Ascograph" action:@selector(menu_item_hit:) keyEquivalent:@"h"] autorelease];
	[hideMenuItem setTag:INT_CONSTANT_BUTTON_HIDE];
	[appMenu addItem:hideMenuItem];

	// . quit
	id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(gonna_terminate:) keyEquivalent:@"q"] autorelease];
	[quitMenuItem setTag:INT_CONSTANT_BUTTON_QUIT];
	[appMenu addItem:quitMenuItem];
	[menubar addItem:appMenuItem];
	[appMenuItem setSubmenu:appMenu];

	//////////////////
	// File
	mFileMenu = [[[NSMenu new] autorelease] initWithTitle:@"File"];
	id fileMenuItem = [[[NSMenuItem alloc] initWithTitle:@"File" action:NULL keyEquivalent:@""] autorelease];
	// . new
	NSString* txt = [@"" stringByAppendingString:@"New score"];
	id newMenuItem = [[[NSMenuItem alloc] initWithTitle:txt action:@selector(menu_item_hit:) keyEquivalent:@"n"] autorelease];
	[newMenuItem setTag:INT_CONSTANT_BUTTON_NEW];
	[mFileMenu addItem:newMenuItem];

	// . load
	txt = [@"" stringByAppendingString:@"Open score"];
	id loadMenuItem = [[[NSMenuItem alloc] initWithTitle:txt action:@selector(menu_item_hit:) keyEquivalent:@"o"] autorelease];
	[loadMenuItem setTag:INT_CONSTANT_BUTTON_LOAD];
	[mFileMenu addItem:loadMenuItem];

	// . open recent items
	populateOpenRecentMenu();

	// . reload
	id reloadMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Reload current score" action:@selector(menu_item_hit:) keyEquivalent:@"r"] autorelease];
	[reloadMenuItem setTag:INT_CONSTANT_BUTTON_RELOAD];
	[mFileMenu addItem:reloadMenuItem];

	// . save 
	id saveMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Save Score" action:@selector(menu_item_hit:) keyEquivalent:@"s"] autorelease];
	[saveMenuItem setTag:INT_CONSTANT_BUTTON_SAVE];
	[mFileMenu addItem:saveMenuItem];

	// . save as
	id saveAsMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Save Score As" action:@selector(menu_item_hit:) keyEquivalent:@"S"] autorelease];
	[saveAsMenuItem setTag:INT_CONSTANT_BUTTON_SAVE_AS];
	[mFileMenu addItem:saveAsMenuItem];


	[fileMenuItem setSubmenu:mFileMenu];
	[menubar addItem:fileMenuItem];

	//////////////////
	// Edit
	id editMenu = [[[NSMenu new] autorelease] initWithTitle:@"Edit"];
	id editMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Edit" action:NULL keyEquivalent:@""] autorelease];
	// . select all
	/*id selectMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Select All" action:@selector(menu_item_hit:) keyEquivalent:@"a"] autorelease];
	[editMenuItem setTag:INT_CONSTANT_BUTTON_SELECTALL];
	[editMenu addItem:selectMenuItem]; */

	// . undo
	id undoMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Undo" action:@selector(menu_item_hit:) keyEquivalent:@"z"] autorelease];
	[undoMenuItem setTag:INT_CONSTANT_BUTTON_UNDO];
	[editMenu addItem:undoMenuItem];

	// . redo
	id redoMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Redo" action:@selector(menu_item_hit:) keyEquivalent:@"Z"] autorelease];
	[redoMenuItem setTag:INT_CONSTANT_BUTTON_REDO];
	[editMenu addItem:redoMenuItem];

	// . find
	id findMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Find text" action:@selector(menu_item_hit:) keyEquivalent:@"f"] autorelease];
	[findMenuItem setTag:INT_CONSTANT_BUTTON_FIND];
	[editMenu addItem:findMenuItem];

	// . autocomplete
	NSMenuItem* autocompleteMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Autocomplete text" action:@selector(menu_item_hit:) keyEquivalent:@"\t"] autorelease];
	[autocompleteMenuItem setTag:INT_CONSTANT_BUTTON_AUTOCOMPLETE];
	autocompleteMenuItem.keyEquivalentModifierMask = NSControlKeyMask;
	[editMenu addItem:autocompleteMenuItem];

	// . lock
	mLockMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Lock AscoGraph" action:@selector(menu_item_hit:) keyEquivalent:@"K"] autorelease];
	[mLockMenuItem setTag:INT_CONSTANT_BUTTON_LOCK];
	[mLockMenuItem setState:NSOffState];
	[editMenu addItem:mLockMenuItem];
	[editMenuItem setSubmenu:editMenu];
	[menubar addItem:editMenuItem];

	//////////////////
	// Create
	id createMenu = [[[NSMenu new] autorelease] initWithTitle:@"Create"];
	id createMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create" action:NULL keyEquivalent:@""] autorelease];
	// group
	NSMenuItem* createGroupMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create Group {...}" action:@selector(menu_item_hit:) keyEquivalent:@"g"] autorelease];
	[createGroupMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_GROUP];
	createGroupMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createMenu addItem:createGroupMenuItem];
	// loop
	NSMenuItem* createLoopMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create Loop {...}" action:@selector(menu_item_hit:) keyEquivalent:@"l"] autorelease];
	createLoopMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createLoopMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_LOOP];
	[createMenu addItem:createLoopMenuItem];

	// curve
	NSMenuItem* createCurveMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create Curve {...}" action:@selector(menu_item_hit:) keyEquivalent:@"c"] autorelease];
	[createCurveMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_CURVE];
	createCurveMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createMenu addItem:createCurveMenuItem];
	// curves
	NSMenuItem* createCurvesMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create Curves {...}" action:@selector(menu_item_hit:) keyEquivalent:@"C"] autorelease];
	createCurvesMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createCurvesMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_CURVES];
	[createMenu addItem:createCurvesMenuItem];
	// whenever
	NSMenuItem* createWheneverMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create Whenever {...}" action:@selector(menu_item_hit:) keyEquivalent:@"w"] autorelease];
	createWheneverMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createWheneverMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_WHENEVER];
	[createMenu addItem:createWheneverMenuItem];
	// oscsend
	NSMenuItem* createOscsendMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create OSCsend ..." action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	createOscsendMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createOscsendMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_OSCSEND];
	[createMenu addItem:createOscsendMenuItem];
	// whenever
	NSMenuItem* createOscrecvMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Create OSCrecv ..." action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	createOscrecvMenuItem.keyEquivalentModifierMask = NSCommandKeyMask|NSAlternateKeyMask;
	[createOscrecvMenuItem setTag:INT_CONSTANT_BUTTON_CREATE_OSCRECV];
	[createMenu addItem:createOscrecvMenuItem];

	[createMenuItem setSubmenu:createMenu];
	[menubar addItem:createMenuItem];

	//////////////////
	// Transport
	id transMenu = [[[NSMenu new] autorelease] initWithTitle:@"Transport"];
	id transMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Transport" action:NULL keyEquivalent:@""] autorelease];
	// . Play
	id playMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Play" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[playMenuItem setTag:INT_CONSTANT_BUTTON_PLAY];
	[transMenu addItem:playMenuItem];
	if (enable_simulate) {
		// . Simulate
		id simulateMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Simulate" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
		[simulateMenuItem setTag:INT_CONSTANT_BUTTON_SIMULATE];
		[transMenu addItem:simulateMenuItem];
		id editMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Edit" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
		[editMenuItem setTag:INT_CONSTANT_BUTTON_EDIT];
		[transMenu addItem:editMenuItem];
	}
	// . Start
	id startMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Start" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[startMenuItem setTag:INT_CONSTANT_BUTTON_START];
	[transMenu addItem:startMenuItem];
	// . Stop
	id stopMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Stop" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[stopMenuItem setTag:INT_CONSTANT_BUTTON_STOP];
	[transMenu addItem:stopMenuItem];
	// . PlayString
	id playStringMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Play string" action:@selector(menu_item_hit:) keyEquivalent:@"p"] autorelease];
	[playStringMenuItem setTag:INT_CONSTANT_BUTTON_PLAYSTRING];
	[transMenu addItem:playStringMenuItem];
	// . Prev Event
	unichar left = NSLeftArrowFunctionKey;
	NSMenuItem *prevEventMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Previous event" action:@selector(menu_item_hit:) keyEquivalent:@"" /*[NSString stringWithCharacters:&left length:1]*/] autorelease];
	//prevEventMenuItem.keyEquivalentModifierMask = NSCommandKeyMask;
	[prevEventMenuItem setTag:INT_CONSTANT_BUTTON_PREVEVENT];
	[transMenu addItem:prevEventMenuItem];
	// . Next Event
	unichar right = NSRightArrowFunctionKey;
	NSMenuItem *nextEventMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Next event" action:@selector(menu_item_hit:) keyEquivalent:@"" /*[NSString stringWithCharacters:&right length:1]*/] autorelease];
	//nextEventMenuItem.keyEquivalentModifierMask = NSCommandKeyMask;
	[nextEventMenuItem setTag:INT_CONSTANT_BUTTON_NEXTEVENT];
	[transMenu addItem:nextEventMenuItem];

	[transMenuItem setSubmenu:transMenu];
	[menubar addItem:transMenuItem];

	//////////////////
	// View
	id viewMenu = [[[NSMenu new] autorelease] initWithTitle:@"View"];
	id viewMenuItem = [[[NSMenuItem alloc] initWithTitle:@"View" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
#ifdef USE_GUIDO
	// . toggle view
	id toggleViewMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle View" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[toggleViewMenuItem setTag:INT_CONSTANT_BUTTON_TOGGLEVIEW];
	[viewMenu addItem:toggleViewMenuItem];
#endif
	// . toggle editor
	id toggleEditorMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle Editor" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[toggleEditorMenuItem setTag:INT_CONSTANT_BUTTON_TOGGLEEDIT];
	[viewMenu addItem:toggleEditorMenuItem];
	// . toggle full editor
	id toggleFullEditorMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle Full Text Editor" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[toggleFullEditorMenuItem setTag:INT_CONSTANT_BUTTON_TOGGLE_FULL_EDITOR];
	[viewMenu addItem:toggleFullEditorMenuItem];
	// . show/hide action track
	mShowhideActiontrackMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle actions track display" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[mShowhideActiontrackMenuItem setTag:INT_CONSTANT_BUTTON_SHOWHIDE_ACTION];
	[mShowhideActiontrackMenuItem setState:NSOnState];

	[viewMenu addItem:mShowhideActiontrackMenuItem];
	// . snap grid
	mSnapMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Snap to grid" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[mSnapMenuItem setTag:INT_CONSTANT_BUTTON_SNAP];
	[mSnapMenuItem setState:NSOnState];
	[viewMenu addItem:mSnapMenuItem];
	// . autoscroll
	mAutoscrollMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Automatic Scroll" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[mAutoscrollMenuItem setTag:INT_CONSTANT_BUTTON_AUTOSCROLL];
	[mAutoscrollMenuItem setState:NSOnState];
	[viewMenu addItem:mAutoscrollMenuItem];
	// line wrap mode
	mLineWrapModeMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle Line Wrapping" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[mLineWrapModeMenuItem setTag:INT_CONSTANT_BUTTON_LINEWRAP];
	[mLineWrapModeMenuItem setState:NSOnState];
	[viewMenu addItem:mLineWrapModeMenuItem];
	// actions view deep levels
	mActionsViewDeepLevelModeMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Actions view Deep Level mode" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[mActionsViewDeepLevelModeMenuItem setTag:INT_CONSTANT_BUTTON_ACTIONVIEWDEEPMODE];
	[mActionsViewDeepLevelModeMenuItem setState:NSOffState];
	[viewMenu addItem:mActionsViewDeepLevelModeMenuItem];

	// open all curves
	id openAllCurvesMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Open all curves" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[openAllCurvesMenuItem setTag:INT_CONSTANT_BUTTON_OPEN_ALL_CURVES];
	[openAllCurvesMenuItem setState:NSOffState];
	[viewMenu addItem:openAllCurvesMenuItem];
	// open all groups
	id openAllGroupsMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Open all groups" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[openAllGroupsMenuItem setTag:INT_CONSTANT_BUTTON_OPEN_ALL_GROUPS];
	[openAllGroupsMenuItem setState:NSOffState];
	[viewMenu addItem:openAllGroupsMenuItem];
	// close all curves
	id closeAllCurvesMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Close all curves" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[closeAllCurvesMenuItem setTag:INT_CONSTANT_BUTTON_CLOSE_ALL_CURVES];
	[viewMenu addItem:closeAllCurvesMenuItem];
	// close all groups
	id closeAllGroupsMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Close all groups" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[closeAllGroupsMenuItem setTag:INT_CONSTANT_BUTTON_CLOSE_ALL_GROUPS];
	[viewMenu addItem:closeAllGroupsMenuItem];

	// get patch receivers
	id getRecvrsMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Get patch receivers names" action:@selector(menu_item_hit:) keyEquivalent:@"R"] autorelease];
	[getRecvrsMenuItem setTag:INT_CONSTANT_BUTTON_GET_PATCH_RECEIVERS];
	[viewMenu addItem:getRecvrsMenuItem];
	// zoom +
	id zoomInMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Zoom in" action:@selector(menu_item_hit:) keyEquivalent:@"+"] autorelease];
	[zoomInMenuItem setTag:INT_CONSTANT_BUTTON_ZOOM_IN];
	[viewMenu addItem:zoomInMenuItem];
	// zoom -
	id zoomOutMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Zoom out" action:@selector(menu_item_hit:) keyEquivalent:@"-"] autorelease];
	[zoomOutMenuItem setTag:INT_CONSTANT_BUTTON_ZOOM_OUT];
	[viewMenu addItem:zoomOutMenuItem];


	[viewMenuItem setSubmenu:viewMenu];
	[menubar addItem:viewMenuItem];

	//////////////////
	// Cues
	mCuesMenu = [[[NSMenu new] autorelease] initWithTitle:@"Labels"];
	mCuesMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Labels" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	/*
	id toggleViewMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle View" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[toggleViewMenuItem setTag:INT_CONSTANT_BUTTON_TOGGLEVIEW];
	[cuesMenu addItem:toggleViewMenuItem];
	*/

	[mCuesMenuItem setSubmenu:mCuesMenu];
	[menubar addItem:mCuesMenuItem];

	//////////////////
	// Help
	id helpMenu = [[[NSMenu new] autorelease] initWithTitle:@"Help"];
	id helpMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Help" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	//[helpMenu addItem:helpMenuItem];
	[helpMenuItem setSubmenu:helpMenu];
	[menubar addItem:helpMenuItem];

	guiBottom = new ofxUICanvas(0, 0, score_x+score_w - 300, score_y);
	guiBottom->setFont(ofFilePath::getCurrentExeDir() + "../Resources/DroidSansMono.ttf");
	guiSetup_OSC = new ofxUICanvas(score_x + 50, score_y, ofGetWindowWidth(), ofGetWindowHeight(), guiBottom);
	guiError = new ofxUIScrollableCanvas(score_x, score_y, ofGetWindowWidth(), ofGetWindowHeight()-100-score_y, guiBottom);
	guiSetup_Colors = new ofxUICanvas(0, 0, ofGetWindowWidth(), ofGetWindowHeight(), guiBottom);
	guiFind = new ofxUICanvas(300, score_y, ofGetWindowWidth()-300, ofGetWindowHeight()-100-score_y, guiBottom);

	//guiFind->setFont("DroidSansMono.ttf");
	/*
	   guiBottom->setFont("NewMedia Fett.ttf");
	   guiSetup_OSC->setFont("NewMedia Fett.ttf");
	   guiSetup_Colors->setFont("NewMedia Fett.ttf");
	   guiError->setFont("NewMedia Fett.ttf");
	   */


	// register double click on editor notification callback
	[[NSNotificationCenter defaultCenter] addObserver:[NSApp delegate] selector:@selector(receiveNotification:) name:@"DoubleClick" object:nil];
	//[[NSNotificationCenter defaultCenter] addObserver:[NSApp delegate] selector:@selector(receiveNotification:) name:NSTextDidChangeNotification object:nil];

	//guiSetup_Colors->setScrollableDirections(false, true);
	guiError->setScrollAreaToScreen();
	guiError->setScrollableDirections(true, false);
	guiError->setColorBack(ofColor(0, 0, 0, 0));
	guiSetup_OSC->setColorBack(ofColor(0, 0, 0, 0));
	guiSetup_Colors->setColorBack(ofColor(0, 0, 0, 0));

	guiBottom->loadSettings("GUI/guiSettings.xml");
	//guiTop->loadSettings("GUI/guiSettings.xml");

	ofxUIColor col = ofxAntescofoNote->color_staves_bg;
	col.a = 255;
	guiBottom->setColorBack(col);
	/*
	guiBottom->setUIColors(ofxAntescofoNote->color_staves_bg, &ofColor(0, 0, 0, 0), ofxAntescofoNote->color_highlight, ofxAntescofoNote->color_staves_fg,
	                    ofxAntescofoNote->color_highlight, ofxAntescofoNote->color_gui_bg, ofxAntescofoNote->color_gui_bg);
	*/

	//guiBottom->addWidgetDown(new ofxUISpacer(ofGetWidth()-5, 1));
	//mSliderBPM = new ofxUISlider(16*4*10, 0, 70, 12, 30, 300, 120, TEXT_CONSTANT_BUTTON_BPM);
	//guiBottom->addWidgetDown(mSliderBPM);
	mBPMbuffer = new float[256];
	for (int i = 0; i < 256; i++) mBPMbuffer[i] = 0.;

	ofxUISpacer *space = new ofxUISpacer(ofGetWidth(), 1);
	space->setVisible(false);
	guiBottom->addWidgetDown(space);

	mLabelBPM = new ofxUILabel(TEXT_CONSTANT_BUTTON_BPM, fontsize);
	//guiBottom->addWidgetSouthOf(mLabelBPM, TEXT_CONSTANT_BUTTON_START);
	guiBottom->addWidgetDown(mLabelBPM);
	mLabelBPM = new ofxUILabel("120", fontsize);
	guiBottom->addWidgetEastOf(mLabelBPM, TEXT_CONSTANT_BUTTON_BPM);

	mLabelBeat = new ofxUILabel(TEXT_CONSTANT_BUTTON_BEAT, fontsize);
	guiBottom->addWidgetSouthOf(mLabelBeat, TEXT_CONSTANT_BUTTON_BPM);
	mLabelBeat = new ofxUILabel("0", fontsize);
	guiBottom->addWidgetEastOf(mLabelBeat, TEXT_CONSTANT_BUTTON_BEAT);

	mLabelPitch = new ofxUILabel(TEXT_CONSTANT_BUTTON_PITCH, fontsize);
	guiBottom->addWidgetSouthOf(mLabelPitch, TEXT_CONSTANT_BUTTON_BEAT);
	mLabelPitch = new ofxUILabel("0", fontsize);
	guiBottom->addWidgetEastOf(mLabelPitch, TEXT_CONSTANT_BUTTON_PITCH);
	mLabelAccompSpeed = new ofxUILabel(TEXT_CONSTANT_BUTTON_SPEED, fontsize);

	// tempo curve
	ofxUISpectrum* tempoCurve = new ofxUISpectrum(313, 64, mBPMbuffer, 256, 0., 290.0, "bpm");
	//tempoCurve->setDrawOutline(true);
	guiBottom->addWidgetDown(tempoCurve);
	tempoCurve->setColorFill(ofColor(ofxAntescofoNote->color_key));
	tempoCurve->setColorFillHighlight(ofColor(ofxAntescofoNote->color_key));
	ofxUIRectangle* r = tempoCurve->getRect();
	r->x = 3; r->y = 3;

	string path_prefix_img = ofFilePath::getCurrentExeDir() + "../Resources/";

	// transport btns

	int wi = 32;
	int xi = 358, yi = 28, dxi = 12;
	string img_path("GUI/prev_.png");
	ofxUIMultiImageToggle* prevToggle = new ofxUIMultiImageToggle(wi, wi, false, img_path, TEXT_CONSTANT_BUTTON_PREV_EVENT);
	prevToggle->setLabelVisible(false);
	prevToggle->setDrawOutline(true);
	guiBottom->addWidgetEastOf(prevToggle, "bpm");
	r = prevToggle->getRect(); r->x = xi; r->y = yi;

	img_path = "GUI/stop_.png";
	ofxUIMultiImageToggle* stopToggle = new ofxUIMultiImageToggle(wi, wi, false, img_path, TEXT_CONSTANT_BUTTON_STOP);
	stopToggle->setLabelVisible(false);
	stopToggle->setDrawOutline(true);
	guiBottom->addWidgetEastOf(stopToggle, TEXT_CONSTANT_BUTTON_PREV_EVENT);
	r = stopToggle->getRect(); r->x = xi + wi + dxi; r->y = yi;

	img_path = "GUI/play_.png";
	ofxUIMultiImageToggle* playToggle = new ofxUIMultiImageToggle(wi, wi, false, img_path, TEXT_CONSTANT_BUTTON_PLAY);
	playToggle->setLabelVisible(false);
	playToggle->setDrawOutline(true);
	guiBottom->addWidgetEastOf(playToggle, TEXT_CONSTANT_BUTTON_STOP);
	r = playToggle->getRect(); r->x = xi + 2*(wi+dxi); r->y = yi;

	img_path = "GUI/start_.png";
	ofxUIMultiImageToggle* startToggle = new ofxUIMultiImageToggle(wi, wi, false, img_path, TEXT_CONSTANT_BUTTON_START);
	startToggle->setLabelVisible(false);
	startToggle->setDrawOutline(true);
	guiBottom->addWidgetEastOf(startToggle, TEXT_CONSTANT_BUTTON_PLAY);
	r = startToggle->getRect(); r->x = xi + 3*(wi+dxi); r->y = yi;

	img_path = "GUI/next_.png";
	ofxUIMultiImageToggle* nextToggle = new ofxUIMultiImageToggle(wi, wi, false, img_path, TEXT_CONSTANT_BUTTON_NEXT_EVENT);
	nextToggle->setLabelVisible(false);
	nextToggle->setDrawOutline(true);
	guiBottom->addWidgetEastOf(nextToggle, TEXT_CONSTANT_BUTTON_START);
	r = nextToggle->getRect(); r->x = xi + 4*(wi+dxi); r->y = yi;
#if 0
	ofxUILabelToggle* b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_START, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetEastOf(b, "bpm");
	b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_STOP, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetRight(b);
	b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_PLAY, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetRight(b);
	
	b = new ofxUILabelToggle(90, false, TEXT_CONSTANT_BUTTON_PREV_EVENT, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetRight(b);
	b = new ofxUILabelToggle(90, false, TEXT_CONSTANT_BUTTON_NEXT_EVENT, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetRight(b);
	if (enable_simulate) {
		//b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_SIMULATE, OFX_UI_FONT_SMALL);
		//guiBottom->addWidgetRight(b);
		mEditButton = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_EDIT, OFX_UI_FONT_SMALL);
		guiBottom->addWidgetRight(mEditButton);
		mEditButton->setVisible(false);
		mEditButton->setLabelVisible(false);
	}
#endif
/*
	ofxUISpacer *space = new ofxUISpacer(ofGetWidth(), 1);
	space->setVisible(false);
	guiBottom->addWidgetDown(space);
*/

	//guiBottom->addWidgetDown(new ofxUISpacer(ofGetWidth()-5, 1));
	ofxUIButton *bu = new ofxUIButton("NOTE", false, 30, 15);
	guiBottom->addWidgetEastOf(bu, "bpm");
	bu->setColorBack(ofxAntescofoNote->color_note);

	bu = new ofxUIButton("CHORD", false, 30, 15);
	guiBottom->addWidgetRight(bu);
	bu->setColorBack(ofxAntescofoNote->color_note_chord);

	bu = new ofxUIButton("MULTI", false, 30, 15); 
	guiBottom->addWidgetRight(bu);
	bu->setColorBack(ofxAntescofoNote->color_note_multi);

	bu = new ofxUIButton("TRILL", false, 30, 15);
	guiBottom->addWidgetRight(bu);
	bu->setColorBack(ofxAntescofoNote->color_note_trill);

	guiBottom->addWidgetDown(mLabelAccompSpeed);
	mLabelAccompSpeed->setVisible(false);
	mLabelAccompSpeed = new ofxUILabel("0", fontsize);
	guiBottom->addWidgetRight(mLabelAccompSpeed);
	mLabelAccompSpeed->setVisible(false);

	guiFind->setColorBack(ofColor(0, 0, 0, 0));

#if NO_UI_BUTTON
	guiTop->addWidgetRight(new ofxUILabel(TEXT_CONSTANT_TITLE, OFX_UI_FONT_MEDIUM));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_LOAD, OFX_UI_FONT_SMALL));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_SAVE, OFX_UI_FONT_SMALL));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_COLOR_SETUP, OFX_UI_FONT_SMALL));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_OSC_SETUP, OFX_UI_FONT_SMALL));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_TOGGLE_VIEW, OFX_UI_FONT_SMALL));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_RELOAD, OFX_UI_FONT_SMALL));
	guiTop->addWidgetRight(new ofxUILabelToggle(false, TEXT_CONSTANT_BUTTON_TOGGLE_EDITOR, OFX_UI_FONT_SMALL));
#endif

	mSaveColorButton = new ofxUILabelButton(TEXT_CONSTANT_BUTTON_SAVE_COLOR, false, ofGetWindowWidth() / 2, ofGetWindowHeight() / 2, 100);
	guiSetup_Colors->addWidget(mSaveColorButton);
	mSaveColorButton->setVisible(false);
	mSaveColorButton->setLabelVisible(false);
	guiSetup_Colors->setVisible(false);

	guiError->setVisible(false);
	guiError->disable();

	mLogoInria.loadImage(path_prefix_img+"logo_inria.png");
	mLogoIrcam.loadImage(path_prefix_img+"logo_ircam.png");

	ofAddListener(guiError->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiBottom->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiFind->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiSetup_Colors->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiSetup_OSC->newGUIEvent, this, &ofxAntescofog::guiEvent);

}

void ofxAntescofog::setupOSC(){
	// listen for OSC
	int port = 0;
	std::istringstream is(mOsc_port);
	is >> port;
	if (! is.good() && port <= 0)
		mOsc_port = "6789";
	std::cout << "Listening on OSC port " << port << endl;
	try {
		mOSCreceiver.setup(atoi(mOsc_port.c_str()));
	} catch(...) {
		ofSetColor(0, 0, 0, 100);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		ofSetColor(255, 255, 255, 240);

		string err = "Error can not listen on port ";
		err += port;
		err += " ! Please verify port is available (is another application blocking this UDP port ?";
		cerr << err << endl;
		ofDrawBitmapString(err, 100, 300);
		ofxAntescofoNote->set_error(err);
		guiError->draw();
		display_error();
	}

	is.str("");
	is.str(mOsc_port_MAX);
	is.clear();
	is >> port;
	if (! is.good() && port <= 0)
	{
		cerr << "Not a number, try again." << endl;
		mOsc_port_MAX = "5678";
	}
	save();
	std::cout << "Connecting OSC on " << mOsc_host << ":"<< atoi(mOsc_port_MAX.c_str()) << endl;
	try {
		mOSCsender.setup(mOsc_host, atoi(mOsc_port_MAX.c_str()));
	} catch (...)
	{ cerr << "ERROR OSC EXCEPTION" << endl; }
}


//--------------------------------------------------------------
void ofxAntescofog::setupTimeline(){
	//ofSetBackgroundAuto(false);
	timeline.setOffset(ofVec2f(score_x, score_y));
	timeline.setup(); //registers events

	timeline.setFrameRate(24);
	timeline.setShowTicker(false);
	timeline.setShowBPMGrid(bSnapToGrid);
	timeline.enableSnapToBPM(bSnapToGrid);
	timeline.setDurationInSeconds(60);
	//timeline.moveToThread(); //increases accuracy of bang call backs

	timeline.setLoopType(OF_LOOP_NORMAL);
	timeline.setBPM(bpm);
	timeline.setLockWidthToWindow(false);

	// use custom zoomer :
	timeline.addTrack("zoom", ofxAntescofoZoom);
	timeline.setZoomer(ofxAntescofoZoom);
	timeline.addTrack("Beats", ofxAntescofoBeatTicker);
	timeline.addTrack("Notes", ofxAntescofoNote);
	ofxAntescofoNote->setDrawRect(ofRectangle(0, 0, score_w, 300));
	ofxAntescofoBeatTicker->setup();
	timeline.setShowTicker(true);
	timeline.setBPM(bpm);

	timeline.enable();
	timeline.setFrameBased(false);
	ofxAntescofoNote->enable();
}



//--------------------------------------------------------------
void ofxAntescofog::setup(){
	console->addln("ofxAntescofo::setup()");
	//ofSetDataPathRoot("../Resources/");
	ofSetDataPathRoot([[NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] resourcePath]] cStringUsingEncoding:NSUTF8StringEncoding]);

	ofSetFrameRate(24);
	ofSetVerticalSync(true);
	ofEnableSmoothing();
	//ofEnableAlphaBlending();
	
	//glewExperimental=TRUE;
	GLenum err=glewInit();
	if(err!=GLEW_OK)
	{
		//Problem: glewInit failed, something is seriously wrong.
		cout<<"glewInit failed, aborting."<<endl;
		abort();
	}
	//cocoaWindow->setUpdateRate(1);
	ofSetLogLevel(OF_LOG_VERBOSE);

	fog = this;
	score_x = 5;
	score_y = 82;
	mUIbottom_y = 40;

	bpm = 120; 

	ofSetEscapeQuitsApp(false);
	score_w = ofGetWindowWidth() - score_x - 5;
	score_h = ofGetWindowHeight()/3;

	ofAddListener(ofEvents().windowResized, this, &ofxAntescofog::windowResized);

	ofxAntescofoZoom = new ofxTLZoomer2D();
	ofxAntescofoNote = new ofxTLAntescofoNote(this);
	ofxAntescofoBeatTicker = new ofxTLBeatTicker(this);

#ifndef IOS_ASCOGRAPH
	load_appsupport(getApplicationSupportSettingFile());
#endif
	load(); // xml settings

	setupTimeline();

	setupUI();

	setupOSC();

	remove(TEXT_CONSTANT_TEMP_FILENAME);
	remove(TEXT_CONSTANT_TEMP_ACTION_FILENAME);

	setEditorMode(bEditorShow, 0);

	drawCache.allocate(ofGetWindowWidth(), ofGetHeight(), GL_RGBA);
	drawCache.begin();
	ofClear(255,255,255, 0);
	drawCache.end();

	bSetupDone = true;

	if (mScore_filename.size())
		loadScore(mScore_filename, true);

#ifdef USE_HTTPD
	if (!disable_httpd) setup_httpd();
#endif
}

//--------------------------------------------------------------
void ofxAntescofog::update() {
	if (!bSetupDone)
		return;
	bool reloadFile = true;

	timeline.setWidth(score_w);
	timeline.setOffset(ofVec2f(score_x, score_y));

	// check for waiting messages
	try {
		while( mOSCreceiver.hasWaitingMessages() )
		{
			// get the next message
			ofxOscMessage m;
			mOSCreceiver.getNextMessage( &m );
			if (_debug) cout << "OSC received: '" << m.getAddress() <<"' ";// << "' args:"<<m.getNumArgs()<< endl;
			if(m.getAddress() == "/antescofo/stop"){
				if (audioTrack) audioTrack->fakeStop();
				mHasReadMessages = true;
			}else if(m.getAddress() == "/antescofo/tempo" && m.getArgType(0) == OFXOSC_TYPE_FLOAT) {
				mOsc_tempo = m.getArgAsFloat(0);
				if (_debug) cout << "OSC received: tempo: "<< mOsc_tempo << endl;
				bpm = mOsc_tempo;
				mLabelBPM->setLabel(ofToString(mOsc_tempo));
				//if (bpm) timeline.setBPM(bpm);
				//mSliderBPM->setValue(bpm);
				mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/event_beatpos" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_beat = m.getArgAsFloat(0);
				mLabelBeat->setLabel(ofToString(mOsc_beat));
				if (_debug) cout << "OSC received: beat: "<< mOsc_beat << endl;
#ifdef USE_HTTPD
				if (!disable_httpd)
					httpd_update_beatpos();
#endif
				if (mOsc_beat == 0)
					ofxAntescofoNote->missedAll();
				mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/rnow" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_rnow = m.getArgAsFloat(0);
				//mLabelBeat->setLabel(ofToString(mOsc_rnow));
				if (_debug) cout << "OSC received: rnow: "<< mOsc_rnow << endl;
				//mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/pitch"  && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_pitch = m.getArgAsFloat(0);
				mLabelPitch->setLabel(ofToString(mOsc_pitch));
				if (_debug) cout << "OSC received: pitch: "<< mOsc_pitch << endl;
				mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/accomp_pos" && m.getArgType(0) == OFXOSC_TYPE_FLOAT) {
				if (_debug) cout << "OSC received: accomp_pos: "<<  m.getArgAsFloat(0) << endl;
				mHasReadMessages = true;
				if (audioTrack) audioTrack->fakePlay(m.getArgAsFloat(0));
			} else if(m.getAddress() == "/antescofo/accomp_speed"  && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_accomp_speed= m.getArgAsFloat(0);
				mLabelAccompSpeed->setLabel(ofToString(mOsc_accomp_speed));
				if (_debug) cout << "OSC received: accomp speed: "<< mOsc_accomp_speed << endl;
				mHasReadMessages = true;
				if (audioTrack) audioTrack->setFakeSpeed(mOsc_accomp_speed);
				mLabelAccompSpeed->setVisible(true);
				guiBottom->getWidget(TEXT_CONSTANT_BUTTON_SPEED)->setVisible(true);
				bShouldRedraw = true;
			} else if(m.getAddress() == "/antescofo/loadscore"  && m.getArgType(0) == OFXOSC_TYPE_STRING){
				string scorefile = m.getArgAsString(0);
				//if (_debug)
					cout << "OSC received: loadscore: "<< scorefile << endl;
				mHasReadMessages = true;
				cues_clear_menu();
				reloadFile = (scorefile != mScore_filename);
				loadScore(scorefile, reloadFile, false);
				bShouldRedraw = true;
			} else if(m.getAddress() == "/antescofo/patch_receivers") {
				patch_receivers.clear();
				cout << "OSC received: " << m.getNumArgs() << endl;
				for(int i = 0; i < m.getNumArgs(); i++){
					string s = m.getArgAsString(i);
					cout << "\t receiver name= " << s << endl;
					patch_receivers.push_back(s);
				}
				[ editor set_action_keywords: patch_receivers ];
			} else if(m.getAddress() == "/antescofo/quit"  && m.getArgType(0) == OFXOSC_TYPE_STRING){
				_Exit(0);
			} else { 
				mHasReadMessages = false;
				// unrecognized message: display on the bottom of the screen
				string msg_string;
				msg_string = m.getAddress();
				msg_string += ": ";
				for(int i = 0; i < m.getNumArgs(); i++){
					// get the argument type
					msg_string += m.getArgTypeName(i);
					msg_string += ":";
					// display the argument - make sure we get the right type
					if(m.getArgType(i) == OFXOSC_TYPE_INT32){
						msg_string += ofToString(m.getArgAsInt32(i));
					}
					else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
						msg_string += ofToString(m.getArgAsFloat(i));
					}
					else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
						msg_string += m.getArgAsString(i);
					}
					else{
						msg_string += "unknown";
					}
				}
				cout << "OSC received: unknown msg: "<< msg_string << endl;
			}
			//no break in order to eat every available messages
		}
	}
	catch (exception& e) {
		cerr << "OSC HasWaitingMessage exception raised" <<  endl;
	}

	// if we read something, advance playhead
	if (mHasReadMessages) {

		mLastOSCmsgDate = ofGetSystemTime();
		if (mOsc_beat != -1 && reloadFile) {
			fAntescofoTimeSeconds = ofxAntescofoNote->convertAntescofoOutputToTime(mOsc_beat, mOsc_tempo, mOsc_pitch);

			if (_debug) cout << "Moving playHead to beat:"<<mOsc_beat << " tempo:"<<mOsc_tempo << " => "<<fAntescofoTimeSeconds << "sec"<<endl;
		}
		mHasReadMessages = false;
		bShouldRedraw = true;
	}
	//guiBottom->update();
	push_tempo_value();

#ifdef USE_HTTPD
	// http update
	if (!disable_httpd)
		update_http_image();
#endif
}

void ofxAntescofog::shouldRedraw()
{
	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::draw() {
	// since update to openFrameworks 8, it is needed to translate down Y of 20 pixel, I don't know why...
	ofTranslate(0, 20);

	if (!bSetupDone)
		return;

	struct timeval now;
	gettimeofday(&now, 0);
#define DRAW_MAX_DELTA_USEC	10000
	if ((now.tv_sec*1000000L + now.tv_usec) - (last_draw_time.tv_sec*1000000L + last_draw_time.tv_usec) < DRAW_MAX_DELTA_USEC) {
		return;
	}
	gettimeofday(&last_draw_time, 0);
	bool bMayUseCache = true;
	/*if (ofGetSystemTime() > mLastOSCmsgDate > 150000) 
		bMayUseCache = true;
	*/
	ofFill();
	if (bShowColorSetup) { // color setup
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_ColorSetup();
	} else if (bShowOSCSetup) { // osc setup
		draw_OSCSetup();
	} else if (bShowError) { // parsing error display
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_error();
	} /*else if (bShowFind) { // find/replace
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_FindText();
	}*/ else { // draw normal
		if (bMayUseCache && !timeline.getIsPlaying() && !bShouldRedraw) {
			drawCache.draw(0, 0);
		} else {
			//cout << "-------------------- ofxAntescofog::draw() redrawing!" << endl;
			ofSetColor(255, 255, 255, 255);
			drawCache.begin();
			ofClear(255,255,255, 0);
			ofPushStyle();
			ofFill();
			ofSetColor(ofxAntescofoNote->color_staves_bg);
			ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());

			if (bIsSimulating) { // simulation
				draw_simulate();
			} else
				timeline.draw();
			guiBottom->draw();
			console->draw();
			ofPopStyle();

			// logos
			mLogoInria.draw(score_w - 290, 9, 142, 52);
			mLogoIrcam.draw(score_w - 120, 8, 102, 61);
		
			drawCache.end();

			//ofBackground(255, 255, 255, 255);
			drawCache.draw(0, 0);
			bShouldRedraw = false;
		}
	}

#ifdef USE_HTTPD
	// http draw
	if (!disable_httpd)
		draw_http_image();
#endif
}

void ofxAntescofog::save_appsupport(string filename) {
	ofxXmlSettings settings;

	settings.addTag("settings");
	settings.pushTag("settings");
	if (mRecentFiles.size()) {
		settings.addTag("recentFiles");
		settings.pushTag("recentFiles");
		// recent files
		for (int i = 0; i < mRecentFiles.size(); i++) {
			settings.addTag("recentFile");
			settings.pushTag("recentFile", i);
			settings.addValue("file", mRecentFiles[i]);
			settings.popTag();
		}
	}
	settings.popTag();
	if(!settings.saveFile(filename)){
		cerr <<  "ERROR: ofxAntescofog: Error saving to xml file " + filename << endl;
	} else 
		cout << "ofxAntescofog::save_appsupport saved " << filename << endl;
}

void ofxAntescofog::load_appsupport(string filename) {
	ofxXmlSettings settings;
	mRecentFiles.clear();
	if(settings.loadFile(filename)){
		settings.pushTag("settings");
		settings.pushTag("recentFiles");
		int getNb = settings.getNumTags("recentFile");
		for (int i = 0; i < getNb; i++) {
			settings.pushTag("recentFile", i);
			string f = settings.getValue("file", "");
			settings.popTag();
			mRecentFiles.push_back(f);
		}
		settings.popTag();
	}
}

void ofxAntescofog::load()
{
	string xmlFileName = "GUI/Ascograph.xml";
	ofxXmlSettings settings;
    
	if(settings.loadFile(xmlFileName)){
		//switches = switchesFromXML(settings);
        // gui bg
        int red		= settings.getValue("colors:gui_bg:r", 30);
        int green	= settings.getValue("colors:gui_bg:g", 110);
        int blue	= settings.getValue("colors:gui_bg:b", 110);
        int alpha   = settings.getValue("colors:gui_bg:a", 255);
        ofxAntescofoNote->color_gui_bg.set(red, green, blue);
        colorString2var["colors:gui_bg"] = &ofxAntescofoNote->color_gui_bg;
        
        // staff bg
        red		= settings.getValue("colors:background:r", 255);
        green	= settings.getValue("colors:background:g", 255);
        blue	= settings.getValue("colors:background:b", 255);
        alpha   = settings.getValue("colors:background:a", 255);
        ofxAntescofoNote->color_staves_bg.set(red, green, blue);
        colorString2var["colors:background"] = &ofxAntescofoNote->color_staves_bg;
        
        // staff fg
        red		= settings.getValue("colors:foreground:r", 0);
        green	= settings.getValue("colors:foreground:g", 0);
        blue	= settings.getValue("colors:foreground:b", 0);
        alpha	= settings.getValue("colors:foreground:a", 255);
        ofxAntescofoNote->color_staves_fg.set(red, green, blue, alpha);
        colorString2var["colors:foreground"] = &ofxAntescofoNote->color_staves_fg;
        
        // note range white
        red		= settings.getValue("colors:rangewhite:r", 255);
        green	= settings.getValue("colors:rangewhite:g", 255);
        blue	= settings.getValue("colors:rangewhite:b", 255);
        alpha	= settings.getValue("colors:rangewhite:a", 250);
        ofxAntescofoNote->color_range_white.set(red, green, blue, alpha);
        colorString2var["colors:rangewhite"] = &ofxAntescofoNote->color_range_white;
        
        // note range black
        red		= settings.getValue("colors:rangeblack:r", 255);
        green	= settings.getValue("colors:rangeblack:g", 255);
        blue	= settings.getValue("colors:rangeblack:b", 255);
        alpha	= settings.getValue("colors:rangeblack:a", 250);
        ofxAntescofoNote->color_range_black.set(red, green, blue, alpha);
        colorString2var["colors:rangeblack"] = &ofxAntescofoNote->color_range_black;

        // note
        red		= settings.getValue("colors:note:r", 0);
        green	= settings.getValue("colors:note:g", 0);
        blue	= settings.getValue("colors:note:b", 0);
        alpha	= settings.getValue("colors:note:a", 255);
        ofxAntescofoNote->color_note.set(red, green, blue, alpha);
        colorString2var["colors:note"] = &ofxAntescofoNote->color_note;
        
        // note CHORD
        red		= settings.getValue("colors:notechord:r", 15);
        green	= settings.getValue("colors:notechord:g", 205);
        blue	= settings.getValue("colors:notechord:b", 35);
        alpha	= settings.getValue("colors:notechord:a", 255);
        ofxAntescofoNote->color_note_chord.set(red, green, blue, alpha);
        colorString2var["colors:notechord"] = &ofxAntescofoNote->color_note_chord;

        // note TRILL
        red		= settings.getValue("colors:notetrill:r", 40);
        green	= settings.getValue("colors:notetrill:g", 50);
        blue	= settings.getValue("colors:notetrill:b", 60);
        alpha	= settings.getValue("colors:notetrill:a", 255);
        ofxAntescofoNote->color_note_trill.set(red, green, blue, alpha);
        colorString2var["colors:notetrill"] = &ofxAntescofoNote->color_note_trill;

        // note MULTI
        red		= settings.getValue("colors:notemulti:r", 240);
        green	= settings.getValue("colors:notemulti:g", 80);
        blue	= settings.getValue("colors:notemulti:b", 190);
        alpha	= settings.getValue("colors:notemulti:a", 255);
        ofxAntescofoNote->color_note_multi.set(red, green, blue, alpha);
        colorString2var["colors:notemulti"] = &ofxAntescofoNote->color_note_multi;
        
        // rest
        red		= settings.getValue("colors:rest:r", 0);
        green	= settings.getValue("colors:rest:g", 0);
        blue	= settings.getValue("colors:rest:b", 0);
        alpha	= settings.getValue("colors:rest:a", 255);
        ofxAntescofoNote->color_note_rest.set(red, green, blue, alpha);
        colorString2var["colors:rest"] = &ofxAntescofoNote->color_note_rest;

        // note selected
        red		= settings.getValue("colors:noteselected:r", 220);
        green	= settings.getValue("colors:noteselected:g", 0);
        blue	= settings.getValue("colors:noteselected:b", 0);
        alpha	= settings.getValue("colors:noteselected:a", 220);
        ofxAntescofoNote->color_note_selected.set(red, green, blue, alpha);
        colorString2var["colors:noteselected"] = &ofxAntescofoNote->color_note_selected;

        // resize note
        red		= settings.getValue("colors:resizenote:r", 240);
        green	= settings.getValue("colors:resizenote:g", 0);
        blue	= settings.getValue("colors:resizenote:b", 0);
        alpha	= settings.getValue("colors:resizenote:a", 40);
        ofxAntescofoNote->color_resize_note.set(red, green, blue, alpha);
        colorString2var["colors:resizenote"] = &ofxAntescofoNote->color_resize_note;
        
        // resize note rest
        red		= settings.getValue("colors:resizenoterest:r", 240);
        green	= settings.getValue("colors:resizenoterest:g", 200);
        blue	= settings.getValue("colors:resizenoterest:b", 0);
        alpha	= settings.getValue("colors:resizenoterest:a", 100);
        ofxAntescofoNote->color_resize_note_rest.set(red, green, blue, alpha);
        colorString2var["colors:resizenoterest"] = &ofxAntescofoNote->color_resize_note_rest;

        ///////////////////////////////////////
        // timeline key color
        red		= settings.getValue("colors:key:r", 240);
        green	= settings.getValue("colors:key:g", 0);
        blue	= settings.getValue("colors:key:b", 0);
        alpha	= settings.getValue("colors:key:a", 100);
        ofxAntescofoNote->color_key.set(red, green, blue, alpha);
        colorString2var["colors:key"] = &ofxAntescofoNote->color_key;
        
        // timeline text color
        red		= settings.getValue("colors:text:r", 0);
        green	= settings.getValue("colors:text:g", 0);
        blue	= settings.getValue("colors:text:b", 0);
        alpha	= settings.getValue("colors:text:a", 255);
        ofxAntescofoNote->color_text.set(red, green, blue, alpha);
        colorString2var["colors:text"] = &ofxAntescofoNote->color_text;

        // timeline highlight color
        red		= settings.getValue("colors:highlight:r", 165);
        green	= settings.getValue("colors:highlight:g", 54);
        blue	= settings.getValue("colors:highlight:b", 71);
        alpha	= settings.getValue("colors:highlight:a", 200);
        ofxAntescofoNote->color_highlight.set(red, green, blue, alpha);
        colorString2var["colors:highlight"] = &ofxAntescofoNote->color_highlight;

        // timeline disabled color
        red		= settings.getValue("colors:disabled:r", 165);
        green	= settings.getValue("colors:disabled:g", 54);
        blue	= settings.getValue("colors:disabled:b", 71);
        alpha	= settings.getValue("colors:disabled:a", 200);
        ofxAntescofoNote->color_disabled.set(red, green, blue, alpha);
        colorString2var["colors:disabled"] = &ofxAntescofoNote->color_disabled;

        // timeline modalBg color
        red		= settings.getValue("colors:modalBackground:r", 98);
        green	= settings.getValue("colors:modalBackground:g", 98);
        blue	= settings.getValue("colors:modalBackground:b", 103);
        alpha	= settings.getValue("colors:modalBackground:a", 200);
        ofxAntescofoNote->color_modalBg.set(red, green, blue, alpha);
        colorString2var["colors:modalBackground"] = &ofxAntescofoNote->color_modalBg;
        
        // timeline outline color
        red		= settings.getValue("colors:outline:r", 0);
        green	= settings.getValue("colors:outline:g", 0);
        blue	= settings.getValue("colors:outline:b", 243);
        alpha	= settings.getValue("colors:outline:a", 200);
        ofxAntescofoNote->color_outline.set(red, green, blue, alpha);
        colorString2var["colors:outline"] = &ofxAntescofoNote->color_outline;
      
        // OSC host
        mOsc_host		= settings.getValue("osc:host", "localhost");
        mOsc_port       = settings.getValue("osc:port", "6789");
        mOsc_port_MAX   = settings.getValue("osc:portremote", "5678");
        oscString2var["osc:host"] = &mOsc_host;
        oscString2var["osc:port"] = &mOsc_port;
        oscString2var["osc:portremote"] = &mOsc_port_MAX;
        
        cout << "UI Colors loaded from xml files." << endl;
	}
	else{
		string str = "ofxTLAntescofoNote -- Error loading from xml file " + xmlFileName;
		console->addln(str);
	}
	/* string xmlFileName = "GUI/defaultColors.xml";
	ofxXmlSettings settings;
	if(settings.loadFile(xmlFileName)){ cout << "UI Colors loaded from xml files." << endl; }
	else{ ofLogError("ofxTLAntescofoNote -- Error loading from xml file " + xmlFileName); } */
}


void ofxAntescofog::save()
{
    string xmlFileName = "GUI/Ascograph.xml";
	ofxXmlSettings settings;
   
    int red, green, blue, alpha;
	for (map<string, ofColor*>::const_iterator c = colorString2var.begin(); c != colorString2var.end(); c++) {
        settings.setValue(c->first + ":r", c->second->r);
        settings.setValue(c->first + ":g", c->second->g);
        settings.setValue(c->first + ":b", c->second->b);
        settings.setValue(c->first + ":a", c->second->a);
    }
    settings.setValue("osc:host", mOsc_host);
    settings.setValue("osc:port", mOsc_port);
    settings.setValue("osc:portremote", mOsc_port_MAX);
    
	if(!settings.saveFile(xmlFileName)){
				string str = "ofxTLAntescofoNote -- Error saving to xml file " + xmlFileName;
				console->addln(str);
    }
    
    
    xmlFileName = "GUI/defaultColors.xml";    
	for (map<string, ofColor*>::const_iterator c = colorString2var.begin(); c != colorString2var.end(); c++) {
        settings.setValue(c->first + ":r", c->second->r);
        settings.setValue(c->first + ":g", c->second->g);
        settings.setValue(c->first + ":b", c->second->b);
        settings.setValue(c->first + ":a", c->second->a);
    }
	if(!settings.saveFile(xmlFileName)){
				string str = "ofxTLAntescofoNote -- Error saving to xml file " + xmlFileName;
				console->addln(str);
    }

    guiBottom->getWidget("NOTE")->setColorBack(ofxAntescofoNote->color_note);
    guiBottom->getWidget("MULTI")->setColorBack(ofxAntescofoNote->color_note_multi);
    guiBottom->getWidget("TRILL")->setColorBack(ofxAntescofoNote->color_note_trill);
    guiBottom->getWidget("CHORD")->setColorBack(ofxAntescofoNote->color_note_chord);
}

void ofxAntescofog::setEditorMode(bool state, float beatn, bool fullTextEditor) {
	bEditorShow = state;

	if (bEditorShow) {
		float editor_x = ofGetWidth();
		if (!fullTextEditor)
			cocoaWindow->setWindowShape(ofGetWidth() + CONSTANT_EDITOR_VIEW_WIDTH, ofGetHeight());
#ifdef TARGET_OSX
		NSSize siz;
		editor = [ ofxCodeEditor alloc];
		const char *normal_keywords = "bpm note chord trill multi variance tempo on off transpose variance";
		const char *major_keywords = "gfwd group lfwd loop cfwd curve kill abort killof abortof whenever closefile openoutfile oscon oscoff oscsend oscrecv";
		const char *procedure_keywords = "@macro_def @fun_def @insert #include let until expr @tight @grain @action @coef @date @fun_def @proc_def @faust_def @fluidsynth_def @csound_def @dsp_inlet @dsp_outlet @dsp_channel faust:: fluidsynth:: csound:: @global @hook @immediate @inlet @jump @label @local @map_history @map_history_date @map_history_rdate @modulate @name @norec @type during for in map imap tab loc glob if else do while := s ms port @linear @linear_in @linear_out @linear_in_out @back_in @back_out @back_in_out @bounce_in @bounce_out @bounce_in_out @cubic_in @cubic_out @cubic_in_out @circ_in @circ_out @circ_in_out @elastic_in @elastic_out @elastic_in_out @exp_in @exp_out @exp_in_out @quad_in @quad_out @quad_in_out @quart_in @quart_out @quart_in_out @quint_in @quint_out @quint_in_out @sine_in @sine_out @sine_in_out @is_undef @is_bool @is_int @is_float @is_numeric @is_string @is_symbol @is_map @is_interpolatedmap @is_fct @is_function @sin @asin @sinh @cos @acos @cosh @tan @atan @exp @log10 @log2 @log @rand @sqrt @round @floor @ceil @pow @make_score_map @make_duration_map @is_integer_indexed @is_list @is_vector @is_defined @shift_map @gshift_map @select_map @mapval @merge @concat @compose_map @min_key @max_key @min_val @max_val @listify @map_reverse @tab_map @concat @min_val @max_val @size @clear @tab_reverse @push_back @resize faust::";
		const char *system_keywords = "$rt_tempo $pitch $beat_pos $now $rnow @exp @max @min @log @integrate @normalize @bounded_integrate @add_pair @date @rdate";

		[ editor  set_normal_keywords: normal_keywords ];
		[ editor  set_major_keywords: major_keywords ];
		[ editor  set_procedure_keywords: procedure_keywords ];
		[ editor  set_system_keywords: system_keywords ];
		if (!fullTextEditor) {
			NSWindow *nswin = [cocoaWindow->delegate getNSWindow]; 
			NSView *nsview_ = [cocoaWindow->delegate getNSView];
			int h = ofGetHeight();
			//if ([ editor tabsSize ])  // if tabs should be open, shift down the editor's view
			//	h -= [ editor tabHeight ];
			ofRectangle r(editor_x, 0, CONSTANT_EDITOR_VIEW_WIDTH, h);
			[ editor setup: nswin glview:nsview_ rect:r];
		} else {
			NSWindow *nswin = [cocoaWindow->delegate getNSWindow]; 
			NSView *nsview_ = [cocoaWindow->delegate getNSView];
			int h = ofGetHeight();
			//if ([ editor tabsSize ]) 
			//	h -= [ editor tabHeight ];
			ofRectangle r(0, 0, ofGetWidth(), h);
			[ editor setup: nswin glview:nsview_ rect:r];
		}

		[ editor setWrapMode:bLineWrapMode ];
		if (mScore_filename.size() && access(mScore_filename.c_str(), R_OK) != -1)
			[ editor loadFile:mScore_filename];
		else if (access(mScore_filename.c_str(), R_OK) != -1)
			[ editor loadFile:TEXT_CONSTANT_TEMP_FILENAME ];

		// add Antescofo internal commands for autocompletion
		if (ofxAntescofoNote->mAntescofo) {
			for (map<string, internal_function>::iterator i = ofxAntescofoNote->mAntescofo->internal_func_map.begin();
			     i != ofxAntescofoNote->mAntescofo->internal_func_map.end(); i++) {
				string cmd = i->first;
				cmd = "antescofo::" + cmd;
				//cout << "autocompletion: adding internal command: " << cmd << endl;
				[editor pushback_keywords:cmd.c_str()];
			}
			for (GlobalEnvironment::dicof_t::iterator i = ofxAntescofoNote->mAntescofo->get_env()->DicoFunctions.begin();
			     i !=  ofxAntescofoNote->mAntescofo->get_env()->DicoFunctions.end(); i++) {
				if (!i->second) break;
				string cmd = i->second->name();
				//cout << "autocompletion: adding internal function: " << cmd << endl;
				[editor pushback_keywords:cmd.c_str()];
			}
		}
	} else {
		if (editor) {
			[ editor die];
			int w = ofGetWidth() - CONSTANT_EDITOR_VIEW_WIDTH;
			cocoaWindow->setWindowShape(w, ofGetHeight());
			score_w = ofGetWindowWidth() - 2*score_x;

			timeline.setWidth(score_w);
			guiBottom->getRect()->width = score_w + score_x;
		}
	}
#endif
}

void ofxAntescofog::save_ColorPicker(string name)
{
    if (mColorChanged.size()) {// color set
        *(colorString2var[mColorChanged]) = mColorPicker.getColor();
        mSaveColorButton->setVisible(false);
				mSaveColorButton->setLabelVisible(false);
        ofxUIWidget *u = guiSetup_Colors->getWidget(mColorChanged);
        if (u) u->setColorBack(mColorPicker.getColor());
        mColorChanged.erase();
        mColorPicker.hide();
        timeline.getColors().load();
    }
}



void ofxAntescofog::draw_ColorPicker(string name)
{
	mColorPicker.setColorRadius(1.0);
	mColorPicker.setColorAngle( 0.5 );

	int x, y, w, h;

	w = 300;
	h = 400;
	x = ofGetWindowWidth() / 2 + w;
	y = ofGetWindowHeight() / 2;

	ofColor c;
	if (colorString2var[name])
		c = *(colorString2var[name]);
	else {
		string str = "Can not find color index for color named ";
		console->addln(str);
	}

	mColorPicker.setColor(c);
	mColorPicker.draw( x-w/2, y-h/2, w, h );
	mColorPicker.addListeners();
	mColorPicker.show();
	mSaveColorButton->setVisible(true);
	mSaveColorButton->setLabelVisible(true);

	ofxUIRectangle *r = mSaveColorButton->getPaddingRect();
	r->x = x + w - 50;
	r->y = y + h + 250;
	mSaveColorButton->update();
}


// draw text and a clickable rect for selecting color
void ofxAntescofog::draw_ColorAsset(string name, ofColor *color)
{
    //guiSetup_Colors->centerWidgets();
    ofSetColor(*color);
    ofxUIButton *b = new ofxUIButton(name, false, 200, 25);
    guiSetup_Colors->addWidgetDown(b);
    b->setColorBack(*color);
}



void ofxAntescofog::draw_ColorSetup()
{
    ofSetColor(0, 0, 0, 100);
    
    ofRect(0, mUIbottom_y, score_w, ofGetWindowHeight() - mUIbottom_y);

    if (!bColorSetupInitDone) {// init and create buttons with color rect

        guiSetup_Colors->addWidgetDown(new ofxUILabel("Choose a color to change then press \"Save color\"", OFX_UI_FONT_LARGE));
        guiSetup_Colors->addWidgetDown(new ofxUILabelToggle(TEXT_CONSTANT_BUTTON_BACK, false, ofGetWindowWidth()/2, score_y, fontsize));
        for (map<string, ofColor*>::const_iterator c = colorString2var.begin(); c != colorString2var.end(); c++)
            draw_ColorAsset(c->first, c->second);
        
        bColorSetupInitDone = true;
    } else {
        guiSetup_Colors->setVisible(true);
        guiSetup_Colors->getWidget(TEXT_CONSTANT_BUTTON_BACK)->setVisible(true);
        ((ofxUILabelToggle*)guiSetup_Colors->getWidget(TEXT_CONSTANT_BUTTON_BACK))->setLabelVisible(true);
        for (map<string, ofColor*>::const_iterator c = colorString2var.begin(); c != colorString2var.end(); c++) {
            guiSetup_Colors->getWidget(c->first)->setVisible(true);
        }
    }
}


void ofxAntescofog::draw_OSCSetup() {
    if (enable_new_window) newWindow();
    ofSetColor(ofxAntescofoNote->color_staves_bg);
    ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    ofSetColor(0, 0, 0, 100);
    ofRect(40, mUIbottom_y, score_w - 40, ofGetWindowHeight() - mUIbottom_y);
    if (!bOSCSetupInitDone) {// init and create buttons with color rect
        guiSetup_OSC->addWidgetDown(new ofxUILabelToggle(TEXT_CONSTANT_BUTTON_BACK, false, ofGetWindowWidth()/2, score_y, fontsize));
	// host
	map<string, string*>::const_iterator c = oscString2var.begin();
	guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	mTextOscHost = new ofxUITextInput(c->first, *(c->second), 300, OFX_UI_FONT_MEDIUM);
	guiSetup_OSC->addWidgetRight(mTextOscHost);
	// port
	c++;
	guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	mTextOscPort = new ofxUITextInput(c->first, *(c->second), 300, OFX_UI_FONT_MEDIUM);
	guiSetup_OSC->addWidgetRight(mTextOscPort);
	// port remote
	c++;
	guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	mTextOscPortRemote = new ofxUITextInput(c->first, *(c->second), 300, OFX_UI_FONT_MEDIUM);
	guiSetup_OSC->addWidgetRight(mTextOscPortRemote);
        
        bOSCSetupInitDone = true;
    } else {
        guiSetup_OSC->setVisible(true);
        guiSetup_OSC->getWidget(TEXT_CONSTANT_BUTTON_BACK)->setVisible(true);
        ((ofxUILabelToggle*)guiSetup_OSC->getWidget(TEXT_CONSTANT_BUTTON_BACK))->setLabelVisible(true);
        for (map<string, string*>::const_iterator c = oscString2var.begin(); c != oscString2var.end(); c++) {
            guiSetup_OSC->getWidget(c->first)->setVisible(true);
        }
    }

}

void ofxAntescofog::draw_FindText() {
    ofSetColor(0, 0, 0, 100);
    ofRect(40, mUIbottom_y, score_w - 40, ofGetWindowHeight() - mUIbottom_y);
    if (!bFindTextInitDone) {// init and create buttons with color rect
        guiFind->addWidgetDown(new ofxUILabelToggle(TEXT_CONSTANT_BUTTON_BACK, false, ofGetWindowWidth()/2, score_y, fontsize));
	guiFind->addWidgetDown(new ofxUILabel("Enter your text to search or replace: ", OFX_UI_FONT_MEDIUM));
	guiFind->addWidgetDown(new ofxUITextInput(TEXT_CONSTANT_BUTTON_TEXT, "", 300, OFX_UI_FONT_MEDIUM));
        guiFind->addWidgetRight(new ofxUILabelToggle(TEXT_CONSTANT_BUTTON_FIND, false, ofGetWindowWidth()/2, score_y, fontsize));
	guiFind->addWidgetDown(new ofxUITextInput(TEXT_CONSTANT_BUTTON_REPLACE_TEXT, "", 300, OFX_UI_FONT_MEDIUM));
        guiFind->addWidgetRight(new ofxUILabelToggle(TEXT_CONSTANT_BUTTON_REPLACE, false, ofGetWindowWidth()/2, score_y, fontsize));
        ((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_BACK))->setLabelVisible(true);
        ((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_FIND))->setLabelVisible(true);
	//((ofxUITextInput*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_TEXT))->setClicked();
	((ofxUITextInput*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_TEXT))->setColorBack(ofColor(200, 0, 0, 100));
	((ofxUITextInput*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE_TEXT))->setColorBack(ofColor(200, 0, 0, 100));
	((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE))->setLabelVisible(true);
	guiFind->addWidgetDown(new ofxUILabel(TEXT_CONSTANT_BUTTON_REPLACE_NB, OFX_UI_FONT_MEDIUM));
	mFindReplaceOccur = new ofxUILabel("0", OFX_UI_FONT_MEDIUM);
	guiFind->addWidgetRight(mFindReplaceOccur);
	mFindReplaceOccur->setVisible(false);
	((ofxUILabel*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE_NB))->setVisible(false);
        bFindTextInitDone = true;
    } else {
        guiFind->setVisible(true);
        guiFind->getWidget(TEXT_CONSTANT_BUTTON_BACK)->setVisible(true);
        ((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_BACK))->setLabelVisible(true);
        guiFind->getWidget(TEXT_CONSTANT_BUTTON_TEXT)->setVisible(true);
        guiFind->getWidget(TEXT_CONSTANT_BUTTON_FIND)->setVisible(true);
	((ofxUILabel*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE_NB))->setVisible(false);
	mFindReplaceOccur->setVisible(false);
	//((ofxUITextInput*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_TEXT))->setClicked();
	((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_FIND))->setLabelVisible(true);
	((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE))->setLabelVisible(true);
    }
}

#define OF_KEY_TAB      9

//--------------------------------------------------------------
void ofxAntescofog::keyPressed(int key){
    if (bLockAscoGraph)
	    return;

    if(0 && key == ' ') {
	    if (bShowError) {
		    bShowError = false;
		    guiError->disable();
		    timeline.enable();
		    guiBottom->enable();
	    } else {
		    timeline.togglePlay();
	    }
    }
#ifdef USE_GUIDO
    if(key == OF_KEY_TAB /*&& !bEditorShow*/) {
        ofxAntescofoNote->toggleView();
	ofxAntescofoBeatTicker->disabled = ofxAntescofoNote->mode_guido();
    }
#endif
    //if(key == 'e'){ setEditorMode(!bEditorShow, 0); } 
    //if(key == 'c'){ console->toggleShow(); }
    else {
	    //ofSetWindowShape(ofGetWidth() - 300, ofGetHeight());
	    //cocoaWindow->setWindowShape(ofGetWidth() - 300, ofGetHeight());
    }
    if (key == OF_KEY_F7){
        ofToggleFullscreen();
    }
	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::keyReleased(int key){
	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::mouseMoved( int x, int y){
	// zoom cursor:
	if (!ofxAntescofoZoom) return;
	static string zoom_icon_path(ofFilePath::getCurrentExeDir() + "../Resources/zoom_cursor.png");
	static NSImage* zoom_img = [[NSImage alloc] initWithContentsOfFile: [NSString stringWithUTF8String:zoom_icon_path.c_str()]];
	static NSCursor* cur = [[NSCursor alloc] initWithImage:zoom_img hotSpot:[[NSCursor currentCursor] hotSpot]];

	NSView *v = [cocoaWindow->delegate getNSView];
	if (timeline.getTrackHeader(ofxAntescofoZoom)->getBounds().inside(x, y) || ofxAntescofoZoom->getBounds().inside(x, y)) {
		NSRect rect = NSMakeRect( timeline.getTrackHeader(ofxAntescofoZoom)->getBounds().x, timeline.getTrackHeader(ofxAntescofoZoom)->getBounds().y,
				ofxAntescofoZoom->getBounds().width, timeline.getTrackHeader(ofxAntescofoZoom)->getBounds().height + ofxAntescofoZoom->getBounds().height);
		[v resetCursorRects];
		[v addCursorRect:rect cursor:cur];
		[cur set];
		bMouseCursorSet = true;
	} else if (bMouseCursorSet) {
		bMouseCursorSet = false;
		[v resetCursorRects];
		[v discardCursorRects];
		[[NSCursor arrowCursor] set];
	}

	// redraw if in action track elevator
	if (ofxAntescofoNote && ofxAntescofoNote->getActionTrack())
		if (ofxAntescofoNote->getActionTrack()->mElevatorRect.inside(x, y))
			bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::mousePressed( int x, int y, int button ) {
	cout << "mouse: " << x << " : " << y << endl;
	//if (args.button == 3) { }
	bShouldRedraw = true;
    //cout << "Fog : mousePressed r:"<< mEditorRect.x << ","<< mEditorRect.y << ","<< mEditorRect.width << "," << mEditorRect.height <<" inside:"<< mEditorRect.inside(x, y) << endl;
}

//--------------------------------------------------------------
void ofxAntescofog::mouseDragged( int x, int y, int button ){
	int topy = guiBottom->getRect()->y + guiBottom->getRect()->height;

	bShouldRedraw = true;
}


//--------------------------------------------------------------
void ofxAntescofog::mouseReleased( int x, int y, int button ){
	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::windowResized(ofResizeEventArgs& resizeEventArgs) { // (int w, int h){
	cout << "ofxAntescofog::windowResized: "<< resizeEventArgs.width << "x"<< resizeEventArgs.height << endl;
	if (!guiBottom) return; //setup();

#ifdef TARGET_OSX
	NSView *glview = [cocoaWindow->delegate getNSView];
	[glview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

	glViewport( 0, 0, ofGetWidth(), ofGetHeight());

	cocoaWindow->setWindowShape(ofGetWidth(), ofGetHeight());

	score_w = resizeEventArgs.width - score_x - 5; 
	guiBottom->getRect()->width = score_w + score_x - 300;

	drawCache.allocate(resizeEventArgs.width, ofGetHeight(), GL_RGBA);

	drawCache.begin();
	ofClear(255,255,255, 0);
	drawCache.end();
#endif
	timeline.windowResized(resizeEventArgs);

	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::gotMessage(ofMessage msg){

}

void ofxAntescofog::draw_error()
{
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    //ofSetColor(255, 255, 255, 240);
    ofSetColor(0, 0, 0, 255);

    guiError->draw();

    string err = ofxAntescofoNote->get_error();
   // cout << "draw_error: err=" << err << endl;
    string err2;
    int c = 0;
    for (int i = 0; i < err.size(); i++, c++) {
	    if (c % 120 == 0) {
		    err2 += "\n";
		    c = 0;
	    }
	    if (err[i] == '\n')
		    c = 0;
	    err2 += err[i];
    }
    //ofSetColor(255, 255, 255, 200);
    ofSetColor(0, 0, 0, 255);
    ofFill();
    ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
    ofDrawBitmapString(err2, 50, 300);

}

void ofxAntescofog::display_error()
{
    bShowError = true;
    timeline.disable();
    timelineSim.disable();
    guiBottom->disable();
    guiError->enable();
    guiError->setVisible(true);
    if (!bErrorInitDone){
	string hdr(bIsSimulating ? TEXT_CONSTANT_SIMULATION_ERROR : TEXT_CONSTANT_PARSE_ERROR);
        guiError->addWidgetRight(new ofxUILabel(ofGetWindowWidth()/2, 100, hdr, OFX_UI_FONT_MEDIUM));
        string err = ofxAntescofoNote->get_error();
        ofDrawBitmapString(ofxAntescofoNote->get_error(), 100, 100);
        bErrorInitDone = true;
        guiError->addWidget(new ofxUILabelToggle(TEXT_CONSTANT_BUTTON_CANCEL, false, ofGetWindowWidth()/2, score_y, 200, 600, fontsize));
    } else {
        guiError->getWidget(TEXT_CONSTANT_BUTTON_CANCEL)->setState(0);
        guiError->getWidget(TEXT_CONSTANT_BUTTON_CANCEL)->setVisible(true);
        ((ofxUILabelToggle*)guiError->getWidget(TEXT_CONSTANT_BUTTON_CANCEL))->setLabelVisible(true);
    }
}


//--------------------------------------------------------------
void ofxAntescofog::dragEvent(ofDragInfo dragInfo){
	if( dragInfo.files.size() > 0 ){
		for(int i = 0; i < dragInfo.files.size(); i++){
			// try audio files
			string audiofile = dragInfo.files[i];
			string ext = ofFilePath::getFileExt(audiofile);
			if (ext == "aif" || ext == "aiff" || ext == "wav") {
				if (!audioTrack)
					audioTrack = new ofxTLAccompAudioTrack();
				if (audioTrack->loadSoundfile(audiofile)) {
					ofxAntescofoNote->clear_error();
					bShowError = false;
					timeline.addTrack(dragInfo.files[i], audioTrack);
					audioTrack->enable();
					//timeline.setDurationInSeconds(audioTrack->getDuration());
					// the 2 following 2 lines should not be done if editing markers 
					//timeline.setTimecontrolTrack(audioTrack);
					ofxAntescofoNote->getAccompanimentMarkers(accomp_map_index, accomp_map_markers);
					audioTrack->setVolume(.0);
					audioTrack->setMarkers(accomp_map_index, accomp_map_markers);
					timeline.setPercentComplete(0);
					break;
				}
			}
			ofxAntescofoNote->clear();
			string str = "ofxAntescofog: trying to load file :" + dragInfo.files[i];
			console->addln(str);
			int n = loadScore(string(dragInfo.files[i]), true);

			if (!n)
				return;
		}
	}
	bShouldRedraw = true;
}


void ofxAntescofog::exit()
{
    guiBottom->saveSettings("GUI/guiSettings.xml");
    save();
    delete guiBottom;
}


void ofxAntescofog::saveScore(bool stopSimu) {
	// for now just save the content of the text editor
	if (bEditorShow) {
		if (bIsSimulating && stopSimu) {
			bIsSimulating = false;
			stop_simulate_and_goedit();
		}
		if (mScore_filename.empty()) {
			saveAsScore();
			return;
		}
		string s;
		[ editor getEditorContent:s ];
		if (!s.empty()) {
			// save and try to parse
			string scorefilename = [ editor tab_get_filename ];
			cout << "Saving score: " << scorefilename << endl;
			ofstream outfile;
			outfile.open (scorefilename.c_str());
			outfile << s;
			outfile.close();
			loadScore(mScore_filename, true);
		}
	}


}

bool ofxAntescofog::edited() {
	string editorcontent;
	[ editor getEditorContent:editorcontent ];

	string filecontent;
	string scorefilename = [ editor tab_get_filename ];
	ifstream file;
	file.open (scorefilename.c_str(), ios::ate);
	int fsize;
	fsize = (int) file.tellg();
	cout << "file: " << scorefilename << " fsize=" << fsize << endl;
	file.seekg (0, ios::beg);
	if (fsize <= 0)
		return false;
	char* memblock = new char[fsize + 1];
	file.read (memblock, fsize);
	memblock[fsize] = 0;
	filecontent = string(memblock);
	file.close();

	bool same = (filecontent.compare(editorcontent) == 0);
	delete memblock;
	cout << "original file len:" << fsize << endl;
	if (same)
		return false;
	else
		return true;
}

int ofxAntescofog::draw_asksave_window()
{
	string scorefilename = [ editor tab_get_filename ];
	string question = "Save changes to \"";
	question += scorefilename;
	question += "\" before closing ?";

	// create alert dialog
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"Cancel"];
	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Don't save"];
	[alert setMessageText:[NSString stringWithCString:question.c_str() encoding:NSUTF8StringEncoding]];
	NSInteger returnCode = [alert runModal];
	// if OK was clicked, assign value to text
	if ( returnCode == NSAlertFirstButtonReturn )
		return INT_CONSTANT_BUTTON_CANCEL_SAVE; // cancel
	
	if ( returnCode == NSAlertSecondButtonReturn )
		return INT_CONSTANT_BUTTON_OK_SAVE; // OK

	if ( returnCode == NSAlertThirdButtonReturn )
		return INT_CONSTANT_BUTTON_DONT_SAVE; // dont save
	return 0;
}


// return true when score is saved
int ofxAntescofog::askToSaveScore() {
	if (edited()) {
		int ret = draw_asksave_window();

		cout << " return => " << ret << endl;
		if (ret == INT_CONSTANT_BUTTON_OK_SAVE) {
			saveScore();
		}
		return ret;

		cout << "askToSaveScore: file modified" << endl;
		//saveAsScore();
	} else
		cout << "askToSaveScore: file not modified" << endl;
	return 0;
}


void ofxAntescofog::newScore() {
	[ editor clear ];
	
	ofxAntescofoNote->clear_actions();
	if (ofxJumpTrack) {
		ofxJumpTrack->clear_jumps();
		if (ofxJumpTrack) timeline.removeTrack(ofxJumpTrack);
		if (ofxJumpTrack) delete ofxJumpTrack;
		ofxJumpTrack = NULL;
	}
	mScore_filename = "";
}

void ofxAntescofog::saveAsScore() {
	if (bEditorShow) {
		string s;
		[ editor getEditorContent:s ];
		if (!s.empty()) {
			ofFileDialogResult openFileResult = ofSystemSaveDialog("ascograph-score.txt", TEXT_CONSTANT_TITLE_SAVE_AS_SCORE);
			if (openFileResult.bSuccess){
				string f = openFileResult.filePath;
				cout << "Selected file: " << f << endl;
				ofLogVerbose("Selected file: " + f);
				// save and try to parse
				// show dialog confirming
				mScore_filename = f;
				ofstream outfile;
				outfile.open (mScore_filename.c_str());
				outfile << s;
				outfile.close();
				loadScore(mScore_filename, false);
			} else {
				ofLogVerbose("Cancel load score hit.");
			}
		}
	}
}


int ofxAntescofog::loadScore(string filename, bool reloadEditor, bool sendOsc) {
	console->clear();
	ofxAntescofoNote->clear_error();
	cout << "Trying to load score (reload=" << reloadEditor << ": " << filename << endl;
	// save zoom view range
	ofRange z = ofxAntescofoZoom->getViewRange(); 
	mOsc_beat = -1;

	// save editor position
	int lineEditorPos = [ editor getCurrentPos];
	int lineEditor = [ editor getCurrentLine];
	int lineEditorScroll = 0; //[ editor getCurrentPosScroll];

	//cout << "loadScore: lineEditor: " << lineEditor <<" linEScroll: " << lineEditorScroll << endl;
	string antescore = filename;
	ofxAntescofoNote->clear_actions();

	// if it's a midi file, convert it to Antescofo language
	string ext = ofFilePath::getFileExt(filename);
	int n = 0;
	bool wasxml = false;
	if (ext == "MID" || ext == "mid") {
		setup_Midi(filename, ofGetModifierPressed(OF_MODIFIER_KEY_SHIFT) || ofGetModifierPressed(OF_MODIFIER_KEY_SPECIAL));
	} else if (ext == "XML" || ext == "xml") {
		n = ofxAntescofoNote->loadscoreMusicXML(filename, TEXT_CONSTANT_TEMP_FILENAME);
		wasxml = true;
		mScore_filename = TEXT_CONSTANT_TEMP_FILENAME;
	}
	if (!n) {
		mScore_filename = antescore = filename;
		n = ofxAntescofoNote->loadscoreAntescofo(antescore);
		if (!n && wasxml) mScore_filename = TEXT_CONSTANT_TEMP_FILENAME;
	}
	if (n || ofxAntescofoNote->get_error().empty()) {
		bShowError = false;
		guiError->disable();
		timeline.enable();
		guiBottom->enable();

		// ensure zoom is restored
		ofxAntescofoNote->setZoomBounds(z);
		ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
		if (!actiontrack) actiontrack = ofxAntescofoNote->createActionTrack();
		if (actiontrack) actiontrack->setZoomBounds(z);
		timeline.getTicker()->setZoomBounds(z);

		bpm = timeline.getBPM();
		// get cuepoints
		cues_clear_menu();
		for (vector<string>::iterator i = ofxAntescofoNote->cuepoints.begin(); i != ofxAntescofoNote->cuepoints.end(); i++) {
			cues_add_menu(*i);
		}
		update();

		if (sendOsc && !bScoreFromCommandLine) {
			// send OSC read msg to Antescofo
			ofxOscMessage m;
			m.setAddress("/antescofo/cmd");
			m.addStringArg("read");
			m.addStringArg(mScore_filename);
			cout << "Sending OSC \"read "<< mScore_filename << "\" to Antescofo Patch." << endl;
			mOSCsender.sendMessage(m);
		} else bScoreFromCommandLine = false;
	} else {
		//if (ofxAntescofoNote->get_error().empty()) ofxAntescofoNote->set_error("Zero event found in score.");
		display_error();
	}
#ifndef IOS_ASCOGRAPH
	NSString *fileNS = [NSString stringWithCString:filename.c_str() encoding:[NSString defaultCStringEncoding]];
	[[cocoaWindow->delegate getNSWindow] setTitleWithRepresentedFilename:fileNS];

	// add to recent files:
	if (mScore_filename.size() && (mRecentFiles.empty() || mRecentFiles.back() != mScore_filename)) {
		mRecentFiles.push_back(filename);
		save_appsupport(getApplicationSupportSettingFile());
		populateOpenRecentMenu();
	}
#endif
	// update editor if opened
	if (bEditorShow && reloadEditor) {
		[[NSNotificationCenter defaultCenter] removeObserver:[NSApp delegate] name:NSTextDidChangeNotification object:nil];

		[ editor loadFile:mScore_filename ];

		[[NSNotificationCenter defaultCenter] addObserver:[NSApp delegate] selector:@selector(receiveNotification:) name:NSTextDidChangeNotification object:nil];
		cout << "Editor scrolling to pos: " << lineEditor << " scroll:"<< lineEditorScroll << endl;
		if (lineEditorPos)
			[ editor gotoPos:lineEditorPos];
		int totLines = [ editor getMaxLines ];
		if (lineEditor > totLines/2) {
			[ editor setCurrentPos:lineEditorPos]; //[ editor scrollLine:lineEditor];
			[ editor scrollLine:totLines/2]; //[ editor scrollLine:lineEditor];
		}
		//int n = [ editor getNbLines ];
		//[ editor scrollLine:(30) ];
	}

	// jumps track
	showJumpTrack();

	bShouldRedraw = true;
#ifdef USE_HTTPD
	imageSaved = false;
#endif
	return n;
}

void ofxAntescofog::showJumpTrack() {
	// check if we need to show jump tracks:
	bool bShowJumpTrack = false; 
	vector<ofxTLAntescofoNoteOn*>& switches = ofxAntescofoNote->getSwitches();
	for (vector<ofxTLAntescofoNoteOn*>::iterator i = switches.begin();
			i!= switches.end(); i++)
		if ((*i)->jump_dests.size()) {
			bShowJumpTrack = true;
			break;
		}

	if (!bShowJumpTrack) {
		if (ofxJumpTrack) timeline.removeTrack(ofxJumpTrack);
		if (ofxJumpTrack) delete ofxJumpTrack;
		ofxJumpTrack = NULL;
		return;
	}

	if (ofxJumpTrack == 0) {
		ofxJumpTrack = new ofxTLBeatJump(this);
		timeline.addTrack("Jumps", ofxJumpTrack);
		timeline.bringTrackToPos(ofxJumpTrack, 2);
	}

	ofxJumpTrack->setZoomBounds(ofxAntescofoZoom->getViewRange());
	ofxJumpTrack->clear_jumps();

	// for every events
	switches = ofxAntescofoNote->getSwitches();
	for (vector<ofxTLAntescofoNoteOn*>::iterator i = switches.begin();
			i!= switches.end(); i++) {
		// for every jump dest
		for (int n = 0; n < (*i)->jump_dests.size(); n++) {
			float destBeat = (*i)->jump_dests[n];
			cout << "showJumpTrack: adding jump: beat:" << (*i)->beat.min << " destBeat:" << destBeat << " label:" << (*i)->label <<endl;
			ofxJumpTrack->add_jump((*i)->beat.min, destBeat, "");
		}
	}
}

void ofxAntescofog::guiEvent(ofxUIEventArgs &e)
{
    //cout << "got guiEvent("<< e.widget->getName() <<")"<< endl;
    // load score
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_LOAD)
		{
        ofFileDialogResult openFileResult = ofSystemLoadDialog(TEXT_CONSTANT_TITLE_LOAD_SCORE);
        if (openFileResult.bSuccess){
			string f = openFileResult.filePath;
			string str = "Selected file: " + f;
			console->addln(str);
			loadScore(mScore_filename, true);
			ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
			b->setValue(false);
				} else {
					console->addln("Cancel load score hit.");
				}
		}
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_RELOAD)
    {
            loadScore(mScore_filename, false);
            ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
            b->setValue(false);
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_SAVE)
		{
        console->addln("Save score : TODO");
    }
    // Colors...
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_COLOR_SETUP || e.widget->getName() == TEXT_CONSTANT_BUTTON_COLOR_SETUP_EXIT)
	{
        if (!bShowColorSetup) {
            timeline.disable();
            bShowColorSetup = true;
            guiBottom->disable();
            e.widget->setName(TEXT_CONSTANT_BUTTON_COLOR_SETUP_EXIT);
        } else {
						bShouldRedraw = true;
            timeline.enable();
            bShowColorSetup = false;
            guiBottom->enable();
            mColorPicker.removeListeners();
            e.widget->setName(TEXT_CONSTANT_BUTTON_COLOR_SETUP);
            guiSetup_Colors->setVisible(false);
            save();
        }
    } else if(e.widget->getName() == TEXT_CONSTANT_BUTTON_OSC_SETUP || e.widget->getName() == TEXT_CONSTANT_BUTTON_OSC_SETUP_EXIT)
	{
        if (!bShowOSCSetup) {
            timeline.disable();
            bShowColorSetup = false;
            bShowOSCSetup = true;
            guiBottom->disable();
            e.widget->setName(TEXT_CONSTANT_BUTTON_OSC_SETUP_EXIT);
        } else {
            timeline.enable();
						bShouldRedraw = true;
            //ofxAntescofoNote->disable();
            bShowOSCSetup = false;
            guiBottom->enable();
            mColorPicker.removeListeners();
            e.widget->setName(TEXT_CONSTANT_BUTTON_OSC_SETUP);
            guiSetup_OSC->setVisible(false);
            save();
        }
#ifdef USE_GUIDO
    } else if(e.widget->getName() == TEXT_CONSTANT_BUTTON_TOGGLE_VIEW)
	{
        ofxAntescofoNote->toggleView();
	ofxAntescofoBeatTicker->disabled = ofxAntescofoNote->mode_guido();
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
#endif
    } else if(e.widget->getName() == TEXT_CONSTANT_BUTTON_TOGGLE_EDITOR)
	{
        setEditorMode(!bEditorShow, 0);
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
    }
    else if (e.widget->getName().compare(0, 7, "colors:") == 0) {
        mColorChanged = e.widget->getName();
        draw_ColorPicker(e.widget->getName());
    }
    else if (e.widget->getName() == TEXT_CONSTANT_BUTTON_SAVE_COLOR) {
        save_ColorPicker(e.widget->getName());
    }
    //Buttons
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_BPM)
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		bpm = slider->getScaledValue();
		cout << "BPM slider change: " << bpm << endl;
        timeline.setBPM(bpm);
        timeline.enableSnapToBPM(bSnapToGrid);
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_SNAP)
	{
		ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
		cout << "Snap to grid button change: " << b->getValue() << endl;
		bSnapToGrid = b->getValue();
   
        timeline.setShowBPMGrid(bSnapToGrid);
        timeline.enableSnapToBPM(bSnapToGrid);
	}
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_AUTOSCROLL)
	{
		ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
		cout << "Auto Scroll button change: " << b->getValue() << endl;
		bAutoScroll = b->getValue();
        
        ofxAntescofoNote->setAutoScroll(bAutoScroll);
	}
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_SIMULATE && enable_simulate)
    {
	    ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
	    cout << "Simulate button change: " << b->getValue() << endl;
	    b->setValue(0);
	    bIsSimulating = true;
	    simulate();
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_EDIT && enable_simulate)
    {
	    ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
	    cout << "Edit button change: " << b->getValue() << endl;
	    b->setVisible(false);
	    b->setLabelVisible(false);
	    b->setValue(0);
	    bIsSimulating = false;
	    stop_simulate_and_goedit();
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_PLAY)
    {
	    ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
	    cout << "Play button change: " << b->getValue() << endl;
	    if (b->getValue() == 1) {
		    ofxOscMessage m;
		    m.setAddress("/antescofo/cmd");
		    if (mGotoPos) {
			    m.addStringArg("playfrombeat");
			    m.addFloatArg(mGotoPos);
		    } else if (mPlayLabel.size()) {
			    cout << "mPlayLabel: " << mPlayLabel << endl;
			    m.addStringArg("playfrom");
			    m.addStringArg(mPlayLabel);
		    } else
			    m.addStringArg("play");
		    mOSCsender.sendMessage(m);
		    b->setValue(false);
	    }
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_START)
    {
	    ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
	    cout << "Start button change: " << b->getValue() << endl;
	    if (b->getValue() == 1) {
		    ofxOscMessage m;
		    m.setAddress("/antescofo/cmd");
		    if (!mGotoPos) {
			    m.addStringArg("start");
			    m.addStringArg("");
		    } else {
			    m.addStringArg("gotobeat");
			    m.addFloatArg(mGotoPos);
		    }
		    mOSCsender.sendMessage(m);
		    b->setValue(false);
	    }
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_NEXT_EVENT || e.widget->getName() == TEXT_CONSTANT_BUTTON_PREV_EVENT)
    {
	    ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
	    cout << "Prev/next event button change: " << b->getValue() << endl;
	    if (b->getValue() == 1) {
		    ofxOscMessage m;
		    m.setAddress("/antescofo/cmd");
		    if (e.widget->getName() == TEXT_CONSTANT_BUTTON_NEXT_EVENT)
			    m.addStringArg("nextevent");
		    if (e.widget->getName() == TEXT_CONSTANT_BUTTON_PREV_EVENT)
			    m.addStringArg("previousevent");
		    mOSCsender.sendMessage(m);
		    b->setValue(false);
	    }
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_STOP)
	{

		mGotoPos = 0.;
		ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
		cout << "Stop button change: " << b->getValue() << endl;
        if (b->getValue() == 1) {
            ofxOscMessage m;
            m.setAddress("/antescofo/cmd");
            m.addStringArg("stop");
	    mPlayLabel.clear();
            mOSCsender.sendMessage(m);
            b->setValue(false);
        }
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_CANCEL)
	{
        cout << "Cancel pressed" << endl;
        bShowError = false;
        e.widget->toggleVisible();
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
	guiError->disable();
        if (!bIsSimulating)
		timeline.enable();
	else timelineSim.enable();
        guiBottom->enable();
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_BACK)
	{
        cout << "Back pressed" << endl;
        if (bShowColorSetup) {
            bShowColorSetup = false;

            guiSetup_Colors->disable();
	    guiSetup_Colors->setVisible(false);
        } else if (bShowOSCSetup) {
            bShowOSCSetup = false;

	    mOsc_host = mTextOscHost->getTextString();
	    mOsc_port = mTextOscPort->getTextString();
	    mOsc_port_MAX = mTextOscPortRemote->getTextString();
            guiSetup_OSC->disable();
	    setupOSC();
        }/* else if (bShowFind) {
	    bShowFind = false;
	    guiFind->disable();
	}*/

        e.widget->toggleVisible();
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
        timeline.enable();
        guiBottom->enable();
    }
#if 0
    if(e.widget->getName() == "elevator")
    {
	    ofxUIRangeSlider *r = (ofxUIRangeSlider*)e.widget;
	    double m = (r->getPercentValueLow() + r->getPercentValueHigh()) / 2;
	    int score_h = timeline.getDrawRect().height;
	    int topy = guiBottom->getRect()->y;
	    double ny = (m - 0.75) * score_h + topy;
	    //cout << "Elevator clicked: medium:" << m << " score_y: " << score_y << " ny:" << ny << endl;
	    if (ny + score_h < topy)
		    ny = score_y + 200;
	    if (topy + ny > ofGetWindowHeight())
		    ny = ofGetWindowHeight() + topy - score_h;
	    score_y = ny;
    }
#endif
}

void ofxAntescofog::editorTextDidChange()
{
	if (editor) {
		[editor tabEdited];
	}
}

void ofxAntescofog::editorDoubleclicked(int line)
{
	ofxAntescofoNote->showNote(line);
	[ editor braceMatch ];
	bShouldRedraw = true;
}


void ofxAntescofog::editorShowLine(int linea, int lineb, int cola, int colb)
{
	if (bEditorShow) {
		[ editor showLine:linea-1 lineb:lineb-1 cola:cola-1 colb:colb-1];
		int n = [ editor getNbLines ];
		if (linea + 35 < n)
			[ editor scrollLine:(-35) ];

		bShouldRedraw = true;
	}
}


void ofxAntescofog::replaceEditorScore(int linebegin, int lineend, int cola, int colb, string actstr) {
	if (bEditorShow && actstr.size() && linebegin <= lineend) {
		cout << "ofxAntescofog::replaceEditorScore: replacing between line " << linebegin-1 << ":" << cola << " -> " << lineend-1 << ":" << colb << " str:" << actstr << endl;
		[ editor replaceString:linebegin-1 lineb:lineend-1 cola:cola-1 colb:colb-1 str:actstr.c_str()];
		//editorShowLine(linebegin, lineend);

		bShouldRedraw = true;
	}

}

void ofxAntescofog::createCodeTemplate(int which)
{
	int pos = [ editor getCurrentPos ];
	string str;

	switch (which) {
		case INT_CONSTANT_BUTTON_CREATE_GROUP:
			str = "0. group /* name @tempo=expr (default $RT_TEMPO), @tight (default: @loose), @local (default @global) */ {\n\t1. action1\n\t1/4 action2\n   \n\t;...\n}\n";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		case INT_CONSTANT_BUTTON_CREATE_LOOP:
			str = "0. loop 6.66 /* name @tempo=expr (default $RT_TEMPO), @tight (default: @loose), @local (default @global) */ {\n\t1. action1\n\t1/4 action2\n   \n\t;...\n} /*until(expr)*/\n";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		case INT_CONSTANT_BUTTON_CREATE_CURVE:
			str = "curve slider  @Grain := 0.05s, @Action := print $x $y\n{\n\t$x, $y\n\t{\n\t    { 0. 2. } /*@type \"exp\"*/\n\t1   { 1. 0. }\n\t2/5 { 3. 1.4}\n\t; ...\n\t}\n}\n";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		case INT_CONSTANT_BUTTON_CREATE_CURVES:
			str = "curve @action := plot $NOW $x $y $z {\n\t$x\n\t{\t\t{ 2.0 }\n\t\t1.0\t{ 4.0 }\n\t\t1.0\t{ 1.4 }\n\t}\n\t$y, $z\n\t{\t\t{5.0, 4.0}\n\t\t3.0\t{0.3 ,7.0}\n\t}\n}\n";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		case INT_CONSTANT_BUTTON_CREATE_WHENEVER:
			str = "whenever(/*expr*/) {\n\t1. action1\n\t1/4 action2\n\t; ...\n} /*until(expr)*/\n";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		case INT_CONSTANT_BUTTON_CREATE_OSCSEND:
			str = "oscsend test \"localhost\" :3004 \"/antescofo/hello2\"";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		case INT_CONSTANT_BUTTON_CREATE_OSCRECV:
			str = "oscrecv receivername 3007 \"/\" $varname";
			[ editor insertStringAtPos:pos posb:pos str:str.c_str() ];
			break;

		default:
			;
	}
}

void AntescofoTimeline::setZoomer(ofxTLZoomer *z)
{
	//XXX if (zoomer) removeTrack(zoomer);
	delete zoomer;
	zoomer = z;
	zoomer->setTimeline(this);
	// zoomer->setDrawRect(ofRectangle(offset.y, tabs->getBottomEdge(), width, 30));
	zoomer->setup();

	bringTrackToTop(zoomer);
	bringTrackToTop(zoomer);
}



///////////////////////////////////
//// MIDI conversion
//////////////////////////////////
void ofxAntescofog::setup_Midi(string& filename, bool do_actions)
{
	cout << "Trying to convert a MIDI file: " << filename << endl;

	string outstr;

	if (do_actions)
		outstr = convertMidiFileToActions(filename);
	else
		outstr = convertMidiFileToNotes(filename);

	cerr << "converted score is:" << endl << outstr << endl;

	ofstream outfile;
	string outfilename = TEXT_CONSTANT_TEMP_FILENAME;

	outfile.open(outfilename.c_str());
	outfile << outstr;
	outfile.close();
	
	filename = outfilename;
}

string ofxAntescofog::convertMidiFileToNotes(string& midifile) {
	NSString *midifileNS = [NSString stringWithCString:midifile.c_str() encoding:[NSString defaultCStringEncoding]];
	NSData* midicontent = [NSData dataWithContentsOfFile:midifileNS ];
	ofxMidiParser* midiParser = [[ofxMidiParser alloc] init];
	
	int num = -1;
	NSString *ret = [midiParser getAntescofoNotesForTrack:midicontent trackWanted:num];

	string rets([ret UTF8String]);
	return rets;
}

string ofxAntescofog::convertMidiFileToActions(string& midifile) {
	NSString *midifileNS = [NSString stringWithCString:midifile.c_str() encoding:[NSString defaultCStringEncoding]];
	NSData* midicontent = [NSData dataWithContentsOfFile:midifileNS ];
	ofxMidiParser* midiParser = [[ofxMidiParser alloc] init];
	
	int num = -1;
	NSString *ret = [midiParser getAntescofoActionsForTrack:midicontent trackWanted:num];

	string rets([ret UTF8String]);
	return rets;
}


#ifdef USE_HTTPD
//////////////////////////////////////
///// HTTP server
//////////////////////////////////////
void ofxAntescofog::draw_http_image() {
	if(!imageSaved){
		int x = 4;
		int y = ofxAntescofoNote->getBounds().y - x + 5;
		int w = ofxAntescofoNote->getBounds().width;
		int h = ofxAntescofoNote->getBounds().height + 30;
		image.grabScreen(x, y, w, h);

		image.saveImage("www/screen.jpg");
		imageSaved = true;
	}

}

void ofxAntescofog::update_http_image() {
	if(postedImgFile!=prevPostedImg){
		postedImg.loadImage("upload/" + postedImgFile);
		prevPostedImg = postedImgFile;
	}
}


void ofxAntescofog::setup_httpd() {
	image.allocate(score_w+10 , ofGetWindowHeight(),OF_IMAGE_COLOR);
	imageSaved  = false;
	httpd_server = ofxHTTPServer::getServer(); // get the instance of the server
	httpd_server->setServerRoot("www");		 // folder with files to be served
	httpd_server->setUploadDir("upload");		 // folder to save uploaded files
	httpd_server->setCallbackExtensions("of");	 // extension of urls that aren't files but will generate a post or get event
	httpd_server->setListener(*this);

	httpd_server->start(8888);

	mOSCsender_www.setup("localhost", 8889);
}

void ofxAntescofog::getRequest(ofxHTTPServerResponse & response){
	cout << "++++++++++ got request: " << response.url << endl;
	if(response.url=="/"){
		response.response="<html> \
			<script type='text/javascript' src='barTest.js'> \
				   function beginrefresh(){ \
					   setTimeout(\"window.location.reload();\",83); \
				   }\
				   window.onload=beginrefresh; \
				BarTest(); \
			</script>\
			</head> <body> <img src=\"screen.jpg\"/> </body></html>";


		imageSaved  = false;
	}
}

void ofxAntescofog::postRequest(ofxHTTPServerResponse & response){
	cout << "++++++++++ got post request: " << response.url << endl;
	if(response.url=="/postImage.of"){
		postedImgName = response.requestFields["name"];
		postedImgFile = response.uploadedFiles[0];
		response.response = "<html> <head> oF http server </head> <body> image " + response.uploadedFiles[0] + " received correctly <body> </html>";
	}
}

void ofxAntescofog::httpd_update_beatpos()
{
	ofxOscMessage m;
	m.setAddress("/antescofo/event_beatpos");
	float n = (mOsc_beat - 1.) / ofxAntescofoNote->get_max_note_beat();
	cout<< "Sending OSC event beat pos: " << mOsc_beat << " :" << n << " : " << ofxAntescofoNote->get_max_note_beat() << endl;
	char b[200];
	getcwd(b, 200);
	cout << b << endl;
	ofstream ofs ("../Resources/www/pos", ofstream::trunc);
	
	ofs << n << endl;

	ofs.close();
}
#endif //USE_HTTPD



void ofxAntescofog::push_tempo_value() {
	static unsigned long long lasttime_pushed = ofGetSystemTimeMicros();
	
	unsigned long long nownow;
	nownow = ofGetSystemTimeMicros();

	if (nownow - lasttime_pushed > 50000) {
		lasttime_pushed = nownow;
		// shift buffer values by one to the left
		for (int i = 0; i < 255; i++)
			mBPMbuffer[i] = mBPMbuffer[i+1];
		mBPMbuffer[255] = mOsc_tempo;
	}
}

void ofxAntescofog::cues_clear_menu() {
	mCuesIndexToString.clear();
	mCuesMaxIndex = 0;

	[mCuesMenu removeAllItems];
}

void ofxAntescofog::cues_add_menu(string c) {
	mCuesMaxIndex++;
	mCuesIndexToString[mCuesMaxIndex] = c;
	id menuItem = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:c.c_str()] action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[menuItem setTag:mCuesMaxIndex + INT_CONSTANT_BUTTON_CUES_INDEX];

	[mCuesMenu addItem:menuItem];
}

class ofxAntescofoSetup : public ofxNSWindowApp {
public:
        ofxAntescofoSetup() { cout << "ofxAntescofoSetup::ctor " << endl;}
        
        void setup() { cout << "ofxAntescofoSetup::setup()" << endl; }
        void update() {}
        void draw() { cout << "ofxAntescofoSetup::draw()" << endl; }
        
        void keyPressed(int key) {}
        void keyReleased(int key) {}
        
        void mouseMoved(int x, int y) {}
        void mouseDragged(int x, int y, int button) {}
        void mousePressed(int x, int y, int button) {}
        void mouseReleased() {}
        void mouseReleased(int x, int y, int button) {}
        
        void mouseScrolled(float x, float y) {}
};

void ofxAntescofog::newWindow() {
	if (!subWindow) {
		cout << "Creating new window" << endl;

		/*
		NSRect frame = NSMakeRect(0, 0, 200, 200);
		subWindow  = [[[NSWindow alloc] initWithContentRect:frame
			styleMask:NSBorderlessWindowMask
			backing:NSBackingStoreBuffered
			defer:NO] autorelease];
		[subWindow setBackgroundColor:[NSColor blueColor]];
		[subWindow makeKeyAndOrderFront:subWindow];
		[subWindow setIdentifier:@"Preferences"];

		NSView* glview = [[NSView alloc] initWithFrame:frame];

		//setup and display the window
		[subWindow setContentView:glview];
		[subWindow makeKeyAndOrderFront:nil];
		[glview release];
		NSWindowDelegate* windowDelegate = [[NSWindowDelegate alloc] init];
		[windowDelegate setApp:NSApp];
		[windowDelegate setView:[window contentView]];
		[window setDelegate:windowDelegate];
		[window setReleasedWhenClosed:YES];
		*/

		//subWindow;
		/*
		ofRemoveListener(ofEvents().windowResized, this, &ofxAntescofog::windowResized);
		subWindow = new ofxCocoaWindow();
		subWindow->setupOpenGL(300, 200, OF_WINDOW); 
		//ofRunApp(new ofxAntescofoSetup(subWindow, this));
		*/

		ofxNSWindower::instance()->addWindow(/*new ofxAntescofoSetup()*/ this, "Setup");
	}
}


