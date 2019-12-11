//
//  ofxGifFrame.cpp
//
//  Created by Jesus.Gollonet on 9/5/11.
//  Edited by Pierre Proske on 11/12/19
//

#include "ofxGifFrame.h"

ofxGifFrame::ofxGifFrame(){
    left = top = 0;
    duration = 0.f;
    bUseTexture = true;
}

void ofxGifFrame::setUseTexture( bool useTexture ){
    if ( bUseTexture == useTexture ) return;

    bUseTexture = useTexture;
    
    if ( bUseTexture ){
        tex.allocate(pixels.getWidth(), pixels.getHeight(), GL_RGBA);
        tex.loadData(pixels);
    } else {
        tex.clear();
    }
}

void ofxGifFrame::setFromPixels(ofPixels& _px, int _left , int _top, float _duration){
    pixels    = _px;
    left      = _left;
    top       = _top;
    duration  = _duration;

    if ( bUseTexture ){
        tex.loadData(pixels);
    }
}

void ofxGifFrame::setFromGifPixels(ofPixels _constructedPx, ofPixels _rawPx , int _left , int _top, float _duration){
    pixels   = _constructedPx;
    rawPixels = _rawPx;
    left     = _left;
    top      = _top;
    duration = _duration;
    
    if ( bUseTexture ){
        tex.loadData(pixels);
    }
}

ofPixels * ofxGifFrame::getRawPixels(){
    if(rawPixels.getWidth() > 0 && rawPixels.getHeight() > 0) return &rawPixels;
    return &pixels;
}

int ofxGifFrame::getWidth(){
    return pixels.getWidth();
}

int ofxGifFrame::getHeight(){
    return pixels.getHeight();
}

int ofxGifFrame::getLeft(){
    return left;
}

int ofxGifFrame::getTop(){
    return top;
}

float ofxGifFrame::getDuration(){
    return duration;
}

void ofxGifFrame::draw(float _x, float _y){
    draw(_x, _y, getWidth(), getHeight());
}

void ofxGifFrame::draw(float _x, float _y, int _w, int _h){
    if ( !bUseTexture ){
        ofLogWarning()<<"ofxGifFrame::setUseTexture() set to false, cannot draw";
        return;
    }
    tex.draw(_x, _y, _w, _h);
}
