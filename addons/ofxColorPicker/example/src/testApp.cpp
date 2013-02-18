#include "testApp.h"

void testApp::setup()
{
				float editor_x = 50;
				float editor_y = 50;

				editor = new ofxCodeEditor();

				NSWindow *nswin = [cocoaWindow->delegate getNSWindow]; 
				NSView *nsview_ = [cocoaWindow->delegate getNSView];
				ofRectangle r(editor_x, editor_y, CONSTANT_EDITOR_VIEW_WIDTH, ofGetHeight());
				editor->setup(nswin, nsview_, r);
}

void testApp::update()
{
}

void testApp::draw()
{
}

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

