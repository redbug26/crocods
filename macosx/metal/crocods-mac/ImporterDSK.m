//
//  ImporterDSK.m
//  CrocoDS
//
//  Created by Miguel Vanhove on 11/10/16.
//  Copyright © 2016 Miguel Vanhove. All rights reserved.
//

#import "ImporterDSK.h"
#import <ZipZap/ZipZap.h>

@interface ImporterDSK () {
    NSArray *games;
    
    IBOutlet NSTableView *tableView;
    IBOutlet NSImageView *imageView1;
    IBOutlet NSImageView *imageView2;
    
    NSString *baseURL;
}

@end

@implementation ImporterDSK

- (id)init {
    self = [super initWithWindowNibName:@"ImporterDSK" owner:self];
    if (self != nil) {
        baseURL = @"http://cpc.devilmarkus.de/games/";

    }
    return self;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    [self loadXML];
    
    // curl  -A "BDDBrowser/CrocoDS"  -O "http://cpc-power.com/_javacpc_markus/api.php?action=detailist"
    // http://cpc-power.com/_javacpc_markus/api.php?action=get&id=screen0_460
    
    
    //    <game id="17262" title="Boulder Dash" genre="GAMES">
    //    <media id="screen0_460" type="screenshot" name="17262_screen0.png">
    //    <media id="screen1_460" type="screenshot" name="17262_screen1.png">
    //    <media id="disk_0_17262" type="Disquette" name="Boulder Dash (UK) (1985) (Trainer).dsk" />
    //    </game>
    

//    "http://amstrad.eu/modules/pdb/",
//    "http://cpc.devilmarkus.de/games/",
//    "http://cpc.devilmarkus.de/demos/",
//    "http://www.cpc-power.com/_javacpc_markus/",
//    "http://cngsoft.no-ip.org/"


    [tableView setAction:@selector(singleClickOnAttachment:)];
    [tableView setDoubleAction:@selector(doubleClickOnAttachment:)];
    
    
    [tableView reloadData];
    [tableView sizeLastColumnToFit];
}

- (NSData *)sendSynchronousRequest:(NSURLRequest *)request returningResponse:(NSURLResponse **)response error:(NSError **)error
{
    
    NSError __block *err = NULL;
    NSData __block *data;
    BOOL __block reqProcessed = false;
    NSURLResponse __block *resp;
    
    [[[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:^(NSData * _Nullable _data, NSURLResponse * _Nullable _response, NSError * _Nullable _error) {
        resp = _response;
        err = _error;
        data = _data;
        reqProcessed = true;
    }] resume];
    
    while (!reqProcessed) {
        [NSThread sleepForTimeInterval:0];
    }
    
    *response = resp;
    *error = err;
    return data;
}

- (void)loadXML {
    NSError* error = nil;
    NSData* data;
    
    data = [NSData dataWithContentsOfFile:@"/tmp/cpc-power.xml"];
    if (data==nil) {
        
        NSString* userAgent = @"BDDBrowser/2.9.5d";
        NSURL* url = [NSURL URLWithString:[baseURL stringByAppendingString:@"api.php?action=detailist"]];
        NSMutableURLRequest* request = [[NSMutableURLRequest alloc] initWithURL:url];
        
        [request setValue:userAgent forHTTPHeaderField:@"User-Agent"];
        
        NSURLResponse* response = nil;
        data = [self sendSynchronousRequest:request returningResponse:&response error:&error];
        
        [data writeToFile:@"/tmp/cpc-power.xml" atomically:TRUE];
    }
    
    NSString *xmlString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    xmlString = [xmlString stringByReplacingOccurrencesOfString:@"png\">" withString:@"png\" />"];
    xmlString = [xmlString stringByReplacingOccurrencesOfString:@"&" withString:@"&amp;"];
    
    NSXMLDocument *document = [[NSXMLDocument alloc] initWithXMLString:xmlString options:NSXMLDocumentTidyXML error:&error];
    NSXMLElement *rootElement = [document rootElement];
    
    //    NSString* string = [document XMLStringWithOptions:NSXMLNodePrettyPrint];
    //    [string writeToFile:@"/tmp/cpc-power-pretty.xml" atomically:TRUE encoding:NSUTF8StringEncoding error:&error];
    
    games = [rootElement elementsForName:@"game"];
    
    //    for (NSXMLElement *game in games) {
    //        NSLog(@"game: %@", game);
    //        for (NSXMLElement *media in [game elementsForName:@"media"]) {
    //            NSLog(@"media: %@", media);
    //        }
    //        NSLog(@"end of game");
    //    }
}


#pragma mark - Datasource

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
    if (aTableView == tableView) {
        NSXMLElement *game = games[rowIndex];
        
        
        
        if ([[aTableColumn identifier] isEqualToString:@"attributeName"]) {
            NSString *title = [[game attributeForName:@"title"] stringValue];
            return title;
        }
    }
    
    return nil;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
    
    if (aTableView == tableView) {
        return [games count];
    }
    return 0;
}

- (void)singleClickOnAttachment:(id)sender {
}

-(void)tableViewSelectionDidChange:(NSNotification *)notification {
    
    NSInteger row = [[notification object] selectedRow];
    if (row == -1) {
        return;
    }
    
    NSXMLElement *game = games[row];
    
    
//    NSString *gameId = [[game attributeForName:@"id"] stringValue];
//    NSString *urlString = [NSString stringWithFormat:@"http://cpc-power.com/_javacpc_markus/api.php?action=detail&id=%@", gameId];
//    
//    NSString *detailString = [NSString stringWithContentsOfURL:[NSURL URLWithString:urlString] encoding:NSUTF8StringEncoding error:nil];
    
    NSInteger count=0;
    
    for (NSXMLElement *media in [game elementsForName:@"media"]) {
        
        NSString *mediaId = [[media attributeForName:@"id"] stringValue];
        
        NSString *urlString = [baseURL stringByAppendingString:[NSString stringWithFormat:@"api.php?action=get&id=%@", mediaId]];
        
        if (count==0) {
            NSImage *image = [[NSImage alloc] initWithContentsOfURL:[NSURL URLWithString:urlString]];
            [imageView1 setImage:image];
        }
        if (count==1) {
            NSImage *image = [[NSImage alloc] initWithContentsOfURL:[NSURL URLWithString:urlString]];
            [imageView2 setImage:image];
        }
        
        count++;
        if (count==2) break;
        
    }
    
    
}

- (void)doubleClickOnAttachment:(id)sender {
    NSInteger row = [tableView clickedRow];
    if (row == -1) {
        return;
    }
    
    [tableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];

    [self btnDownload:sender];
}

- (IBAction)btnDownload:(id)sender {
    
    NSInteger row = [tableView selectedRow];
    if (row == -1) {
        return;
    }
    
    NSXMLElement *game = games[row];
    for (NSXMLElement *media in [game elementsForName:@"media"]) {
        
        NSString *mediaId = [[media attributeForName:@"id"] stringValue];
        NSString *mediaType = [[media attributeForName:@"type"] stringValue];
        
        if ([mediaType isEqualToString:@"Disquette"]) {
            NSString *urlString = [baseURL stringByAppendingString:[NSString stringWithFormat:@"api.php?action=get&id=%@", mediaId]];
            NSURL *url = [NSURL URLWithString:urlString];
            
            
            NSString *filename = [[media attributeForName:@"name"] stringValue];
            if (filename==nil) {
                filename = [[media attributeForName:@"file"] stringValue];
            }
            
            NSData *data = [[NSData alloc] initWithContentsOfURL:url];
            NSString *filePath = [NSTemporaryDirectory() stringByAppendingPathComponent:filename];

            ZZArchive* archive = [ZZArchive archiveWithData:data error:nil];

            if (archive!=nil) {
                ZZArchiveEntry* entry = archive.entries[0];
                
                [[entry newDataWithError:nil] writeToFile:filePath atomically:NO];
            } else {
                [data writeToFile:filePath atomically:true];
            }
            
            NSDocumentController *dc = [NSDocumentController sharedDocumentController];
            
            [dc openDocumentWithContentsOfURL:[NSURL fileURLWithPath:filePath]
                                      display:true
                            completionHandler:^void (NSDocument * theDoc, BOOL aBool, NSError * error) {}];
            
            
            
        }
        
    }
    
}



@end
