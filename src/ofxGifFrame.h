//
//  ofxGifFrame.h
//
//  Created by Jesus.Gollonet on 9/5/11.
//  Edited by Pierre Proske on 11/12/19
//

#pragma once


#include "ofMain.h"

class ofxGifFrame{
    
public:
    ofxGifFrame();

    // for user
    void setFromPixels(ofPixels& _px , int _left , int _top, float _duration = 0.f);
    
    // for ofxGifFile 
    void setFromGifPixels(ofPixels _constructedPx, ofPixels _rawPx , int _left , int _top, float _duration = 0.f);
    void draw(float _x, float _y);
    void draw(float _x, float _y, int _w, int _h);
    ofPixels * 	getRawPixels();
    int getWidth();
    int getHeight();
    int getLeft();
    int getTop();
    float getDuration();
    
    // texture stuff
    void setUseTexture( bool useTexture );
    
private:
    int top;
    int left;
    ofPixels pixels;
    ofPixels rawPixels;
    ofTexture tex;
    float duration;
    vector<ofColor> palette;
    bool bUseTexture;
};
