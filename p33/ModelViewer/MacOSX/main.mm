#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <mach/mach_time.h>

#include "../Classes/Interfaces.hpp"

@class View;

@interface View : NSOpenGLView <NSWindowDelegate> {
	NSRect m_frameRect;
    IApplicationEngine* m_applicationEngine;
    IRenderingEngine* m_renderingEngine;
    IResourceManager* m_resourceManager;
    BOOL m_didInit;
    uint64_t m_previousTime;
    NSTimer* m_timer;
}

- (void) animate;

@end

@implementation View

-(void)windowWillClose:(NSNotification *)note {
    [[NSApplication sharedApplication] terminate:self];
}

- (void) timerFired:(NSTimer*) timer
{
    [self animate];	
}

- (id) initWithFrame: (NSRect) frame
{
	int attribs[] = {
        NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFANoRecovery,
		kCGLPFASampleBuffers, 1, kCGLPFASamples, 16,
		0};
	
	NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc]
                                initWithAttributes:(NSOpenGLPixelFormatAttribute*) attribs];
    
	self = [self initWithFrame:frame pixelFormat:fmt];
    
	[fmt release];
    
    m_frameRect = frame;
    m_renderingEngine = SolidGL2::CreateRenderingEngine();
    m_applicationEngine = ParametricViewer::CreateApplicationEngine(m_renderingEngine);
    m_previousTime = mach_absolute_time();

    m_timer = [[NSTimer
                       scheduledTimerWithTimeInterval:1.0f/120.0f
                       target:self 
                       selector:@selector(timerFired:)
                       userInfo:nil
                       repeats:YES] retain];

	return self;
}

- (void) drawRect:(NSRect) theRect
{
    if (!m_didInit) {
        
		[[NSColor clearColor] set];
		NSRectFill([self bounds]);
        
        int opaque = NO;
        [[self openGLContext]
         setValues:&opaque
         forParameter:NSOpenGLCPSurfaceOpacity];
        
        [[self window] setOpaque:NO];
		[[self window] setAlphaValue:0.99];
        
        int width = theRect.size.width;
        int height = theRect.size.height;
        m_applicationEngine->Initialize(width, height);
        glEnable(GL_MULTISAMPLE);
        
		m_didInit = YES;
	}
    
    m_applicationEngine->Render();
    [[self openGLContext] flushBuffer];	
}

- (void) mouseDragged: (NSEvent*) theEvent
{
    NSPoint curPoint = [theEvent locationInWindow];
    ivec2 newLocation(curPoint.x, m_frameRect.size.height - curPoint.y);
    ivec2 oldLocation(newLocation.x - theEvent.deltaX, newLocation.y - theEvent.deltaY);
    m_applicationEngine->OnFingerMove(oldLocation, newLocation);
}

- (void) mouseUp: (NSEvent*) theEvent
{
    NSPoint curPoint = [theEvent locationInWindow];
    ivec2 location(curPoint.x, m_frameRect.size.height - curPoint.y);
    m_applicationEngine->OnFingerUp(location);
}

- (void) mouseDown: (NSEvent*) theEvent
{
    NSPoint curPoint = [theEvent locationInWindow];
    ivec2 location(curPoint.x, m_frameRect.size.height - curPoint.y);
    m_applicationEngine->OnFingerDown(location);
}

- (void) animate
{
    uint64_t currentTime = mach_absolute_time();
    uint64_t elapsedTime = currentTime - m_previousTime;
    m_previousTime = currentTime;
    
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    
    elapsedTime *= info.numer;
    elapsedTime /= info.denom;
    
    float timeStep = elapsedTime / 1000000000.0f;
    
    m_applicationEngine->UpdateAnimation(timeStep);
    [self display];
}

- (BOOL) isOpaque
{
	return YES;
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

- (BOOL) becomeFirstResponder
{
	return YES;
}

- (BOOL) resignFirstResponder
{
	return YES;
}

- (void) onKey: (unichar) character downEvent: (BOOL) flag
{
	switch(character)
	{
		case 27:
		case 'q':
            [[NSApplication sharedApplication] terminate:self];
			break;
	}
}

- (void) keyDown:(NSEvent *)theEvent
{
	NSString *characters;
	unsigned int characterIndex, characterCount;
	
	characters = [theEvent charactersIgnoringModifiers];
	characterCount = [characters length];
    
	for (characterIndex = 0; characterIndex < characterCount; characterIndex++) {
		[self onKey:[characters characterAtIndex:characterIndex] downEvent:YES];
	}
}

@end

int main(int argc, const char *argv[])
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    NSApplication *NSApp = [NSApplication sharedApplication];
    NSRect frame = NSMakeRect( 100., 100., 300., 300. );
    
    NSRect screenBounds = [[NSScreen mainScreen] frame];
    
	//int height = 1334;
	int height = screenBounds.size.height - 175;
    int width = 320 * height / 480;
    NSRect viewBounds = NSMakeRect(0, 0, width, height);
    
	View* view = [[View alloc] initWithFrame:viewBounds];
    
    NSRect centered = NSMakeRect(NSMidX(screenBounds) - NSMidX(viewBounds),
                                 NSMidY(screenBounds) - NSMidY(viewBounds),
                                 viewBounds.size.width, viewBounds.size.height);
    
	NSWindow *window = [[NSWindow alloc]
                        initWithContentRect:centered
//                        styleMask:NSBorderlessWindowMask
                        styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
                        backing:NSBackingStoreBuffered
                        defer:NO];

    [window setContentView:view];
    [window setDelegate:view];
 	[view release];
    [window makeKeyAndOrderFront:nil];
    
    [NSApp run];
    
    [pool release];
    return EXIT_SUCCESS;
}
