#import "GBViewMetal.h"

static const float rect_test[] =
{
    -0.5, -0.5,
    -0.5, 0.5,
    0.5,  -0.5,     // X is ignored... Why ?
    0.5,  0.5,

    -0.5, -0.5,
    -0.5, 0.5,
    0.5,  -0.5,
    0.5,  0.5,
};

static const float rect[] =
{
    -1.0, -1.0,
    -1.0, 1.0,
    1.0,  -1.0,     // X is ignored... Why ?
    1.0,  1.0,

    -1.0, -1.0,
    -1.0, 1.0,
    1.0,  -1.0,
    1.0,  1.0,
};

//static const float controlPointPositionsQuad[] = {
//    -0.8, 0.8,  0.0, 1.0,   // upper-left
//    0.8,  0.8,  0.0, 1.0,   // upper-right
//    0.8,  -0.8, 0.0, 1.0,   // lower-right
//    -0.8, -0.8, 0.0, 1.0,   // lower-left
//};

/*
 static const vector_float2 rect[] =
 {
     {-1, -1},
     { 1, -1},
     {-1,  1},
     { 1,  1},
 };
 */

@implementation GBViewMetal
{
    id<MTLDevice> device;
    id<MTLTexture> texture, previous_texture;
    id<MTLBuffer> vertices;
    id<MTLRenderPipelineState> pipeline_state;
    id<MTLCommandQueue> command_queue;
    id<MTLBuffer> mix_previous_buffer;
    id<MTLBuffer> output_resolution_buffer;
    vector_float2 output_resolution;

//    int height, width;
}

+ (bool)isSupported
{
    /*
    if (MTLCopyAllDevices) {
        return [MTLCopyAllDevices() count];
    }
    return false;
     */

    return true;
}

- (void)allocateTextures
{
    if (!device) return;

    MTLTextureDescriptor *texture_descriptor = [[MTLTextureDescriptor alloc] init];

    texture_descriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;

    texture_descriptor.width = self.width;
    texture_descriptor.height = self.height;
    
    NSLog(@"allocateTexture: %dx%d", self.width, self.height);

    texture = [device newTextureWithDescriptor:texture_descriptor];
    previous_texture = [device newTextureWithDescriptor:texture_descriptor];
}

- (void)createInternalView
{
    // TODO: update width
//    width = 384;
//    height = 272;

    MTKView *view = [[MTKView alloc] initWithFrame:self.frame device:(device = MTLCreateSystemDefaultDevice())];
    view.delegate = self;
    self.internalView = view;
    view.paused = YES;
    view.enableSetNeedsDisplay = YES;

    // Allocate memory for the control points buffers
    // These are shared or managed buffers whose contents are immediately populated by the CPU
    MTLResourceOptions controlPointsBufferOptions;
    #if TARGET_OS_IOS
    // In iOS, the storage mode can only be shared
    controlPointsBufferOptions = MTLResourceStorageModeShared;
    #elif TARGET_OS_OSX
    // In OS X, the storage mode can be shared or managed, but managed may yield better performance
    controlPointsBufferOptions = MTLResourceStorageModeManaged;
    #endif

    vertices = [device newBufferWithBytes:rect
                                   length:sizeof(rect)
                                  options:controlPointsBufferOptions];

    static const bool default_mix_value = false;
    mix_previous_buffer = [device newBufferWithBytes:&default_mix_value
                                              length:sizeof(default_mix_value)
                                             options:MTLResourceStorageModeShared];

    output_resolution_buffer = [device newBufferWithBytes:&output_resolution
                                                   length:sizeof(output_resolution)
                                                  options:MTLResourceStorageModeShared];

    output_resolution = (simd_float2) { view.drawableSize.width, view.drawableSize.height };
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(loadShader) name:@"GBFilterChanged" object:nil];
    [self loadShader];
}

- (void)loadShader
{
    NSError *error = nil;
    NSString *shader_source = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"MasterShader"
                                                                                                 ofType:@"metal"
                                                                                            inDirectory:nil]
                                                        encoding:NSUTF8StringEncoding
                                                           error:nil];

//    NSString *shader_name = @"NearestNeighbor";
    NSString *shader_name = @"LCD";
    NSString *scaler_source = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:shader_name
                                                                                                 ofType:@"fsh"
                                                                                            inDirectory:nil]
                                                        encoding:NSUTF8StringEncoding
                                                           error:nil];

    shader_source = [shader_source stringByReplacingOccurrencesOfString:@"{filter}"
                                                             withString:scaler_source];

    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    options.fastMathEnabled = YES;
    id<MTLLibrary> library = [device newLibraryWithSource:shader_source
                                                  options:options
                                                    error:&error];
    if (error) {
        NSLog(@"Error: %@", error);
        if (!library) {
            return;
        }
    }

    id<MTLFunction> vertex_function = [library newFunctionWithName:@"vertex_shader"];
    id<MTLFunction> fragment_function = [library newFunctionWithName:@"fragment_shader"];

    // Set up a descriptor for creating a pipeline state object
    MTLRenderPipelineDescriptor *pipeline_state_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipeline_state_descriptor.vertexFunction = vertex_function;
    pipeline_state_descriptor.fragmentFunction = fragment_function;
    pipeline_state_descriptor.colorAttachments[0].pixelFormat = ((MTKView *)self.internalView).colorPixelFormat;

    error = nil;
    pipeline_state = [device newRenderPipelineStateWithDescriptor:pipeline_state_descriptor
                                                            error:&error];
    if (error) {
        NSLog(@"Failed to created pipeline state, error %@", error);
        return;
    }

    command_queue = [device newCommandQueue];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    output_resolution = (vector_float2) { size.width, size.height };
    dispatch_async(dispatch_get_main_queue(), ^{
        [(MTKView *)self.internalView draw];
    });
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    #if TARGET_OS_OSX
    if (!(view.window.occlusionState & NSWindowOcclusionStateVisible)) return;
#endif

    if ((texture.width  != self.width) || (texture.height != self.height)) {
        [self allocateTextures];
    }

    MTLRegion region = {
        { 0,             0,              0                                                                               }, // MTLOrigin
        { texture.width, texture.height, 1                                                                               } // MTLSize
    };

//    uint32_t *buf =[self currentBuffer];
//    NSLog(@"%x-%x-%x-%x", buf[0], buf[1], buf[2], buf[3]);

    [texture replaceRegion:region
               mipmapLevel:0
                 withBytes:[self currentBuffer]
               bytesPerRow:texture.width * 4];

   

    MTLRenderPassDescriptor *render_pass_descriptor = view.currentRenderPassDescriptor;
    id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];

    if (render_pass_descriptor != nil) {
        *(bool *)[mix_previous_buffer contents] = 0;
        *(vector_float2 *)[output_resolution_buffer contents] = output_resolution;

        id<MTLRenderCommandEncoder> render_encoder =
            [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];

        [render_encoder setViewport:(MTLViewport) { 0.0, 0.0,
                                                    output_resolution.x,
                                                    output_resolution.y,
                                                    0.0, 1.0 }];

        [render_encoder setRenderPipelineState:pipeline_state];

        [render_encoder setVertexBuffer:vertices
                                 offset:0
                                atIndex:0];

        [render_encoder setFragmentBuffer:mix_previous_buffer
                                   offset:0
                                  atIndex:0];

        [render_encoder setFragmentBuffer:output_resolution_buffer
                                   offset:0
                                  atIndex:1];

        [render_encoder setFragmentTexture:texture
                                   atIndex:0];

        [render_encoder setFragmentTexture:previous_texture
                                   atIndex:1];

//        [render_encoder setTriangleFillMode:MTLTriangleFillModeLines];  // Enable wireframe

        [render_encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip        // MTLPrimitiveTypeTriangle
                           vertexStart:0
                           vertexCount:8];

        [render_encoder endEncoding];

        [command_buffer presentDrawable:view.currentDrawable];
    }

    [command_buffer commit];

//    NSLog(@"drawInMTKView");
}

- (void)flip
{
    [super flip];
    dispatch_async(dispatch_get_main_queue(), ^{
        #if TARGET_OS_IOS
        [self.internalView setNeedsDisplay];
        #elif TARGET_OS_OSX
        [(MTKView *)self.internalView setNeedsDisplay:YES];
        #endif
    });
}

@end
