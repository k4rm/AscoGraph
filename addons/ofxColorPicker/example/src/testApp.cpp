#include "testApp.h"

///////////////////////////////////////////
//	INIT.
///////////////////////////////////////////

void testApp::setup()
{
	ofSetFrameRate( frameRate = 60 );
	ofSetVerticalSync( true );
	
	colorPicker0.setColorRadius( 1.0 );
	colorPicker1.setColorRadius( 1.0 );
	
	colorPicker0.setColorAngle( 0.5 );
	colorPicker1.setColorAngle( 0.83 );
}

///////////////////////////////////////////
//	UPDATE.
///////////////////////////////////////////

void testApp::update()
{
	float ang;
	ang += frameRate * 0.00002;

	colorPicker1.setColorAngle( colorPicker1.getColorAngle() + ang );
	
	colorPicker0.update();
	colorPicker1.update();
	
	rect.setCornerColor( colorPicker0.getColor(), 0 );
	rect.setCornerColor( colorPicker0.getColor(), 1 );
	rect.setCornerColor( colorPicker1.getColor(), 2 );
	rect.setCornerColor( colorPicker1.getColor(), 3 );
}

///////////////////////////////////////////
//	DRAW.
///////////////////////////////////////////

void testApp::draw()
{
	rect.draw();
	
	//--

	int x, y, w, h, g;

	w = 150;
	h = 300;
	x = 20;
	
	g = (int)( ( ofGetHeight() - h * 2 ) / 3 );		// gap.
	y = g;
	
	colorPicker0.draw( x, y, w, h );
	
	y = y + h + g;
	
	colorPicker1.draw( x, y, w, h );
}

///////////////////////////////////////////
//	HANDLERS.
///////////////////////////////////////////

void testApp::keyPressed(int key)
{

}

void testApp::keyReleased(int key)
{

}

void testApp::mouseMoved(int x, int y )
{

}

void testApp::mouseDragged(int x, int y, int button)
{

}

void testApp::mousePressed(int x, int y, int button)
{

}

void testApp::mouseReleased(int x, int y, int button)
{

}

void testApp::windowResized(int w, int h)
{

}

