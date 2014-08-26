#import "ofxCodeEditor.h"

#import "ScintillaView.h"
#import "ScintillaCocoa.h"
#import "InfoBar.h"

#define USE_EDITOR_TABS 1

static const int MARGIN_SCRIPT_FOLD_INDEX = 1;

/*
   @implementation MyScintillaView

   - (void)notification: (Scintilla::SCNotification*)notification
   {
   cout << "MyScintillaView: notify " << endl;

   }
   @end
   */

@implementation ofxCodeEditor
//@synthesize delegate;

@synthesize delegate = mDelegate;

- (void)notification: (Scintilla::SCNotification*)notification
{
	cout << "MyScintillaView: notify " << endl;
	//[self.delegate notification:self];
}

- (void) set_normal_keywords: (const char*)normal_keywords_ {
	normal_keywords = normal_keywords_;
}

- (void) set_major_keywords: (const char*)major_keywords_ {
	major_keywords = major_keywords_;
}

- (void) set_procedure_keywords: (const char*)procedure_keywords_ {
	procedure_keywords = procedure_keywords_;
}

- (void) set_system_keywords: (const char*)system_keywords_ {
	system_keywords = system_keywords_;
}

- (void) set_action_keywords: (vector<string>&)system_keywords_ {
	action_keywords = system_keywords_;

	if (action_keywords.size()) dic_keywords.insert(dic_keywords.end(), action_keywords.begin(), action_keywords.end());
}

- (void) pushback_keywords: (const char*)keyw {
	if (keyw) {
		string tmp;
		for (int i = 0; i < strlen(keyw); i++) {
			if (keyw[i] != ' ')
				tmp.push_back(keyw[i]);
			else {
				dic_keywords.push_back(tmp);
				tmp.clear();
			}
			//NSLog(@"--> PUSH auto-completion dic: %s", tmp.c_str());
		}
		if (tmp.size()) // last elt
			dic_keywords.push_back(tmp);
	}
}


- (void) setAutoCompleteOn
{
	if (dic_keywords.size())
		return;
	vector<string>::iterator i = std::unique (dic_keywords.begin(), dic_keywords.end());
	dic_keywords.resize(i - dic_keywords.begin());

	if (normal_keywords) [self pushback_keywords:normal_keywords];
	if (major_keywords) [self pushback_keywords:major_keywords];
	if (procedure_keywords) [self pushback_keywords:procedure_keywords];
	if (system_keywords) [self pushback_keywords:system_keywords];
	if (action_keywords.size()) dic_keywords.insert(dic_keywords.end(), action_keywords.begin(), action_keywords.end());

	std::sort(dic_keywords.begin(), dic_keywords.end());
	//for (int i = 0; i < dic_keywords.size(); i++) NSLog(@"--> auto-completion dic: %s", dic_keywords[i].c_str());

	int cnt = 0;
	for (int i = 0; i < dic_keywords.size(); i++) {
		cnt += dic_keywords[i].size();
	}
	dic_char_list = new char[cnt + dic_keywords.size() + 2];
	int j = 0;
	for (int i = 0; i < dic_keywords.size(); i++) {
		for (int c = 0; c < dic_keywords[i].size(); c++, j++)
			dic_char_list[j] = dic_keywords[i][c];
		j++;
		dic_char_list[j] = ' ';
		j++;
	}
	//[mEditor setReferenceProperty: SCI_SETKEYWORDS parameter: 8 value:dic_char_list];
	//[mEditor setGeneralProperty: SCI_AUTOCSHOW parameter:(uptr_t)0 value:(sptr_t)dic_char_list];
}

- (void) autocomplete
{
	// get last word:
	int pos = [self getCurrentPos];
	int line = [self getCurrentLine];

	bool notspace = true;
	string prefix;
	pos--;
	while (pos >= 0) {
		char ch = (char)[mEditor getGeneralProperty:SCI_GETCHARAT parameter:pos];
		if (ch == ' ' || ch == ',' || ch == '.' || ch == '\n' || ch == '(' || ch == ')'
				|| ch == '[' || ch ==']' || ch == '{'||  ch == '}')
			break;
		prefix = ch + prefix;;
		//NSLog(@"autocomplete: last char='%c' prefix=%s", ch, prefix.c_str());
		pos--;
	}

	NSLog(@"autocomplete: prefix=\"%s\"  dic size:%ld", prefix.c_str(), dic_keywords.size());

	curr_dic_keywords.clear();
	// now we've got our prefix, for each prefix char, we filter the full list
	for (int j = 0; j < dic_keywords.size(); j++) {
		int sz = MIN(prefix.size(), dic_keywords[j].size());
		int i = 0; 
		for (;i < sz; i++) {
			if (tolower(prefix[i]) != tolower(dic_keywords[j][i]))
				break;
		}
		if (i == sz) { // if for each letter of prefix, the dic words matches
			string s = dic_keywords[j].substr(sz, dic_keywords[j].size());
			if (0) { // completion list contains only the possible rests of words..
				NSLog(@"autocomplete: adding \"%s\"", dic_keywords[j].c_str());
				curr_dic_keywords.push_back(dic_keywords[j]);
			} else {
				NSLog(@"autocomplete: adding \"%s\"", s.c_str());
				curr_dic_keywords.push_back(s);
			}
		}
	}
	int cnt = 0;
	for (int i = 0; i < curr_dic_keywords.size(); i++) {
		cnt += curr_dic_keywords[i].size();
	}
	if (cnt) {
		if (curr_dic_char_list) { delete[] curr_dic_char_list; curr_dic_char_list = 0; }
		curr_dic_char_list = new char[cnt + curr_dic_keywords.size()];
		int j = 0;
		for (int i = 0; i < curr_dic_keywords.size(); i++) {
			if (i >= 1) {
				curr_dic_char_list[j] = ' ';
				j++;
			}
			//NSLog(@"autocomplete: j=%d adding to list \"%s\"", j, curr_dic_keywords[i].c_str());
			for (int c = 0; c < curr_dic_keywords[i].size(); c++, j++)
				curr_dic_char_list[j] = curr_dic_keywords[i][c];

			//NSLog(@"autocomplete: So eventually: list is \"%s\"", curr_dic_char_list);
		}
		curr_dic_char_list[j] = 0;
		NSLog(@"autocomplete: So eventually: list is \"%s\"", curr_dic_char_list);

		[mEditor setScreen:[mWindow screen]];
		[mEditor setGeneralProperty: SCI_AUTOCSHOW parameter:(uptr_t)0 value:(sptr_t)curr_dic_char_list];
	} else
		NSLog(@"autocomplete: nothing to complete.");
}

- (void) setup: (NSWindow*) window glview: (NSView*) glview rect: (ofRectangle&) rect {
	NSLog(@"ofxCodeEditor: setup: %.1f, %.1f : %.1fx%.1f", rect.x, rect.y, rect.width, rect.height);
	NSRect r;
	r.origin.x = rect.x;
	r.origin.y = rect.y;
	r.size.width = rect.width;
	r.size.height = rect.height;

	mGLview = glview;
	mWindow = window;
	editorContent = nil;
	mEditor = [[[ScintillaView alloc] initWithFrame:r] autorelease];
	[mEditor setScreen:[window screen]];

	[mEditor setOwner:mWindow];

	[mEditor setBounds:NSMakeRect(0, 0, rect.width, rect.height)];

	//[mEditor setBorderType:NSNoBorder];
	//[mEditor setHasVerticalScroller:YES];
	//[mEditor setHasHorizontalScroller:NO];
	[mEditor setAutoresizingMask:NSViewWidthSizable];
	[glview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];


	NSRect s;
	[window setContentView:nil];
	s = [glview frame]; s.size.width -= rect.width; [ glview setFrame:s];


	s = [glview bounds];
	NSLog(@"ofxCodeEditor: setup: GLView bounds: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);
	s = [glview frame];
	NSLog(@"ofxCodeEditor: setup: GLView frame: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);
	s = [mEditor bounds];
	NSLog(@"ofxCodeEditor: setup: ScintillaView bounds: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);
	s = [mEditor frame];
	NSLog(@"ofxCodeEditor: setup: ScintillaView frame: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);


	s = [[window contentView] bounds];
	NSLog(@"ofxCodeEditor: setup: window bounds: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);
	s = [[window contentView] frame];
	NSLog(@"ofxCodeEditor: setup: window frame: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);


	splitView = [[NSSplitView alloc] initWithFrame:[[window contentView] frame]];
	SplitViewDelegate *splitViewDelegate = [[SplitViewDelegate alloc] init];
	[splitView setVertical:true];

	//CGFloat dividerThickness = [splitView dividerThickness];
	//[scrollview frame].origin.x += dividerThickness; [scrollview frame].size.width -= dividerThickness;
	[splitView setDelegate:splitViewDelegate];
	NSLog(@"ofxCodeEditor: setup: splitview: adding glview");
	[splitView addSubview:glview];// positioned:NSWindowAbove relativeTo:nil];
	NSLog(@"ofxCodeEditor: setup: splitview: adding scintillaview");

#if USE_EDITOR_TABS
	// tabsView
	tabsView = [[NSView alloc] initWithFrame:[mEditor frame]];
	NSRect tf = [mEditor frame];
	[tabsView setBounds:tf];

	[tabsView addSubview:mEditor];
	mCurrentTabEditor = 0;
	[splitView addSubview:tabsView];
#else
	// right: direct view
	[splitView addSubview:mEditor];
#endif

	mEditorsList.push_back(mEditor);

	s = [splitView frame];
	NSLog(@"ofxCodeEditor: setup: splitView frame: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);
	s = [splitView bounds];
	NSLog(@"ofxCodeEditor: setup: splitView bounds: %.1f, %.1f : %.1fx%.1f", s.origin.x, s.origin.y, s.size.width, s.size.height);

	NSLog(@"ofxCodeEditor: setup: splitviews subviews count %d, setting splitview to window contentView", [[splitView subviews] count]);
	[window setContentView:splitView];
	NSLog(@"ofxCodeEditor: setup: window subviews count %d", [[[window contentView] subviews] count]);
	//[splitView release];

	[self setupEditor:mEditor];
}

- (void) loadFileInTab:(NSString*)insertedfile
{
	NSLog(@"ofxCodeEditor: loadFileInTab: [%@]", insertedfile);
	mEditorsFilenames.push_back([insertedfile UTF8String]);

	NSError* error = nil;
	NSString *insertedfile_fullpath = [[NSString stringWithUTF8String:mFilePath.c_str()] stringByAppendingString:insertedfile];

	editorContent = [NSString stringWithContentsOfFile:insertedfile_fullpath
		encoding:NSUTF8StringEncoding
		error: &error];
	if (error && [[error domain] isEqual: NSCocoaErrorDomain]) {
		NSLog(@"%@", error);
		editorContent = [NSString stringWithContentsOfFile:insertedfile_fullpath
			encoding:NSMacOSRomanStringEncoding
			error: &error];
		if (error && [[error domain] isEqual: NSCocoaErrorDomain])
			NSLog(@"%@", error);
	}
	editorContentsList.push_back(editorContent);

	// create space for Tab Bar
	NSRect bounds = [mEditor bounds];
	[mEditor setBounds:NSMakeRect(bounds.origin.x, 20, bounds.size.width, bounds.size.height)]; // XXX diminuer H
	bounds = [mEditor bounds];
	NSRect frame = [mEditor frame];

	[self tabCreate:[insertedfile UTF8String] index:editorContentsList.size()-1];

	// create editor instance for this tab
	ScintillaView* anEditor = [[[ScintillaView alloc] initWithFrame:frame] autorelease];
	[anEditor setScreen:[mWindow screen]];

	[anEditor setOwner:tabsView];

	[anEditor setString: editorContent];
	[anEditor setBounds:bounds];
	[anEditor setAutoresizingMask:NSViewWidthSizable];
	[tabsView addSubview:anEditor positioned:NSWindowBelow relativeTo:mEditor];

	[self setupEditor:anEditor];
	mEditorsList.push_back(anEditor);
}

- (void) tabEdited
{
	if (mTabButtons.size()) {
		NSString* t = [mTabButtons[mCurrentTabEditor] title];
		if (t && [t characterAtIndex:0] != '*') {
			NSString* s = @"* ";
			s = [s stringByAppendingString:t];
			[mTabButtons[mCurrentTabEditor] setTitle:s];
		}
	}
}

- (void) tabCreate:(string)tabname index:(int)index
{
	int tabsize = 180;

	NSRect frame = [mEditor frame];
	int x = frame.origin.x + index*tabsize;
	//if (index) x += 20;
	NSButton *btn = [[NSButton alloc] initWithFrame:NSMakeRect(x, frame.size.height - 20, tabsize, 20)];
	frame = [mEditor frame];
	NSRect btnbounds = [btn bounds];
	NSRect btnframe = [btn frame];
	NSRect bounds = [mEditor bounds];
	//cout << "ofxCodeEditor: tabCreate: " << tabname << " index=" << index << endl; 
	cout << "ofxCodeEditor: Editor frame : " << frame.origin.x << ", "<< frame.origin.y << " - " << frame.size.width << " x "<< frame.size.height << endl;
	cout << "ofxCodeEditor: Editor bounds : " << bounds.origin.x << ", "<< bounds.origin.y << " - " << bounds.size.width << " x "<< bounds.size.height << endl;
	cout << "ofxCodeEditor: button frame : " << btnframe.origin.x << ", "<< btnframe.origin.y << " - " << btnframe.size.width << " x "<< btnframe.size.height << endl;
	cout << "ofxCodeEditor: button bounds : " << btnbounds.origin.x << ", "<< btnbounds.origin.y << " - " << btnbounds.size.width << " x "<< btnbounds.size.height << endl;

	[[btn cell] setControlSize:NSRegularControlSize];
	//[[btn cell] setHighlightsBy:NSCellLightsByBackground];
	//[btn setButtonType:NSMomentaryPushInButton];
	[btn setButtonType:NSOnOffButton];
	[btn setBordered:YES];
	[btn setBezelStyle:NSSmallSquareBezelStyle]; //NSThickSquareBezelStyle];
	[btn setImagePosition:NSNoImage];
	//[btn setAlignment:NSLeftTextAlignment]; 
	[btn setAlignment:NSCenterTextAlignment];
	//[[btn cell] setControlTint:NSBlueControlTint];
	[btn setEnabled:YES];
	[btn setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[btn setTitle:[NSString stringWithUTF8String:tabname.c_str()]];
	[btn setTarget:self];
	[btn setTag:index];
	[btn setAction:@selector(tab_pressed:)];

	[tabsView addSubview:btn positioned:NSWindowAbove relativeTo:mEditor];
	mTabButtons.push_back(btn);
}

- (void) tab_pressed:(id)sender
{
	int tabnb = [sender tag];
	//NSLog(@"ofxCodeEditor: -------> Tab %d pressed.", [sender tag]);

	if (tabnb <= mEditorsList.size()) {

		NSLog(@"ofxCodeEditor: -------> Editors:%lu Swapping Views: %d and %d.", mEditorsList.size(), mCurrentTabEditor, tabnb);

		//[mEditorsList[tabnb] setOwner:mWindow]; // mWindow
		//[mEditorsList[mCurrentTabEditor] setOwner:tabsView]; // mWindow
		
		// change other tabs to color back
		NSButton* btn;
		for (int i = 0; i < mTabButtons.size(); i++) {
			btn = mTabButtons[i];
			[[btn cell] setState:YES];
		}

		ScintillaView* viewfrom = mEditorsList[mCurrentTabEditor];
		ScintillaView* viewto = mEditorsList[tabnb];

		// change viewto tab size 
		[viewto setFrame:[viewfrom frame]];
		[viewto setBounds:[viewfrom bounds]];

		[viewfrom setOwner:tabsView];
		[viewto setOwner:tabsView];

		//if (mCurrentTabEditor != 0)
		[viewfrom retain];
		//[viewto retain];

		[tabsView replaceSubview:viewfrom with:viewto];

		mEditor = mEditorsList[tabnb];

		//[viewto release];
		//[viewfrom release];
		mCurrentTabEditor = tabnb;

		editorContent = editorContentsList[tabnb];
		[mWindow setTitle:[NSString stringWithUTF8String:mEditorsFilenames[tabnb].c_str()]];

		// change tab button title
		btn = mTabButtons[tabnb];
		[[btn cell] setState:NO];
	}

}

/*
   Find inserted files (for example for macros) and open them in tabbed editors
 */
- (void) checkForInsertedFiles
{
	//if (!editorContent) return;

	NSString* scorecontent = editorContent;
	int len = [editorContent length];
	// search for @insert "
	int n = 0;
	NSRange fullrange = NSMakeRange(0, len);
	NSRange range = [editorContent rangeOfString:@"@insert \"" options:NSCaseInsensitiveSearch range:fullrange];
	while (range.length > 0) {
		if (n == 0) {
			[self tabCreate:mEditorsFilenames[n] index:n];
			n++;
		}

		range.location += 1;
		range.length -= 1;
		NSString *substring = [[scorecontent substringFromIndex:NSMaxRange(range)] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
		NSRange quoterange = [substring rangeOfString:@"\""];
		if (quoterange.location != NSNotFound)
		{
			n++;
			NSString *insertedfile = [substring substringToIndex:quoterange.location];
			NSLog(@"ofxCodeEditor: checkForInsertedFiles: [%@]\nquoterange:%d-%d\n", insertedfile, quoterange.location, quoterange.length);
			[self loadFileInTab:insertedfile];

		}
		len -= range.location + range.length;
		fullrange = NSMakeRange(range.location + range.length, len);
		//NSLog(@"ofxCodeEditor: checkForInsertedFiles: searching for in range: %d-%d", fullrange.location, fullrange.length);
		range = [scorecontent rangeOfString:@"@insert \"" options:NSCaseInsensitiveSearch range:fullrange];
	}

	if (n) {
		// set all tabs to no selected except first
		for (int i = 0; i < mTabButtons.size(); i++) {
			NSButton* btn = mTabButtons[i];
			[[btn cell] setState:YES];
		}
		NSButton* btn = mTabButtons[0];
		[[btn cell] setState:NO];

		// set view on top :
		mEditor = mEditorsList[0];
		[mEditor removeFromSuperview];
		[tabsView addSubview:mEditor positioned:NSWindowAbove relativeTo:nil];

	}
}

- (string) tab_get_filename
{
	//cout << "tab_get_filename: mCurrentTabEditor=" << mCurrentTabEditor << " mEditorsFilenames.size()=" << mEditorsFilenames.size() << " path=" << mFilePath << endl;
		//" et " << [ mEditorsFilenames[mCurrentTabEditor] UTF8String ]<< endl;
	string s = mFilePath + mEditorsFilenames[mCurrentTabEditor];

	return s;
}

//--------------------------------------------------------------------------------------------------

typedef void(*SciNotifyFunc) (intptr_t windowid, unsigned int iMessage, uintptr_t wParam, uintptr_t lParam);


/**
 * Initialize scintilla editor (styles, colors, markers, folding etc.].
 */
- (void) setupEditor: (ScintillaView*)editor
{  
	editorContent = 0;
	bMatchCase = false;
	bWrapMode = YES;
	[editor setGeneralProperty: SCI_SETLEXER parameter: SCLEX_ANTESCOFO value: (sptr_t) "Antescofo"];
	// alternatively: [mEditor setEditorProperty: SCI_SETLEXERLANGUAGE parameter: nil value: (sptr_t) "mysql"];

	// Number of styles we use with this lexer.
	[editor setGeneralProperty: SCI_SETSTYLEBITS value: [mEditor getGeneralProperty: SCI_GETSTYLEBITSNEEDED]];

	// Keywords to highlight. Indices are:
	// 0 - Major keywords (reserved keywords)
	// 1 - Normal keywords (everything not reserved but integral part of the language)
	// 2 - Database objects
	// 3 - Function keywords
	// 4 - System variable keywords
	// 5 - Procedure keywords (keywords used in procedures like "begin" and "end")
	// 6..8 - User keywords 1..3
	if (major_keywords)
		[editor setReferenceProperty: SCI_SETKEYWORDS parameter: 0 value: major_keywords];
	if (normal_keywords)
		[editor setReferenceProperty: SCI_SETKEYWORDS parameter: 1 value: normal_keywords];
	if (system_keywords)
		[editor setReferenceProperty: SCI_SETKEYWORDS parameter: 4 value: system_keywords];
	if (procedure_keywords)
		[editor setReferenceProperty: SCI_SETKEYWORDS parameter: 5 value: procedure_keywords];
	if (client_keywords)
		[editor setReferenceProperty: SCI_SETKEYWORDS parameter: 6 value: client_keywords];
	if (user_keywords)
		[editor setReferenceProperty: SCI_SETKEYWORDS parameter: 7 value: user_keywords];

	[editor setGeneralProperty: SCI_COLOURISE parameter:-1 value: 0];
	// Colors and styles for various syntactic elements. First the default style.
	//[mEditor setStringProperty: SCI_STYLESETFONT parameter: STYLE_DEFAULT value: @"Helvetica"];
	// [mEditor setStringProperty: SCI_STYLESETFONT parameter: STYLE_DEFAULT value: @"Monospac821 BT"]; // Very pleasing programmer's font.
	//[mEditor setGeneralProperty: SCI_STYLESETSIZE parameter: STYLE_DEFAULT value: 14];
	//[mEditor setColorProperty: SCI_STYLESETFORE parameter: STYLE_DEFAULT value: [NSColor blackColor]];//

	//[mEditor setGeneralProperty: SCI_STYLECLEARALL parameter: 0 value: 0];	

	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_DEFAULT value: [NSColor blackColor]];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_COMMENT fromHTML: @"#097BF7"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_COMMENTLINE fromHTML: @"#097BF7"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_HIDDENCOMMAND fromHTML: @"#097BF7"];
	[editor setColorProperty: SCI_STYLESETBACK parameter: SCE_ANTESCOFO_HIDDENCOMMAND fromHTML: @"#F0F0F0"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_NUMBER fromHTML: @"#7F7F00"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_SQSTRING fromHTML: @"#FFAA3E"];

	// Note: if we were using ANSI quotes we would set the DQSTRING to the same color as the 
	//       the back tick string.
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_DQSTRING fromHTML: @"#274A6D"];

	// Keyword highlighting.


	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_MAJORKEYWORD fromHTML: @"#007F00"];
	[editor setGeneralProperty: SCI_STYLESETBOLD parameter: SCE_ANTESCOFO_MAJORKEYWORD value: 1];

	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_KEYWORD fromHTML: @"#FF0000"];
	[editor setGeneralProperty: SCI_STYLESETBOLD parameter: SCE_ANTESCOFO_KEYWORD value: 1];

	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_PROCEDUREKEYWORD fromHTML: @"#0DB01E"];
	//[mEditor setGeneralProperty: SCI_STYLESETBOLD parameter: SCE_ANTESCOFO_PROCEDUREKEYWORD value: 1];


	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_VARIABLE fromHTML: @"#ff1111"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_SYSTEMVARIABLE fromHTML: @"#FF0000"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_KNOWNSYSTEMVARIABLE fromHTML: @"#0000A5"];


	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_USER1 fromHTML: @"#808080"];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_USER2 fromHTML: @"#808080"];
	[editor setColorProperty: SCI_STYLESETBACK parameter: SCE_ANTESCOFO_USER2 fromHTML: @"#F0E0E0"];

	// The following 3 styles have no impact as we did not set a keyword list for any of them.
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_FUNCTION value: [NSColor redColor]];

	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_IDENTIFIER value: [NSColor blackColor]];
	[editor setColorProperty: SCI_STYLESETFORE parameter: SCE_ANTESCOFO_QUOTEDIDENTIFIER fromHTML: @"#274A6D"];
	[editor setGeneralProperty: SCI_STYLESETBOLD parameter: SCE_ANTESCOFO_OPERATOR value: 1];

	// Line number style.
	[editor setColorProperty: SCI_STYLESETFORE parameter: STYLE_LINENUMBER fromHTML: @"#F0F0F0"];
	[editor setColorProperty: SCI_STYLESETBACK parameter: STYLE_LINENUMBER fromHTML: @"#808080"];

	[editor setGeneralProperty: SCI_SETMARGINTYPEN parameter: 0 value: SC_MARGIN_NUMBER];
	[editor setGeneralProperty: SCI_SETMARGINWIDTHN parameter: 0 value: 40];

	// Markers.
	[editor setGeneralProperty: SCI_SETMARGINWIDTHN parameter: 1 value: 10];

	[editor setLexerProperty: @"braces.check" value: @"1"];

	// Some special lexer properties.
	[editor setLexerProperty: @"fold" value: @"1"];
	[editor setLexerProperty: @"fold.compact" value: @"1"];
	[editor setLexerProperty: @"fold.comment" value: @"1"];
	[editor setLexerProperty: @"fold.preprocessor" value: @"1"];

	// Folder setup.
	[editor setGeneralProperty: SCI_SETMARGINWIDTHN parameter: 2 value: 16];
	[editor setGeneralProperty: SCI_SETMARGINMASKN parameter: 2 value: SC_MASK_FOLDERS];
	[editor setGeneralProperty: SCI_SETMARGINSENSITIVEN parameter: 2 value: 1];
	[editor setGeneralProperty: SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDEROPEN value: SC_MARK_BOXMINUS];
	[editor setGeneralProperty: SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDER value: SC_MARK_BOXPLUS];
	[editor setGeneralProperty: SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDERSUB value: SC_MARK_VLINE];
	[editor setGeneralProperty: SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDERTAIL value: SC_MARK_LCORNER];
	[editor setGeneralProperty: SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDEREND value: SC_MARK_BOXPLUSCONNECTED];
	[mEditor setGeneralProperty: SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDEROPENMID value: SC_MARK_BOXMINUSCONNECTED];
	[editor setGeneralProperty : SCI_MARKERDEFINE parameter: SC_MARKNUM_FOLDERMIDTAIL value: SC_MARK_TCORNER];
	for (int n= 25; n < 32; ++n) // Markers 25..31 are reserved for folding.
	{
		[editor setColorProperty: SCI_MARKERSETFORE parameter: n value: [NSColor whiteColor]];
		[editor setColorProperty: SCI_MARKERSETBACK parameter: n value: [NSColor blackColor]];
	}

	// Init markers & indicators for highlighting of syntax errors.
	[editor setColorProperty: SCI_INDICSETFORE parameter: 0 value: [NSColor redColor]];
	[editor setGeneralProperty: SCI_INDICSETUNDER parameter: 0 value: 1];
	[editor setGeneralProperty: SCI_INDICSETSTYLE parameter: 0 value: INDIC_SQUIGGLE];

	[editor setColorProperty: SCI_MARKERSETBACK parameter: 0 fromHTML: @"#B1151C"];

	[editor setColorProperty: SCI_SETSELBACK parameter: 1 value: [NSColor selectedTextBackgroundColor]];

	// Uncomment if you wanna see auto wrapping in action.
	//[mEditor setGeneralProperty: SCI_SETWRAPMODE parameter: SC_WRAP_WORD value: 0];

	InfoBar* infoBar = [[[InfoBar alloc] initWithFrame: NSMakeRect(0, 0, 400, 0)] autorelease];
	[infoBar setDisplay: IBShowAll];
	[editor setInfoBar: infoBar top: NO];

	[editor setGeneralProperty: SCI_SETTABWIDTH parameter: 4 value: 0];

	intptr_t windowid;
	//[mEditor registerNotifyCallback:windowid value:notify];


	/* TODO try to register for dragndrop
	   [[[splitView subviews] objectAtIndex:1] registerForDraggedTypes: [NSArray arrayWithObjects:
	   NSStringPboardType, ScintillaRecPboardType, NSFilenamesPboardType, nil]];
	   */

	//[mEditor setGeneralProperty: SCI_AUTOCCOMPLETE parameter:0 value:0];
	[self setAutoCompleteOn];
	[mEditor setStatusText: @"Operation complete"];
}

//--------------------------------------------------------------------------------------------------
/**
 * Called when an external drag operation enters the view. 
 */
- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>) sender
{
	NSLog(@"CodeEditor: dragginEntered");
	// return mOwner.backend->DraggingEntered(sender);
}

//--------------------------------------------------------------------------------------------------

/**
 * Called frequently during an external drag operation if we are the target.
 */
- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>) sender
{
	NSLog(@"CodeEditor: draggingU");
	//return mOwner.backend->DraggingUpdated(sender);
}

//--------------------------------------------------------------------------------------------------

/**
 * Drag image left the view. Clean up if necessary.
 */
- (void) draggingExited: (id <NSDraggingInfo>) sender
{
	NSLog(@"CodeEditor: draggingE");
	//mOwner.backend->DraggingExited(sender);
}

//--------------------------------------------------------------------------------------------------

- (BOOL) prepareForDragOperation: (id <NSDraggingInfo>) sender
{
	NSLog(@"CodeEditor: drag prep");
	//return YES;
}

//--------------------------------------------------------------------------------------------------

- (BOOL) performDragOperation: (id <NSDraggingInfo>) sender
{
	NSLog(@"CodeEditor: drag perf");
	//return mOwner.backend->PerformDragOperation(sender);  
}

//--------------------------------------------------------------------------------------------------

/**
 * Returns operations we allow as drag source.
 */
- (NSDragOperation) draggingSourceOperationMaskForLocal: (BOOL) flag
{
	NSLog(@"CodeEditor: drag sour");
	// return NSDragOperationCopy | NSDragOperationMove | NSDragOperationDelete;
}

//--------------------------------------------------------------------------------------------------

/**
 * Finished a drag: may need to delete selection.
 */

- (void) draggedImage: (NSImage *) image endedAt: (NSPoint) screenPoint operation: (NSDragOperation) operation
{
	NSLog(@"CodeEditor: drag im");
	//if (operation == NSDragOperationDelete) mOwner.backend->WndProc(SCI_CLEAR, 0, 0);
}

//--------------------------------------------------------------------------------------------------

/**
 * Drag operation is done. Notify editor.
 */
- (void) concludeDragOperation: (id <NSDraggingInfo>) sender
{
	// Clean up is the same as if we are no longer the drag target.
	//mOwner.backend->DraggingExited(sender);
}



/*j

  - (void) notify(intptr_t windowid, unsigned int iMessage, uintptr_t wParam, uintptr_t lParam)
  {
  cout << "CodeEditor: notify " << iMessage << endl;


//[mEditor setGeneralProperty: SCI_AUTOCCOMPLETE parameter: 0 value: 0];
}
*/

- (void) setWrapMode:(bool)mode
{
	//[mEditor setGeneralProperty: SCI_SETWRAPMODE parameter:mode value:mode];
	bWrapMode = mode;
}

- (void) setMatchCase:(bool)mode
{
	bMatchCase = mode;
}

//--------------------------------------------------------------------------------------------------

/* XPM */
static const char * box_xpm[] = {
	"12 12 2 1",
	" 	c None",
	".	c #800000",
	"   .........",
	"  .   .   ..",
	" .   .   . .",
	".........  .",
	".   .   .  .",
	".   .   . ..",
	".   .   .. .",
	".........  .",
	".   .   .  .",
	".   .   . . ",
	".   .   ..  ",
	".........   "};

/*
   - (void) showAutocompletion
   {
   const char *words = normal_keywords;
   [mEditor setGeneralProperty: SCI_AUTOCSETIGNORECASE parameter: 1 value:0];
   [mEditor setGeneralProperty: SCI_REGISTERIMAGE parameter: 1 value:(sptr_t)box_xpm];
   const int imSize = 12;
   [mEditor setGeneralProperty: SCI_RGBAIMAGESETWIDTH parameter: imSize value:0];
   [mEditor setGeneralProperty: SCI_RGBAIMAGESETHEIGHT parameter: imSize value:0];
   char image[imSize * imSize * 4];
   for (size_t y = 0; y < imSize; y++) {
   for (size_t x = 0; x < imSize; x++) {
   char *p = image + (y * imSize + x) * 4;
   p[0] = 0xFF;
   p[1] = 0xA0;
   p[2] = 0;
   p[3] = x * 23;
   }
   }
   [mEditor setGeneralProperty: SCI_REGISTERRGBAIMAGE parameter: 2 value:(sptr_t)image];
   [mEditor setGeneralProperty: SCI_AUTOCSHOW parameter: 0 value:(sptr_t)words];
   }
   */

- (void) searchText: (string) str backwards:(bool)pBackWards
{
	NSString *text = [NSString stringWithUTF8String:str.c_str() ];
	cout << "CodeEditor: will search text: " << str.c_str() << endl;

	bool res = [mEditor findAndHighlightText: text
		matchCase: bMatchCase
		wholeWord: NO
		scrollTo: YES
		wrap: bWrapMode
		//backwards: YES];
		backwards: pBackWards];

	//if (!res) return;
#if 0
	long matchStart = [mEditor getGeneralProperty: SCI_GETSELECTIONSTART parameter: 0];
	long matchEnd = [mEditor getGeneralProperty: SCI_GETSELECTIONEND parameter: 0];
	[mEditor setGeneralProperty: SCI_FINDINDICATORFLASH parameter: matchStart value:matchEnd];
	//[mEditor setGeneralProperty: SCI_FINDINDICATORSHOW parameter: matchStart value:matchEnd];
	cout << "searchText: " << str << "-->"<< matchStart<< ":" << matchEnd << endl;
	//[mEditor setGeneralProperty: SCI_FINDINDICATORHIDE parameter:nil value:nil];
#endif


	int searchFlags= 0;
	/*if (matchCase)
	  searchFlags |= SCFIND_MATCHCASE;
	  if (wholeWord)
	  searchFlags |= SCFIND_WHOLEWORD;
	  */

	/*
	   NSString *text = [NSString stringWithUTF8String:str.c_str() ];

	   bool result = [ mEditor directCall:mEditor
message: SCI_SEARCHNEXT
wParam: searchFlags
lParam: (sptr_t) text];

if (result)
[self setGeneralProperty: SCI_SCROLLCARET value: 0];
*/
}

-(int) searchNreplaceText:(string)str str2:(string)str2 doAll:(bool)pDoAll
{
	NSString *text1 = [NSString stringWithUTF8String:str.c_str() ];
	NSString *text2 = [NSString stringWithUTF8String:str2.c_str() ];
	cout << "CodeEditor: will searchNreplace for text: " << str << " with text:" << str2 << endl;

	int res = [ mEditor findAndReplaceText: text1
		byText: text2
		matchCase: bMatchCase
		wholeWord: NO
		doAll: pDoAll];
	cout << "searchNreplace Text: res:" << res << endl;
	return res;
}

-(void) searchFinish
{
	[mEditor setGeneralProperty: SCI_FINDINDICATORHIDE parameter:nil value:nil];
}


- (string) getSelection
{
	long matchStart = [mEditor getGeneralProperty: SCI_GETSELECTIONSTART parameter: 0];
	long matchEnd = [mEditor getGeneralProperty: SCI_GETSELECTIONEND parameter: 0];

	long n = matchEnd - matchStart;
	cout << "getSelection: n=" << n << " start:" << matchStart << " end:" << matchEnd << endl;

	if (n > 0) {
		char cstr[n];
		//[mEditor getGeneralProperty:SCI_GETSELTEXT parameter:(sptr_t)cstr];
		[ScintillaView directCall:mEditor message:SCI_GETSELTEXT wParam:(uptr_t)(0) lParam:(sptr_t)cstr];
		string ret(cstr);
		return ret;
	}
	return string("");
}

- (void) braceMatch //:  (int) pos
{
	int pos = [mEditor getGeneralProperty:SCI_GETCURRENTPOS];
	cout << pos << endl;
	int pm = (int)[mEditor getGeneralProperty:SCI_BRACEMATCH parameter:pos];
	//cout << "pm= " << pm << endl;
}


- (void) searchText: (string) str
{
	NSString *text = [NSString stringWithUTF8String:str.c_str() ];
	cout << "CodeEditor: will search for text: " << str.c_str() << endl;

	bool res = [mEditor findAndHighlightText: text
		matchCase: bMatchCase
		wholeWord: NO
		scrollTo: YES
		wrap: bWrapMode
		//backwards: YES];
		backwards: NO];

	//if (!res) return;
	long matchStart = [mEditor getGeneralProperty: SCI_GETSELECTIONSTART parameter: 0];
	long matchEnd = [mEditor getGeneralProperty: SCI_GETSELECTIONEND parameter: 0];
	[mEditor setGeneralProperty: SCI_FINDINDICATORFLASH parameter: matchStart value:matchEnd];
	cout << "searchText: " << str << "-->"<< matchStart<< ":" << matchEnd << endl;
	//[ self showLine:matchStart lineb:matchEnd ];

	//if ([[searchField stringValue] isEqualToString: @"XX"]) [self showAutocompletion];
}

- (void) die {
	NSRect s = [mWindow frame];
	s.size.width -= 400;
	[ mGLview setFrame:s ];
	[ mWindow setContentView:mGLview ];
	//[ splitView release ];
}

- (int) tabsSize
{
	return mEditorsList.size();
}

- (int) tabsHeight
{
	return EDITOR_TAB_HEIGHT;
}


// load text
- (void) loadFile: (string) filename
{
	cout << "ofxCodeEditor: loadfile:" << filename << endl;
	NSError* error = nil;
	mCurrentTabEditor = 0;

	// clear tabs
	bool resetframe = mTabButtons.size();
	for (int i = 0; i < mTabButtons.size(); i++) {
		[ mTabButtons[i] removeFromSuperview];
	}
	mTabButtons.clear();
	/*
	for (int i = 1; i < mEditorsList.size(); i++) {
		[ mEditorsList[i] removeFromSuperview];
	}
	*/

	if (mEditorsList.size() > 1) {
		while(mEditorsList.size() > 1) {
			ScintillaView* s = mEditorsList.back();
			[ s removeFromSuperview ];
			//[ s release ];
			mEditorsList.pop_back();
		}
		mEditor = mEditorsList[0];
	}
	editorContentsList.clear();

	if (resetframe) {
		NSRect frame = [mEditor frame];
		NSRect bounds = [mEditor bounds];
		frame.origin.y = bounds.origin.y = 0;
		frame.size.height = bounds.size.height = [tabsView bounds].size.height;
		[ mEditor setFrame:frame];
		[ mEditor setBounds:bounds];
	}

	mEditorsFilenames.clear();
	mFilePath = ofFilePath::getEnclosingDirectory(filename, false);

	string justfilename = ofFilePath::getFileName(filename);
	mEditorsFilenames.push_back(justfilename);

	NSString* nsfilename = [NSString stringWithUTF8String:justfilename.c_str()];

	/*if (editorContent) {
		[editorContent release];
		editorContent = nil;
	}*/
	[self setupEditor:mEditor];
	editorContent = [NSString stringWithContentsOfFile:nsfilename
		encoding:NSUTF8StringEncoding
		error: &error];
	if (error && [[error domain] isEqual: NSCocoaErrorDomain]) {
		NSLog(@"%@", error);
		editorContent = [NSString stringWithContentsOfFile:[[NSString alloc] initWithCString:filename.c_str()]
			encoding:NSMacOSRomanStringEncoding
			error: &error];
		if (error && [[error domain] isEqual: NSCocoaErrorDomain])
			NSLog(@"%@", error);
	}
	[mEditor setString: editorContent];
	if (!error)
		[mWindow setTitle:[NSString stringWithUTF8String:filename.c_str()]];

	editorContentsList.push_back(editorContent);
#if USE_EDITOR_TABS
	[self checkForInsertedFiles];
#endif
}

- (void) scrollLine: (int) line
{
	[mEditor setGeneralProperty: SCI_LINESCROLL parameter:0 value:line];
}

- (int) getMaxLines
{
	return [mEditor getGeneralProperty:SCI_LINESONSCREEN];
}

- (int) getCurrentPos
{
	return [mEditor getGeneralProperty:SCI_GETCURRENTPOS];

}
- (int) getCurrentLine
{
	return [mEditor getGeneralProperty:SCI_LINEFROMPOSITION parameter:[mEditor getGeneralProperty:SCI_GETCURRENTPOS]];

}


- (void) setCurrentPos:(int)pos
{
	[mEditor setGeneralProperty:SCI_SETCURRENTPOS parameter:pos value:pos];
	[mEditor setGeneralProperty:SCI_SETSEL parameter:pos value:pos];
	[mEditor setGeneralProperty:SCI_SCROLLCARET parameter:pos value:pos];

}

- (void) clear
{
	[mEditor setString: @""];
	[mWindow setTitle: @""];
}

- (int) modified
{
	return [mEditor getGeneralProperty:SCI_GETMODIFY];
}



- (void) gotoPos: (int) pos
{
	[mEditor setGeneralProperty:SCI_GOTOPOS parameter:pos value:pos];
	/*
	[mEditor setGeneralProperty:SCI_SETANCHOR parameter:pos value:pos];
	[mEditor setGeneralProperty:SCI_SETCURRENTPOS parameter:pos value:pos];
	*/
	[mEditor setGeneralProperty:SCI_SETSEL parameter:pos value:pos];


}

- (void) undo
{
	[mEditor setGeneralProperty:SCI_UNDO parameter:0 value:0];
}

- (void) redo
{
	[mEditor setGeneralProperty:SCI_REDO parameter:0 value:0];
}

//void ofxCodeEditor::showLine(int linea, int lineb)
- (void) showLine: (int) linea lineb:(int)lineb cola:(int)cola colb:(int)colb
{
	//[mEditor mBackend]->ScrollText(-1000); [mEditor mBackend]->ScrollText(linea);

	//cout << "ofxCodeEditor: showLine: " << linea << ":"<< cola << " -> " << lineb << ":"<< colb << endl;
	// line scroll ok
	[mEditor setGeneralProperty: SCI_LINESCROLL parameter:0 value:0];
	[mEditor setGeneralProperty: SCI_LINESCROLL parameter:0 value:linea];

	[mEditor setGeneralProperty: SCI_CLEARSELECTIONS value:0];

	[mEditor setGeneralProperty: SCI_SETSEL 
		parameter: [mEditor getGeneralProperty: SCI_FINDCOLUMN parameter:lineb extra:colb]
		value: [mEditor getGeneralProperty: SCI_FINDCOLUMN parameter:linea extra:cola]];
}

- (void) showLine: (int) linea lineb: (int) lineb
{
	//[mEditor mBackend]->ScrollText(-1000); [mEditor mBackend]->ScrollText(linea);

	//cout << "ofxCodeEditor: showLine: " << linea << ":" << lineb << endl;
	// line scroll ok
	[mEditor setGeneralProperty: SCI_LINESCROLL parameter:0 value:0];
	[mEditor setGeneralProperty: SCI_LINESCROLL parameter:0 value:linea];

	[mEditor setGeneralProperty: SCI_CLEARSELECTIONS value:0];

	[mEditor setGeneralProperty: SCI_SETSEL 
		parameter: [mEditor getGeneralProperty: SCI_FINDCOLUMN parameter:lineb]
		value: [mEditor getGeneralProperty: SCI_FINDCOLUMN parameter:linea]];
}
- (int) getNbLines
{
	return [mEditorsList[mCurrentTabEditor] getGeneralProperty: SCI_GETLINECOUNT];
}


- (int) getLenght
{
	return [mEditorsList[mCurrentTabEditor] getGeneralProperty: SCI_GETLENGTH];
}

- (void) setEditable: (bool) ed
{
	[mEditorsList[mCurrentTabEditor] setEditable:ed];
}

- (void) getEditorContent: (string&) content
{
	int n = [self getLenght];
	if (n) {
		cout << "ofxCodeEditor : content len: " << n << endl;
		char cstr[n];

		[ScintillaView directCall:mEditorsList[mCurrentTabEditor] message:SCI_GETTEXT wParam:(uptr_t)(n+1) lParam:(sptr_t)cstr];
		if (cstr) {
			//[ mEditor setGeneralProperty: SCI_SETSAVEPOINT value:0]; // mark the file saved : TODO wait until antescofo parsing is OK ?
			content = cstr;
		}
	}
}

- (void) replaceString:(int)linea lineb:(int)lineb cola:(int)cola colb:(int)colb str:(const char*)str
{
	cout << "editor::replacestring: from " << linea << ":" << cola << " -> " << lineb << ":" << colb << endl;
	[ self showLine:linea lineb:lineb cola:cola colb:colb];
	[ mEditor setGeneralProperty: SCI_REPLACESEL parameter:0 value:(long)str ];
	//[ self showLine:linea lineb:lineb ];
}

- (void) replaceString:(int)linea lineb:(int)lineb str:(const char*)str
{
	cout << "editor::replacestring: from " << linea << " to " << lineb << endl;
	[ self showLine:linea lineb:lineb ];
	[ mEditor setGeneralProperty: SCI_REPLACESEL parameter:0 value:(long)str ];
	//[ self showLine:linea lineb:lineb ];
}

- (void) insertStringAtPos:(int)posa posb:(int)posb str:(const char*)str;
{
	cout << "editor::insertstringAtPos: from " << posa << " to " << posb << endl;
	//[ self gotopos:posa ];
	[ mEditor setGeneralProperty: SCI_INSERTTEXT parameter:posa value:(long)str ];
	//[ self showLine:linea lineb:lineb ];
}


- (NSSplitView*) get_splitview
{
	return splitView;
}
@end


@implementation SplitViewDelegate

- (void)notification: (Scintilla::SCNotification*)notification
{
	cout << "SplitViewDelegate: notify " << endl;

}
#define kMinOutlineViewSplit    120.0f

// -------------------------------------------------------------------------------
//  splitView:constrainMinCoordinate:
//
//  What you really have to do to set the minimum size of both subviews to kMinOutlineViewSplit points.
// -------------------------------------------------------------------------------
- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedCoordinate ofSubviewAt:(int)index
{
	return proposedCoordinate + kMinOutlineViewSplit;
}

// -------------------------------------------------------------------------------
//  splitView:constrainMaxCoordinate:
// -------------------------------------------------------------------------------
- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedCoordinate ofSubviewAt:(int)index
{
	return proposedCoordinate - kMinOutlineViewSplit;
}

// -------------------------------------------------------------------------------
//  splitView:resizeSubviewsWithOldSize:
//
// -------------------------------------------------------------------------------
- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize
{ 
	//NSLog(@"resizeSubviewsWithOldSize: oldSize: w:%.1f h:%.1f", oldSize.width, oldSize.height);
	NSRect newFrame = [sender frame]; // get the new size of the whole splitView
	//NSLog(@"resizeSubviewsWithOldSize: subviews count: %d", [[sender subviews] count]);
	NSView *left = [[sender subviews] objectAtIndex:0];
	NSRect leftFrame = [left frame];
	CGFloat dividerThickness = [sender dividerThickness];

	leftFrame.size.height = newFrame.size.height;

	if ([[sender subviews] count] >= 2)
	{
		NSView *right = [[sender subviews] objectAtIndex:1];
		NSRect rightFrame = [right frame];
		//NSLog(@"resizeSubviewsWithOldSize 2 is good !");
		rightFrame.size.width = newFrame.size.width - leftFrame.size.width - dividerThickness;
		rightFrame.origin.x = leftFrame.size.width + dividerThickness;
#if USE_EDITOR_TABS
		if ([[right subviews] count ] > 1) {
			NSLog(@"resizeSubviewsWithOldSize: count=%d", [[right subviews] count]);
			// set y and h
			for (int i = 0; i < [[right subviews] count]; i++) {
				NSView* n = [[right subviews] objectAtIndex:i];
				if ([n isKindOfClass: [NSButton class]]) {
					NSRect f = [n frame];
					f.origin.y = newFrame.size.height - 20;
					[n setFrame:f];
				}
				if ([n isKindOfClass: [ScintillaView class]]) {
					NSRect editorframe = [n frame];
					editorframe.size.height = newFrame.size.height - 20;
					editorframe.origin.y = 0;
					[n setFrame:editorframe];

					NSRect editorbounds = [n bounds];
					editorbounds.size.height = newFrame.size.height - 20;
					editorbounds.origin.y = 0;
					[n setBounds:editorbounds];

					rightFrame.size.height = newFrame.size.height;
					rightFrame.origin.y = newFrame.origin.y;
				}
			}
		} else {
			NSView *editor = [[right subviews] objectAtIndex:0];
			NSRect editorframe = [editor frame];
			editorframe.size.height = newFrame.size.height;
			[editor setFrame:editorframe];

		}
#endif
		rightFrame.size.height = newFrame.size.height;
		//rightFrame.origin.y = newFrame.origin.y;
		[right setFrame:rightFrame];
	} else {
		NSLog(@"resizeSubviewsWithOldSize: error only one view present in NSSplitView");
		leftFrame = newFrame;
		//leftFrame.size.width -= dividerThickness;
	}
	[left setFrame:leftFrame];
}


@end


