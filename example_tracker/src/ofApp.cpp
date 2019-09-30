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
}

//--------------------------------------------------------------
void ofApp::update(){
	k4a.update();
	k4a.updatePointcloud();
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.begin();
	ofEnableDepthTest();
	auto& tex = k4a.getTexture();
	auto& pointcloud = k4a.getPointcloudVbo();
	tex.bind();
	pointcloud.draw(GL_POINTS, 0, pointcloud.getNumVertices());
	tex.unbind();
	ofDisableDepthTest();
	ofSetColor(ofColor::red);
	auto heading = glm::vec3(0, 1, 0);
	for (auto& b : k4a.getBodies()) {
		for (int i = 0; i < K4ABT_JOINT_COUNT;i++) {
			auto& pos = b.positon[i];
			ofDrawSphere(pos, 10);
			ofDrawLine(pos, pos + glm::rotate(b.quaternion[i], heading) * 100);
			ofDrawBitmapString(joint_id_to_name.at((k4abt_joint_id_t)i),pos);
		}
	}
	ofSetColor(255);
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
