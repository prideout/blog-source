#import <Cocoa/Cocoa.h>

@interface CustomWindow : NSWindow {
    // This point is used in dragging to mark the initial click location
    NSPoint initialLocation;
}

@property (assign) NSPoint initialLocation;

@end
