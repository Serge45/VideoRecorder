//
//  cameradevice_osx.m
//  VideoRecorder
//
//  Created by Serge Lu on 2015/1/11.
//
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#include <QString>
#include "cameradevice.h"

int CameraDevice::listDevicesOSX(bool /*silent*/) {
    deviceList.clear();
    
    NSArray *devices = [AVCaptureDevice devices];
    
    for (AVCaptureDevice *device in devices) {
        if ([device hasMediaType: AVMediaTypeVideo]) {
            deviceList.push_back(QString::fromNSString(device.localizedName));
        }
    }
    return deviceList.size();
}