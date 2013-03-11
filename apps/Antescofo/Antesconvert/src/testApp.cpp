#include "testApp.h"
#include "partsummary.h"
#include "smartlist.h"
#include "xml_tree_browser.h"

using namespace std;
using namespace MusicXML2;

static string str_error; // filled by our error()
static ofxUITextInput *static_measures;
static ofxUIRangeSlider *_slider;
static ofxUICanvas *_guiContent, *_guiSlider;
static map<string, pair<string,bool> > radios_val; // map : button name and value/state
static float _maxy = 0;
static list<ofxUIWidget*> _widgets;
bool _notenames = true;

string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}

int stringToInt(string s)
{
    std::istringstream is(s);
		int i;
		is >> i;

    return i;
}

//_______________________________________________________________________________
class mypartsummary : 
    public partsummary,
    public visitor<S_part_name>
{
	public:
		virtual void visitEnd ( S_part& elt);
		virtual void visitStart ( S_part_name& elt);
                string partname;
};


void pre_antescofo::error(const char *fmt,...)
{
    va_list ap;
    va_start(ap, fmt);
    
    char buf[1024];
    vsnprintf(buf,sizeof buf,fmt, ap);
    buf[sizeof buf-1] = 0; // in case of full buffer
    cerr << "---------------- PARSING ERROR ----------------" << endl;
    cerr << buf << endl;
    cerr.flush();
    str_error += buf;
    str_error += "\n";
    va_end(ap);
}


string testApp::get_error()
{
    if (str_error.empty())
        return string("Error : invalid MusicXml input file.");
    else return str_error;
}

void sliderEnable()
{
	_slider->setVisible(true);
	_slider->setLabelVisible(false);
	_guiSlider->enable();
	_guiSlider->setVisible(true);
	_guiContent->getRect()->x = 25;
	_guiContent->getRect()->width = ofGetWidth() - 25;
}

void sliderDisable()
{
	_slider->setVisible(false);
	_guiSlider->disable();
	_guiSlider->setVisible(false);
	_guiContent->getRect()->x = 0;
	_guiContent->getRect()->width = ofGetWidth();
}

// content
void testApp::setupContent()
{
    guiContent = new ofxUICanvas(25, 0, ofGetWidth()-25, ofGetHeight());
    guiContent->setFont("DroidSansMono.ttf");
    ofImage *im = new ofImage();
    if (im->loadImage("Arrow.png"))
        guiContent->addWidgetDown(new ofxUIImage(0, 0, 748, 200, im, ""));
    else {
        cerr << "Can not load Arrow.png" << endl;
        abort();
    }

    guiContent->addWidgetDown(new ofxUILabelToggle(false, string(TEXT_CONSTANT_BUTTON_QUIT), OFX_UI_FONT_SMALL));

		guiContent->addWidgetRight(new ofxUILabelToggle(false, string(TEXT_CONSTANT_BUTTON_LOAD), OFX_UI_FONT_SMALL));

    mWSave = new ofxUILabelToggle(false, string(TEXT_CONSTANT_BUTTON_SAVE), OFX_UI_FONT_SMALL);
    guiContent->addWidgetRight(mWSave);
    mWSave->setVisible(false);
    mWSave->setLabelVisible(false);

		mWConvert = new ofxUILabelToggle(false, string(TEXT_CONSTANT_BUTTON_CONVERT), OFX_UI_FONT_SMALL);
		guiContent->addWidgetRight(mWConvert);
		mWConvert->setVisible(false);
		mWConvert->setLabelVisible(false);

		ofxUISpacer *space = new ofxUISpacer(ofGetWidth()-10, 3);
		space->setVisible(false);
		guiContent->addWidgetDown(space);


		guiContent->setTheme(OFX_UI_THEME_BLUEBLUE);
		//guiTop->setTheme(OFX_UI_THEME_BLUEBLUE);
		//guiTop->setTheme(OFX_UI_THEME_MINBLACK);

		_guiContent = guiContent;
		_widgets = widgets;
		sliderDisable();
                _guiContent->setPlacer(mWConvert);
}

void testApp::cleanContent()
{
	for (list<ofxUIWidget*>::iterator i = _widgets.begin(); i != _widgets.end(); i++) {
		guiContent->removeWidget(*i);
	}
	_widgets.clear();
	widgets = _widgets;
        //guiContent->resetPlacer();
        _guiContent->setPlacer(mWConvert);
}

//--------------------------------------------------------------
void testApp::setup(){
    bShowError = false;
    
    ofSetDataPathRoot("../Resources/"); 
    ofBackground(255, 255, 255, 255);
	ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableSmoothing();
    ofEnableAlphaBlending();
	ofSetLogLevel(OF_LOG_VERBOSE);
    //load(); // xml settings
    //guiTop = new ofxUICanvas(0, 0, ofGetWidth(), 215);//, ofGetWidth(), ofGetHeight());
    guiSlider = new ofxUICanvas(0, 0, 25, ofGetHeight());
    guiSlider->setFont("DroidSansMono.ttf");
    //guiContent->setScrollAreaToScreen();
		//guiContent->setScrollableDirections(false, true); 
		//guiContent->setSnapping(false);
		//guiContent->setDrawWidgetPadding(false);
		//guiTop->autoSizeToFitWidgets();
    //guiTop->getRect()->setHeight(ofGetHeight());
		
    guiError = new ofxUIScrollableCanvas(10, ofGetHeight()/3, ofGetWidth()-20, ofGetHeight()/3);//-100-10);
    guiError->setFont("DroidSansMono.ttf");
		/*
    string title(TEXT_CONSTANT_TITLE);
    title += TEXT_CONSTANT_VERSION;
    guiTop->addWidgetRight(new ofxUILabel(title, OFX_UI_FONT_LARGE));
    guiTop->addWidgetDown(new ofxUILabel(TEXT_CONSTANT_SUBTITLE, OFX_UI_FONT_MEDIUM));
    guiTop->addWidgetDown(new ofxUILabel(TEXT_CONSTANT_SUBTITLE2, OFX_UI_FONT_MEDIUM));
    guiTop->addWidgetDown(new ofxUILabel(TEXT_CONSTANT_SUBTITLE3, OFX_UI_FONT_MEDIUM));
    mWInfo = new ofxUILabel(TEXT_CONSTANT_DONE, OFX_UI_FONT_MEDIUM);
    guiTop->addWidgetDown(mWInfo);
    mWInfo->setVisible(false);
		*/
	
		// slider
    slide = new ofxUIRangeSlider(4, 4, 16, ofGetHeight()-4, 0, 25, 0, 255, "slide", OFX_UI_FONT_SMALL);
		slide->setLabelPrecision(0);
		slide->setLabelVisible(false);
		guiSlider->addWidgetDown(slide);
		guiSlider->setTheme(OFX_UI_THEME_BLUEBLUE);

    ofColor cb(255, 255, 255, 255);
    ofColor co(0, 100, 100, 255);
    ofColor coh(0, 0, 100, 255);
    ofColor cf(0, 0, 25, 255);
    ofColor cfh(20, 0, 245, 255);
    ofColor cp(0, 200, 0, 255);
    ofColor cpo(0, 200, 0, 255);
    //guiTop->setUIColors(cb, co, coh, cf, cfh, cp, cpo);
    ofColor cbE(0, 0, 0, 255);
    ofColor cfE(250, 250, 250, 255);
    guiError->setUIColors(cbE, co, coh, cfE, cfh, cp, cpo);
    guiError->setVisible(false);
    guiError->disable();

		_guiSlider = guiSlider;
		_slider = slide;

		setupContent();

    ofAddListener(guiSlider->newGUIEvent, this, &testApp::guiEvent);
    ofAddListener(guiContent->newGUIEvent, this, &testApp::guiEvent);
    ofAddListener(guiError->newGUIEvent, this, &testApp::guiEvent);


    AntescofoWriter = new MusicXML2::antescofowriter();
}

//--------------------------------------------------------------
void testApp::update(){


}

//--------------------------------------------------------------
void testApp::draw(){
    ofFill();
    ofBackground(0, 0, 0, 255);
    //ofSetColor(0, 0, 0, 255);
		ofPushStyle(); 
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);     
		//  ofRect(0, 0, ofGetWidth(), ofGetHeight());
    //ofSetColor(100, 0, 0, 255);
    if (bShowError) {
        ofSetColor(0, 0, 0, 200);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
        ofSetColor(0, 0, 0, 240);
        
        ofDrawBitmapString(get_error(), 100, 300);
        
        guiError->draw();
    } else {
        //guiTop->draw();
				//guiSlider->draw(); guiContent->draw();
				usleep(100);
		}
		ofPopStyle(); 
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	//static_guiTop->getRect()->setWidth(w);
	//static_guiTop->getRect()->setHeight(h);	
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

void testApp::display_error()
{
	bShowError = true;
	guiContent->setVisible(false);
	guiContent->disable();
	guiError->enable();
	guiError->setVisible(true);
	if (!bErrorInitDone){
		guiError->addWidgetRight(new ofxUILabel(ofGetWidth()/2, 100, string(TEXT_CONSTANT_PARSE_ERROR), OFX_UI_FONT_MEDIUM));
		string err = get_error();
		/*
			 int len = 80, i;
			 for (i = 0; i < err.size(); i += len) {
			 string sub(err, i, MIN(i+len, err.size()));
			 guiError->addWidgetDown(new ofxUILabel(sub, OFX_UI_FONT_MEDIUM));
			 }
			 if (i != err.size()) { string sub(err, i, err.size()); guiError->addWidgetDown(new ofxUILabel(sub, OFX_UI_FONT_MEDIUM)); }

		//guiError->addWidgetDown(new ofxUILabel(ofGetWidth()/3, 200, score_w - ofGetWidth()/3, err, OFX_UI_FONT_MEDIUM));

		ofDrawBitmapString(ofxAntescofoNote->get_error(), 100, 100);
		*/
		bErrorInitDone = true;
		guiError->addWidgetDown(new ofxUILabelToggle(ofGetWidth()/2, 10, false, TEXT_CONSTANT_BUTTON_CANCEL, OFX_UI_FONT_SMALL));
	} else {
		guiError->getWidget(TEXT_CONSTANT_BUTTON_CANCEL)->setState(0);
		guiError->getWidget(TEXT_CONSTANT_BUTTON_CANCEL)->setVisible(true);
		((ofxUILabelToggle*)guiError->getWidget(TEXT_CONSTANT_BUTTON_CANCEL))->setLabelVisible(true);
	}
}


//_______________________________________________________________________________
void mypartsummary::visitStart ( S_part_name& elt) 
{
    partname = elt->getValue();
}

//_______________________________________________________________________________
void mypartsummary::visitEnd ( S_part& elt)
{
	ostringstream out, resume;
	out.str("");

	resume << "Part "<< partname<<" ("<< elt->getAttributeValue("id") << ") contains " << countVoices()
		<< (countVoices() > 1 ? " voices on ":" voice on ") << countStaves() 
		<< (countStaves() > 1 ? " staves:":" staff:");
	//cout << resume << endl;
	ofxUIWidget *e = new ofxUILabel(resume.str(), OFX_UI_FONT_SMALL);
	_widgets.push_back(e);
	_guiContent->addWidgetDown(e);

	ofxUIToggle* r;
	smartlist<int>::ptr voices;
	smartlist<int>::ptr staves = getStaves();
	for (vector<int>::const_iterator i = staves->begin(); i != staves->end(); i++) { 
		out.str("");
		out << "    staff \"" << *i << "\": " << getStaffNotes(*i) << " notes - "
			<< countVoices(*i) << (countVoices() > 1 ? " voices on ":" voice on ") <<" [";
		//static_guiTop->addWidgetRight(new ofxUILabel(out, OFX_UI_FONT_SMALL));

		bool sep = false;
		voices = getVoices(*i);
		for (vector<int>::const_iterator v = voices->begin(); v != voices->end(); v++) {
			if (sep) out << ", ";
			else sep = true;
			out << *v << ":" << getVoiceNotes(*i, *v);
		}
		out << "]";
		string s = intToString(*i);
		radios_val[out.str()] = pair<string,bool>(s, true);
		r = new ofxUIToggle(20, 20, true, out.str(), OFX_UI_FONT_SMALL);
		_widgets.push_back(r);
		_guiContent->addWidgetDown(r);
	}
	out.clear(); out.str("");
	voices = getVoices();
	for (vector<int>::const_iterator i = voices->begin(); i != voices->end(); i++) {
		out.str("");
		out << "    voice \"" << *i << "\": " << getVoiceNotes(*i) << " notes - ";
		staves = getStaves(*i);
		out << staves->size() << (staves->size() > 1 ? " staves" :" staff");
		/* << " [";
		bool sep = false;
		for (vector<int>::const_iterator s = staves->begin(); s != staves->end(); s++) {
			if (sep) out << ", ";
			else sep = true;
			out << *s ;
		}
		out << "] main staff: " << getMainStaff(*i);// << endl;
		*/
		string s = intToString(*i);
		radios_val[out.str()] = pair<string,bool>(s, true);
		r = new ofxUIToggle(20, 20, true, out.str(), OFX_UI_FONT_SMALL);
		_widgets.push_back(r);
		_guiContent->addWidgetDown(r);
	}
	if (r) { //store last toggle y for scrolling it
		_maxy = r->getRect()->y;
		if (_maxy > _guiContent->getRect()->height) {
			_guiContent->getRect()->height = _maxy + 100;
			sliderEnable();
		} else {
			sliderDisable();
		}
	}
}



// load and display part summary
int testApp::loadScore(string filename, string outfilename) {
	cout << "Trying to load score : " << filename << endl;
	xmlreader r;
	SXMLFile xmlfile;
	xmlfile = r.read(filename.c_str());
	if (xmlfile) {
		ofxUIWidget *e = new ofxUILabel(TEXT_CONSTANT_TEXTINPUT_MEASURES, OFX_UI_FONT_SMALL);
		widgets.push_back(e);
		guiContent->addWidgetDown(e);
		static_measures = new ofxUITextInput(200, TEXT_CONSTANT_TEXTINPUT_MEASURES, "", OFX_UI_FONT_SMALL);
		e = static_measures;
		widgets.push_back(e);
		guiContent->addWidgetRight(static_measures);

		ofxUIToggle *t = new ofxUIToggle(20, 20, true, TEXT_CONSTANT_TOGGLE_NOTENAME, OFX_UI_FONT_SMALL);
		guiContent->addWidgetDown(t);

		mWConvert->setVisible(true);
		mWConvert->setLabelVisible(true);

		//static_guiTop->addWidgetDown(new ofxUISpacer(ofGetWidth()-4, 1));


		Sxmlelement elt = xmlfile->elements();
		if (elt) {
			mypartsummary nv;
			xml_tree_browser browser(&nv);
			browser.browse(*elt);
		}
		mScore_filename = filename;
	}
	else cerr << "error reading \"" << filename << "\"" << endl;
	return 0;
}



vector<string> testApp::getVoicesVect(map<string, pair<string,bool> > &radios_val)
{
	vector<string> vect;
	for (map<string, pair<string,bool> >::const_iterator i = radios_val.begin(); i != radios_val.end(); i++) {
		string voice("    voice \"");
		cout << "---->"<<i->first << "<----" << endl;
		if (i->first.compare(0, voice.size(), voice) == 0 && !i->second.second) {
			vect.push_back(i->second.first);
		}
	}
	return vect;
}

vector<string> testApp::getStavesVect(map<string, pair<string,bool> > &radios_val)
{
	vector<string> vect;
	for (map<string, pair<string,bool> >::const_iterator i = radios_val.begin(); i != radios_val.end(); i++) {
		string staff("    staff \"");
		if (i->first.compare(0, staff.size(), staff) == 0 && !i->second.second) {
                    cout << "COMPARE: " << i->first << " and " << staff << "--> true and state:"<< !i->second.second << endl;
			vect.push_back(i->second.first);
		}
	}
	return vect;
}

vector<string> testApp::getPartsVect(map<string, pair<string,bool> > &radios_val)
{
/*
	vector<string> vect;
	for (map<string, pair<string,bool> >::const_iterator i = radios_val.begin(); i != radios_val.end(); i++) {
		string voice("   voice \"");
		if (i->first.compare(0, voice.size(), voice) == 0 && i->second.second) {
			vect.push_back(i->second.first);
		}
	}
	return vect;
	*/
}

vector<int> testApp::getMeasuresVect()
{
	vector<int> v;
	string m = static_measures->getTextString();
	if (static_measures && m.size()) {
		int t = m.find("-");
		if (t != std::string::npos) {
			int from = stringToInt(m.substr(0, t));
			int to = stringToInt(m.substr(t+1, m.size()));
			cout<< "getMeasures: from:" << from << endl;
			cout<< "getMeasures: to:" << to << endl;
			for (int k = from; k < to; k++) {
				v.push_back(k);
			}
		} else {
			int i = stringToInt(m);
			cout<< "getMeasures: got:" << i << endl;
			if (i)
				v.push_back(i);	
		}
  }
	for (vector<int>::iterator i = v.begin(); i != v.end(); i++) {
		cout << "goint to output measure " << *i << endl; 
	}
	return v;
}

// after loadScore, convert
int testApp::convertScore(string filename, string outfilename) {
    cout << "Trying to load score : " << filename << endl;
    xmlreader r;
    SXMLFile xmlfile;

    xmlfile = r.read(filename.c_str());
    if (xmlfile) {
        Sxmlelement st = xmlfile->elements();
        if (st) {
					if (AntescofoWriter) delete AntescofoWriter;
					AntescofoWriter = new MusicXML2::antescofowriter();

					  AntescofoWriter->setSelectedVoices(getVoicesVect(radios_val));
					  AntescofoWriter->setSelectedStaves(getStavesVect(radios_val));
					  AntescofoWriter->setSelectedMeasures(getMeasuresVect());
						AntescofoWriter->print_notes_names = _notenames;

            xml2antescofovisitor v(*AntescofoWriter, true, true, false);
            Santescofoelement as = v.convert(st);
            std::cout << ";  Antescofo code converted using libmusicxml v." << musicxmllibVersionStr() << std::endl;
            std::cout << "; and the embedded xml2antescofo converter v." << musicxml2antescofoVersionStr() << std::endl;
            
            AntescofoWriter->print();
						AntescofoWriter->write(outfilename.c_str());
            
        } else {
            display_error();
            return 0;
        }
        //mWInfo->setVisible(true);
        mWSave->setVisible(true);
        mWSave->setLabelVisible(true);
        return 1;
    } else display_error();
		return 0;
}

void testApp::clear() {
    delete AntescofoWriter;
    AntescofoWriter = new MusicXML2::antescofowriter();
		str_error.erase();
		//delete static_measures;
		radios_val.clear();
		cleanContent();
}
//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
	if( dragInfo.files.size() > 0 ){
		for(int i = 0; i < dragInfo.files.size(); i++){
			clear(); // TODO ask for Saving file
			ofLogVerbose("Antesconvert: trying to load Music XML file :" + dragInfo.files[i]);
			int n = loadScore(string(dragInfo.files[i]), TEXT_CONSTANT_TEMP_FILENAME);
            if (!n)
                return;
			break; // only one file supported yet
		}
	}
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
    //cout << "got guiEvent("<< e.widget->getName() <<")"<< endl;
    // load score
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_LOAD)
	{
        ofFileDialogResult openFileResult = ofSystemLoadDialog(TEXT_CONSTANT_TITLE_LOAD_SCORE);
        if (openFileResult.bSuccess){
			string f = openFileResult.filePath;
			ofLogVerbose("Selected file: " + f);
			clear();
            loadScore(f, TEXT_CONSTANT_TEMP_FILENAME);
            ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
            b->setValue(false);
		} else {
			ofLogVerbose("Cancel load score hit.");
		}
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_SAVE)
    {
        string proposedfile = mScore_filename;
        string postfix = "asco.txt";
        proposedfile.replace(proposedfile.end() - 3, proposedfile.end(), postfix);
        cout << "Proposed filename to save: " << proposedfile << endl;
        //ofFileDialogResult saveFileResult = ofSystemSaveDialog(TEXT_CONSTANT_TITLE_SAVE_SCORE, TEXT_CONSTANT_TEMP_FILENAME);
        ofFileDialogResult saveFileResult = ofSystemSaveDialog(proposedfile, "Save converted Antescofo score as :");
        if (saveFileResult.bSuccess){
            string f = saveFileResult.filePath;
            ofLogVerbose("Selected file: " + f);
            std::ifstream  src(TEXT_CONSTANT_TEMP_FILENAME);
            std::ofstream  dst(f.c_str());
            dst << src.rdbuf();

            ofxUILabelToggle *b = (ofxUILabelToggle *) e.widget;
            b->setValue(false);
        } else {
            ofLogVerbose("Cancel load score hit.");
        }
    }
    if(e.widget->getName() == TEXT_CONSTANT_TOGGLE_NOTENAME)
		{
				ofxUIToggle *t = (ofxUIToggle*)e.widget;
				t->update();
				cout << "Toggle notes name hit" << endl;
				_notenames = !_notenames;
		}
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_CANCEL)
	{
        bShowError = false;
        guiError->setVisible(false);
        guiError->disable();
        //guiTop->setVisible(true); guiTop->enable();
    }
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_QUIT)
	{
        exit();
        ofExit();
    }
		for (map<string, pair<string,bool> >::iterator i = radios_val.begin(); i != radios_val.end(); i++) {
			if (e.widget->getName() == i->first) {
				ofxUIToggle *t = (ofxUIToggle*)e.widget;
				//cout << "wid clicked:"<< e.widget->getName() << " value:" << t->getValue() <<endl;
				i->second.second = !i->second.second;
				//pair<int,bool> *p = &(i->second); (*p)->second = !(*p)->second;
				t->update();
			}
		}
    if(e.widget->getName() == TEXT_CONSTANT_BUTTON_CONVERT)
		{
			convertScore(mScore_filename, TEXT_CONSTANT_TEMP_FILENAME);
		}

    if(e.widget->getName() == TEXT_CONSTANT_TEXTINPUT_MEASURES)
		{
		}

    if(e.widget->getName() == "slide")
		{
			if (_maxy > ofGetHeight()) {
				ofxUIRangeSlider *r = (ofxUIRangeSlider*)e.widget;
				float m = (r->getPercentValueLow() + r->getPercentValueHigh()) / 2;
				cout << "maxy: " << _maxy << endl;
				float ny = (m - 0.75) * _maxy;// - _maxy;//_guiContent->getRect()->height + 200;
				cout << "got slide clicked: medium percent = " << m << " oldy:"<< _guiContent->getRect()->y << " ny:" << ny << endl;
				if (ny + _maxy + 200 < 0) ny = _maxy + 200;
				if (ny + _maxy - 200 > ofGetHeight()) ny = 0;//ofGetHeight() - _maxy + 200;
				_guiContent->getRect()->y = ny;
			}
		}
}
