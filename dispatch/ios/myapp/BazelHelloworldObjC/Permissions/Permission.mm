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
