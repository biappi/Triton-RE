//
//  AppDelegate.m
//  sh2ui
//
//  Created by Antonio Malara on 26/02/2018.
//  Copyright © 2018 Antonio Malara. All rights reserved.
//

#import "AppDelegate.h"
#import "machine.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSImageCell *imageCell;

@end

@implementation AppDelegate
{
    Machine machine;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceGray();

    machine.frameCallback = ^(const uint8_t * bitmap, const size_t len) {
        auto data = [NSData dataWithBytes:bitmap length:len];
        
        dispatch_async(dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0), ^{
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
            
            auto image = [[NSImage alloc] initWithCGImage:iref size:NSMakeSize(320, 240)];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                _imageCell.image = image;
            });

        });
    };
    
    [NSThread detachNewThreadWithBlock:^{
        machine.run();
    }];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
