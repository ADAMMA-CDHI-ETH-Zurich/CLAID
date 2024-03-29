/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#import "Permission.hh"
#import <UIKit/UIKit.h>
@implementation Permission

- (instancetype)init:(NSString*) permissionTitle permissionBody:(NSString*)permissionBody;
{
    self = [super init];
    if (self) {
        _permissionTitle = permissionTitle;
        _permissionBody = permissionBody;
    }
    return self;
}

- (void)displayBlockingAlertDialog
{
    dispatch_async(dispatch_get_main_queue(), ^(void){
        UIWindowScene *windowScene = (UIWindowScene *)[[UIApplication sharedApplication].connectedScenes anyObject];
        UIWindow *window = windowScene.windows.firstObject;
        
        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:_permissionTitle message:_permissionBody preferredStyle:UIAlertControllerStyleAlert];
        
        UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:UIApplicationOpenSettingsURLString] options:@{} completionHandler:nil];
        }];
        
        [alertController addAction:okAction];
        
        [window.rootViewController presentViewController:alertController animated:YES completion:nil];
    });
}


- (bool)isGranted{}

- (void)blockingRequest{}

@end
