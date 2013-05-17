#include <sstream>
#include <string>
#include <sys/time.h>

//#include "ofxConsole.h"
#include "ofxAntescofog.h"
#include "ofxCocoaWindow.h"
#include "ofxTLBeatTicker.h"

int _debug = 0;
static string str_error; // filled by our error()

//int fontsize = OFX_UI_FONT_SMALL;
int fontsize = OFX_UI_FONT_MEDIUM;

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
    bAutoScroll = false;
    bColorSetupInitDone = false;
    bOSCSetupInitDone = false;
    bShowError = false;
    bErrorInitDone = false;
    bEditorShow = true;
    bSetupDone = false;
    editor = 0;
    bShouldRedraw = true;
    audioTrack = NULL;
	
    if (argc > 1) mScore_filename = argv[1];

		gettimeofday(&last_draw_time, 0);
}


void ofxAntescofog::menu_item_hit(int n)
{
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
						ofxAntescofoNote->clear(); // TODO ask for Saving file
            loadScore(mScore_filename);
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
            if (!bShowColorSetup) {
                timeline.disable();
                bShowColorSetup = true;
                guiBottom->disable();
            } 
            break;
        case INT_CONSTANT_BUTTON_OSCSETUP:
            if (!bShowOSCSetup) {
                timeline.disable();
                bShowColorSetup = false;
                bShowOSCSetup = true;
                guiBottom->disable();
            }
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
            mOSCsender.sendMessage(m);
						break;
        }
        case INT_CONSTANT_BUTTON_STOP:
        {
            ofxOscMessage m;
            m.setAddress("/antescofo/cmd");
            m.addStringArg("stop");
            mOSCsender.sendMessage(m);
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
    // . load
    NSString* txt = [@"" stringByAppendingString:@"Load score"];
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
    id saveAsMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Save Score As" action:@selector(menu_item_hit:) keyEquivalent:@"s"] autorelease];
    [saveAsMenuItem setTag:INT_CONSTANT_BUTTON_SAVE_AS];
    [fileMenu addItem:saveAsMenuItem];


    [fileMenuItem setSubmenu:fileMenu];
    [menubar addItem:fileMenuItem];

    //////////////////
    // Edit
    id editMenu = [[[NSMenu new] autorelease] initWithTitle:@"Edit"];
    id editMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Edit" action:NULL keyEquivalent:@""] autorelease];
    // . select all
    id selectMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Select All" action:@selector(menu_item_hit:) keyEquivalent:@"A"] autorelease];
    [editMenuItem setTag:INT_CONSTANT_BUTTON_SELECTALL];
    [editMenu addItem:selectMenuItem];
    [editMenuItem setSubmenu:editMenu];
    [menubar addItem:editMenuItem];

    //////////////////
    // Transport
    id transMenu = [[[NSMenu new] autorelease] initWithTitle:@"Transport"];
    id transMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Transport" action:NULL keyEquivalent:@""] autorelease];
    // . Play
    id playMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Play" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
    [playMenuItem setTag:INT_CONSTANT_BUTTON_PLAY];
    [transMenu addItem:playMenuItem];
    // . Start
    id startMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Start" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
    [startMenuItem setTag:INT_CONSTANT_BUTTON_START];
    [transMenu addItem:startMenuItem];
    // . Stop
    id stopMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Stop" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
    [stopMenuItem setTag:INT_CONSTANT_BUTTON_STOP];
    [transMenu addItem:stopMenuItem];
 
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

    [viewMenuItem setSubmenu:viewMenu];
    [menubar addItem:viewMenuItem];


    //////////////////
    // Help
    id helpMenu = [[[NSMenu new] autorelease] initWithTitle:@"Help"];
    id helpMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Help" action:@selector(menu_item_hit:) keyEquivalent:@""] autorelease];
    //[helpMenu addItem:helpMenuItem];
    [helpMenuItem setSubmenu:helpMenu];
    [menubar addItem:helpMenuItem];

    //guiTop = new ofxUICanvas(0, 0, ofGetWindowWidth(), mUIbottom_y);//score_y);
    //guiBottom = new ofxUICanvas(0, 2*ofGetWindowHeight()/3, ofGetWindowWidth(), 1*ofGetWindowHeight()/3);
    guiBottom = new ofxUICanvas(0, 0, score_x+score_w, score_y);
    guiSetup_OSC = new ofxUICanvas(score_x + 50, score_y, ofGetWindowWidth(), ofGetWindowHeight());
    guiError = new ofxUIScrollableCanvas(score_x, score_y, ofGetWindowWidth(), ofGetWindowHeight()-100-score_y);
    guiSetup_Colors = new ofxUICanvas(0, 0, ofGetWindowWidth(), ofGetWindowHeight());

    guiBottom->setFont("DroidSansMono.ttf");
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
    guiBottom->addWidgetDown(mLabelAccompSpeed);
    mLabelAccompSpeed = new ofxUILabel("0", fontsize);
    guiBottom->addWidgetRight(mLabelAccompSpeed);

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
    /*
    logoInria = ofRectangle(600, 7, 150, 40);
    ofSetColor(255);
    ofFill();
    ofRect(logoInria);
    */

    //guiTop->disable(); guiBottom->disable(); guiTop->setVisible(false); guiBottom->setVisible(false);


    //ofAddListener(guiTop->newGUIEvent, this, &ofxAntescofog::guiEvent);
    ofAddListener(guiError->newGUIEvent, this, &ofxAntescofog::guiEvent);
    ofAddListener(guiBottom->newGUIEvent, this, &ofxAntescofog::guiEvent);
    ofAddListener(guiSetup_Colors->newGUIEvent, this, &ofxAntescofog::guiEvent);
    ofAddListener(guiSetup_OSC->newGUIEvent, this, &ofxAntescofog::guiEvent);

}

void ofxAntescofog::setupOSC(){
	// listen for OSC
	//mOsc_port = 3002;
	int port;
	std::istringstream is(mOsc_port);
	is >> port;
	if (! is.good())
	{
		//cerr << "Not a number, try again." << endl;
		mOsc_port = "6789";
	}
	std::cout << "Listening on OSC port " << port << endl;
	try {
		mOSCreceiver.setup(atoi(mOsc_port.c_str()));
	} catch(exception& e) { 
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
	is.str(mOsc_port_MAX);
	is >> port;
	if (! is.good())
	{
		//cerr << "Not a number, try again." << endl;
		mOsc_port_MAX = "5678";
	}
	std::cout << "Connecting OSC on " << mOsc_host << ":"<< atoi(mOsc_port_MAX.c_str()) << endl;
	mOSCsender.setup(mOsc_host, atoi(mOsc_port_MAX.c_str()));
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
}

//--------------------------------------------------------------
void ofxAntescofog::update() {
    if (!bSetupDone)
        return;
	//if (!bEditorShow) score_w = ofGetWindowWidth() - score_x;
	//else score_w = ofGetWindowWidth() - CONSTANT_EDITOR_VIEW_WIDTH;
    timeline.setWidth(score_w);
    timeline.setOffset(ofVec2f(score_x, score_y));

#if 0
  // stick to the bottom of the window
	//ofxTLZoomer* o = timeline.getZoomer();
	ofRectangle r = ofxAntescofoAction->getDrawRect();
	r.y = ofGetHeight() - r.height;
	ofxAntescofoAction->setDrawRect(r);
//#else
	// stick to the bottom of the window
	ofxTLZoomer* z = timeline.getZoomer();
	ofRectangle rz = z->getDrawRect(); 
	ofRectangle ra = ofxAntescofoAction->getDrawRect();
	ra.height = ofGetHeight() - ra.y - rz.height;
	ofxAntescofoAction->setDrawRect(ra);
#endif

    // check for waiting messages
    while( mOSCreceiver.hasWaitingMessages() )
    {
        // get the next message
        ofxOscMessage m;
        mOSCreceiver.getNextMessage( &m );
        if (_debug) cout << "OSC received: '" << m.getAddress() <<"' ";// << "' args:"<<m.getNumArgs()<< endl;
        for (int i = 0; i < 20; i++) mOSCmsg_string[i] = 0;
        if(m.getAddress() == "/antescofo/stop"){
		if (audioTrack) audioTrack->fakeStop();
		mHasReadMessages = true;
	}else if(m.getAddress() == "/antescofo/tempo" && m.getArgType(0) == OFXOSC_TYPE_FLOAT) {
	    mOsc_tempo = m.getArgAsFloat(0);
            if (_debug) cout << "OSC received: tempo: "<< mOsc_tempo << endl;
            bpm = mOsc_tempo;
            //timeline.setBPM(bpm);
            mSliderBPM->setValue(bpm);
            mHasReadMessages = true;
	} /*else if(m.getAddress() == "/antescofo/rnow" && m.getArgType(0) == OFXOSC_TYPE_FLOAT) {
	    mOsc_rnow = m.getArgAsFloat(0);
            if (_debug) cout << "OSC received: rnow: "<< mOsc_rnow << endl;
            bpm = mOsc_rnow;
            //timeline.setBPM(bpm);
            //mSliderBPM->setValue(bpm);
            mHasReadMessages = true;
	} */else if(m.getAddress() == "/antescofo/event_beatpos" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
            mOsc_beat = m.getArgAsFloat(0);
            mLabelBeat->setLabel(ofToString(mOsc_beat));
            if (_debug) cout << "OSC received: beat: "<< mOsc_beat << endl;
	    if (mOsc_beat == 0)
		    ofxAntescofoNote->missedAll();
            mHasReadMessages = true;
	} else if(m.getAddress() == "/antescofo/rnow" && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
            mOsc_rnow = m.getArgAsFloat(0);
            //mLabelBeat->setLabel(ofToString(mOsc_rnow));
            if (_debug) cout << "OSC received: rnow: "<< mOsc_rnow << endl;
            mHasReadMessages = true;
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
	    bShouldRedraw = true;
	} else if(m.getAddress() == "/antescofo/loadscore"  && m.getArgType(0) == OFXOSC_TYPE_STRING){
            string scorefile = m.getArgAsString(0);
            if (_debug) cout << "OSC received: loadscore: "<< scorefile << endl;
            mHasReadMessages = true;
	    loadScore(scorefile);
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
			break;
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
	if (bShowColorSetup) {
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_ColorSetup();
	} else if (bShowOSCSetup) {
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_OSCSetup();
	} else if (bShowError) {
		ofSetColor(ofxAntescofoNote->color_staves_bg);
		ofRect(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
		draw_error();
	} else {
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

			timeline.draw();
			guiBottom->draw();
			console->draw();
			ofPopStyle();

			// logos
			//mLogoInria.draw(score_w - 290, 2, 148, 54);
			//mLogoIrcam.draw(score_w - 120, 0, 109, 64);
			mLogoIrcam.draw(score_w - 170, 0, 109, 64);
		
			drawCache.end();

			//ofBackground(255, 255, 255, 255);
			drawCache.draw(0, 0);
			bShouldRedraw = false;
		}
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
        mOsc_port       = settings.getValue("osc:port", "3002");
        mOsc_port_MAX   = settings.getValue("osc:portremote", "3003");
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
		const char *normal_keywords = "bpm note chord trill multi variance tempo on off";
		const char *major_keywords = "gfwd group lfwd loop cfwd curve kill abort killof abortof whenever";
		const char *procedure_keywords = "@macro_def @fun_def @insert #include let until expr tight grain guard map imap loc glob if := s ms";
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

		if (mScore_filename.size() && access(mScore_filename.c_str(), R_OK))
			[ editor loadFile:mScore_filename];
		else if (access(mScore_filename.c_str(), R_OK))
			[ editor loadFile:TEXT_CONSTANT_TEMP_FILENAME ];
	} else {
		if (editor) {
			[ editor die];
			score_w = ofGetWindowWidth() - score_x;
			cocoaWindow->setWindowShape(ofGetWidth() - CONSTANT_EDITOR_VIEW_WIDTH, ofGetHeight());
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
        for (map<string, string*>::const_iterator c = oscString2var.begin(); c != oscString2var.end(); c++) {
            guiSetup_OSC->addWidgetDown(new ofxUILabel(c->first, OFX_UI_FONT_MEDIUM));
            guiSetup_OSC->addWidgetRight(new ofxUITextInput(300, c->first, *(c->second), OFX_UI_FONT_MEDIUM));
        }
        
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

#define OF_KEY_TAB      9

//--------------------------------------------------------------
void ofxAntescofog::keyPressed(int key){
    // space key is cancel on error window
    if(key == ' ') {
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
    if(key == 'e'){
			setEditorMode(!bEditorShow, 0);
		} 
		if(key == 'c'){
			console->toggleShow();
		}
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
void ofxAntescofog::mouseDragged( int x, int y, int button ){
	bShouldRedraw = true;
}

//--------------------------------------------------------------
void ofxAntescofog::mousePressed( int x, int y, int button ) {
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

    ofDrawBitmapString(ofxAntescofoNote->get_error(), 100, 300);

    guiError->draw();
}

void ofxAntescofog::display_error()
{
    bShowError = true;
    timeline.disable();
    guiBottom->disable();
    guiError->enable();
    guiError->setVisible(true);
    if (!bErrorInitDone){
        guiError->addWidgetRight(new ofxUILabel(ofGetWindowWidth()/2, 100, string(TEXT_CONSTANT_PARSE_ERROR), OFX_UI_FONT_MEDIUM));
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
			string str = "ofxAntescofog: trying to load Music XML file :" + dragInfo.files[i];
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


void ofxAntescofog::saveScore() {
	// for now just save the content of the text editor
	if (bEditorShow) {
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


int ofxAntescofog::loadScore(string filename) {
	ofxAntescofoNote->clear_error();
	cout << "Trying to load score : " << filename << endl;
	// save zoom view range
	ofRange z = ofxAntescofoZoom->getViewRange(); 

	// save editor position
	int lineEditor = [ editor getCurrentPos];
	cout << "Editor cur line: " << lineEditor << endl;

	string antescore = filename;
	ofxAntescofoNote->clear_actions();
	int n = ofxAntescofoNote->loadscoreMusicXML(filename, TEXT_CONSTANT_TEMP_FILENAME);
	mScore_filename = TEXT_CONSTANT_TEMP_FILENAME;
	if (!n) {
		mScore_filename = antescore = filename;

		n = ofxAntescofoNote->loadscoreAntescofo(antescore);
	}
	if (n) {
		bShowError = false;
		guiError->disable();
		timeline.enable();
		guiBottom->enable();

		// ensure zoom is restored
		ofxAntescofoNote->setZoomBounds(z); //timeline.getZoomer()->setViewRange(z);
		ofxTLAntescofoAction* actiontrack = ofxAntescofoNote->getActionTrack();
		if (actiontrack) actiontrack->setZoomBounds(z);
		timeline.getTicker()->setZoomBounds(z);

		bpm = timeline.getBPM();
		mSliderBPM->setValue(bpm);
		update();

		// send OSC read msg to Antescofo
		ofxOscMessage m;
		m.setAddress("/antescofo/cmd");
		m.addStringArg("read");
		m.addStringArg(mScore_filename);
		cout << "Sending OSC \"read "<< mScore_filename << "\" to Antescofo Patch." << endl;
		mOSCsender.sendMessage(m);
	} else {
		//mScore_filename = TEXT_CONSTANT_TEMP_FILENAME;
		display_error();
	}

	// update editor if opened
	if (bEditorShow) {
		[ editor loadFile:mScore_filename ];
		cout << "Editor scrolling to pos: " << lineEditor << endl;
		[ editor gotoPos:lineEditor]; //[ editor scrollLine:lineEditor];
		int n = [ editor getNbLines ];
		[ editor scrollLine:(30) ];

	}

	bShouldRedraw = true;
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
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_PLAY)
    {
	    ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
	    cout << "Play button change: " << b->getValue() << endl;
	    if (b->getValue() == 1) {
		    ofxOscMessage m;
		    m.setAddress("/antescofo/cmd");
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
			    m.addStringArg("prevevent");
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
        timeline.enable();
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
            guiSetup_OSC->disable();
        }

        e.widget->toggleVisible();
        ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
        b->setValue(false);
        timeline.enable();
        guiBottom->enable();
    }
}

void ofxAntescofog::editorDoubleclicked(int line)
{
	ofxAntescofoNote->showNote(line);
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


