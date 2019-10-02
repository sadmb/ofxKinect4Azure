#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofxKinect4AzureSettings settings;
	settings.use_tracker=true;
	settings.make_pointcloud = true;
	settings.transform_type = COLOR_TO_DEPTH;

	k4a.setup(settings);

	cam.setVFlip(true);

	ofSetVerticalSync(true);

	bones.resize(10);
	axises.resize(10);

	for (auto& b:bones) {
		b.resize(K4ABT_JOINT_COUNT);
		for (int i = 0; i < K4ABT_JOINT_COUNT; i++) {
			b[i].set(10, 100, 4, 1);
		}
	}

	for (auto& a : axises) {
		a.resize(K4ABT_JOINT_COUNT);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
	k4a.updatePointcloud();

	if (k4a.isTrackerNew()) {
		int id = 0;

		for (auto& b : k4a.getBodies()) {
			float height = 0;

			for (int i = 0; i < pelvis_to_head.size(); i++) {
				k4abt_joint_id_t j_id = pelvis_to_head[i];
				if (i < pelvis_to_head.size() - 2) {
					height = glm::length(b.joint[pelvis_to_head[i + 1]].position - b.joint[j_id].position);
				}
				else {
					height = 100;
				}
				bones[id][j_id].set(height / 10.f, height);
				bones[id][j_id].setGlobalPosition(b.joint[j_id].position);
				bones[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
				axises[id][j_id].setGlobalPosition(b.joint[j_id].position);
				axises[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
				bones[id][j_id].move(bones[id][j_id].getXAxis()*height / 2.f);
				bones[id][j_id].rotateDeg(90, bones[id][j_id].getZAxis());
			}

			for (auto& lr : left_right) {
				for (int i = 0; i < clavicle_to_wrist.size(); i++) {
					k4abt_joint_id_t j_id = name_to_joint_id.at(clavicle_to_wrist[i] + lr);
					if (i != clavicle_to_wrist.size() - 1) {
						height = glm::length(b.joint[j_id + 1].position - b.joint[j_id].position);
					}
					bones[id][j_id].set(height / 10.f, height);
					bones[id][j_id].setGlobalPosition(b.joint[j_id].position);
					bones[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
					axises[id][j_id].setGlobalPosition(b.joint[j_id].position);
					axises[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
					float sign = 2 * (lr == "LEFT") - 1;
					bones[id][j_id].move(bones[id][j_id].getXAxis()*height / 2.f*sign);
					bones[id][j_id].rotateDeg(90, bones[id][j_id].getZAxis()*sign);
				}

				for (int i = 0; i < hip_to_foot.size(); i++) {
					k4abt_joint_id_t j_id = name_to_joint_id.at(hip_to_foot[i] + lr);
					if (i != (hip_to_foot.size() - 1)) {
						height = glm::length(b.joint[j_id + 1].position - b.joint[j_id].position);
					}
					else {
						height = 100;
					}
					bones[id][j_id].set(height / 10.f, height);
					bones[id][j_id].setGlobalPosition(b.joint[j_id].position);
					bones[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
					axises[id][j_id].setGlobalPosition(b.joint[j_id].position);
					axises[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
					float sign = (1-2 * (lr == "LEFT"))*(1-2*(i==(hip_to_foot.size()-1)));
					bones[id][j_id].move(bones[id][j_id].getXAxis()*height / 2.f*sign);
					bones[id][j_id].rotateDeg(90, bones[id][j_id].getZAxis()*sign);
				}

				for (int i = 0; i < eye_ear.size(); i++) {
					k4abt_joint_id_t j_id = name_to_joint_id.at(eye_ear[i] + lr);
					height = 100;
					bones[id][j_id].set(height / 10.f, height);
					bones[id][j_id].setGlobalPosition(b.joint[j_id].position);
					bones[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
					axises[id][j_id].setGlobalPosition(b.joint[j_id].position);
					axises[id][j_id].setGlobalOrientation(b.joint[j_id].quaternion);
					float sign = 2 * (lr == "LEFT"||eye_ear[i]=="EYE_")-1;
					bones[id][j_id].move(bones[id][j_id].getXAxis()*height / 2.f*sign);
					bones[id][j_id].rotateDeg(90, bones[id][j_id].getZAxis()*sign);
				}
			}

			id++;
		}
		num_bodies = id;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.begin();
	ofPushMatrix();
	ofScale(1, 1, -1);
	ofEnableDepthTest();
	auto& tex = k4a.getTexture();
	auto& pointcloud = k4a.getPointcloudVbo();
	tex.bind();
	pointcloud.draw(GL_POINTS, 0, pointcloud.getNumVertices());
	tex.unbind();
	ofDisableDepthTest();
	auto& b = k4a.getBodies();
	for (int i = 0; i < b.size();i++) {
		ofSetColor(id_to_color[b[i].id],100);
		for (auto& b : bones[i]) {
			b.draw();
		}
		ofSetColor(255);

		for (auto&a : axises[i]) {
			a.draw();
			a.drawAxes(100);
		}
		ofSetColor(0);
		ofDrawBitmapString(b[i].id, bones[i][K4ABT_JOINT_HEAD].getGlobalPosition());
		ofSetColor(255);
	}
	ofPopMatrix();
	cam.end();
	ofDrawBitmapString(ofGetFrameRate(), 10, 10);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
