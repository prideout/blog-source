#import "CustomView.h"
#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>
#import <mach/mach_time.h>
#import <AppKit/NSDragging.h>
#import "../tinylib/pez.h"

@implementation CustomView

- (BOOL) performDragOperation:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pboard = [sender draggingPasteboard];
	
    if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
        NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
        for (NSString* item in files)
        {
            // PezReceiveDrop([item UTF8String]);
        }
    }
    return YES;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
    return YES;
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    if ([[sender draggingPasteboard] availableTypeFromArray:[NSArray arrayWithObject:NSFilenamesPboardType]])
    {
        return NSDragOperationCopy;
    }
    return NSDragOperationNone;
}

- (void)awakeFromNib
{
    //NSRect screenVisibleFrame = [[NSScreen mainScreen] visibleFrame];
    [self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];
    self.frame = NSMakeRect(0, 0, PezGetConfig().Width, PezGetConfig().Height);

    NSRect frame = [self frame];
    
    m_didInit = FALSE;
    
    int attribs[] = {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFANoRecovery,
        0
    };

    NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc]
                            initWithAttributes:(NSOpenGLPixelFormatAttribute*) attribs];

    self = [self initWithFrame:frame pixelFormat:fmt];

    [fmt release];
	
    m_frameRect = frame;
    m_previousTime = mach_absolute_time();

    m_timer = [[NSTimer
                       scheduledTimerWithTimeInterval:1.0f/120.0f
                       target:self 
                       selector:@selector(timerFired:)
                       userInfo:nil
                       repeats:YES] retain];

}

- (void)dealloc
{
    [super dealloc];
}

- (void)drawRect:(NSRect)rect
{
	[[NSColor clearColor] set];
	NSRectFill([self bounds]);

    if (!m_didInit) {
            
        int TransparentWindow = 1;
        if (TransparentWindow) {
            int opaque = NO;
            [[self openGLContext]
                setValues:&opaque
                forParameter:NSOpenGLCPSurfaceOpacity];
    
            [[self window] setOpaque:NO];
            [[self window] setAlphaValue:1.0f];
        }
        
        glewInit();
        PezInitialize();
        m_didInit = YES;
        
//        [[self window] setLevel: NSFloatingWindowLevel];
        [[self window] makeKeyAndOrderFront: self];
        [[self window] setTitle: [NSString stringWithUTF8String: PezGetConfig().Title]];
    }

    PezRender();
    [[self openGLContext] flushBuffer]; 
}

-(void)windowWillClose:(NSNotification *)note {
    [[NSApplication sharedApplication] terminate:self];
}

- (void) timerFired:(NSTimer*) timer
{
    [self animate];     
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
    
    float timeStep = elapsedTime / 1000.0f;

    PezUpdate(timeStep);
    
    [self display];
}

@end

