/////////////////////////////////////////////////////
//
//  ofxCocoaDelegate.h
//  ofxCocoaWindow
//
//  Created by lukasz karluk on 16/11/11.
//  http://julapy.com/blog
//
/////////////////////////////////////////////////////

#pragma once

#import "GLView.h"
#import "GLWindow.h"

@interface ofxCocoaDelegate : NSObject <GLViewDelegate>
{
    GLWindow        *openGLWindow;
    GLView          *openGLView;

    GLWindow        *fullScreenWindow;
    GLView          *fullScreenView;
    
    ofWindowMode    windowMode;
    ofWindowMode    windowModeInit;
    
	float	timeNow;
    float   timeThen;
    float   fps;
    int		nFrameCount;
	double	lastFrameTime;
	float	frameRate;
}

@property (retain) GLWindow *openGLWindow;
@property (retain) GLView   *openGLView;
@property (retain) GLWindow *fullScreenWindow;
@property (retain) GLView   *fullScreenView;
@property (assign) ofWindowMode windowMode;
@property (assign) ofWindowMode windowModeInit;

@property (assign)   float timeNow;
@property (assign)   float timeThen;
@property (assign)   float fps;
@property (readonly) int nFrameCount;
@property (readonly) double lastFrameTime;
@property (readonly) float frameRate;

- (id)initWithWidth : (int)width 
             height : (int)height 
         windowMode : (ofWindowMode)windowMode;

- (void) goFullScreenOnAllDisplays;
- (void) goFullScreenOnDisplay : (int)displayIndex;
- (void) goFullScreen : (NSRect)displayRect;
- (void) goWindow;
- (void) updateOpenGLContext;

- (float) getFrameRate;
- (double) getLastFrameTime;
- (int) getFrameNum;
- (NSRect) getViewFrame;
- (NSRect) getWindowFrame;
- (NSRect) getScreenFrame;
- (void) setWindowPosition : (NSPoint)position;
- (void) setWindowShape : (NSRect)shape;
- (void) enableSetupScreen;
- (void) disableSetupScreen;
- (void) registerForDraggedTypes;
- (NSWindow*) getNSWindow;
- (NSView*) getNSView;
- (GLWindow*) getGLWindow;
- (GLView*) getGLView;
@end
