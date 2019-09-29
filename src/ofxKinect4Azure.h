#pragma once

#include "ofMain.h"
#include "k4a.hpp"
#include "k4abt.h"
#include "ofxKinect4AzureSettings.h"

#define STR(var) #var


class ofxKinect4Azure {

public:
	ofxKinect4AzureSettings settings;

	//k4a handles
	k4a_device_t device;
	k4a_calibration_t calibration;
	k4a_transformation_t transformation;
	k4a_imu_sample_t imu;
	//k4abt handles
	k4abt_tracker_t tracker;
	vector<k4abt_body_t> body;

	//color/depth resolution
	ofVec2f color_size, depth_size;

	//pixels
	ofPixels pix;
	ofShortPixels depth_pix;
	ofPixels colorized_depth_pix;
	ofShortPixels ir_pix;

	//textures
	ofTexture color;
	ofTexture depth;
	ofTexture colorized_depth;
	ofTexture ir;

	//for pointcloud
	ofVbo pointcloud_vbo;
	int vertices_num = 0;
	vector<glm::vec3> pointcloud_vert;
	vector<glm::vec2> pointcloud_uv;

	//for tracker
	vector<vector<glm::vec3>> bones;

	//flag for 
	bool is_frame_new = false;
	bool b_tex_new = false;
	bool b_depth_tex_new = false;
	bool b_ir_tex_new = false;

	int device_index = -1;
	int device_count = 0;

	ofxKinect4Azure()
	{
		// Check for devices
		//
		device_count = k4a::device::get_installed_count();
		if (device_count == 0)
		{
			ofLogError("ofxKinect4Azure") << "No Azure Kinect devices detected.";
		}

	};

	ofxKinect4Azure(const ofxKinect4Azure& k4a)
	{
		ofxKinect4Azure();
	}

	ofxKinect4Azure &operator=(const ofxKinect4Azure& k4a)
	{
		ofxKinect4Azure();
		return (*this);
	}
	~ofxKinect4Azure() {
		k4a_device_stop_cameras(device);
		if (settings.enable_imu) {
			k4a_device_stop_imu(device);
		}
		k4a_device_close(device);
	};

	void setup();
	void setup(ofxKinect4AzureSettings settings);
	void setup(int index);
	void setup(int index, ofxKinect4AzureSettings settings);
	void saveCalibration(string filename = "calibration.json");
	void update();
	void updatePointcloud();
	void draw(float x = 0, float y = 0, float w = 0, float h = 0);
	void drawColorizedDepth(float x = 0, float y = 0, float w = 0, float h = 0);
	void drawIR(float x = 0, float y = 0, float w = 0, float h = 0);
	float getWidth() { return color_size.x; }
	float getHeight() { return color_size.y; }
	float getDepthWidth() { return depth_size.x; }
	float getDepthHeight() { return depth_size.y; }
	bool isFrameNew() { return is_frame_new; }

	//set transform mode COLOR_TO_DEPTH or DEPTH_TO_COLOR or NONE
	void setTransformType(OFX_K4A_TRANSFORM_TYPE _transform_type) { settings.transform_type = _transform_type; }

	//set IMU able state 
	void enableIMU() {
		k4a_device_start_imu(device);
		settings.enable_imu = true;
	}
	void disableIMU() {
		k4a_device_stop_imu(device);
		settings.enable_imu = false;
	}

	ofPixels& getPixels() { return pix; }
	ofShortPixels& getDepthPixels() { return depth_pix; }
	ofVbo& getPointcloudVbo() { return pointcloud_vbo; }

	ofTexture& getTexture() {
		if (is_frame_new) {
			if (!b_tex_new) {
				color.allocate(pix);
				b_tex_new = true;
			}
		}
		return color;
	}
	ofTexture& getDepthTexture() {
		if (is_frame_new) {
			if (!b_depth_tex_new) {
				depth.allocate(depth_pix);
				b_depth_tex_new = true;
			}
		}
		return depth;
	}
	ofTexture& getIRTexture() {
		if (is_frame_new) {
			if (!b_ir_tex_new) {
				ir.allocate(ir_pix);
				b_ir_tex_new = true;
			}
		}
		return ir;
	}

	ofVec2f getDepthModeRange()
	{
		return getDepthModeRange(settings.depth_mode);
	}

	ofVec2f getDepthModeRange(const k4a_depth_mode_t depth_mode)
	{
		ofVec2f range;
		switch (depth_mode)
		{
		case K4A_DEPTH_MODE_NFOV_2X2BINNED:
			return range = ofVec2f(500, 5800);
		case K4A_DEPTH_MODE_NFOV_UNBINNED:
			return range = ofVec2f(500, 4000);
		case K4A_DEPTH_MODE_WFOV_2X2BINNED:
			return range = ofVec2f(250, 3000);
		case K4A_DEPTH_MODE_WFOV_UNBINNED:
			return range = ofVec2f(250, 2500);

		case K4A_DEPTH_MODE_PASSIVE_IR:
		default:
			ofLogError("ofxKinect4Azure") << "Invalid depth mode!";
			return range = ofVec2f();
		}
	}
private:
	ofVec2f getColorResolution() {
		return getColorResolution(settings.color_resolution);
	}

	ofVec2f getColorResolution(const k4a_color_resolution_t color_resolution)
	{
		ofVec2f resolution;
		switch (color_resolution)
		{
		case K4A_COLOR_RESOLUTION_720P:
			return resolution = ofVec2f(1280, 720);
		case K4A_COLOR_RESOLUTION_2160P:
			return resolution = ofVec2f(3840, 2160);
		case K4A_COLOR_RESOLUTION_1440P:
			return resolution = ofVec2f(2560, 1440);
		case K4A_COLOR_RESOLUTION_1080P:
			return resolution = ofVec2f(1920, 1080);
		case K4A_COLOR_RESOLUTION_3072P:
			return resolution = ofVec2f(4096, 3072);
		case K4A_COLOR_RESOLUTION_1536P:
			return resolution = ofVec2f(2048, 1536);

		default:
			ofLogError("ofxKinect4Azure") << "Invalid color dimensions value!";
			return resolution = ofVec2f();
		}
	}

	ofVec2f getDepthResolution() {
		return getDepthResolution(settings.depth_mode);
	}

	ofVec2f getDepthResolution(const k4a_depth_mode_t depth_mode)
	{
		ofVec2f resolution;
		switch (depth_mode)
		{
		case K4A_DEPTH_MODE_NFOV_2X2BINNED:
			return resolution = ofVec2f(320, 288);
		case K4A_DEPTH_MODE_NFOV_UNBINNED:
			return resolution = ofVec2f(640, 576);
		case K4A_DEPTH_MODE_WFOV_2X2BINNED:
			return resolution = ofVec2f(512, 512);
		case K4A_DEPTH_MODE_WFOV_UNBINNED:
			return resolution = ofVec2f(1024, 1024);
		case K4A_DEPTH_MODE_PASSIVE_IR:
			return resolution = ofVec2f(1024, 1024);

		default:
			ofLogError("ofxKinect4Azure") << "Invalid depth dimensions value!";
			return resolution = ofVec2f();
		}
	}

	static inline ofColor ColorizeBlueToRed(const unsigned short &depthPixel,
		float &min,
		float &max)
	{
		constexpr unsigned char PixelMax = std::numeric_limits<unsigned char>::max();

		// Default to opaque black.
		//
		ofColor result = ofColor(0, 0, 0, PixelMax);

		// If the pixel is actual zero and not just below the min value, make it black
		//
		if (depthPixel == 0)
		{
			return result;
		}

		unsigned short clampedValue = depthPixel;
		clampedValue = std::min(clampedValue, (unsigned short)max);
		clampedValue = std::max(clampedValue, (unsigned short)min);

		// Normalize to [0, 1]
		//
		float hue = (clampedValue - min) / (max - min);

		// The 'hue' coordinate in HSV is a polar coordinate, so it 'wraps'.
		// Purple starts after blue and is close enough to red to be a bit unclear,
		// so we want to go from blue to red.  Purple starts around .6666667,
		// so we want to normalize to [0, .6666667].
		//
		constexpr float range = 2.f / 3.f;
		hue *= range;

		// We want blue to be close and red to be far, so we need to reflect the
		// hue across the middle of the range.
		//
		hue = range - hue;

		result.setHsb(hue * PixelMax, PixelMax, PixelMax, PixelMax);
		return result;
	}

	// Computes a greyscale representation of a depth pixel.
	//
	static inline ofColor ColorizeGreyscale(const unsigned short &value, const unsigned short &min, const unsigned short &max)
	{
		// Clamp to max
		//
		unsigned short pixelValue = std::min(value, max);

		constexpr unsigned char PixelMax = std::numeric_limits<unsigned char>::max();
		const auto normalizedValue = static_cast<unsigned char>((pixelValue - min) * (double(PixelMax) / (max - min)));

		// All color channels are set the same (image is greyscale)
		//
		return ofColor(normalizedValue, normalizedValue, normalizedValue, PixelMax);
	}

	static string get_serial(k4a_device_t device)
	{
		size_t serial_number_length = 0;

		if (K4A_BUFFER_RESULT_TOO_SMALL != k4a_device_get_serialnum(device, NULL, &serial_number_length))
		{
			ofLogError("ofxKinect4Azure") << "Failed to get serial number length" << endl;
			k4a_device_close(device);
			exit(-1);
		}

		char *serial_number = new (std::nothrow) char[serial_number_length];
		if (serial_number == NULL)
		{
			ofLogError("ofxKinect4Azure") << "Failed to allocate memory for serial number (" << serial_number_length << " bytes)" << endl;
			k4a_device_close(device);
			exit(-1);
		}

		if (K4A_BUFFER_RESULT_SUCCEEDED != k4a_device_get_serialnum(device, serial_number, &serial_number_length))
		{
			ofLogError("ofxKinect4Azure") << "Failed to get serial number" << endl;
			delete[] serial_number;
			serial_number = NULL;
			k4a_device_close(device);
			exit(-1);
		}

		string s(serial_number);
		delete[] serial_number;
		serial_number = NULL;
		return s;
	}
};

class ofxKinect4AzureManager {
	vector<string> serial_list;
	std::map<string, int> serial_to_id;
	int device_count = 0;
public:
	ofxKinect4AzureManager() {
		serial_list.clear();
		serial_to_id.clear();
		device_count = k4a_device_get_installed_count();
		for (int i = 0; i < device_count; i++) {
			k4a_device_t d;
			k4a_device_open(i, &d);
			std::string serialnum;
			size_t buffer = 0;
			k4a_buffer_result_t result = k4a_device_get_serialnum(d, &serialnum[0], &buffer);
			if (result == K4A_BUFFER_RESULT_TOO_SMALL && buffer > 1)
			{
				serialnum.resize(buffer);
				result = k4a_device_get_serialnum(d, &serialnum[0], &buffer);
				if (result == K4A_BUFFER_RESULT_SUCCEEDED && serialnum[buffer - 1] == 0)
				{
					serialnum.resize(buffer - 1);
				}
			}
			if (result != K4A_BUFFER_RESULT_SUCCEEDED)
			{
				ofLogError("ofxKinect4Azure") << "Failed to read device serial number!";
			}
			else {
				serial_to_id.emplace(serialnum, i);
				serial_list.push_back(serialnum);
				ofLogNotice("ofxKinect4Azure") << "[device " << i << "] : " << serialnum;
			}
			k4a_device_close(d);
		}
	}

	~ofxKinect4AzureManager() {}

	vector<string> getDeviceSerialList() {
		return serial_list;
	}

	void setupWithSerial(string _serial_num, ofxKinect4Azure* _k4a, ofxKinect4AzureSettings _settings = ofxKinect4AzureSettings()) {
		if (serial_to_id.find(_serial_num) != serial_to_id.end()) {
			_k4a->setup(serial_to_id[_serial_num], _settings);
		}
		else {
			ofLogError("ofxKinect4Azure") << "NO device found matching serial number";
		}
	}

	int getDeviceNum() {
		return device_count;
	}
};