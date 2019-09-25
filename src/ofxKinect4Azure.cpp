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
		transformation = k4a_transformation_create(&calibration);
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
		is_frame_new = is_depth_frame_new = false;
		k4a_capture_t capture = nullptr;
		if (k4a_device_get_capture(device, &capture, 0) != K4A_WAIT_RESULT_SUCCEEDED)
		{
			ofLogError("ofxKinct4Azure") << "capture failed.";
			return;
		}

		k4a_image_t color_image, depth_image;
		k4a_image_t	*c_ptr = nullptr, *d_ptr = nullptr;

		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			depth_image = k4a_capture_get_depth_image(capture);
			if (transform_type==DEPTH_TO_COLOR) {
				k4a_image_t transformed_depth_image;
				k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, color_size.first, color_size.second, color_size.first * sizeof(unsigned short), &transformed_depth_image);
				k4a_transformation_depth_image_to_color_camera(transformation, depth_image, transformed_depth_image);
				d_ptr = &transformed_depth_image;
				k4a_image_release(depth_image);
			}
			else {
				d_ptr = &depth_image;
			}
			const int depth_width = k4a_image_get_width_pixels(*d_ptr);
			const int depth_height = k4a_image_get_height_pixels(*d_ptr);
			depth_size = pair<int, int>(depth_width, depth_height);
			is_depth_frame_new = true;
			b_depth_tex_new = false;
			ofVec2f range = getDepthModeRange(settings.depth_mode);
			uint8_t* d_buffer = k4a_image_get_buffer(*d_ptr);
			depth_pix.setFromPixels(reinterpret_cast<const unsigned short*>(d_buffer), depth_size.first, depth_size.second, OF_PIXELS_GRAY);
			colorized_depth_pix.allocate(depth_size.first, depth_size.second, OF_PIXELS_BGRA);
			auto d_data = depth_pix.getData();
			auto colorized_depth_pix_data = colorized_depth_pix.getData();
			for (int h = 0; h < depth_size.second; ++h)
			{
				for (int w = 0; w < depth_size.first; ++w)
				{
					const size_t current_pixel = static_cast<size_t>(h * depth_size.first + w);
					
					ofColor colorized = ColorizeBlueToRed(d_data[current_pixel], range.x, range.y);
					colorized_depth_pix_data[current_pixel * 4] = colorized.g;
					colorized_depth_pix_data[current_pixel * 4 + 1] = colorized.b;
					colorized_depth_pix_data[current_pixel * 4 + 2] = colorized.r;
					colorized_depth_pix_data[current_pixel * 4 + 3] = colorized.a;
				}
			}
		}

		if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
		{
			color_image = k4a_capture_get_color_image(capture);
			if (transform_type==COLOR_TO_DEPTH){
				k4a_image_t transformed_color_image;
				k4a_image_create(settings.color_format, depth_size.first, depth_size.second, depth_size.first * sizeof(unsigned char)*4, &transformed_color_image);
				k4a_transformation_color_image_to_depth_camera(transformation, depth_image, color_image,transformed_color_image);
				c_ptr = &transformed_color_image;
				k4a_image_release(color_image);
			}
			else {
				c_ptr = &color_image;
			}
			const int color_width = k4a_image_get_width_pixels(*c_ptr);
			const int color_height = k4a_image_get_height_pixels(*c_ptr);
			color_size = pair<int, int>(color_width, color_height);
			is_frame_new = true;
			b_tex_new = false;
			uint8_t* c_buffer = k4a_image_get_buffer(*c_ptr);
			pix.setFromPixels((const unsigned char*)(c_buffer), color_size.first, color_size.second, OF_PIXELS_BGRA);
		}

		if(c_ptr!=nullptr)k4a_image_release(*c_ptr);
		if(d_ptr!=nullptr)k4a_image_release(*d_ptr);
		
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
			if (!b_tex_new) {
				color.allocate(pix);
				b_tex_new = true;
			}
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

void ofxKinect4Azure::drawColorizedDepth(float x, float y, float w, float h)
{
	if (device_index >= 0)
	{
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			if (!b_depth_tex_new) {
				colorized_depth.allocate(colorized_depth_pix);
				b_depth_tex_new = true;
			}
			if (colorized_depth.isAllocated())
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
				colorized_depth.draw(x, y, w, h);
				ofPopStyle();
			}
		}
	}
}

