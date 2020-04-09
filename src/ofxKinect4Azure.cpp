#include "ofxKinect4Azure.h"

//--------------------------------------------------------------
void ofxKinect4Azure::setup()
{
	setup(0);
}

//--------------------------------------------------------------
void ofxKinect4Azure::setup(ofxKinect4AzureSettings _settings)
{
	settings = _settings;
	setup(0, settings);
}

//--------------------------------------------------------------
void ofxKinect4Azure::setup(int index)
{
	device_count = k4a::device::get_installed_count();
	if (device_count == 0)
	{
		ofLogError("ofxKinect4Azure") << "No Azure Kinect devices detected.";
	}

	setup(index, settings);
}


//--------------------------------------------------------------
void ofxKinect4Azure::setup(int index, ofxKinect4AzureSettings _settings)
{
	settings = _settings;
	device_count = k4a::device::get_installed_count();
	if (device_count == 0)
	{
		ofLogError("ofxKinect4Azure") << "No Azure Kinect devices detected.";
		return;
	}
	else {
		ofLogNotice("ofxKinect4Azure") << ofToString(device_count) + " Azure Kinect devices detected.";
	}
	if (index >= 0 && device_count > index)
	{
		ofLogNotice("ofxKinect4Azure") << "Started opening Kinect4Azure device...";
		device = k4a::device::open(index);
		device.start_cameras(&settings);
		if (settings.enable_imu)
		{
			device.start_imu();
		}
		calibration = device.get_calibration(settings.depth_mode, settings.color_resolution);
		if (settings.use_tracker)
		{
			tracker = k4abt::tracker::create(calibration, body_settings);
		}
		switch (settings.transform_type) {
		case DEPTH_TO_COLOR:
			color_size = getColorResolution();
			depth_size = getColorResolution();
			break;
		case COLOR_TO_DEPTH:
			color_size = getDepthResolution();
			depth_size = getDepthResolution();
			break;
		case NONE:
			color_size = getColorResolution();
			depth_size = getDepthResolution();
			break;
		}

		if (settings.make_pointcloud)
		{
			setupPointcloud();
		}

		if (settings.enable_imu) {
			startImuRecording();
		}

		device_index = index;
		ofLogNotice("ofxKinect4Azure") << "Finished opening Kinect4Azure device.";
	}
	else
	{
		ofLogError("ofxKinect4Azure") << ofToString(index) << " is not the right index for Kinect4Azure device.";
	}


}

//--------------------------------------------------------------
void ofxKinect4Azure::setupPointcloud()
{
	settings.make_pointcloud = true;
	NUM_VERTICES = depth_size.x * depth_size.y;
	pointcloud_vert.resize(NUM_VERTICES);
	pointcloud_uv.resize(NUM_VERTICES);
	int w = depth_size.x;
	for (int i = 0; i < NUM_VERTICES; i++) {
		pointcloud_uv[i] = glm::vec2(i % w, i / w);
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::saveCalibration(string filename)
{
	vector<uint8_t> calibration_buffer = device.get_raw_calibration();
	ofBuffer buffer;
	buffer.append(reinterpret_cast<const char *>(&calibration_buffer[0]), calibration_buffer.size());
	ofBufferToFile(filename, buffer);
	cout << "Calibration blob for device " << (int)device_index << " (serial no. " << getSerialnum()
		<< ") is saved to " << filename << endl;
}

//--------------------------------------------------------------
void ofxKinect4Azure::update(){
	b_color_new = false;
	b_depth_new = false;
	b_colorized_depth_new = false;
	b_ir_new = false;
	if (device_index >= 0)
	{
		k4a::capture capture = nullptr;
		if (!device.get_capture(&capture))
		{
//			ofLogError("ofxKinct4Azure") << "capture failed.";
			return;
		}
		k4a::image color_image;
		k4a::image depth_image;
		k4a::image ir_image;
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			depth_image = capture.get_depth_image();
			depth_size = glm::vec2(depth_image.get_width_pixels(), depth_image.get_height_pixels());
			if (settings.transform_type == DEPTH_TO_COLOR)
			{
				k4a::image transformed_depth_image;
				transformed_depth_image = k4a::image::create(K4A_IMAGE_FORMAT_DEPTH16, depth_size.x, depth_size.y, depth_size.x * sizeof(unsigned short));
				transformed_depth_image = transformation.depth_image_to_color_camera(depth_image);
				depth_image = transformed_depth_image;
			}

			glm::vec2 range = getDepthModeRange();
			uint8_t* d_buffer = depth_image.get_buffer();

			//copy to depth pix
			depth_pix.setFromPixels(reinterpret_cast<const unsigned short*>(d_buffer), depth_size.x, depth_size.y, OF_PIXELS_GRAY);
			b_depth_new = true;
			b_depth_tex_new = false;

			if (settings.make_colorize_depth) {
				colorized_depth_pix.allocate(depth_size.x, depth_size.y, OF_PIXELS_BGRA);
				auto d_data = depth_pix.getData();
				auto colorized_depth_pix_data = colorized_depth_pix.getData();
				for (int h = 0; h < depth_size.y; ++h)
				{
					for (int w = 0; w < depth_size.x; ++w)
					{
						const size_t current_pixel = static_cast<size_t>(h * depth_size.x + w);
						ofColor colorized = ColorizeBlueToRed(d_data[current_pixel], range.x, range.y);
						colorized_depth_pix_data[current_pixel * 4] = colorized.g;
						colorized_depth_pix_data[current_pixel * 4 + 1] = colorized.b;
						colorized_depth_pix_data[current_pixel * 4 + 2] = colorized.r;
						colorized_depth_pix_data[current_pixel * 4 + 3] = colorized.a;
					}
				}
				b_colorized_depth_new = true;
				b_colorized_depth_tex_new = false;
			}

			if (settings.make_pointcloud) {
				if (NUM_VERTICES != depth_size.x * depth_size.y)
				{
					setupPointcloud();
				}
				k4a::image pointcloud_image = k4a::image::create(K4A_IMAGE_FORMAT_DEPTH16, depth_size.x, depth_size.y, depth_size.x * sizeof(short) * 3);
				if (settings.transform_type == DEPTH_TO_COLOR) {
					pointcloud_image = transformation.depth_image_to_point_cloud(depth_image, K4A_CALIBRATION_TYPE_COLOR);
				}
				else {
					pointcloud_image = transformation.depth_image_to_point_cloud(depth_image, K4A_CALIBRATION_TYPE_DEPTH);
				}
				uint8_t* p_buffer = pointcloud_image.get_buffer();
				auto p_data = pointcloud_vert.data();

				for (int i = 0; i < NUM_VERTICES; i++) {
					p_data[i].x = p_buffer[i * 3];
					p_data[i].y = p_buffer[i * 3 + 1];
					p_data[i].z = p_buffer[i * 3 + 2];
				}
				b_pointcloud_new = true;

				pointcloud_image.reset();
			}

			if (settings.use_ir_image) {
				ir_image = capture.get_ir_image();
				if (settings.transform_type == DEPTH_TO_COLOR) {
					k4a::image transformed_ir_image;
					transformed_ir_image = k4a::image::create(K4A_IMAGE_FORMAT_IR16, depth_size.x, depth_size.y, depth_size.x * sizeof(unsigned short));
					transformed_ir_image = transformation.depth_image_to_color_camera(ir_image);
					ir_image = transformed_ir_image;
				}
				uint8_t* ir_buffer = ir_image.get_buffer();
				ir_pix.setFromPixels(reinterpret_cast<const unsigned short*>(ir_buffer), depth_size.x, depth_size.y, OF_PIXELS_GRAY);
				b_ir_new = true;
				b_ir_tex_new = false;
			}
		}

		if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
		{
			color_image = capture.get_color_image();
			color_size = glm::vec2(color_image.get_width_pixels(), color_image.get_height_pixels());
			if (settings.transform_type == COLOR_TO_DEPTH) {
				if (depth_image.is_valid())
				{
					k4a::image transformed_color_image;
					transformed_color_image = k4a::image::create(settings.color_format, depth_size.x, depth_size.y, depth_size.x * sizeof(unsigned char) * 4);
					transformed_color_image = transformation.color_image_to_depth_camera(depth_image, color_image);
					color_image = transformed_color_image;
				}
				else
				{
					ofLogError("ofxKinect4Azure") << "capture depth image is invalid.";
				}
			}

			uint8_t* c_buffer = color_image.get_buffer();
			pix.setFromPixels((const unsigned char*)(c_buffer), color_size.x, color_size.y, OF_PIXELS_BGRA);
			b_color_new = true;
			b_tex_new = false;
		}

		color_image.reset();
		depth_image.reset();
		ir_image.reset();

		if (settings.use_tracker) {
			k4abt::frame frame = tracker.pop_result();
			if (frame)
			{
				b_tracker_processing = false;
				b_tracker_new = true;
			}

			if (!b_tracker_processing) {
				if (tracker.enqueue_capture(capture)) {
					b_tracker_processing = true;
				}
			}

			if (!frame) {
				capture.reset();
				return;
			}

			bodies.resize(frame.get_num_bodies());
			for (int i = 0; i < bodies.size(); i++) {
				k4abt_body_t body;
				bodies[i].id = frame.get_body_id(i);
				body.skeleton = frame.get_body_skeleton(i);
				for (int j = 0; j < K4ABT_JOINT_COUNT; j++) {
					auto pos = body.skeleton.joints[j].position.v;
					bodies[i].joint[j].position.x = pos[0];
					bodies[i].joint[j].position.y = pos[1];
					bodies[i].joint[j].position.z = pos[2];
					auto quat = body.skeleton.joints[j].orientation.wxyz;
					bodies[i].joint[j].quaternion.w = quat.w;
					bodies[i].joint[j].quaternion.x = quat.x;
					bodies[i].joint[j].quaternion.y = quat.y;
					bodies[i].joint[j].quaternion.z = quat.z;
				}
			}

			frame.reset();
		}
		capture.reset();
	}
	else
	{
		ofLogError("ofxKinect4Azure") << "please setup before calling update";
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::updatePointcloud()
{
	if (b_pointcloud_new)
	{
		pointcloud_vbo.setVertexData(pointcloud_vert.data(), NUM_VERTICES, GL_STREAM_DRAW);
		pointcloud_vbo.setTexCoordData(pointcloud_uv.data(), NUM_VERTICES, GL_STREAM_DRAW);
		b_pointcloud_new = false;
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::draw(float x, float y, float w, float h)
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
			if (b_color_new)
			{
				if (!b_tex_new)
				{
					color.allocate(pix);
					b_tex_new = true;
				}
				if (color.isAllocated())
				{
					if (w == 0)
					{
						w = color_size.x;
					}
					if (h == 0)
					{
						h = color_size.y;
					}
					ofPushStyle();
					ofSetColor(255);
					color.draw(x, y, w, h);
					ofPopStyle();
				}
			}
			ofPushStyle();
			ofSetColor(255);
			color.draw(x, y, w, h);
			ofPopStyle();
		}
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::drawDepth(float x, float y, float w, float h)
{
	if (device_index >= 0)
	{
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			if (b_depth_new)
			{
				if (!b_depth_tex_new) {
					depth.allocate(depth_pix);
					b_depth_tex_new = true;
				}
				if (depth.isAllocated())
				{
					if (w == 0)
					{
						w = depth_size.x;
					}
					if (h == 0)
					{
						h = depth_size.y;
					}
					ofPushStyle();
					ofSetColor(255);
					depth.draw(x, y, w, h);
					ofPopStyle();
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::drawColorizedDepth(float x, float y, float w, float h)
{
	if (device_index >= 0)
	{
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			if (b_colorized_depth_new)
			{
				if (!b_colorized_depth_tex_new) {
					colorized_depth.allocate(colorized_depth_pix);
					b_colorized_depth_tex_new = true;
				}
				if (colorized_depth.isAllocated())
				{
					if (w == 0)
					{
						w = depth_size.x;
					}
					if (h == 0)
					{
						h = depth_size.y;
					}
					ofPushStyle();
					ofSetColor(255);
					colorized_depth.draw(x, y, w, h);
					ofPopStyle();
				}
			}
			ofPushStyle();
			ofSetColor(255);
			colorized_depth.draw(x, y, w, h);
			ofPopStyle();
		}
	}
}

//--------------------------------------------------------------
void ofxKinect4Azure::drawIR(float x, float y, float w, float h)
{
	if (device_index >= 0)
	{
		if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
		{
			if (b_ir_new)
			{
				if (!b_ir_tex_new) {
					ir.allocate(ir_pix);
					b_ir_tex_new = true;
				}
				if (ir.isAllocated())
				{
					if (w == 0)
					{
						w = depth_size.x;
					}
					if (h == 0)
					{
						h = depth_size.y;
					}
					ofPushStyle();
					ofSetColor(255);
					ir.draw(x, y, w, h);
					ofPopStyle();
				}
			}
		}
	}
}
