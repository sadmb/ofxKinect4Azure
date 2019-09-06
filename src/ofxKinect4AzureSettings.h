#pragma once

#include "k4a.h"

#define ofxKinect4AzureSettings k4a_device_configuration_t

//struct ofxKinect4AzureSettings : k4a_device_configuration_t {
//
//public:
//	ofxKinect4AzureSettings() {
//		color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
//		color_resolution = K4A_COLOR_RESOLUTION_OFF;
//		depth_mode = K4A_DEPTH_MODE_OFF;
//		camera_fps = K4A_FRAMES_PER_SECOND_30;
//		synchronized_images_only = false;
//		depth_delay_off_color_usec = 0;
//		wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE;
//		subordinate_delay_off_master_usec = 0;
//		disable_streaming_indicator = false;
//	};
//
//	~ofxKinect4AzureSettings() {};
//};
