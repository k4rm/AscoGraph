//
//  iOSAscoGraph.h
//  AscoGraph
//
//  Created by Thomas Coffy on 29/03/14.
//  Copyright (c) 2014 IRCAM. All rights reserved.
//

#ifndef AscoGraph_ofApp_h
#define AscoGraph_ofApp_h
#pragma once

#include "ofxiOS.h"
#include "ofxiOSExtras.h"
#include "ofxGui.h"

#define TEXT_CONSTANT_TEMP_FILENAME             "/tmp/tmpfile-ascograph.txt"
#define TEXT_CONSTANT_TEMP_ACTION_FILENAME      "/tmp/ascograph_tmp.asco.txt"

class iOSAscoGraph : public ofxiOSApp{
	
public:
    void setup();
    void update();
    void draw();
    void exit();
	
    void touchDown(ofTouchEventArgs & touch);
    void touchMoved(ofTouchEventArgs & touch);
    void touchUp(ofTouchEventArgs & touch);
    void touchDoubleTap(ofTouchEventArgs & touch);
    void touchCancelled(ofTouchEventArgs & touch);
    
    void lostFocus();
    void gotFocus();
    void gotMemoryWarning();
    void deviceOrientationChanged(int newOrientation);
    
    void display_error() { cerr << "iOSAscoGraph::display_error() TODO" << endl; }
    
    bool bHide;
    
};




#endif
