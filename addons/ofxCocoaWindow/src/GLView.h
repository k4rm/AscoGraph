/////////////////////////////////////////////////////
//
//  GLView.h
//  ofxCocoaWindow
//
//  Original code from,
//  http://developer.apple.com/library/mac/#samplecode/GLFullScreen/Introduction/Intro.html#//apple_ref/doc/uid/DTS40009820
//
//  Created by lukasz karluk on 16/11/11.
//  http://julapy.com/blog
//
/////////////////////////////////////////////////////

#pragma once

#include "ofMain.h"
#include "ofxNSWindowApp.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>

class ofxNSWindowApp;

@protocol GLViewDelegate <NSObject>
@required
- (void) glViewUpdate;
@end

@interface GLView : NSView 
{
    id <GLViewDelegate> delegate;
	
	NSOpenGLContext *openGLContext;
	NSOpenGLPixelFormat *pixelFormat;
	
	CVDisplayLinkRef displayLink;
	BOOL isAnimating;
    bool bEnableSetupScreen;
    ofxNSWindowApp *windowApp;
    int frameRate;
}

@property(nonatomic, assign) id delegate;
@property (assign)   bool bEnableSetupScreen;

- (NSOpenGLContext*) openGLContext;

- (void) drawView;
- (void) setup;
- (void) prepareOpenGL;
- (void) setApp: (ofxNSWindowApp*) app;
- (void) setFrameRate: (float) fr;
- (float) getFrameRate;


- (void) timerFired:(id)sender;
- (id) initWithFrame:(NSRect)frameRect;
- (id) initWithFrame:(NSRect)frameRect shareContext:(NSOpenGLContext*)context;
- (id) initWithFrame:(NSRect)frame : (ofxNSWindowApp*) app : (int) fr;


- (void) startAnimation;
- (void) stopAnimation;

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;

@end
