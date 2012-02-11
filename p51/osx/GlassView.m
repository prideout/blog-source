#import "GlassView.h"
#import "../Platform.h"

@implementation GlassView

- (id)initWithFrame:(NSRect)frame {
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
        (NSOpenGLPixelFormatAttribute)0
    };
    
	NSOpenGLPixelFormat * pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
    self = [super initWithFrame: frame pixelFormat: pf];

    [[self openGLContext] makeCurrentContext];	
    glewInit();

    PezInitialize(frame.size.width, frame.size.height);
    
	// set start values...
	time = CFAbsoluteTimeGetCurrent ();
    
	// start animation timer
	timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode];
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    PezRender(0);
    [[self openGLContext] flushBuffer];	
}

- (void)animationTimer:(NSTimer *)timer
{
    CFTimeInterval deltaTime = CFAbsoluteTimeGetCurrent () - time;
    PezUpdate(deltaTime * 1000000);
    time = CFAbsoluteTimeGetCurrent ();
    [self drawRect:[self bounds]];
}

- (void) mouseDown: (NSEvent*) theEvent
{
    [[NSApplication sharedApplication] terminate:self];
}

@end
