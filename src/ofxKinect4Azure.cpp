#include "ofxKinect4Azure.h"

//--------------------------------------------------------------
void ofxKinect4Azure::setup()
{
	setup(0);
}

//--------------------------------------------------------------
void ofxKinect4Azure::setup(ofxKinect4AzureSettings settings)
{
	setup(0, settings);
}

void ofxKinect4Azure::setup(int index)
{
	device_count = k4a::device::get_installed_count();
	if (device_count == 0)
	{
		ofLogError("ofxKinect4Azure") << "No Azure Kinect devices detected.";
	}
	settings = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	settings.camera_fps = K4A_FRAMES_PER_SECOND_30;
	settings.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	settings.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	settings.color_resolution = K4A_COLOR_RESOLUTION_720P;

	settings.synchronized_images_only = true;

	setup(index, settings);
}


//--------------------------------------------------------------
void ofxKinect4Azure::setup(int index, ofxKinect4AzureSettings settings)
{
	device_count = k4a::device::get_installed_count();
	if (device_count == 0)
	{
		ofLogError("ofxKinect4Azure") << "No Azure Kinect devices detected.";
	}
	if (device_count > index)
	{
		ofLogNotice("ofxKinect4Azure") << "Started opening Kinect4Azure device...";

		if (k4a_device_open(index, &device) != K4A_WAIT_RESULT_SUCCEEDED)
		{
			ofLogError("ofxKinect4Azure") << "open Azure Kinect device failed!";
			return;
		}
		if (k4a_device_start_cameras(device, &settings) != K4A_WAIT_RESULT_SUCCEEDED)
		{
			ofLogError("ofxKinect4Azure") << "can't start Kinect Azure camera.";
			return;
		}
		k4a_device_get_calibration(device, settings.depth_mode, settings.color_resolution, &calibration);
		device_index = index;
		ofLogNotice("ofxKinect4Azure") << "Finished opening Kinect4Azure device.";

		//if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		//{
		//	ofVec2f depth_resolution = getDepthResolution(settings.depth_mode);
		//	depth.allocate((int)depth_resolution.x, (int)depth_resolution.y, GL_LUMINANCE);
		//}
		//if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
		//{
		//	ofVec2f color_resolution = getColorResolution(settings.color_resolution);
		//	color.allocate((int)color_resolution.x, (int)color_resolution.y, GL_RGB);
		//}
	}
	else
	{
		ofLogError("ofxKinect4Azure") << ofToString(index) << " is not the right index for Kinect4Azure device.";
	}


}

void ofxKinect4Azure::saveCalibration(string filename)
{
	size_t size;
	uint8_t* calibration_buffer;
	k4a_device_get_raw_calibration(device, calibration_buffer, &size);
	ofBuffer buffer;
	buffer.append(reinterpret_cast<const char *>(&calibration_buffer[0]), (long)size);
	ofBufferToFile(filename, buffer);
	cout << "Calibration blob for device " << (int)device_index << " (serial no. " << get_serial(device)
		<< ") is saved to " << filename << endl;
}

//--------------------------------------------------------------
void ofxKinect4Azure::update(){
	if (device_index >= 0)
	{
		k4a_capture_t capture = nullptr;
		if (k4a_device_get_capture(device, &capture, 0) != K4A_WAIT_RESULT_SUCCEEDED)
		{
			ofLogError("ofxKinct4Azure") << "capture failed.";
			return;
		}
		if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
		{
			k4a_image_t color_image = k4a_capture_get_color_image(capture);
			const int color_width = k4a_image_get_width_pixels(color_image);
			const int color_height = k4a_image_get_height_pixels(color_image);
			color_size = pair<int, int>(color_width, color_height);
			is_frame_new = true;
			uint8_t* c_buffer = k4a_image_get_buffer(color_image);
			pix.setFromPixels((const unsigned char*)(c_buffer), color_size.first, color_size.second, OF_PIXELS_BGRA);
			color.allocate(pix);
			k4a_image_release(color_image);
		}
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			k4a_image_t depth_image = k4a_capture_get_depth_image(capture);
			const int depth_width = k4a_image_get_width_pixels(depth_image);
			const int depth_height = k4a_image_get_height_pixels(depth_image);
			depth_size = pair<int, int>(depth_width, depth_height);
			is_depth_frame_new = true;
			ofVec2f range = getDepthModeRange(settings.depth_mode);
			depth_pix.allocate(depth_size.first, depth_size.second, OF_PIXELS_BGRA);
			for (int h = 0; h < depth_size.second; ++h)
			{
				for (int w = 0; w < depth_size.first; ++w)
				{
					const size_t current_pixel = static_cast<size_t>(h * depth_size.first + w);
					uint8_t* d_buffer = k4a_image_get_buffer(depth_image);
					ofColor colorized = ColorizeBlueToRed(reinterpret_cast<const unsigned short*>(d_buffer)[current_pixel], range.x, range.y);
					depth_pix[current_pixel * 4] = colorized.g;
					depth_pix[current_pixel * 4 + 1] = colorized.b;
					depth_pix[current_pixel * 4 + 2] = colorized.r;
					depth_pix[current_pixel * 4 + 3] = colorized.a;
				}
			}
			depth.allocate(depth_pix);
			k4a_image_release(depth_image);
		}
		k4a_capture_release(capture);
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::draw(float x, float y, float w, float h)
{
	if (device_index >= 0)
	{
		if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
		{
			ofVec2f resolution = getColorResolution(settings.color_resolution);
			if (color.isAllocated())
			{
				if (w == 0)
				{
					w = resolution.x;
				}
				if (h == 0)
				{
					h = resolution.y;
				}
				ofPushStyle();
				ofSetColor(255);
				color.draw(x, y, w, h);
				ofPopStyle();
			}
		}
	}
}

void ofxKinect4Azure::drawDepth(float x, float y, float w, float h)
{
	if (device_index >= 0)
	{
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			if (depth.isAllocated())
			{
				if (w == 0)
				{
					w = depth_size.first;
				}
				if (h == 0)
				{
					h = depth_size.second;
				}
				ofPushStyle();
				ofSetColor(255);
				depth.draw(x, y, w, h);
				ofPopStyle();
			}
		}
	}
}

