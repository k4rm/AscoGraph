#include <sstream>
#include <string>
#include <sys/time.h>

//#include "ofxConsole.h"
#include "ofxHotKeys.h"
#include "ofxAntescofog.h"
#include "ofxCocoaWindow.h"
#include "ofxTLBeatTicker.h"
#include "ofxMidiparser.h"

bool enable_simulate = true;
bool disable_httpd = true;

int _debug = 0;
static string str_error; // filled by our error()

int fontsize = OFX_UI_FONT_SMALL;
//int fontsize = OFX_UI_FONT_MEDIUM;

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
	bOSCSetupInitDone = false;
	bShowError = false;
	bShowFind = false;
	bErrorInitDone = false;
	bEditorShow = true;
	bSetupDone = false;
	editor = 0;
	bShouldRedraw = true;
	bLineWrapMode = false;
	bIsSimulating = false;
	audioTrack = NULL;
	ofxAntescofoSim = 0;

	if (argc > 1 && argv[1][0] != '-')
		mScore_filename = argv[1];

	gettimeofday(&last_draw_time, 0);
}


void ofxAntescofog::menu_item_hit(int n)
{
	//cout << "menu = " << n << endl;
	switch (n) {
		case INT_CONSTANT_BUTTON_LOAD:
			{
				ofFileDialogResult openFileResult = ofSystemLoadDialog(TEXT_CONSTANT_TITLE_LOAD_SCORE);
				if (openFileResult.bSuccess){
					string f = openFileResult.filePath;
					ofLogVerbose("Selected file: " + f);
					loadScore(f);
				} else {
					ofLogVerbose("Cancel load score hit.");
				}
				break;
			}
		case INT_CONSTANT_BUTTON_RELOAD:
			askToSaveScore();
			ofxAntescofoNote->clear(); // TODO ask for Saving file
			loadScore(mScore_filename);
			break;
		case INT_CONSTANT_BUTTON_NEW:
			askToSaveScore();
			ofxAntescofoNote->clear(); // TODO ask for Saving file
			newScore();
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
		case INT_CONSTANT_BUTTON_TOGGLEVIEW:
			ofxAntescofoNote->toggleView();
			break;
		case INT_CONSTANT_BUTTON_TOGGLEEDIT:
			setEditorMode(!bEditorShow, 0);
			break;
		case INT_CONSTANT_BUTTON_SNAP:
			bSnapToGrid = !bSnapToGrid;
			cout << "Setting SnapToGrid to : " << bSnapToGrid << endl;
			timeline.setShowBPMGrid(bSnapToGrid);
			timeline.enableSnapToBPM(bSnapToGrid);
			break;
		case INT_CONSTANT_BUTTON_AUTOSCROLL:
			bAutoScroll = !bAutoScroll;
			cout << "Setting autoscroll mode:" << bAutoScroll << endl; 
			ofxAntescofoNote->setAutoScroll(bAutoScroll);
			break;
		case INT_CONSTANT_BUTTON_LINEWRAP:
			bLineWrapMode = !bLineWrapMode;
			cout << "Setting line wrapping mode:" << bLineWrapMode << endl; 
			[ editor setWrapMode:bLineWrapMode ];
			break;
		case INT_CONSTANT_BUTTON_FIND:
			cout << "Setting find text mode" << endl; 
			if (!bShowFind) {
				timeline.disable();
				bShowColorSetup = false;
				bShowOSCSetup = false;
				bShowFind = true;
				guiBottom->disable();
			}
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
				cout << "<<<<<<<<<<<<<<<<<<<< sending playstring >>>>>>>>>>>>>>>>>>>>>" << endl;
				cout << msg << endl;
				msg += "\n";
				m.addStringArg("playstring");
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
	}

	bShouldRedraw = true;
}

static ofxAntescofog *fog;

@implementation ofxCocoaDelegate (ofxAntescofogAdditions)
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
	//NSLog([notification name]);
	if ([[notification name] isEqualToString:@"DoubleClick"]) {

		NSDictionary *userInfo = notification.userInfo;
		int line = [[userInfo objectForKey:@"line"] intValue];
		NSLog(@"DoubleClick on line: %d", line);
		if (fog) {
			fog->editorDoubleclicked(line);
		}
	}
}
@end

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

	// . quit
	id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[quitMenuItem setTag:INT_CONSTANT_BUTTON_QUIT];
	[appMenu addItem:quitMenuItem];
	[menubar addItem:appMenuItem];
	[appMenuItem setSubmenu:appMenu];

	//////////////////
	// File
	id fileMenu = [[[NSMenu new] autorelease] initWithTitle:@"File"];
	id fileMenuItem = [[[NSMenuItem alloc] initWithTitle:@"File" action:NULL keyEquivalent:@""] autorelease];
	// . new
	NSString* txt = [@"" stringByAppendingString:@"New score"];
	id newMenuItem = [[[NSMenuItem alloc] initWithTitle:txt action:@selector(menu_item_hit:) keyEquivalent:@"n"] autorelease];
	[newMenuItem setTag:INT_CONSTANT_BUTTON_NEW];
	[fileMenu addItem:newMenuItem];

	// . load
	txt = [@"" stringByAppendingString:@"Load score"];
	id loadMenuItem = [[[NSMenuItem alloc] initWithTitle:txt action:@selector(menu_item_hit:) keyEquivalent:@"o"] autorelease];
	[loadMenuItem setTag:INT_CONSTANT_BUTTON_LOAD];
	[fileMenu addItem:loadMenuItem];


	// . reload
	id reloadMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Reload current score" action:@selector(menu_item_hit:) keyEquivalent:@"r"] autorelease];
	[reloadMenuItem setTag:INT_CONSTANT_BUTTON_RELOAD];
	[fileMenu addItem:reloadMenuItem];

	// . save 
	id saveMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Save Score" action:@selector(menu_item_hit:) keyEquivalent:@"s"] autorelease];
	[saveMenuItem setTag:INT_CONSTANT_BUTTON_SAVE];
	[fileMenu addItem:saveMenuItem];

	// . save as
	id saveAsMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Save Score As" action:@selector(menu_item_hit:) keyEquivalent:@"S"] autorelease];
	[saveAsMenuItem setTag:INT_CONSTANT_BUTTON_SAVE_AS];
	[fileMenu addItem:saveAsMenuItem];


	[fileMenuItem setSubmenu:fileMenu];
	[menubar addItem:fileMenuItem];

	//////////////////
	// Edit
	id editMenu = [[[NSMenu new] autorelease] initWithTitle:@"Edit"];
	id editMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Edit" action:NULL keyEquivalent:@""] autorelease];
	// . select all
	id selectMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Select All" action:@selector(menu_item_hit:) keyEquivalent:@"a"] autorelease];
	[editMenuItem setTag:INT_CONSTANT_BUTTON_SELECTALL];
	[editMenu addItem:selectMenuItem];
	// . find
	id findMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Find text" action:@selector(menu_item_hit:) keyEquivalent:@"f"] autorelease];
	[findMenuItem setTag:INT_CONSTANT_BUTTON_FIND];
	[editMenu addItem:findMenuItem];

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
	// . toggle view
	id toggleViewMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle View" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[toggleViewMenuItem setTag:INT_CONSTANT_BUTTON_TOGGLEVIEW];
	[viewMenu addItem:toggleViewMenuItem];
	// . toggle editor
	id toggleEditorMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle Editor" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[toggleEditorMenuItem setTag:INT_CONSTANT_BUTTON_TOGGLEEDIT];
	[viewMenu addItem:toggleEditorMenuItem];
	// . snap grid
	id snapMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Snap to grid" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[snapMenuItem setTag:INT_CONSTANT_BUTTON_SNAP];
	[viewMenu addItem:snapMenuItem];
	// . autoscroll
	id autoscrollMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Automatic Scroll" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[autoscrollMenuItem setTag:INT_CONSTANT_BUTTON_AUTOSCROLL];
	[viewMenu addItem:autoscrollMenuItem];
	// line wrap mode
	id lineWrapModeMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Toggle Line Wrapping" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	[lineWrapModeMenuItem setTag:INT_CONSTANT_BUTTON_LINEWRAP];
	[viewMenu addItem:lineWrapModeMenuItem];
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
	// Help
	id helpMenu = [[[NSMenu new] autorelease] initWithTitle:@"Help"];
	id helpMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Help" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
	//[helpMenu addItem:helpMenuItem];
	[helpMenuItem setSubmenu:helpMenu];
	[menubar addItem:helpMenuItem];

	guiBottom = new ofxUICanvas(0, 0, score_x+score_w, score_y);
	guiSetup_OSC = new ofxUICanvas(score_x + 50, score_y, ofGetWindowWidth(), ofGetWindowHeight());
	guiError = new ofxUIScrollableCanvas(score_x, score_y, ofGetWindowWidth(), ofGetWindowHeight()-100-score_y);
	guiSetup_Colors = new ofxUICanvas(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	guiFind = new ofxUICanvas(300, score_y, ofGetWindowWidth()-300, ofGetWindowHeight()-100-score_y);

	guiBottom->setFont("DroidSansMono.ttf");
	guiFind->setFont("DroidSansMono.ttf");
	/*
	   guiBottom->setFont("NewMedia Fett.ttf");
	   guiSetup_OSC->setFont("NewMedia Fett.ttf");
	   guiSetup_Colors->setFont("NewMedia Fett.ttf");
	   guiError->setFont("NewMedia Fett.ttf");
	   */


	// register double click on editor notification callback
	[[NSNotificationCenter defaultCenter] addObserver:[NSApp delegate] selector:@selector(receiveNotification:) name:@"DoubleClick" object:nil];

	//guiSetup_Colors->setScrollableDirections(false, true);
	guiError->setScrollAreaToScreen();
	guiError->setScrollableDirections(true, false);
	guiSetup_OSC->setColorBack(ofColor(0, 0, 0, 0));
	guiSetup_Colors->setColorBack(ofColor(0, 0, 0, 0));

	guiBottom->loadSettings("GUI/guiSettings.xml");
	//guiTop->loadSettings("GUI/guiSettings.xml");

	//guiTop->setUIColors(ofxAntescofoNote->color_gui_bg, ofxAntescofoNote->color_key, ofxAntescofoNote->color_highlight, ofxAntescofoNote->color_staves_fg,
	//                    ofxAntescofoNote->color_highlight, ofxAntescofoNote->color_gui_bg, ofxAntescofoNote->color_gui_bg);

	//guiBottom->addWidgetDown(new ofxUISpacer(ofGetWidth()-5, 1));
	mSliderBPM = new ofxUISlider(16*4*10, 0, 70, 12, 30, 300, 120, TEXT_CONSTANT_BUTTON_BPM);
	guiBottom->addWidgetDown(mSliderBPM);


#if NO_UIBUTTONS
	guiBottom->addWidgetRight(new ofxUILabelToggle(bSnapToGrid, TEXT_CONSTANT_BUTTON_SNAP, OFX_UI_FONT_SMALL));
	guiBottom->addWidgetRight(new ofxUILabelToggle(bAutoScroll, TEXT_CONSTANT_BUTTON_AUTOSCROLL, OFX_UI_FONT_SMALL));
#endif
	//guiBottom->addWidgetDown(new ofxUISpacer(ofGetWidth()-5, 1));
	ofxUIButton *b = new ofxUIButton("NOTE", false, 30, 15);
	guiBottom->addWidgetRight(b);
	b->setColorBack(ofxAntescofoNote->color_note);

	b = new ofxUIButton(30, 15, false, "CHORD");
	guiBottom->addWidgetRight(b);
	b->setColorBack(ofxAntescofoNote->color_note_chord);

	b = new ofxUIButton(30, 15, false, "MULTI"); 
	guiBottom->addWidgetRight(b);
	b->setColorBack(ofxAntescofoNote->color_note_multi);

	b = new ofxUIButton(30, 15, false, "TRILL"); 
	guiBottom->addWidgetRight(b);
	b->setColorBack(ofxAntescofoNote->color_note_trill);

	ofxUISpacer *space = new ofxUISpacer(ofGetWidth(), 1);
	space->setVisible(false);
	guiBottom->addWidgetDown(space);
	mLabelBeat = new ofxUILabel(TEXT_CONSTANT_BUTTON_BEAT, fontsize);
	guiBottom->addWidgetDown(mLabelBeat);
	mLabelBeat = new ofxUILabel("0", fontsize);
	guiBottom->addWidgetRight(mLabelBeat);
	mLabelPitch = new ofxUILabel(TEXT_CONSTANT_BUTTON_PITCH, fontsize);
	guiBottom->addWidgetDown(mLabelPitch);
	mLabelPitch = new ofxUILabel("0", fontsize);
	guiBottom->addWidgetRight(mLabelPitch);
	mLabelAccompSpeed = new ofxUILabel(TEXT_CONSTANT_BUTTON_SPEED, fontsize);

	 
	// drop down list
	//guiBottom->addWidgetDown(new ofxUILabel("Cue points", OFX_UI_FONT_MEDIUM)); 
	mCuepointsDdl = new ofxUIDropDownList(100, TEXT_CONSTANT_BUTTON_CUEPOINTS, cuepoints, OFX_UI_FONT_SMALL);
	mCuepointsDdl->setAllowMultiple(false);
	mCuepointsDdl->setAutoClose(true);
	guiBottom->addWidgetDown(mCuepointsDdl);

	// event buttons
	space = new ofxUISpacer(3, 1);
	space->setVisible(false);
	guiBottom->addWidgetLeft(space, OFX_UI_ALIGN_RIGHT);

	b = new ofxUILabelToggle(90, false, TEXT_CONSTANT_BUTTON_NEXT_EVENT, OFX_UI_FONT_SMALL);
	//guiBottom->addWidgetLeft(b, OFX_UI_ALIGN_RIGHT);
	guiBottom->addWidgetLeft(b);
	b = new ofxUILabelToggle(90, false, TEXT_CONSTANT_BUTTON_PREV_EVENT, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetLeft(b);
	b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_START, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetLeft(b);
	b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_STOP, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetLeft(b);
	b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_PLAY, OFX_UI_FONT_SMALL);
	guiBottom->addWidgetLeft(b);
	if (enable_simulate) {
		b = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_SIMULATE, OFX_UI_FONT_SMALL);
		guiBottom->addWidgetLeft(b);
		mEditButton = new ofxUILabelToggle(50, false, TEXT_CONSTANT_BUTTON_EDIT, OFX_UI_FONT_SMALL);
		guiBottom->addWidgetLeft(mEditButton);
		mEditButton->setVisible(false);
		mEditButton->setLabelVisible(false);
	}

	guiBottom->addWidgetDown(mLabelAccompSpeed);
	mLabelAccompSpeed->setVisible(false);
	mLabelAccompSpeed = new ofxUILabel("0", fontsize);
	guiBottom->addWidgetRight(mLabelAccompSpeed);
	mLabelAccompSpeed->setVisible(false);

	guiElevator = new ofxUICanvas(0, score_y, 17, ofGetWindowHeight() - score_y);
	elevator = new ofxUIRangeSlider(3, 0*score_y+2, 12, ofGetWindowHeight() - score_y - 6, 1, 255, 200, 255, "elevator", OFX_UI_FONT_SMALL);
	guiElevator->addWidget(elevator);

	elevator->setLabelPrecision(0);
	elevator->setLabelVisible(false);
	elevator->setColorFill(ofColor(timeline.getColors().outlineColor, 72)); //ofColor(0, 0, 0, 255));
	elevator->setVisible(false);
	guiElevator->setVisible(false);
	guiElevator->setFont("DroidSansMono.ttf");
	guiElevator->disable();

	guiFind->setColorBack(ofColor(0, 0, 0, 0));


	/*vector<string> items;
	  items.push_back("Play to midi synth");
	  items.push_back("Open MusicXML score");
	  items.push_back("Open Antescofo score");
	  items.push_back("Save to Antescofo score");
	  items.push_back("Save to Antescofo score as...");
	  items.push_back("Setup OSC");
	  items.push_back("About Antescofog");
	  items.push_back("Colors preferences");
	  items.push_back("Help");
	  */

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

	mSaveColorButton = new ofxUILabelButton(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2, 100, false, TEXT_CONSTANT_BUTTON_SAVE_COLOR);
	guiSetup_Colors->addWidget(mSaveColorButton);
	mSaveColorButton->setVisible(false);
	mSaveColorButton->setLabelVisible(false);
	guiSetup_Colors->setVisible(false);

	guiError->setVisible(false);
	guiError->disable();

	mLogoInria.loadImage("logo_inria.png");
	mLogoIrcam.loadImage("logo_ircam.png");

	ofAddListener(guiError->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiBottom->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiFind->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiSetup_Colors->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiSetup_OSC->newGUIEvent, this, &ofxAntescofog::guiEvent);
	ofAddListener(guiElevator->newGUIEvent, this, &ofxAntescofog::guiEvent);

}

void ofxAntescofog::setupOSC(){
	// listen for OSC
	//mOsc_port = 3002;
	int port = 0;
	std::istringstream is(mOsc_port);
	is >> port;
	if (! is.good() && port <= 0)
	{
		//cerr << "Not a number, try again." << endl;
		mOsc_port = "6789";
	}
	std::cout << "Listening on OSC port " << port << endl;
	try {
		mOSCreceiver.setup(atoi(mOsc_port.c_str()));
	} catch(...) { //} catch(exception& e) { 
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

	//mOsc_port_MAX = 3003;
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

	//timeline.setWidth(score_w - 100);
	timeline.setFrameRate(24);
	timeline.setShowTicker(false);
	timeline.setShowBPMGrid(bSnapToGrid);
	timeline.enableSnapToBPM(bSnapToGrid);
	timeline.setDurationInSeconds(60);
	//timeline.moveToThread(); //increases accuracy of bang call backs

	timeline.setLoopType(OF_LOOP_NORMAL);//LOOP_NONE); ///NORMAL); //turns the timeline to loop
	timeline.setBPM(bpm);
	timeline.setLockWidthToWindow(false);
	//timeline.setWidth(ofGetWidth());

	// use custom zoomer :
	timeline.addTrack("zoom", ofxAntescofoZoom);
	timeline.setZoomer(ofxAntescofoZoom);

	timeline.addTrack("Beats", ofxAntescofoBeatTicker);
	timeline.addTrack("Notes", ofxAntescofoNote);
	ofxAntescofoNote->setDrawRect(ofRectangle(0, 0, score_w, 300));
	ofxAntescofoBeatTicker->setup();
	timeline.setShowTicker(true);
	timeline.setBPM(bpm);

	//timeline.addCurves("test", "test.xml");
	timeline.enable();
	timeline.setFrameBased(false);
	ofxAntescofoNote->enable();

	/*
	// stick to the bottom of the window
	ofxTLZoomer* z = timeline.getZoomer();
	ofRectangle rz = z->getDrawRect(); 
	ofRectangle ra = ofxAntescofoAction->getDrawRect();
	ra.height = ofGetHeight() - ra.y - rz.height - 200;
	ofxAntescofoAction->setDrawRect(ra);
	*/
}



//--------------------------------------------------------------
void ofxAntescofog::setup(){
	console->addln("ofxAntescofo::setup()");
	ofSetDataPathRoot("../Resources/");

	//ofBackground(.15*255);
	//ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableAlphaBlending();

	//cocoaWindow->setUpdateRate(1);
	ofSetLogLevel(OF_LOG_VERBOSE);

	fog = this;
	score_x = 5;
	score_y = 81+10;
	mUIbottom_y = 40;

	bpm = 120; 

	ofSetEscapeQuitsApp(false);
	score_w = ofGetWindowWidth() - score_x - 5;
	score_h = ofGetWindowHeight()/3;

	ofAddListener(ofEvents().windowResized, this, &ofxAntescofog::windowResized);

	ofxAntescofoZoom = new ofxTLZoomer2D();
	ofxAntescofoNote = new ofxTLAntescofoNote(this);
	ofxAntescofoBeatTicker = new ofxTLBeatTicker(this);

	load(); // xml settings

	setupTimeline();

	setupUI();

	setupOSC();

	remove(TEXT_CONSTANT_TEMP_FILENAME);
	remove(TEXT_CONSTANT_TEMP_ACTION_FILENAME);
	// finally open a score if given on command line
	//if (mScore_filename.size()) loadScore(mScore_filename);

	setEditorMode(bEditorShow, 0);

	drawCache.allocate(ofGetWindowWidth(), ofGetHeight(), GL_RGBA);
	drawCache.begin();
	ofClear(255,255,255, 0);
	drawCache.end();

	bSetupDone = true;

	if (mScore_filename.size())
		loadScore(mScore_filename);

	if (!disable_httpd)
		setup_httpd();
}

//--------------------------------------------------------------
void ofxAntescofog::update() {
	if (!bSetupDone)
		return;
	//if (!bEditorShow) score_w = ofGetWindowWidth() - score_x;
	//else score_w = ofGetWindowWidth() - CONSTANT_EDITOR_VIEW_WIDTH;
	timeline.setWidth(score_w);
	timeline.setOffset(ofVec2f(score_x, score_y));

	if (timeline.getDrawRect().height + score_y > ofGetWindowHeight()) {
		if (!elevator->isVisible())
			elevatorEnable();
	} // else if (elevator->isVisible() && ) elevatorDisable();

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
				//if (bpm) timeline.setBPM(bpm);
				mSliderBPM->setValue(bpm);
				mHasReadMessages = true;
			} else if(m.getAddress() == "/antescofo/event_beatpos" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				mOsc_beat = m.getArgAsFloat(0);
				mLabelBeat->setLabel(ofToString(mOsc_beat));
				if (_debug) cout << "OSC received: beat: "<< mOsc_beat << endl;
				if (!disable_httpd)
					httpd_update_beatpos();
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
				if (_debug) cout << "OSC received: loadscore: "<< scorefile << endl;
				mHasReadMessages = true;
				mCuepointsDdl->clearToggles();
				loadScore(scorefile, false);
				bShouldRedraw = true;
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
				// add to the list of strings to display
				/*	msg_string[current_msg_string] = msg_string;
					timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
					current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
				// clear the next line
				msg_strings[current_msg_string] = "";
				*/
				cout << "OSC received: unknown msg: "<< msg_string << endl;
			}
			//break;
		}
	}
	catch (exception& e) {
		cerr << "OSC HasWaitingMessage exception raised" <<  endl;
	}

	// if we read something, advance playhead
	if (mHasReadMessages) {

		mLastOSCmsgDate = ofGetSystemTime();
		fAntescofoTimeSeconds = ofxAntescofoNote->convertAntescofoOutputToTime(mOsc_beat, mOsc_tempo, mOsc_pitch);

		if (_debug) cout << "Moving playHead to beat:"<<mOsc_beat << " tempo:"<<mOsc_tempo << " => "<<fAntescofoTimeSeconds << "sec"<<endl;
		//timeline.setCurrentTimeSeconds(fAntescofoTimeSeconds);
		//ofNotifyEvent(playbackStarted, )
		mHasReadMessages = false;
		//timeline.play();
		bShouldRedraw = true;
	}
	//guiBottom->update();

	// http update
	if (!disable_httpd)
		update_http_image();
}

//--------------------------------------------------------------
void ofxAntescofog::draw() {
	
	//ofRectangle r = timeline.getDrawRect();
	//cout << "ofxAntescofog: draw: total draw rect:" << r.x << ", " << r.y << " : " << r.width << "x" << r.height << endl;
	if (!bSetupDone)
		return;

	/*
	struct timeval now;
	gettimeofday(&now, 0);
#define DRAW_MAX_DELTA_USEC	100000
	if ((now.tv_sec*1000000L + now.tv_usec) - (last_draw_time.tv_sec*1000000L + last_draw_time.tv_usec) < DRAW_MAX_DELTA_USEC) {
		return;
	}
	*/
	bool bMayUseCache = false;
	//cout << "---- " << ofGetSystemTime() - mLastOSCmsgDate << endl;
	if (ofGetSystemTime() - mLastOSCmsgDate > 15000) 
		bMayUseCache = true;

	ofFill();
	if (bShowColorSetup) { // color setup
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_ColorSetup();
	} else if (bShowOSCSetup) { // osc setup
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_OSCSetup();
	} else if (bShowError) { // parsing error display
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_error();
	} else if (bShowFind) { // find/replace
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_FindText();
	} else { // draw normal
		if (bMayUseCache && !timeline.getIsPlaying() && !bShouldRedraw) {
			drawCache.draw(0, 0);
		} else {
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
			if (guiElevator->isEnabled())
				guiElevator->draw();
			console->draw();
			ofPopStyle();

			// logos
			mLogoInria.draw(score_w - 290, 2, 148, 54);
			mLogoIrcam.draw(score_w - 120, 0, 109, 64);
			//mLogoIrcam.draw(score_w - 170, 0, 109, 64);
		
			drawCache.end();

			//ofBackground(255, 255, 255, 255);
			drawCache.draw(0, 0);
			bShouldRedraw = false;
		}
	}

	// http draw
	if (!disable_httpd)
		draw_http_image();
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
    
   /*
    string xmlFileName = "GUI/defaultColors.xml";
	ofxXmlSettings settings;
    
	if(settings.loadFile(xmlFileName)){
               
        cout << "UI Colors loaded from xml files." << endl;
    }
	else{
		ofLogError("ofxTLAntescofoNote -- Error loading from xml file " + xmlFileName);
	}
    */
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

void ofxAntescofog::setEditorMode(bool state, float beatn) {
	bEditorShow = state;

	if (bEditorShow) {
		float editor_x = ofGetWidth();
		cocoaWindow->setWindowShape(ofGetWidth() + CONSTANT_EDITOR_VIEW_WIDTH, ofGetHeight());
#ifdef TARGET_OSX
		NSSize siz;
		editor = [ ofxCodeEditor alloc];
		const char *normal_keywords = "bpm note chord trill multi variance tempo on off transpose variance";
		const char *major_keywords = "gfwd group lfwd loop cfwd curve kill abort killof abortof whenever closefile openoutfile oscon oscoff oscsend oscrecv";
		const char *procedure_keywords = "@macro_def @fun_def @insert #include let until expr @tight @grain @action @coef @date @fun_def @proc_def @global @hook @immediate @inlet @jump @label @local @map_history @map_history_date @map_history_rdate @modulate @name @norec @type during for in map imap tab loc glob if else do while := s ms port @linear @linear_in @linear_out @linear_in_out @back_in @back_out @back_in_out @bounce_in @bounce_out @bounce_in_out @cubic_in @cubic_out @cubic_in_out @circ_in @circ_out @circ_in_out @elastic_in @elastic_out @elastic_in_out @exp_in @exp_out @exp_in_out @quad_in @quad_out @quad_in_out @quart_in @quart_out @quart_in_out @quint_in @quint_out @quint_in_out @sine_in @sine_out @sine_in_out @is_undef @is_bool @is_int @is_float @is_numeric @is_string @is_symbol @is_map @is_interpolatedmap @is_fct @is_function @sin @asin @sinh @cos @acos @cosh @tan @atan @exp @log10 @log2 @log @rand @sqrt @round @floor @ceil @pow @make_score_map @make_duration_map @is_integer_indexed @is_list @is_vector @is_defined @shift_map @gshift_map @select_map @mapval @merge @concat @compose_map @min_key @max_key @min_val @max_val @listify @map_reverse @tab_map @concat @min_val @max_val @size @clear @tab_reverse @push_back @resize";
		const char *system_keywords = "$rt_tempo $pitch $beat_pos $now $rnow @exp @max @min @log @integrate @normalize @bounded_integrate @add_pair @date @rdate";

		[ editor  set_normal_keywords: normal_keywords ];
		[ editor  set_major_keywords: major_keywords ];
		[ editor  set_procedure_keywords: procedure_keywords ];
		[ editor  set_system_keywords: system_keywords ];
		NSWindow *nswin = [cocoaWindow->delegate getNSWindow]; 
		//string str = "ofxAntescofog: window: "; str += ofGetWidth() + "x"+ ofGetHeight(); console->addln(str);

		NSView *nsview_ = [cocoaWindow->delegate getNSView];
		ofRectangle r(editor_x, 0, CONSTANT_EDITOR_VIEW_WIDTH, ofGetHeight());
		[ editor setup: nswin glview:nsview_ rect:r];

		if (mScore_filename.size() && access(mScore_filename.c_str(), R_OK) != -1)
			[ editor loadFile:mScore_filename];
		else if (access(mScore_filename.c_str(), R_OK) != -1)
			[ editor loadFile:TEXT_CONSTANT_TEMP_FILENAME ];
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
    ofxUIButton *b = new ofxUIButton(200, 25, false, name);
    guiSetup_Colors->addWidgetDown(b);
    b->setColorBack(*color);
}



void ofxAntescofog::draw_ColorSetup()
{
    //ofFill();
    //ofSetColor(255, 255, 255, 100);
    ofSetColor(0, 0, 0, 100);
    
    ofRect(0, mUIbottom_y, score_w, ofGetWindowHeight() - mUIbottom_y);

    if (!bColorSetupInitDone) {// init and create buttons with color rect

        guiSetup_Colors->addWidgetDown(new ofxUILabel("Choose a color to change then press \"Save color\"", OFX_UI_FONT_LARGE));
        guiSetup_Colors->addWidgetDown(new ofxUILabelToggle(ofGetWindowWidth()/2, score_y, false, TEXT_CONSTANT_BUTTON_BACK, fontsize));
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
    //draw_ColorPicker();
}


void ofxAntescofog::draw_OSCSetup() {
    ofSetColor(0, 0, 0, 100);
    ofRect(40, mUIbottom_y, score_w - 40, ofGetWindowHeight() - mUIbottom_y);
    if (!bOSCSetupInitDone) {// init and create buttons with color rect
        //guiSetup->addWidgetDown(new ofxUILabel("OSC local port", OFX_UI_FONT_MEDIUM));
        guiSetup_OSC->addWidgetDown(new ofxUILabelToggle(ofGetWindowWidth()/2, score_y, false, TEXT_CONSTANT_BUTTON_BACK, fontsize));
	/*
        for (map<string, string*>::const_iterator c = oscString2var.begin(); c != oscString2var.end(); c++) {
            guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	    cout << "===============> adding text input:" << c->first << " 2nd:" << *(c->second) << endl;

        }
	*/
	// host
	map<string, string*>::const_iterator c = oscString2var.begin();
	guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	mTextOscHost = new ofxUITextInput(300, c->first, *(c->second), OFX_UI_FONT_MEDIUM);
	guiSetup_OSC->addWidgetRight(mTextOscHost);
	// port
	c++;
	guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	mTextOscPort = new ofxUITextInput(300, c->first, *(c->second), OFX_UI_FONT_MEDIUM);
	guiSetup_OSC->addWidgetRight(mTextOscPort);
	// port remote
	c++;
	guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
	mTextOscPortRemote = new ofxUITextInput(300, c->first, *(c->second), OFX_UI_FONT_MEDIUM);
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
        guiFind->addWidgetDown(new ofxUILabelToggle(ofGetWindowWidth()/2, score_y, false, TEXT_CONSTANT_BUTTON_BACK, fontsize));
	guiFind->addWidgetDown(new ofxUILabel("Enter your text to search or replace: ", OFX_UI_FONT_MEDIUM));
	guiFind->addWidgetDown(new ofxUITextInput(300, TEXT_CONSTANT_BUTTON_TEXT, "", OFX_UI_FONT_MEDIUM));
        guiFind->addWidgetRight(new ofxUILabelToggle(ofGetWindowWidth()/2, score_y, false, TEXT_CONSTANT_BUTTON_FIND, fontsize));
	guiFind->addWidgetDown(new ofxUITextInput(300, TEXT_CONSTANT_BUTTON_REPLACE_TEXT, "", OFX_UI_FONT_MEDIUM));
        guiFind->addWidgetRight(new ofxUILabelToggle(ofGetWindowWidth()/2, score_y, false, TEXT_CONSTANT_BUTTON_REPLACE, fontsize));
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
    // space key is cancel on error window

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
    if(key == OF_KEY_TAB /*&& !bEditorShow*/) {
        ofxAntescofoNote->toggleView();
    }
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
	//bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::mousePressed( int x, int y, int button ) {
	//if (args.button == 3) { }
	bShouldRedraw = true;
    //cout << "Fog : mousePressed r:"<< mEditorRect.x << ","<< mEditorRect.y << ","<< mEditorRect.width << "," << mEditorRect.height <<" inside:"<< mEditorRect.inside(x, y) << endl;
    
#if 0
    if (bEditorMode && !mEditorRect.inside(x, y)) {
        
        cout << "Fog : change action" << endl;
        setEditorMode(false, mEditorBeatnum);
        string a, line;
        ifstream f;
        f.open(TEXT_CONSTANT_TEMP_ACTION_FILENAME);
        while (f.good()) {
            getline(f, line);// >> tmp;
            a += line;
            a += "\n";
        }
        f.close();
        ofxAntescofoNote->change_action(mEditorBeatnum, a);
        ofxAntescofoAction->bEditorShow = false;
    }
#endif
}

//--------------------------------------------------------------
void ofxAntescofog::mouseDragged( int x, int y, int button ){
	int topy = guiBottom->getRect()->y + guiBottom->getRect()->height;
	if (button == 3 && ofGetWindowHeight() < topy + timeline.getDrawRect().height) { // scroll events
		ofxUIRangeSlider *r = (ofxUIRangeSlider*)guiElevator->getWidget("elevator");
		double vlow = r->getPercentValueLow() * 255;
		double vhigh = r->getPercentValueHigh() * 255;
		int score_h = timeline.getDrawRect().height;
		double ny = score_y + 3*y;
		if (ny > topy)
			ny = topy;
		if (ny + score_h < topy)
			ny = score_y + 200;
		if (topy + ny > ofGetWindowHeight())
			ny = ofGetWindowHeight() + topy - score_h;
		double m = ny / (score_h + topy) + 0.1;
		double nvl = m - 1;
		double nvh = m - 0.5;
		r->setValueLow(nvl*255);
		r->setValueHigh(nvh*255);

		//cout << "Elevator scrolled: score_y: " << score_y << " ny:" << ny << " barvalues: [ "<< nvl << " - "<< nvh<<" ]"<< endl;
		score_y = ny;
	}

	bShouldRedraw = true;
}


//--------------------------------------------------------------
void ofxAntescofog::mouseReleased( int x, int y, int button ){
	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::windowResized(ofResizeEventArgs& resizeEventArgs) { // (int w, int h){
	cout << "ofxAntescofog::windowResized: "<< resizeEventArgs.width << "x"<< resizeEventArgs.height << endl;


#ifdef TARGET_OSX
	NSView *glview = [cocoaWindow->delegate getNSView];
	[glview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

	glViewport( 0, 0, ofGetWidth(), ofGetHeight());

	cocoaWindow->setWindowShape(ofGetWidth(), ofGetHeight());

	score_w = resizeEventArgs.width - score_x - 10;
	guiBottom->getRect()->width = score_w + score_x;

	drawCache.allocate(resizeEventArgs.width, ofGetHeight(), GL_RGBA);

	drawCache.begin();
	ofClear(255,255,255, 0);
	drawCache.end();
	 

#endif

	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::gotMessage(ofMessage msg){

}

void ofxAntescofog::draw_error()
{
    ofSetColor(0, 0, 0, 100);
    ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    ofSetColor(255, 255, 255, 240);

    string err = ofxAntescofoNote->get_error();
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
    ofDrawBitmapString(err2, 50, 300);

    guiError->draw();
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
        /*
         int len = 80, i;
         for (i = 0; i < err.size(); i += len) {
         string sub(err, i, MIN(i+len, err.size()));
         guiError->addWidgetDown(new ofxUILabel(sub, OFX_UI_FONT_MEDIUM));
         }
         if (i != err.size()) { string sub(err, i, err.size()); guiError->addWidgetDown(new ofxUILabel(sub, OFX_UI_FONT_MEDIUM)); }
 
         //guiError->addWidgetDown(new ofxUILabel(ofGetWidth()/3, 200, score_w - ofGetWidth()/3, err, OFX_UI_FONT_MEDIUM));

         */
        ofDrawBitmapString(ofxAntescofoNote->get_error(), 100, 100);
        bErrorInitDone = true;
        guiError->addWidgetDown(new ofxUILabelToggle(ofGetWindowWidth()/2, score_y, false, TEXT_CONSTANT_BUTTON_CANCEL, fontsize));
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

			//.addPatch( dragInfo.files[i], dragInfo.position );
			ofxAntescofoNote->clear(); // TODO ask for Saving file
			string str = "ofxAntescofog: trying to load file :" + dragInfo.files[i];
			console->addln(str);
			int n = loadScore(string(dragInfo.files[i]));

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
			mEditButton->setVisible(false);
			mEditButton->setLabelVisible(false);
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
			// show dialog confirming
			ofstream outfile;
			outfile.open (mScore_filename.c_str());
			outfile << s;
			outfile.close();
			loadScore(mScore_filename);
		}
	}


}

bool ofxAntescofog::edited() {
	string editorcontent;
	[ editor getEditorContent:editorcontent ];

	string filecontent;
	ifstream file;
	file.open (mScore_filename.c_str(), ios::ate);
	int fsize;
	fsize = (int) file.tellg();
	cout << "file: " << mScore_filename << " fsize=" << fsize << endl;
	file.seekg (0, ios::beg);
	if (fsize <= 0)
		return false;
	char* memblock = new char[fsize];
	file.read (memblock, fsize);
	filecontent = string(memblock);
	file.close();
	delete memblock;

	cout << "original file len:" << filecontent.size() << endl;
	if (filecontent.size() != editorcontent.size()
		|| filecontent.compare(editorcontent)) {
		return true;
	} else {
		return false;
	}
}

void ofxAntescofog::askToSaveScore() {
	if (edited()) {
		cout << "askToSaveScore: file modified" << endl;
		saveAsScore();

	} else {
		cout << "askToSaveScore: file not modified" << endl;
	}
}


void ofxAntescofog::newScore() {
	[ editor clear ];
	
	ofxAntescofoNote->clear_actions();
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
				ofLogVerbose("Selected file: " + f);
				// save and try to parse
				// show dialog confirming
				mScore_filename = f;
				ofstream outfile;
				outfile.open (mScore_filename.c_str());
				outfile << s;
				outfile.close();
				loadScore(mScore_filename);
			} else {
				ofLogVerbose("Cancel load score hit.");
			}
		}
	}
}


int ofxAntescofog::loadScore(string filename, bool sendOsc) {
	ofxAntescofoNote->clear_error();
	cout << "Trying to load score : " << filename << endl;
	// save zoom view range
	ofRange z = ofxAntescofoZoom->getViewRange(); 

	// save editor position
	int lineEditorPos = [ editor getCurrentPos];
	int lineEditor = [ editor getCurrentLine];
	int lineEditorScroll = 0; //[ editor getCurrentPosScroll];

	cout << "loadScore: lineEditor: " << lineEditor <<" linEScroll: " << lineEditorScroll << endl;
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
		ofxAntescofoNote->setZoomBounds(z); //timeline.getZoomer()->setViewRange(z);
		ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
		if (!actiontrack) actiontrack = ofxAntescofoNote->createActionTrack();
		if (actiontrack) actiontrack->setZoomBounds(z);
		timeline.getTicker()->setZoomBounds(z);

		bpm = timeline.getBPM();
		mSliderBPM->setValue(bpm);
		// get cuepoints
		mCuepointsDdl->clearToggles();
		for (vector<string>::iterator i = ofxAntescofoNote->cuepoints.begin(); i != ofxAntescofoNote->cuepoints.end(); i++) {
			mCuepointsDdl->addToggle(*i);
		}
		update();

		if (sendOsc) {
			// send OSC read msg to Antescofo
			ofxOscMessage m;
			m.setAddress("/antescofo/cmd");
			m.addStringArg("read");
			m.addStringArg(mScore_filename);
			cout << "Sending OSC \"read "<< mScore_filename << "\" to Antescofo Patch." << endl;
			mOSCsender.sendMessage(m);
		}
	} else {
		//mScore_filename = TEXT_CONSTANT_TEMP_FILENAME;
		//if (ofxAntescofoNote->get_error().empty()) ofxAntescofoNote->set_error("Zero event found in score.");
		display_error();
	}

	// update editor if opened
	if (bEditorShow) {
		[ editor loadFile:mScore_filename ];
		cout << "Editor scrolling to pos: " << lineEditor << " scroll:"<< lineEditorScroll << endl;
		[ editor gotoPos:lineEditorPos];
		int totLines = [ editor getMaxLines ];
		if (lineEditor > totLines/2) {
			[ editor setCurrentPos:lineEditorPos]; //[ editor scrollLine:lineEditor];
			[ editor scrollLine:totLines/2]; //[ editor scrollLine:lineEditor];
		}
		//int n = [ editor getNbLines ];
		//[ editor scrollLine:(30) ];

	}

	bShouldRedraw = true;
	imageSaved = false;
	return n;
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
			loadScore(mScore_filename);
			ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
			b->setValue(false);
				} else {
					console->addln("Cancel load score hit.");
				}
		}
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_RELOAD)
    {
            loadScore(mScore_filename);
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
    } else if(e.widget->getName() == TEXT_CONSTANT_BUTTON_TOGGLE_VIEW)
	{
        ofxAntescofoNote->toggleView();
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
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
	    if (!mEditButton) {
		    cerr << "ERROR: Can not get widget edit"<< endl;
		    abort();
	    }
	    mEditButton->setVisible(true);
	    mEditButton->setLabelVisible(true);
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
		    cout << "mPlayLabel: " << mPlayLabel << endl;
		    if (mPlayLabel.size()) {
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
		    m.addStringArg("start");
		    m.addStringArg("");
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
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_FIND) {
	    cout << "Find button hit!" << endl;
	    ofxUILabel *l = ((ofxUIWidgetWithLabel*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_TEXT))->getLabelWidget();
	    if (l) {
		    string str = l->getLabel();
		    if (str.size()) {
			    [ editor searchText:str ];
			    //guiFind->getWidget(TEXT_CONSTANT_BUTTON_FIND_NEXT)->setVisible(true);
			    //((ofxUILabelToggle*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_FIND_NEXT))->setLabelVisible(true);
		    } else cout << "empty search string" << endl;
	    } else { cout << "can not get search string " << endl; }
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_REPLACE) {
	    cout << "Replace button hit!" << endl;
	    ofxUILabel *l = ((ofxUIWidgetWithLabel*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_TEXT))->getLabelWidget();
	    ofxUILabel *l2 = ((ofxUIWidgetWithLabel*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE_TEXT))->getLabelWidget();
	    int res = 0;
	    if (l) {
		    string str = l->getLabel();
		    string str2 = l2 ? l2->getLabel() : "";
		    if (str.size()) {
			    res = [ editor searchNreplaceText:str str2:str2];
			    ofxUILabel *l = ((ofxUIWidgetWithLabel*)guiFind->getWidget(TEXT_CONSTANT_BUTTON_REPLACE_NB))->getLabelWidget();
			    l->setVisible(true);
			    mFindReplaceOccur->setVisible(true);
			    mFindReplaceOccur->setLabel(ofToString(res));
			    string out;
			    [ editor getEditorContent:out ];
			    cout << out << endl;
		    } else cout << "empty search string" << endl;
	    } else { cout << "can not get search string " << endl; }
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
        } else if (bShowFind) {
	    bShowFind = false;
	    guiFind->disable();
	}

        e.widget->toggleVisible();
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
        timeline.enable();
        guiBottom->enable();
    }
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
    if (e.widget->getName() == TEXT_CONSTANT_BUTTON_CUEPOINTS) {
	    cout << "Cuepoints Drop Down List hit: " << endl; 
	    vector<ofxUIWidget *> &selected = mCuepointsDdl->getSelected(); 
	    for(int i = 0; i < selected.size(); i++)
	    {
		    mPlayLabel = selected[i]->getName();
		    cout << "Sending OSC: gotolabel " << mPlayLabel << endl; 
		    ofxOscMessage m;
		    m.setAddress("/antescofo/cmd");
		    m.addStringArg("gotolabel");
		    m.addStringArg(mPlayLabel);
		    mOSCsender.sendMessage(m);
		    break;
	    }
	    mCuepointsDdl->clearSelected();
    }
}

void ofxAntescofog::editorDoubleclicked(int line)
{
	ofxAntescofoNote->showNote(line);
	[ editor braceMatch ];
	bShouldRedraw = true;
}


void ofxAntescofog::editorShowLine(int linea, int lineb)
{
	if (bEditorShow) {
		[ editor showLine:linea-1 lineb:lineb-1 ];
		int n = [ editor getNbLines ];
		if (linea + 35 < n)
			[ editor scrollLine:(-35) ];

		bShouldRedraw = true;
	}
}


void ofxAntescofog::replaceEditorScore(int linebegin, int lineend, string actstr) {
	if (bEditorShow && actstr.size() && linebegin <= lineend) {
		cout << "ofxAntescofog::replaceEditorScore: replacing between line " << linebegin << " - " << lineend << " str:" << actstr << endl;
		[ editor replaceString:linebegin-1 lineb:lineend str:actstr.c_str()];
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

void ofxAntescofog::elevatorEnable()
{
	elevator->setVisible(true);
	guiElevator->enable();
	guiElevator->setVisible(true);
	int var = 14;
	score_x += var;
	score_w -= var;
	ofRectangle r = timeline.getDrawRect();
	int h = r.height;
	//elevator->setValueLow(0);
	//elevator->setValueHigh(0);
	//elevator->setMax(0);
	//elevator->setMin(0);
	double d = (ofGetWindowHeight() - score_y);
	double valueHigh = d / h;
	cout << "ofxAntescofog::elevatorEnable: timeline h : " << h << endl;
	cout << "ofxAntescofog::elevatorEnable:(ofGetWindowHeight() - score_y) : " << d <<endl; 
	cout << "ofxAntescofog::elevatorEnable: valueHigh : " << valueHigh << endl;
	//elevator->setValueLow(valueHigh);
	//elevator->setValueHigh(valueHigh*100);
	//elevator->setColorBack(ofColor(0, 0, 0, 255));

	//elevator->setMin(valueHigh);
	//elevator->setMax(valueHigh*25);
}

void ofxAntescofog::elevatorDisable()
{
	cout << " ofxAntescofog::elevatorDisable()" << endl;
	int var = 14;
	score_x -= var;
	score_w += var;
	elevator->setVisible(false);
	guiElevator->disable();
	guiElevator->setVisible(false);
}

void AntescofoTimeline::setZoomer(ofxTLZoomer *z)
{
	//if (zoomer) removeTrack(zoomer);
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
	//if(response.url=="/showScreen.of"){
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
/*
	m.addFloatArg(n );
	mOSCsender_www.sendMessage(m);
	*/

	char b[200];
	getcwd(b, 200);
	cout << b << endl;
	ofstream ofs ("../Resources/www/pos", ofstream::trunc);
	
	ofs << n << endl;

	ofs.close();
}



