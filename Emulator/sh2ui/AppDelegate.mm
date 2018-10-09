//
//  AppDelegate.m
//  sh2ui
//
//  Created by Antonio Malara on 26/02/2018.
//  Copyright © 2018 Antonio Malara. All rights reserved.
//

#import "AppDelegate.h"
#import "machine.h"

@interface BitmapView : NSView
@property (nonatomic) CGImageRef image;
@end

@implementation BitmapView

- (void)setImage:(CGImageRef)image;
{
    self->_image = image;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect;
{
    CGContextRef c = [NSGraphicsContext currentContext].CGContext;
    CGContextDrawImage(c, self.bounds, _image);
}

@end


@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSImageCell *imageCell;
@property (weak) IBOutlet NSImageView *imageView;

@end

@implementation AppDelegate
{
    Machine machine;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceGray();

    BitmapView * bv = [[BitmapView alloc] init];
    [self.window.contentView addSubview:bv];
    bv.frame = self.window.contentView.bounds;
    
    machine.frameCallback = ^(const uint8_t * bitmap, const size_t len) {
        auto data = [NSData dataWithBytes:bitmap length:len];
        
        auto provider = CGDataProviderCreateWithCFData((CFDataRef)data);
        auto iref = CGImageCreate(320,
                                  240,
                                  1,
                                  1,
                                  320 / 8,
                                  colorSpaceRef,
                                  kCGBitmapByteOrderDefault,
                                  provider,
                                  NULL,
                                  YES,
                                  kCGRenderingIntentDefault);
        CFRelease(provider);
        
        
        dispatch_async(dispatch_get_main_queue(), ^{
            bv.image = iref;
        });
        
    };
    
    [NSThread detachNewThreadWithBlock:^{
        self->machine.run();
    }];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
