#pragma once
#include "ofMain.h"
#include "ofxKinect4Azure.h"
#include <mutex>

class k4aThread:public ofThread
{
	ofxKinect4Azure k4a;
	ofPixels pix;
	ofTexture tex;
	bool is_frame_new = false;
	std::mutex lock;
public:
	k4aThread() {};
	~k4aThread() {
		stopThread();
		waitForThread();
	};

	ofxKinect4Azure* getK4aRef() {
		return &k4a;
	}

	void threadedFunction() {
		while (isThreadRunning()) {
			k4a.update();
			if (k4a.isFrameNew()) {
				lock.lock();
				auto& _pix = k4a.getPixels();
				pix.setFromPixels(_pix.getData(), _pix.getWidth(), _pix.getHeight(), _pix.getPixelFormat());
				is_frame_new = true;
				lock.unlock();
			}
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
	}

	void updatePointcloud() {
		lock.lock();
		k4a.updatePointcloud();
		lock.unlock();
	}

	ofTexture& getTexture() {
		lock.lock();
		if (is_frame_new) {
			tex.allocate(pix);
			is_frame_new = false;
		}
		lock.unlock();
		return tex;
	}

	ofVbo& getPointcloud() {
		return k4a.getPointcloudVbo();
	}

private:

};