//
//  GoldenTriangleAppDelegate.h
//  GoldenTriangle
//
//  Created by Philip Rideout on 5/9/10.
//  Copyright 2010 Apple Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface GoldenTriangleAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
