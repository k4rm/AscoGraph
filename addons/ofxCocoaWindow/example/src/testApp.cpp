

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() 
{
    ofBackground( 0, 0, 0 );
    ofEnableAlphaBlending();
    ofSetVerticalSync( ( bVSync = true ) );
    ofSetWindowPosition( 100, 100 );
//    ofSetWindowShape( 1024, 768 );
    
    image.loadImage( "transparency.png" );
    imagePos.set( 0, 100 );
    
    bShowCursor = true;
    bWindowShape = false;
}

//--------------------------------------------------------------
void testApp::update()
{
	imagePos.x += 1;
    if( imagePos.x > ofGetWidth() - image.width )
        imagePos.x = 0;
}

//--------------------------------------------------------------
void testApp::draw()
{
    vector<ofColor> colors;
    colors.push_back( ofColor :: red );
    colors.push_back( ofColor :: green );
    colors.push_back( ofColor :: blue );
    colors.push_back( ofColor :: magenta );
    colors.push_back( ofColor :: yellow );
    colors.push_back( ofColor :: cyan );
    
    int w   = ofGetWidth();
    int h   = ofGetHeight();
    int dy  = h / colors.size();
    int x   = 0;
    int y   = 0;
    
    for( int i=0; i<colors.size(); i++ )
    {
        float p = i / (float)( colors.size() - 1 );
        
        ofSetColor( colors[ i ] );
        ofRect( x, y, w, dy );
        
        y += dy;
    }
    
    ofSetColor( 255 );
    image.draw( imagePos.x, imagePos.y );
	
    y = 10;
    
	ofSetColor( 0 );
	ofDrawBitmapString( "press 'f' to toggle fullscreen ", 20, y+=20 );
    ofDrawBitmapString( "press 'v' to toggle vertical sync, currently " + ( bVSync ? string("on") : string("off") ), 20, y+=20 );
    ofDrawBitmapString( "press 'm' to show/hide cursor, currently " + ( bShowCursor ? string("on") : string("off") ), 20, y+=20 );
    ofDrawBitmapString( "press 's' to reshape the window", 20, y+=20 );
    ofDrawBitmapString( "press the arrow keys to nudge the window. note: works only in OF_WINDOW mode", 20, y+=20 );
    
    y+=20;
    
    ofDrawBitmapString( "fps = " + ofToString( ofGetFrameRate() ), 20, y+=20 );
    ofDrawBitmapString( "frame no. = " + ofToString( ofGetFrameNum() ), 20, y+=20 );
    ofDrawBitmapString( "window position x = " + ofToString( ofGetWindowPositionX() ), 20, y+=20 );
    ofDrawBitmapString( "window position y = " + ofToString( ofGetWindowPositionY() ), 20, y+=20 );
    ofDrawBitmapString( "window width = " + ofToString( ofGetWidth() ), 20, y+=20 );
    ofDrawBitmapString( "window height = " + ofToString( ofGetHeight() ), 20, y+=20 );
    ofDrawBitmapString( "screen width = " + ofToString( ofGetScreenWidth() ), 20, y+=20 );
    ofDrawBitmapString( "screen height = " + ofToString( ofGetScreenHeight() ), 20, y+=20 );
}

void testApp :: exit ()
{
    //
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
    if( key == 'f' )
        ofToggleFullscreen();
    
    if( key == 'v' )
        ofSetVerticalSync( ( bVSync = !bVSync ) );
    
    if( key == 'm' )
    {
        bShowCursor = !bShowCursor;
        if( bShowCursor )
            ofShowCursor();
        else
            ofHideCursor();
    }
    
    if( key == 's' )
    {
        bWindowShape = !bWindowShape;
        if( bWindowShape )
            ofSetWindowShape( 1024, 768 );
        else
            ofSetWindowShape( 800, 600 );
    }
    
    int windowMoveInc = 20;
    
    if( key == OF_KEY_LEFT )
    {
        ofSetWindowPosition( MAX( ofGetWindowPositionX() - windowMoveInc, 0 ), ofGetWindowPositionY() );
    }
    else if( key == OF_KEY_RIGHT )
    {
        ofSetWindowPosition( MIN( ofGetWindowPositionX() + windowMoveInc, ofGetScreenWidth() - ofGetWidth() ), ofGetWindowPositionY() );
    }
    else if( key == OF_KEY_UP )
    {
        ofSetWindowPosition( ofGetWindowPositionX(), MAX( ofGetWindowPositionY() - windowMoveInc, 0 ) );
    }
    else if( key == OF_KEY_DOWN )
    {
        ofSetWindowPosition( ofGetWindowPositionX(), MIN( ofGetWindowPositionY() + windowMoveInc, ofGetScreenHeight() - ofGetHeight() ) );
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

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

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}
