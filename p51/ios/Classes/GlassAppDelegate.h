#import <UIKit/UIKit.h>
#import "GlassView.h"

@interface GlassAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	GlassView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet GlassView *glView;

@end

