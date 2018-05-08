/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <Cocoa/Cocoa.h>
#import "mcWebView.h"

@interface MCAppDelegate : NSObject<NSApplicationDelegate, MCWebViewDelegate>
{
}

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet MCWebView *webview;

+ (MCAppDelegate *)instance;

@end
