#pragma once
#include "dispatch/core/CLAID.hh"
#import <UIKit/UIKit.h>


@interface Permission : NSObject <UIAlertViewDelegate>

@property (strong, nonatomic) UIAlertController *alertController;

@property (strong, nonatomic) NSString *permissionTitle;

@property (strong, nonatomic) NSString *permissionBody;

@property (assign, nonatomic) BOOL shouldDisplayAlert;

- (instancetype)init:(NSString*) permissionTitle permissionBody:(NSString*)permissionBody;

- (void)blockingRequest;

- (bool)isGranted;

- (void)displayBlockingAlertDialog;

@end