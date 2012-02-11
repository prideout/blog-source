#import "GlassView.h"
#import "Platform.h"

@implementation GlassView

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}


- (void) drawView: (CADisplayLink*) displayLink
{
    if (displayLink != nil)
	{
        float elapsedSeconds = displayLink.timestamp - m_timestamp;
        m_timestamp = displayLink.timestamp;
        unsigned int microseconds = elapsedSeconds * 1000 * 1000;
		PezUpdate(microseconds);
    }
	
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorbuffer);
	PezRender(m_fbo);
	PezCheckCondition(GL_NO_ERROR == glGetError(), "OpenGL Error.");
	
    [m_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (id)initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
	{
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		if (!m_context || ![EAGLContext setCurrentContext:m_context])
		{
			[self release];
			return nil;
		}
		
		glGenRenderbuffers(1, &m_colorbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_colorbuffer);
		
        [m_context renderbufferStorage:GL_RENDERBUFFER
						  fromDrawable:eaglLayer];
		
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER, m_colorbuffer);
		
		glBindRenderbuffer(GL_RENDERBUFFER, m_colorbuffer);
        glViewport(0, 0, 768, 1024);
		
		PezInitialize(768, 1024);
		
        [self drawView: nil];
        m_timestamp = CACurrentMediaTime();
		
        CADisplayLink* displayLink;
        displayLink = [CADisplayLink displayLinkWithTarget:self
												  selector:@selector(drawView:)];
        
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop]
						  forMode:NSDefaultRunLoopMode];
	}
    return self;
}

- (void)dealloc
{
    [super dealloc];
}


@end
