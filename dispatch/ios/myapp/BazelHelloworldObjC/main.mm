//
//  main.m
//  BazelHelloworldObjC
//
//  Created by ivan on 13/7/2022.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include "test.hpp"
int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    void* handle = start_core("test", "test", "test", "test", "tst");
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
