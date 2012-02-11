#import <Cocoa/Cocoa.h>
#import <AppKit/NSDragging.h>

@interface CustomView : NSOpenGLView <NSWindowDelegate> {
    NSRect m_frameRect;
    BOOL m_didInit;
    uint64_t m_previousTime;
    NSTimer* m_timer;
}

- (void) animate;

@end
