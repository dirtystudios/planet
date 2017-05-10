#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "RenderDelegate.h"

@interface MetalView : NSView

@property(nonatomic, assign) RenderDelegate*       delegate;
@property(nonatomic, readonly) id<MTLDevice>       device;
@property(nonatomic, readonly) id<CAMetalDrawable> currentDrawable;

// This call may block until the framebuffer is available.
@property(nonatomic, readonly) MTLRenderPassDescriptor* renderPassDescriptor;

@property(nonatomic) MTLPixelFormat colorPixelFormat;
@property(nonatomic) MTLPixelFormat depthPixelFormat;
@property(nonatomic) MTLPixelFormat stencilPixelFormat;
@property(nonatomic) NSUInteger     sampleCount;

- (void)shouldResize;
- (void)display;
- (void)releaseTextures;

@end
