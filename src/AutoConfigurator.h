#ifndef _AUTO_CONFIGURATOR_H_
#define _AUTO_CONFIGURATOR_H_

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "corners.h"

const int FRONT_END_DELAY_TIME = 20;

class AutoConfigurator {
public:
	AutoConfigurator() { 
		timer = 0;
	}
	~AutoConfigurator() { }

	void init(/*ofPoint p_corners[4],*/ int p_camWidth, int p_camHeight) {
		//corners = p_corners;
		camWidth = p_camWidth;
		camHeight = p_camHeight;
		configured = false;
	}

	void reconfigure() {
		timer = 0;
		configured = false;
	}

    void configure(cv::Mat image) {
		timer++;

        cv::Mat copyMat = image;
        
		if (timer > FRONT_END_DELAY_TIME) {
			timer = 0;

			corners[0] = ofPoint(0, 0);
			corners[1] = ofPoint(camWidth, 0);
			corners[2] = ofPoint(0, camHeight);
			corners[3] = ofPoint(camWidth, camHeight);

			contourFinder.setTargetColor(ofColor::white);
			contourFinder.setMinArea(100); // TODO tweak. Seems good tho.
			contourFinder.setThreshold(100); // TODO tweak. Seems good tho.
			contourFinder.resetMaxArea();
            
            contourFinder.findContours(copyMat);

			//TODO cant just give up like this.
			if (contourFinder.size() < 1) {
				ofLogNotice("Less than 1 contour(s) found");
				return;
			}
			if (contourFinder.size() > 1) {
				ofLogNotice("More than 1 contour(s) found");
				return;
			}
				
			const std::vector<ofPoint> contPts = contourFinder.getPolyline(0).getVertices();
			get_corners(contPts, &contCorners);

			corners[0] = contCorners.tl;
			corners[1] = contCorners.tr;
			corners[2] = contCorners.br;
			corners[3] = contCorners.bl;

			configured = true;
		}
	}

	bool isConfigured() {
		return configured;
	}

	void getCorners(ofPoint* dest) {
        dest[0] = corners[0];
        dest[1] = corners[1];
        dest[2] = corners[2];
        dest[3] = corners[3];
	}

private:
    Corners contCorners;
    
	bool configured;
	ofPoint corners[4];

	int timer;

	int camWidth, camHeight;

	ofxCv::ContourFinder contourFinder;
};

#endif