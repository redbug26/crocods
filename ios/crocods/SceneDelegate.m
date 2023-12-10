#import "AppDelegate.h"
#import "SceneDelegate.h"
#import "ViewController.h"

@interface SceneDelegate ()

@end

@implementation SceneDelegate

- (void)sceneDidDisconnect:(UIScene *)scene {
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not neccessarily discarded (see `application:didDiscardSceneSessions` instead).
}

- (void)sceneDidBecomeActive:(UIScene *)scene {
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
}

- (void)sceneWillResignActive:(UIScene *)scene {
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
}

- (void)sceneWillEnterForeground:(UIScene *)scene {
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
}

- (void)sceneDidEnterBackground:(UIScene *)scene {
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
}

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions  API_AVAILABLE(ios(13.0))
{
    NSSet *c = connectionOptions.URLContexts;
    if (c && [c count] > 0) {
        NSURL *url = ((UIOpenURLContext *)[[c allObjects] firstObject]).URL;

        NSLog(@"load %@", url);

        BOOL accessAllow = [url startAccessingSecurityScopedResource];

        __block NSData *data;

        NSFileCoordinator *coordinator = [[NSFileCoordinator alloc] init];
        NSError *error = nil;

        [coordinator coordinateReadingItemAtURL:url options:0 error:&error byAccessor:^(NSURL *newURL) {
            data = [NSData dataWithContentsOfURL:newURL];
            
            [AppDelegate sharedSession].dataToLoad = data;
            [AppDelegate sharedSession].fileToLoad = [newURL lastPathComponent];
        }];

        if (accessAllow) {
            [url stopAccessingSecurityScopedResource];
        }
    }
}

- (void)scene:(UIScene *)scene openURLContexts:(nonnull NSSet<UIOpenURLContext *> *)URLContexts
    API_AVAILABLE(ios(13.0))
{
    NSURL *url = [[URLContexts allObjects] firstObject].URL;

    BOOL accessAllow = [url startAccessingSecurityScopedResource];

    __block NSData *data;

    NSFileCoordinator *coordinator = [[NSFileCoordinator alloc] init];
    NSError *error = nil;

    [coordinator coordinateReadingItemAtURL:url options:0 error:&error byAccessor:^(NSURL *newURL) {
        data = [NSData dataWithContentsOfURL:newURL];

        [[ViewController sharedInstance] loadData:data withAutorun:true andFilename:[newURL lastPathComponent]];
    }];

    if (accessAllow) {
        [url stopAccessingSecurityScopedResource];
    }
}

@end
