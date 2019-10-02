#include "ofxKinect4Azure.h"

//--------------------------------------------------------------
void ofxKinect4Azure::setup()
{
	setup(device_index);
}

//--------------------------------------------------------------
void ofxKinect4Azure::setup(ofxKinect4AzureSettings _settings)
{
	settings = _settings;
	setup(device_index, settings);
}

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
	}
	else {
		ofLogNotice("ofxKinect4Azure") << ofToString(device_count) + " Azure Kinect devices detected.";
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
		if (settings.enable_imu) {
			if (k4a_device_start_imu(device) != K4A_RESULT_SUCCEEDED) {
				ofLogError("ofxKinect4Azure") << "can't start IMU.";
			}
		}

		k4a_device_get_calibration(device, settings.depth_mode, settings.color_resolution, &calibration);
		transformation = k4a_transformation_create(&calibration);
		if (settings.use_tracker) {
			k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
			if (k4abt_tracker_create(&calibration, tracker_config, &tracker) != K4A_RESULT_SUCCEEDED) {
				ofLogError("ofxKinect4Azure") << "Failed create tracker.";
			}
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
		vertices_num = depth_size.x*depth_size.y;
		pointcloud_vert.resize(vertices_num);
		pointcloud_uv.resize(vertices_num);
		int w = depth_size.x;
		for (int i = 0; i < vertices_num; i++) {
			pointcloud_uv[i] = glm::vec2(i%w, i / w);
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
void ofxKinect4Azure::update() {
	is_frame_new = false;
	b_tracker_new = false;

	k4a_capture_t capture = nullptr;
	if (k4a_device_get_capture(device, &capture, 0) != K4A_WAIT_RESULT_SUCCEEDED)
	{
		return;
	}
	else {
		is_frame_new = true;
		b_tex_new = false;
		b_depth_tex_new = false;
		b_ir_tex_new = false;
	}
	
	k4a_image_t color_image, depth_image,ir_image;
	k4a_image_t	*c_ptr = nullptr, *d_ptr = nullptr, *ir_ptr = nullptr;

	if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
	{
		depth_image = k4a_capture_get_depth_image(capture);
		if (settings.transform_type == DEPTH_TO_COLOR) {
			k4a_image_t transformed_depth_image;
			k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, depth_size.x, depth_size.y, depth_size.x * sizeof(unsigned short), &transformed_depth_image);
			k4a_transformation_depth_image_to_color_camera(transformation, depth_image, transformed_depth_image);
			d_ptr = &transformed_depth_image;
			k4a_image_release(depth_image);
		}
		else {
			d_ptr = &depth_image;
		}

		ofVec2f range = getDepthModeRange(settings.depth_mode);
		uint8_t* d_buffer = k4a_image_get_buffer(*d_ptr);

		//copy to depth pix
		depth_pix.setFromPixels(reinterpret_cast<const unsigned short*>(d_buffer), depth_size.x, depth_size.y, OF_PIXELS_GRAY);

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
		}

		if (settings.make_pointcloud) {
			k4a_image_t pointcloud_image;
			k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, depth_size.x, depth_size.y, depth_size.x * sizeof(short) * 3, &pointcloud_image);
			if (settings.transform_type == DEPTH_TO_COLOR) {
				k4a_transformation_depth_image_to_point_cloud(transformation, *d_ptr, K4A_CALIBRATION_TYPE_COLOR, pointcloud_image);
			}
			else {
				k4a_transformation_depth_image_to_point_cloud(transformation, *d_ptr, K4A_CALIBRATION_TYPE_DEPTH, pointcloud_image);
			}
			short* p_buffer = (short*)k4a_image_get_buffer(pointcloud_image);
			auto p_data = pointcloud_vert.data();

			for (int i = 0; i < vertices_num; i++) {
				p_data[i].x = p_buffer[i * 3];
				p_data[i].y = p_buffer[i * 3 + 1];
				p_data[i].z = p_buffer[i * 3 + 2];
			}

			k4a_image_release(pointcloud_image);
		}

		if (settings.use_ir_image) {
			ir_image = k4a_capture_get_ir_image(capture);
			if (settings.transform_type == DEPTH_TO_COLOR) {
				k4a_image_t transformed_ir_image;
				k4a_image_create(K4A_IMAGE_FORMAT_IR16, depth_size.x, depth_size.y, depth_size.x * sizeof(unsigned short), &transformed_ir_image);
				k4a_transformation_depth_image_to_color_camera(transformation, ir_image, transformed_ir_image);
				ir_ptr = &transformed_ir_image;
				k4a_image_release(ir_image);
			}
			else {
				ir_ptr = &ir_image;
			}
			uint8_t* ir_buffer = k4a_image_get_buffer(*ir_ptr);

			ir_pix.setFromPixels(reinterpret_cast<const unsigned short*>(ir_buffer), depth_size.x, depth_size.y, OF_PIXELS_GRAY);
		}
	}

	if (settings.color_resolution != K4A_COLOR_RESOLUTION_OFF)
	{
		color_image = k4a_capture_get_color_image(capture);
		if (settings.transform_type == COLOR_TO_DEPTH) {
			k4a_image_t transformed_color_image;
			k4a_image_create(settings.color_format, depth_size.x, depth_size.y, depth_size.x * sizeof(unsigned char) * 4, &transformed_color_image);
			k4a_transformation_color_image_to_depth_camera(transformation, depth_image, color_image, transformed_color_image);
			c_ptr = &transformed_color_image;
			k4a_image_release(color_image);
		}
		else {
			c_ptr = &color_image;
		}

		uint8_t* c_buffer = k4a_image_get_buffer(*c_ptr);
		pix.setFromPixels((const unsigned char*)(c_buffer), color_size.x, color_size.y, OF_PIXELS_BGRA);
	}

	if (c_ptr != nullptr)k4a_image_release(*c_ptr);
	if (d_ptr != nullptr)k4a_image_release(*d_ptr);
	if (ir_ptr != nullptr)k4a_image_release(*ir_ptr);

	if (settings.use_tracker) {
		k4abt_frame_t frame = nullptr;
		k4a_wait_result_t frame_get_result;
		frame_get_result = k4abt_tracker_pop_result(tracker, &frame, 0);

		if (frame_get_result == K4A_WAIT_RESULT_SUCCEEDED) {
			b_tracker_processing = false;
			b_tracker_new = true;
		}

		if (!b_tracker_processing) {
			if (k4abt_tracker_enqueue_capture(tracker, capture, 0) == K4A_WAIT_RESULT_SUCCEEDED) {
				b_tracker_processing = true;
			}
		}

		if (frame_get_result != K4A_WAIT_RESULT_SUCCEEDED) {
			k4a_capture_release(capture);
			return;
		}
		
		bodies.resize(k4abt_frame_get_num_bodies(frame));
		for (int i = 0; i < bodies.size(); i++) {
			k4abt_body_t body;
			bodies[i].id = k4abt_frame_get_body_id(frame, i);
			k4abt_frame_get_body_skeleton(frame, i, &body.skeleton);
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
		
		k4abt_frame_release(frame);
	}

	k4a_capture_release(capture);
}

void ofxKinect4Azure::updatePointcloud()
{
	if (is_frame_new) {
		pointcloud_vbo.setVertexData(pointcloud_vert.data(), vertices_num, GL_STREAM_DRAW);
		pointcloud_vbo.setTexCoordData(pointcloud_uv.data(), vertices_num, GL_STREAM_DRAW);
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

void ofxKinect4Azure::drawColorizedDepth(float x, float y, float w, float h)
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
}

void ofxKinect4Azure::drawIR(float x, float y, float w, float h)
{
	if (settings.depth_mode != K4A_DEPTH_MODE_OFF)
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
