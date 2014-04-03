#pragma once

#include "ofMain.h"
#include "ofxBonjourIp.h"
//#include "ofxBonjourIp.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        ofxBonjourIp* bonjour;
    
    // events (optional)
    void onPublishedService(const void* sender, string &serviceIp);
    void onDiscoveredService(const void* sender, string &serviceIp);
    void onRemovedService(const void* sender, string &serviceIp);
		
};
