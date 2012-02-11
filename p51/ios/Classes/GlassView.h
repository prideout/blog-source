#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

@interface GlassView : UIView {

@private
    EAGLContext* m_context;
    float m_timestamp;
	unsigned int m_fbo;
    unsigned int m_colorbuffer;
}

@end
