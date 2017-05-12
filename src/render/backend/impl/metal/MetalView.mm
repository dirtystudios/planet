#import "MetalView.h"

@implementation MetalView {
@private
    CAMetalLayer* _metalLayer;

    BOOL _layerSizeDidUpdate;

    id<MTLTexture> _depthTex;
    id<MTLTexture> _stencilTex;
    id<MTLTexture> _msaaTex;
}
@synthesize currentDrawable      = _currentDrawable;
@synthesize renderPassDescriptor = _renderPassDescriptor;

+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (void)setColorPixelFormat:(MTLPixelFormat)colorPixelFormat {
    _metalLayer.pixelFormat = colorPixelFormat;
}

- (MTLPixelFormat)colorPixelFormat {
    return _metalLayer.pixelFormat;
}

- (void)initCommon {
    self.wantsLayer = YES;
    self.layer = _metalLayer = [CAMetalLayer layer];
    self.sampleCount         = 1;

    _device = MTLCreateSystemDefaultDevice();

    _metalLayer.device      = _device;
    _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    // this is the default but if we wanted to perform compute on the final
    // rendering layer we could set this to no
    _metalLayer.framebufferOnly = NO;
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];

    if (self) {
        [self initCommon];
    }

    return self;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
    self = [super initWithCoder:coder];

    if (self) {
        [self initCommon];
    }
    return self;
}

- (void)releaseTextures {
    _depthTex   = nil;
    _stencilTex = nil;
    _msaaTex    = nil;
}

- (void)setupRenderPassDescriptorForTexture:(id<MTLTexture>)texture {
    _renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];

    MTLRenderPassColorAttachmentDescriptor* colorAttachment = _renderPassDescriptor.colorAttachments[0];
    colorAttachment.texture                                 = texture;
    colorAttachment.loadAction                              = MTLLoadActionClear;
    colorAttachment.clearColor                              = MTLClearColorMake(0.15f, 0.15f, 0.15f, 1.0f);

    // if sample count is greater than 1, render into using MSAA, then resolve
    // into our color texture
    if (_sampleCount > 1) {
        bool shouldCreateMsaaTex = !_msaaTex || (_metalLayer.pixelFormat != [_msaaTex pixelFormat]) || (_msaaTex.width != texture.width) ||
                                   (_msaaTex.height != texture.height) || (_msaaTex.sampleCount != _sampleCount);

        if (shouldCreateMsaaTex) {
            MTLTextureDescriptor* desc =
                [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_metalLayer.pixelFormat width:texture.width height:texture.height mipmapped:NO];
            desc.textureType = MTLTextureType2DMultisample;
            desc.sampleCount = _sampleCount;

            _msaaTex = [_device newTextureWithDescriptor:desc];
        }

        colorAttachment.texture        = _msaaTex;
        colorAttachment.resolveTexture = texture;
        colorAttachment.storeAction    = MTLStoreActionMultisampleResolve;
    } else {
        colorAttachment.storeAction = MTLStoreActionStore;
    }

    // depth
    if (_depthPixelFormat != MTLPixelFormatInvalid) {
        BOOL shouldCreateDepthTex = !_depthTex || (_depthPixelFormat != [_depthTex pixelFormat]) || (_depthTex.width != texture.width) ||
                                    (_depthTex.height != texture.height) || (_depthTex.sampleCount != _sampleCount);

        if (shouldCreateDepthTex) {
            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_depthPixelFormat width:texture.width height:texture.height mipmapped:NO];

            desc.textureType = (_sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
            desc.sampleCount = _sampleCount;
            desc.usage       = MTLTextureUsageUnknown;
            desc.storageMode = MTLStorageModePrivate;

            _depthTex = [_device newTextureWithDescriptor:desc];
        }

        MTLRenderPassDepthAttachmentDescriptor* depthAttachment = _renderPassDescriptor.depthAttachment;
        depthAttachment.texture                                 = _depthTex;
        depthAttachment.loadAction                              = MTLLoadActionClear;
        depthAttachment.storeAction                             = MTLStoreActionDontCare;
        depthAttachment.clearDepth                              = 1.0;
    }

    // stencil
    if (_stencilPixelFormat != MTLPixelFormatInvalid) {
        BOOL shouldCreateStencilTex = !_stencilTex || (_stencilPixelFormat != [_stencilTex pixelFormat]) || (_stencilTex.width != texture.width) ||
                                      (_stencilTex.height != texture.height) || (_stencilTex.sampleCount != _sampleCount);

        if (shouldCreateStencilTex) {
            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_stencilPixelFormat width:texture.width height:texture.height mipmapped:NO];

            desc.textureType = (_sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
            desc.sampleCount = _sampleCount;

            _stencilTex = [_device newTextureWithDescriptor:desc];
        }

        MTLRenderPassStencilAttachmentDescriptor* stencilAttachment = _renderPassDescriptor.stencilAttachment;
        stencilAttachment.texture                                   = _stencilTex;
        stencilAttachment.loadAction                                = MTLLoadActionClear;
        stencilAttachment.storeAction                               = MTLStoreActionDontCare;
        stencilAttachment.clearStencil                              = 0;
    }
}

- (MTLRenderPassDescriptor*)renderPassDescriptor {
    id<CAMetalDrawable> drawable = self.currentDrawable;
    if (!drawable) {
        _renderPassDescriptor = nil;
    } else {
        [self setupRenderPassDescriptorForTexture:drawable.texture];
    }

    return _renderPassDescriptor;
}

- (id<CAMetalDrawable>)currentDrawable {
    if (_currentDrawable == nil) {
        _currentDrawable = [_metalLayer nextDrawable];
    }

    return _currentDrawable;
}

- (void)shouldResize {
    _layerSizeDidUpdate = true;
}

- (void)display {
    @autoreleasepool {
        if (_layerSizeDidUpdate) {
            // we are a subview of the SDLView
            CGSize drawableSize = self.superview.bounds.size;

            // scale drawableSize so that drawable is 1:1 width pixels not 1:1 to
            // points
            NSScreen* screen = self.window.screen ?: [NSScreen mainScreen];
            drawableSize.width *= screen.backingScaleFactor;
            drawableSize.height *= screen.backingScaleFactor;
            
            _metalLayer.drawableSize = drawableSize;
            _layerSizeDidUpdate      = NO;
        }

        self.delegate->SubmitToGPU();

        // do not retain current drawable beyond the frame.
        _currentDrawable = nil;
    }
}
- (CGSize)extents
{
    return _metalLayer.drawableSize;
}

// using SDL, we are a subview so these are handled by the SDLView
//
//- (void)setFrameSize:(NSSize)newSize
//{
//    [super setFrameSize:newSize];
//    _layerSizeDidUpdate = YES;
//}
//
//- (void)setBoundsSize:(NSSize)newSize
//{
//    [super setBoundsSize:newSize];
//    _layerSizeDidUpdate = YES;
//}
//- (void)viewDidChangeBackingProperties
//{
//    [super viewDidChangeBackingProperties];
//    _layerSizeDidUpdate = YES;
//}

@end
