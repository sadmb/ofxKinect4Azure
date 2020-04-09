#pragma once

#include "ofMain.h"
#include "k4a.hpp"
#include "k4abt.hpp"
#include "utils/ofxKinect4AzureSettings.h"

class ofxKinect4Azure {

protected:
	ofxKinect4AzureSettings settings;
	ofxKinect4AzureBodySettings body_settings;

	// k4a interfaces
	k4a::device device;
	k4a::calibration calibration;

	// k4abt interfaces
	k4abt::tracker tracker;

	glm::vec2 color_size, depth_size;

	// pixels
	ofPixels pix;
	ofShortPixels depth_pix;
	ofPixels colorized_depth_pix;
	ofShortPixels ir_pix;

	// textures
	ofTexture color;
	ofTexture depth;
	ofTexture colorized_depth;
	ofTexture ir;

	// for pointcloud
	ofVbo pointcloud_vbo;
	int NUM_VERTICES = 0;
	vector<glm::vec3> pointcloud_vert;
	vector<glm::vec2> pointcloud_uv;

	// for tracker
	vector<ofxKinect4AzureBody> bodies;
	bool b_tracker_processing = false;
	bool b_tracker_new = false;

	// flags for k4a
	bool b_color_new = false;
	bool b_depth_new = false;
	bool b_colorized_depth_new = false;
	bool b_ir_new = false;
	bool b_pointcloud_new = false;
	bool b_tex_new = false;
	bool b_depth_tex_new = false;
	bool b_colorized_depth_tex_new = false;
	bool b_ir_tex_new = false;

	string serial_num = "";
	int device_index = -1;
	int device_count = 0;

	//for imu
	std::thread imu_recorder;
	mutable std::mutex mutex;
	vector<ofxKinect4AzureImuSample> imu_sample_queue[2];
	std::atomic<bool> buffer_switch = false;
	int imu_record_desire_step_usec = 0;
	int imu_record_max_queue_size = 100;
	std::atomic<bool> b_imu_recording = false;
	std::atomic<bool> b_imu_poped = false;

public:
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

	virtual ~ofxKinect4Azure() {
		device.stop_cameras();
		if (settings.enable_imu) {
			device.stop_imu();
		}
		device.close();
	};

	void setup();
	void setup(ofxKinect4AzureSettings settings);
	void setup(int index);
	void setup(int index, ofxKinect4AzureSettings settings);

	void setupPointcloud();

	void saveCalibration(string filename = "calibration.json");
	void update();
	void updatePointcloud();
	void draw(float x = 0, float y = 0, float w = 0, float h = 0);
	void drawDepth(float x = 0, float y = 0, float w = 0, float h = 0);
	void drawColorizedDepth(float x = 0, float y = 0, float w = 0, float h = 0);
	void drawIR(float x = 0, float y = 0, float w = 0, float h = 0);

	float getWidth() { return color_size.x; }
	float getHeight() { return color_size.y; }
	float getDepthWidth() { return depth_size.x; }
	float getDepthHeight() { return depth_size.y; }

	ofPixels& getPixels() { return pix; }
	ofShortPixels& getDepthPixels() { return depth_pix; }
	ofVbo& getPointcloudVbo() { return pointcloud_vbo; }
	vector<ofxKinect4AzureBody>& getBodies() { return bodies; }

	ofTexture& getTexture() {
		if (b_color_new) {
			if (!b_tex_new) {
				color.allocate(pix);
				b_tex_new = true;
			}
		}
		return color;
	}
	ofTexture& getDepthTexture() {
		if (b_depth_new) {
			if (!b_depth_tex_new) {
				depth.allocate(depth_pix);
				b_depth_tex_new = true;
			}
		}
		return depth;
	}
	ofTexture& getColorizedDepthTexture() {
		if (b_colorized_depth_new) {
			if (!b_colorized_depth_tex_new) {
				colorized_depth.allocate(colorized_depth_pix);
				b_colorized_depth_tex_new = true;
			}
		}
		return colorized_depth;
	}
	ofTexture& getIRTexture() {
		if (b_ir_new) {
			if (!b_ir_tex_new) {
				ir.allocate(ir_pix);
				b_ir_tex_new = true;
			}
		}
		return ir;
	}

	vector<ofxKinect4AzureImuSample>& popImuSampleQueue() {
		std::unique_lock<std::mutex> lck(mutex);
		b_imu_poped = true;
		return imu_sample_queue[buffer_switch];
	}

	bool isFrameNew() { return b_color_new; }
	bool isColorNew() { return b_color_new; }
	bool isDepthNew() { return b_depth_new; }
	bool isColorizedDepthNew() { return b_colorized_depth_new; }
	bool isIRNew() { return b_ir_new; }
	bool isPointcloudNew() { return b_pointcloud_new; }
	bool isTrackerNew() { return b_tracker_new; }

	//set transform mode COLOR_TO_DEPTH or DEPTH_TO_COLOR or NONE
	void setTransformType(OFX_K4A_TRANSFORM_TYPE _transform_type) { settings.transform_type = _transform_type; }
	//set IMU able state 
	void enableIMU() {
		device.start_imu();
		settings.enable_imu = true;
		startImuRecording();
	}
	void disableIMU() {
		if (settings.enable_imu)
		{
			device.stop_imu();
			settings.enable_imu = false;
			b_imu_recording = false;
		}
	}

	k4a::device& getDevice()
	{
		return device;
	}

	ofxKinect4AzureSettings& getSettings()
	{
		return settings;
	}

	ofxKinect4AzureBodySettings& getBodySettings()
	{
		return body_settings;
	}

	k4a::calibration& getCalibration()
	{
		return calibration;
	}

	k4abt::tracker& getTracker()
	{
		return tracker;
	}

	glm::vec2 getDepthModeRange()
	{
		return getDepthModeRange(settings.depth_mode);
	}

	glm::vec2 getDepthModeRange(const k4a_depth_mode_t depth_mode)
	{
		glm::vec2 range;
		switch (depth_mode)
		{
		case K4A_DEPTH_MODE_NFOV_2X2BINNED:
			return range = glm::vec2(500, 5800);
		case K4A_DEPTH_MODE_NFOV_UNBINNED:
			return range = glm::vec2(500, 4000);
		case K4A_DEPTH_MODE_WFOV_2X2BINNED:
			return range = glm::vec2(250, 3000);
		case K4A_DEPTH_MODE_WFOV_UNBINNED:
			return range = glm::vec2(250, 2500);

		case K4A_DEPTH_MODE_PASSIVE_IR:
		default:
			ofLogError("ofxKinect4Azure") << "Invalid depth mode!";
			return range = glm::vec2();
		}
	}

	glm::vec2 getColorResolution()
	{
		return getColorResolution(settings.color_resolution);
	}

	glm::vec2 getColorResolution(const k4a_color_resolution_t color_resolution)
	{
		glm::vec2 resolution;
		switch (color_resolution)
		{
		case K4A_COLOR_RESOLUTION_720P:
			return resolution = glm::vec2(1280, 720);
		case K4A_COLOR_RESOLUTION_2160P:
			return resolution = glm::vec2(3840, 2160);
		case K4A_COLOR_RESOLUTION_1440P:
			return resolution = glm::vec2(2560, 1440);
		case K4A_COLOR_RESOLUTION_1080P:
			return resolution = glm::vec2(1920, 1080);
		case K4A_COLOR_RESOLUTION_3072P:
			return resolution = glm::vec2(4096, 3072);
		case K4A_COLOR_RESOLUTION_1536P:
			return resolution = glm::vec2(2048, 1536);

		default:
			ofLogError("ofxKinect4Azure") << "Invalid color dimensions value!";
			return resolution = glm::vec2();
		}
	}

	glm::vec2 getDepthResolution() {
		return getDepthResolution(settings.depth_mode);
	}

	glm::vec2 getDepthResolution(const k4a_depth_mode_t depth_mode)
	{
		glm::vec2 resolution;
		switch (depth_mode)
		{
		case K4A_DEPTH_MODE_NFOV_2X2BINNED:
			return resolution = glm::vec2(320, 288);
		case K4A_DEPTH_MODE_NFOV_UNBINNED:
			return resolution = glm::vec2(640, 576);
		case K4A_DEPTH_MODE_WFOV_2X2BINNED:
			return resolution = glm::vec2(512, 512);
		case K4A_DEPTH_MODE_WFOV_UNBINNED:
			return resolution = glm::vec2(1024, 1024);
		case K4A_DEPTH_MODE_PASSIVE_IR:
			return resolution = glm::vec2(1024, 1024);

		default:
			ofLogError("ofxKinect4Azure") << "Invalid depth dimensions value!";
			return resolution = glm::vec2();
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

	string getSerialnum()
	{
		return device.get_serialnum();
	}

	static string getSerialnum(k4a::device device)
	{
		return device.get_serialnum();
	}

protected:
	void updateIMU()
	{
		while (settings.enable_imu)
		{
			auto start = ofGetElapsedTimeMicros();
			if (b_imu_poped)
			{
				buffer_switch = !buffer_switch;
				imu_sample_queue[buffer_switch].clear();
				b_imu_poped = false;
			}
			k4a_imu_sample_t imu;
			if (!device.get_imu_sample(&imu))
			{
				continue;
			}
			ofxKinect4AzureImuSample imu_s;
			imu_s.temperature = imu.temperature;
			auto& acc = imu.acc_sample.xyz;
			imu_s.acc.first = imu.acc_timestamp_usec;
			imu_s.acc.second.x = acc.x;
			imu_s.acc.second.y = acc.y;
			imu_s.acc.second.z = acc.z;
			auto& gyro = imu.gyro_sample.xyz;
			imu_s.gyro.first = imu.gyro_timestamp_usec;
			imu_s.gyro.second.x = gyro.x;
			imu_s.gyro.second.y = gyro.y;
			imu_s.gyro.second.z = gyro.z;
			mutex.lock();
			imu_sample_queue[buffer_switch].push_back(imu_s);
			if (imu_sample_queue[buffer_switch].size() > imu_record_max_queue_size) {
				imu_sample_queue[buffer_switch].erase(imu_sample_queue[buffer_switch].begin());
			}
			mutex.unlock();

			auto end = ofGetElapsedTimeMicros();
			int sleep_usec = imu_record_desire_step_usec - (end - start);
			if (sleep_usec > 0) {
				this_thread::sleep_for(std::chrono::microseconds(sleep_usec));
			}
		}
	}

	void startImuRecording()
	{
		//start imu sampling
		if (!b_imu_recording) {
			imu_recorder = std::thread(std::bind(&ofxKinect4Azure::updateIMU, this));
			imu_recorder.detach();
		}
		b_imu_recording = true;
	}
};

class ofxKinect4AzureManager {
	vector<string> serial_list;
	std::map<string, int> serial_to_id;
	int device_count = 0;
public:
	ofxKinect4AzureManager() {
		refresh();
	}

	~ofxKinect4AzureManager() {}

	vector<string> getDeviceSerialList() {
		return serial_list;
	}

	void refresh()
	{
		serial_list.clear();
		serial_to_id.clear();

		device_count = k4a::device::get_installed_count();
		for (int i = 0; i < device_count; i++) {
			k4a::device d = k4a::device::open(i);
			std::string serialnum = d.get_serialnum();
			serial_to_id.emplace(serialnum, i);
			serial_list.push_back(serialnum);
			ofLogNotice("ofxKinect4Azure") << "[device " << i << "] : " << serialnum;
			d.close();
		}
	}

	void setupWithSerial(string _serial_num, ofxKinect4Azure* k4a, ofxKinect4AzureSettings _settings = ofxKinect4AzureSettings()) {
		if (serial_to_id.find(_serial_num) != serial_to_id.end()) {
			k4a->setup(serial_to_id[_serial_num], _settings);
		}
		else {
			ofLogError("ofxKinect4Azure") << "NO device found matching serial number: " << _serial_num;
		}
	}

	int getDeviceNum() {
		return device_count;
	}
};