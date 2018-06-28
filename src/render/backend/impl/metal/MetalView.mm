//
//  MetalView.mm
//  planet
//
//  Created by Eugene Sturm on 6/28/18.
//

#import "MetalView.h"
#import <QuartzCore/CAMetalLayer.h>

@implementation MetalView

- (CALayer*)makeBackingLayer
{
    return [CAMetalLayer layer];
}

+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    
    if (self) {
        self.wantsLayer = YES;
        [self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    }
    
    return self;
}

@end
