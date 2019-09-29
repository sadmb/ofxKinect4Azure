#pragma once

#include "k4a.h"

//#define ofxKinect4AzureSettings k4a_device_configuration_t
enum OFX_K4A_TRANSFORM_TYPE {
	NONE,
	COLOR_TO_DEPTH,
	DEPTH_TO_COLOR
};

static const std::map<k4abt_joint_id_t, string> joint_id_to_name
{
	{ K4ABT_JOINT_PELVIS ,"K4ABT_JOINT_PELVIS" },
	{ K4ABT_JOINT_SPINE_NAVAL ,"K4ABT_JOINT_SPINE_NAVAL" },
	{ K4ABT_JOINT_SPINE_CHEST ,"K4ABT_JOINT_SPINE_CHEST" },
	{ K4ABT_JOINT_NECK ,"K4ABT_JOINT_NECK" },
	{ K4ABT_JOINT_CLAVICLE_LEFT ,"K4ABT_JOINT_CLAVICLE_LEFT" },
	{ K4ABT_JOINT_SHOULDER_LEFT ,"K4ABT_JOINT_SHOULDER_LEFT" },
	{ K4ABT_JOINT_ELBOW_LEFT ,"K4ABT_JOINT_ELBOW_LEFT" },
	{ K4ABT_JOINT_WRIST_LEFT ,"K4ABT_JOINT_WRIST_LEFT" },
	{ K4ABT_JOINT_CLAVICLE_RIGHT ,"K4ABT_JOINT_CLAVICLE_RIGHT" },
	{ K4ABT_JOINT_SHOULDER_RIGHT ,"K4ABT_JOINT_SHOULDER_RIGHT" },
	{ K4ABT_JOINT_ELBOW_RIGHT ,"K4ABT_JOINT_ELBOW_RIGHT" },
	{ K4ABT_JOINT_WRIST_RIGHT ,"K4ABT_JOINT_WRIST_RIGHT" },
	{ K4ABT_JOINT_HIP_LEFT ,"K4ABT_JOINT_HIP_LEFT" },
	{ K4ABT_JOINT_KNEE_LEFT ,"K4ABT_JOINT_KNEE_LEFT" },
	{ K4ABT_JOINT_ANKLE_LEFT ,"K4ABT_JOINT_ANKLE_LEFT" },
	{ K4ABT_JOINT_FOOT_LEFT ,"K4ABT_JOINT_FOOT_LEFT" },
	{ K4ABT_JOINT_HIP_RIGHT ,"K4ABT_JOINT_HIP_RIGHT" },
	{ K4ABT_JOINT_KNEE_RIGHT ,"K4ABT_JOINT_KNEE_RIGHT" },
	{ K4ABT_JOINT_ANKLE_RIGHT ,"K4ABT_JOINT_ANKLE_RIGHT" },
	{ K4ABT_JOINT_FOOT_RIGHT ,"K4ABT_JOINT_FOOT_RIGHT" },
	{ K4ABT_JOINT_HEAD ,"K4ABT_JOINT_HEAD" },
	{ K4ABT_JOINT_NOSE ,"K4ABT_JOINT_NOSE" },
	{ K4ABT_JOINT_EYE_LEFT ,"K4ABT_JOINT_EYE_LEFT" },
	{ K4ABT_JOINT_EAR_LEFT ,"K4ABT_JOINT_EAR_LEFT" },
	{ K4ABT_JOINT_EYE_RIGHT ,"K4ABT_JOINT_EYE_RIGHT" },
	{ K4ABT_JOINT_EAR_RIGHT ,"K4ABT_JOINT_EAR_RIGHT" }
};

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

