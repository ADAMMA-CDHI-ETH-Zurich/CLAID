//  main.m
//  BazelHelloworldObjC
//
//  Created by ivan on 13/7/2022.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include "CLAIDWrapper.hpp"
#include <string>
#include "dispatch/core/Logger/Logger.hh"
@interface FileLister : NSObject

- (void)listFilesInMainBundle;

@end

@implementation FileLister

- (void)listFilesInMainBundle {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error;
    NSArray *bundleContents = [fileManager contentsOfDirectoryAtPath:[[NSBundle mainBundle] bundlePath] error:&error];
    
    if (error) {
        NSLog(@"Error accessing bundle contents: %@", [error localizedDescription]);
        return;
    }
    
    NSLog(@"Files in the main bundle:");
    for (NSString *file in bundleContents) {
        NSLog(@"%@", file);
    }
}
std::string temporaryFilePath() {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];

    return [documentsDirectory UTF8String];
}


@end


int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
        
        // Create an instance of FileLister and call the method to list files in the main bundle
        FileLister *fileLister = [[FileLister alloc] init];
        [fileLister listFilesInMainBundle];
    }
    std::string socketPath = "/tmp";
    socketPath += "/claid_local.grpc";
        // Use tempDir as needed
        
    NSLog(@"Temporary directory: %s", socketPath.c_str());
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"claid_config" ofType:@"json"];
    NSLog(@"%@", filePath);
    std::string configPath = [filePath UTF8String];
    CLAIDWrapper* wrapper = new CLAIDWrapper();
claid::Logger::logInfo("Calling start");
    wrapper->start(configPath.c_str(), "test_client", "test", "tst");
    claid::Logger::logInfo("Wrapper started");
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
