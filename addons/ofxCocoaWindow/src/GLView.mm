/////////////////////////////////////////////////////
//
//  GLView.mm
//  ofxCocoaWindow
//
//  Original code from,
//  http://developer.apple.com/library/mac/#samplecode/GLFullScreen/Introduction/Intro.html#//apple_ref/doc/uid/DTS40009820
//
//  Created by lukasz karluk on 16/11/11.
//  http://julapy.com/blog
//
/////////////////////////////////////////////////////

#import "GLView.h"
#import <OpenGL/gl.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

@implementation GLView

@synthesize delegate;
@synthesize bEnableSetupScreen;

- (NSOpenGLContext*) openGLContext
{
	return openGLContext;
}

- (NSOpenGLPixelFormat*) pixelFormat
{
	return pixelFormat;
}

- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
	// There is no autorelease pool when this method is called because it will be called from a background thread
	// It's important to create one or you will leak objects
	//NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        //[ delegate glViewUpdate ];

    
     
        [self performSelectorOnMainThread:@selector(timerFired:) withObject:nil waitUntilDone:NO ];
 
    return kCVReturnSuccess;
}


// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(GLView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (void) setupDisplayLink
{
	// Create a display link capable of being used with all active displays
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	
	// Set the renderer output callback function
	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
	
	// Set the display link for the current renderer
	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
        cerr << "GLView setupDisplayLink completed." << endl;
}

- (void) setupNSTimer
{
    

    NSTimer* renderTimer = [NSTimer timerWithTimeInterval:0.001
                             target:self
                                         selector:@selector(timerFired:)
                                         userInfo:nil
                                          repeats:YES];
                   
                   [[NSRunLoop currentRunLoop] addTimer:renderTimer
                                                forMode:NSDefaultRunLoopMode];
                   [[NSRunLoop currentRunLoop] addTimer:renderTimer
                                                forMode:NSEventTrackingRunLoopMode]; //Ensure timer fires during resize
}
                   
                   // Timer callback method
- (void)timerFired:(id)sender
    {
        // It is good practice in a Cocoa application to allow the system to send the -drawRect:
        // message when it needs to draw, and not to invoke it directly from the timer.
        // All we do here is tell the display it needs a refresh
        
        
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        
        [ delegate glViewUpdate ];
        
        [self setNeedsDisplay:YES];
        
        [pool release];
        //[self drawView];
    }
       


- (id) initWithFrame:(NSRect)frameRect shareContext:(NSOpenGLContext*)context
{
    bEnableSetupScreen = true;
    /*
    NSOpenGLPixelFormatAttribute attribs[] =
    {
		kCGLPFAAccelerated,
		kCGLPFANoRecovery,
		kCGLPFADoubleBuffer,
		kCGLPFAColorSize, 24,
		kCGLPFADepthSize, 16,
                //kCGLPFADepthSize, 0,

		//NSOpenGLPFASampleBuffers, NSOpenGLPFASamples,
		kCGLPFASampleBuffers, 1,
		kCGLPFASamples, 4,
		0
    };
*/
                
                  
    NSOpenGLPixelFormatAttribute attribs[] =
    {
		kCGLPFAAccelerated,
		kCGLPFANoRecovery,
		kCGLPFADoubleBuffer,
		kCGLPFAColorSize, 24,
		kCGLPFADepthSize, 0,
		0
    };
    
	
    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
    if (!pixelFormat) {
        cerr << "!!!!!!!!!!!!!!!!!!!!! CocoaWindow: GLView: no pixel format" << endl;
		NSLog(@"No OpenGL pixel format");
            }
	
	// NSOpenGLView does not handle context sharing, so we draw to a custom NSView instead
	openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:context];
	
	if (self = [super initWithFrame:frameRect]) {
		[[self openGLContext] makeCurrentContext];
		
		// Synchronize buffer swaps with vertical refresh rate
		GLint swapInt = 1;
		[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; 
		
		[self setupDisplayLink];
                //[self setupNSTimer];
		
		// Look for changes in view size
		// Note, -reshape will not be called automatically on size changes because NSView does not export it to override 
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(reshape) 
													 name:NSViewGlobalFrameDidChangeNotification
												   object:self];
		 //glEnable(GL_MULTISAMPLE_ARB);
	} else {
            cerr << "!!!!!!!!!!!!!!!!!!!!! CocoaWindow: GLView: no frameRect" << endl;
        }
	
	return self;
}

- (id) initWithFrame:(NSRect)frameRect
{
	self = [self initWithFrame:frameRect shareContext:nil];
	return self;
}

- (void) lockFocus
{
	[super lockFocus];
	if ([[self openGLContext] view] != self)
		[[self openGLContext] setView:self];
}

- (void) reshape
{
	// This method will be called on the main thread when resizing, but we may be drawing on a secondary thread through the display link
	// Add a mutex around to avoid the threads accessing the context simultaneously
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
  NSOpenGLContext    *currentContext = [self openGLContext];
  [currentContext makeCurrentContext];
 
	int w = self.frame.size.width;
	int h = self.frame.size.height;
	
	//NSLog(@"GLView : reshape: %dx%d", w, h);
	ofNotifyWindowResized(w, h);
	ofSetupScreen();
   
  
	[[self openGLContext] update];

	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (void) drawRect:(NSRect)dirtyRect
{
	//if( CVDisplayLinkIsRunning(displayLink) )   // display link running, do not draw.
	//	return;
    
	// This method will be called on both the main thread (through -drawRect:) and a secondary thread (through the display link rendering loop)
	// Also, when resizing the view, -reshape is called on the main thread, but we may be drawing on a secondary thread
	// Add a mutex around to avoid the threads accessing the context simultaneously
	//CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
	// Make sure we draw to the right context
	[[self openGLContext] makeCurrentContext];
	
        if( bEnableSetupScreen )
        ofSetupScreen();
    
	if( ofbClearBg() )
    {
		float * bgPtr = ofBgColorPtr();
		glClearColor(bgPtr[0],bgPtr[1],bgPtr[2], bgPtr[3]);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
    
    
	if (ofGetFrameNum() > 0) {
    ofNotifyUpdate();
    ofNotifyDraw();
	}
    
    
	//[[self openGLContext] flushBuffer];
	CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
	//CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
}

- (void) drawView
{
	// This method will be called on both the main thread (through -drawRect:) and a secondary thread (through the display link rendering loop)
	// Also, when resizing the view, -reshape is called on the main thread, but we may be drawing on a secondary thread
	// Add a mutex around to avoid the threads accessing the context simultaneously
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
	// Make sure we draw to the right context
	[[self openGLContext] makeCurrentContext];
	
    if( bEnableSetupScreen )
        ofSetupScreen();
    
	if( ofbClearBg() )
    {
		float * bgPtr = ofBgColorPtr();
		glClearColor(bgPtr[0],bgPtr[1],bgPtr[2], bgPtr[3]);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
    
    ofNotifyUpdate();
    ofNotifyDraw();
    
	[[self openGLContext] flushBuffer]; 
	
	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}
 
- (BOOL) acceptsFirstResponder
{
    // We want this view to be able to receive key events
    return YES;
}

- (void) startAnimation
{
	if (displayLink && !CVDisplayLinkIsRunning(displayLink))
		CVDisplayLinkStart(displayLink);
}

- (void) stopAnimation
{
	if (displayLink && CVDisplayLinkIsRunning(displayLink))
		CVDisplayLinkStop(displayLink);
}

- (void) dealloc
{
	// Stop and release the display link
	CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
	
	// Destroy the context
	[openGLContext release];
	[pixelFormat release];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self 
													name:NSViewGlobalFrameDidChangeNotification
												  object:self];
	[super dealloc];
}

#pragma mark Events
//------------------------------------------------------------
-(void)keyDown:(NSEvent *)theEvent 
{
	NSString *characters = [ theEvent characters ];
	if( [ characters length ] ) 
    {
		unichar key = [ characters characterAtIndex : 0 ];
        
        if( key ==  OF_KEY_ESC )
        {
            //[ NSApp terminate : nil ];
        }
        else if( key == 63232 )
        {
            key = OF_KEY_UP;
        }
        else if( key == 63235 )
        {
            key = OF_KEY_RIGHT;
        }
        else if( key == 63233 )
        {
            key = OF_KEY_DOWN;
        }
        else if( key == 63234 )
        {
            key = OF_KEY_LEFT;
        }
        
		ofNotifyKeyPressed( key );
	}
}

//------------------------------------------------------------
-(void)keyUp:(NSEvent *)theEvent {
	// TODO: make this more exhaustive if needed
	NSString *characters = [theEvent characters];
	if ([characters length]) {
		unichar key = [characters characterAtIndex:0];
		ofNotifyKeyReleased(key);
	}
}

//------------------------------------------------------------
-(ofPoint) ofPointFromEvent:(NSEvent*)theEvent {
	NSPoint p = [theEvent locationInWindow];
	return ofPoint(p.x, self.frame.size.height - p.y, 0);
}

//------------------------------------------------------------
-(void)mouseDown:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMousePressed(p.x, p.y, 0);
}

//------------------------------------------------------------
-(void)rightMouseDown:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMousePressed(p.x, p.y, 2);
}

//------------------------------------------------------------
-(void)otherMouseDown:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMousePressed(p.x, p.y, 1);
}

//------------------------------------------------------------
-(void)mouseMoved:(NSEvent *)theEvent{
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseMoved(p.x, p.y);
}

//------------------------------------------------------------
-(void)mouseUp:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseReleased(p.x, p.y, 0);
}

//------------------------------------------------------------
-(void)rightMouseUp:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseReleased(p.x, p.y, 2);
}

//------------------------------------------------------------
-(void)otherMouseUp:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseReleased(p.x, p.y, 1);
}

//------------------------------------------------------------
-(void)mouseDragged:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseDragged(p.x, p.y, 0);
}

//------------------------------------------------------------
-(void)rightMouseDragged:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseDragged(p.x, p.y, 2);
}

//------------------------------------------------------------
-(void)otherMouseDragged:(NSEvent *)theEvent {
	ofPoint p = [self ofPointFromEvent:theEvent];
	ofNotifyMouseDragged(p.x, p.y, 1);
}

-(void)beginGestureWithEvent:(NSEvent *)theEvent
{
    ofPoint p = [self ofPointFromEvent:theEvent];
    ofNotifyMousePressed(p.x, p.y, 3);
}

//------------------------------------------------------------
-(void)scrollWheel:(NSEvent *)theEvent {
    ofPoint p = [self ofPointFromEvent:theEvent];
   // NSLog(@"User scrolled on (%f,%f) : %f horizontally and %f vertically", p.x, p.y, [theEvent deltaX], [theEvent deltaY]);

    ofNotifyMouseDragged([theEvent deltaX], [theEvent deltaY], 3);
    /*
    ofTouchEventArgs args;
    args.x = p.x;
    args.y = p.y;
    args.minoraxis = [theEvent deltaX];
    args.majoraxis = [theEvent deltaY];
    ofNotifyEvent(&ofEvents().touchMoved, &args);//, NULL);
    */
}

-(void)endGestureWithEvent:(NSEvent *)theEvent
{
    ofPoint p = [self ofPointFromEvent:theEvent];
    ofNotifyMouseReleased(p.x, p.y, 3);
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
    NSPasteboard *pboard;
    NSDragOperation sourceDragMask;
    
    sourceDragMask = [sender draggingSourceOperationMask];
    pboard = [sender draggingPasteboard];
    
    if ( [[pboard types] containsObject:NSColorPboardType] ) {
        if (sourceDragMask & NSDragOperationGeneric) {
            return NSDragOperationGeneric;
        }
    }
    if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
        if (sourceDragMask & NSDragOperationLink) {
            return NSDragOperationLink;
        } else if (sourceDragMask & NSDragOperationCopy) {
            return NSDragOperationCopy;
        }
    }
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {    
    NSDragOperation sourceDragMask;
    sourceDragMask = [sender draggingSourceOperationMask];
    NSPasteboard *pboard = [sender draggingPasteboard];
    BOOL loaded = NO;
    id ts = nil;
    
    ofDragInfo di;

    NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
    unsigned i = [files count];
    while (i-- > 0) {
        NSString *f = [files objectAtIndex:i];
        di.files.push_back([f UTF8String]);
    }
    ofNotifyDragEvent(di);
    return YES;
}

@end
