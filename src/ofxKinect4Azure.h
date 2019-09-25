#pragma once

#include "ofMain.h"
#include "k4a.hpp"
#include "ofxKinect4AzureSettings.h"

class ofxKinect4Azure {

public:
	ofxKinect4AzureSettings settings;
	k4a_device_t device;
	k4a_calibration_t calibration;

	pair<int, int> color_size, depth_size;

	ofPixels pix;
	ofPixels depth_pix;

	ofTexture color;
	ofTexture depth;
	
	bool is_frame_new = false;
	bool is_depth_frame_new = false;

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
	~ofxKinect4Azure() {};

	void setup();
	void setup(ofxKinect4AzureSettings settings);
	void setup(int index);
	void setup(int index, ofxKinect4AzureSettings settings);
	void saveCalibration(string filename = "calibration.json");
	void update();
	void draw(float x = 0, float y = 0, float w = 0, float h = 0);
	void drawDepth(float x = 0, float y = 0, float w = 0, float h = 0);

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
		ofColor result = ofColor( 0, 0, 0, PixelMax );

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
