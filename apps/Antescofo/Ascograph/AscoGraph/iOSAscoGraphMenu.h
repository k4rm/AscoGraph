//
//  iOSAscoGraphMenu.h
//  AscoGraph
//
//  Created by Thomas Coffy on 06/04/14.
//  Copyright (c) 2014 IRCAM. All rights reserved.
//

#ifndef __AscoGraph__iOSAscoGraphMenu__
#define __AscoGraph__iOSAscoGraphMenu__

#include <iostream>
#import <UIKit/UIKit.h>

@interface iOSAscoGraphMenu : UIViewController

//@property(retain, nonatomic) IBOutlet UISlider *radiusSl;
@property(retain, nonatomic) IBOutlet UISwitch *autoScrollSwitch;
@property(retain, nonatomic) IBOutlet UISwitch *fastForwardOnOffSwitch;

@end

#endif /* defined(__AscoGraph__iOSAscoGraphMenu__) */
