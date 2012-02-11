//
//  GlassAppDelegate.h
//  Glass
//
//  Created by Philip Rideout on 10/6/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GlassView;

@interface GlassAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
    GlassView *view;
}

@property (assign) IBOutlet NSWindow *window;

@end
