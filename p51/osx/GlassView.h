//
//  GlassView.h
//  Glass
//
//  Created by Philip Rideout on 10/6/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface GlassView : NSOpenGLView {
	NSTimer* timer;
	CFAbsoluteTime time;
}

@end
