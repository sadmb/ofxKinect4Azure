#pragma once

#include "k4a.hpp"
#include "k4abt.hpp"
#include "ofxKinect4AzureUtils.h"

struct ofxKinect4AzureSettings : k4a_device_configuration_t {
public:
	OFX_K4A_TRANSFORM_TYPE transform_type = NONE;
	bool make_pointcloud = false;
	bool enable_imu = false;
	bool make_colorize_depth = false;
	bool use_ir_image = false;
	bool use_tracker = false;

	ofxKinect4AzureSettings() {
		color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
		color_resolution = K4A_COLOR_RESOLUTION_720P;
		depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
		camera_fps = K4A_FRAMES_PER_SECOND_30;
		synchronized_images_only = true;
		depth_delay_off_color_usec = 0;
		wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE;
		subordinate_delay_off_master_usec = 0;
		disable_streaming_indicator = false;
	};

	~ofxKinect4AzureSettings() {};
};

struct ofxKinect4AzureBodySettings : k4abt_tracker_configuration_t {
	ofxKinect4AzureBodySettings()
	{
		this->sensor_orientation = K4ABT_SENSOR_ORIENTATION_DEFAULT;
	}
};
