//
//  GlassAppDelegate.m
//  Glass
//
//  Created by Philip Rideout on 10/6/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "GlassAppDelegate.h"
#import "GlassView.h"

@implementation GlassAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    
    NSRect screenBounds = [[NSScreen mainScreen] frame];

    NSRect viewBounds = NSMakeRect(0, 0, 768, 1024);
    
	view = [[GlassView alloc] initWithFrame:viewBounds];
    
    NSRect centered = NSMakeRect(NSMidX(screenBounds) - NSMidX(viewBounds),
                                 NSMidY(screenBounds) - NSMidY(viewBounds),
                                 viewBounds.size.width, viewBounds.size.height);
    
	window = [[NSWindow alloc] initWithContentRect:centered
                                         styleMask:NSBorderlessWindowMask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    
	[window setContentView:view];
	[view release];
    
	[window makeKeyAndOrderFront:nil];
}

@end
