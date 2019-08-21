#include "ofxKinect4Azure.h"

//--------------------------------------------------------------
void ofxKinect4Azure::setup()
{
	settings.camera_fps = K4A_FRAMES_PER_SECOND_30;
	settings.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	settings.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	settings.color_resolution = K4A_COLOR_RESOLUTION_720P;

	settings.synchronized_images_only = true;

	setup(settings);

}

//--------------------------------------------------------------
void ofxKinect4Azure::setup(ofxKinect4AzureSettings settings)
{
	if (device_count > 0)
	{
		ofLogNotice("ofxKinect4Azure") << "Started opening Kinect4Azure device...";

		device = k4a::device::open(K4A_DEVICE_DEFAULT);
		device.start_cameras(&settings);

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


}

//--------------------------------------------------------------
void ofxKinect4Azure::update(){
	if (device_count > 0)
	{
		k4a::capture capture;
		if (device.get_capture(&capture, std::chrono::milliseconds(0)))
		{
			const k4a::image depth_image = capture.get_depth_image();
			const k4a::image color_image = capture.get_color_image();

			const int depth_width = depth_image.get_width_pixels();
			const int depth_height = depth_image.get_height_pixels();

			const int color_width = color_image.get_width_pixels();
			const int color_height = color_image.get_height_pixels();

			ofPixels depth_pix;
			const uint16_t* depth_data = reinterpret_cast<const uint16_t*>(depth_image.get_buffer());
			ofVec2f range = getDepthModeRange(settings.depth_mode);
			depth_pix.allocate(depth_width, depth_height, OF_PIXELS_BGRA);

			for (int h = 0; h < depth_height; ++h)
			{
				for (int w = 0; w < depth_width; ++w)
				{
					const size_t current_pixel = static_cast<size_t>(h * depth_width + w);
					depth_pix[current_pixel * 4] = ColorizeBlueToRed(depth_data[current_pixel], (unsigned short)range.x, (unsigned short)range.y).g;
					depth_pix[current_pixel * 4 + 1] = ColorizeBlueToRed(depth_data[current_pixel], (unsigned short)range.x, (unsigned short)range.y).b;
					depth_pix[current_pixel * 4 + 2] = ColorizeBlueToRed(depth_data[current_pixel], (unsigned short)range.x, (unsigned short)range.y).r;
					depth_pix[current_pixel * 4 + 3] = ColorizeBlueToRed(depth_data[current_pixel], (unsigned short)range.x, (unsigned short)range.y).a;
				}
			}
			depth.allocate(depth_pix);

			ofPixels color_pix;
			unsigned char* color_data = (unsigned char*)(color_image.get_buffer());
			color_pix.setFromExternalPixels(color_data, color_width, color_height, OF_PIXELS_BGRA);
			color.allocate(color_pix);
		}
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::draw(){
	if (device_count > 0)
	{
		ofPushStyle();
		ofSetColor(255);
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			if (depth.isAllocated())
			{
				depth.draw(0, 0, depth.getWidth(), depth.getHeight());
			}
		}
		if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
		{
			if (color.isAllocated())
			{
				color.draw(0, depth.getHeight() + 10, color.getWidth() * 0.5, color.getHeight() * 0.5);
			}
		}
		ofPopStyle();
	}
}

