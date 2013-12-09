#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxHTTPServer.h"



class testApp : public ofBaseApp, public ofxHTTPServerListener{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		void getRequest(ofxHTTPServerResponse & response);
		void postRequest(ofxHTTPServerResponse & response);
		void fileNotFound(ofxHTTPServerResponse& response){}

        ofxHTTPServer * server;

        // for drawing
        ofPoint radius[20];
        ofImage image;
        bool imageServed;
        bool imageSaved;

        string postedImgName;
        string postedImgFile;
        string prevPostedImg;
        ofImage postedImg;


};

#endif
