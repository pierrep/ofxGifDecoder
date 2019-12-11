//
//  ofxGifFile.cpp
//  ofxGifDecoderAddon
//
//  Created by Jesus.Gollonet on 9/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ofxGifFile.h"

ofxGifFile::ofxGifFile()
{
    w = h = 0;
    lastDrawn = 0;
    frameIndex = 0;
}

ofxGifFile::~ofxGifFile() {}

void ofxGifFile::setup(int _w, int _h, vector<ofColor> _globalPalette, int _nPages)
{
    w = _w;
    h = _h;
    globalPalette = _globalPalette;
    nPages = _nPages;
}

// by now we're copying everything (no pointers)
void ofxGifFile::addFrame(ofPixels _px, unsigned int _left, unsigned int _top, bool useTexture, GifFrameDisposal disposal, float _duration)
{
    ofxGifFrame f;

    if (getNumFrames() == 0) {
        accumPx = _px; // we assume 1st frame is fully drawn
        if (!useTexture) {
            f.setUseTexture(false);
        }
        f.setFromPixels(_px, _left, _top, _duration);
        gifDuration = _duration;
    } else {
        // add new pixels to accumPx
        unsigned int cropOriginX = _left;
        unsigned int cropOriginY = _top;

        // [todo] make this loop only travel through _px, not accumPx
        for (unsigned int i = 0; i < accumPx.getWidth() * accumPx.getHeight(); i++) {
            unsigned int x = i % accumPx.getWidth();
            unsigned int y = i / accumPx.getWidth();

            if ((x >= _left) && (x < _left + _px.getWidth()) && (y >= _top) && (y < _top + _px.getHeight())) {
                unsigned int cropX = x - cropOriginX;
                unsigned int cropY = y - cropOriginY;

                unsigned int alpha = _px.getColor(cropX, cropY).a;
                if (alpha < 255) {
                    switch (disposal) {
                    case GIF_DISPOSAL_BACKGROUND:
                        accumPx.setColor(x, y, bgColor);
                        break;

                    case GIF_DISPOSAL_LEAVE: // leave pixels as per previous frame
                    case GIF_DISPOSAL_UNSPECIFIED:
                        break;

                    case GIF_DISPOSAL_PREVIOUS:
                        ofLogError() << "GIF_DISPOSAL_PREVIOUS not yet implemented";
                        break;
                    }
                } else {
                    accumPx.setColor(x, y, _px.getColor(cropX, cropY));
                }
            }

        }

        if (!useTexture) {
            f.setUseTexture(false);
        }
        f.setFromPixels(accumPx, _left, _top, _duration);
    }

    gifFrames.push_back(f);
}

int ofxGifFile::getWidth()
{
    return w;
}

int ofxGifFile::getHeight()
{
    return h;
}

float ofxGifFile::getDuration()
{
    return gifDuration;
}

void ofxGifFile::updateFrameIndex()
{
    // never drawn, lets kick off
    if (lastDrawn == 0) {
        lastDrawn = ofGetElapsedTimef();
    }
    if ((ofGetElapsedTimef() - lastDrawn) >= gifFrames[frameIndex].getDuration()) {
        lastDrawn = ofGetElapsedTimef();
        frameIndex++;
        frameIndex %= getNumFrames();
    }
    return;
}

void ofxGifFile::draw(float _x, float _y)
{
    if (getNumFrames() == 0) {
        ofLogWarning() << "ofxGifFile::No frames to draw!";
        return;
    }

    updateFrameIndex();
    drawFrame(frameIndex, _x, _y, gifFrames[frameIndex].getWidth(), gifFrames[frameIndex].getHeight());
}

void ofxGifFile::draw(float _x, float _y, float _w, float _h)
{
    if (getNumFrames() == 0) {
        ofLogWarning() << "ofxGifFile::No frames to draw!";
        return;
    }

    updateFrameIndex();
    drawFrame(frameIndex, _x, _y, _w, _h);
}

void ofxGifFile::drawFrame(int _frameNum, float _x, float _y)
{
    if (_frameNum < 0 || _frameNum >= gifFrames.size()) {
        ofLog(OF_LOG_WARNING, "ofxGifFile::drawFrame frame out of bounds. not drawing");
        return;
    }
    drawFrame(_frameNum, _x, _y, gifFrames[_frameNum].getWidth(), gifFrames[_frameNum].getHeight());
}

void ofxGifFile::drawFrame(int _frameNum, float _x, float _y, int _w, int _h)
{
    if (_frameNum < 0 || _frameNum >= gifFrames.size()) {
        ofLog(OF_LOG_WARNING, "ofxGifFile::drawFrame frame out of bounds. not drawing");
        return;
    }
    gifFrames[_frameNum].draw(_x, _y, _w, _h);
}

void ofxGifFile::setBackgroundColor(ofColor _c)
{
    bgColor = _c;
    ofLogVerbose() << "Gif Background Color: " << bgColor;
}

ofColor ofxGifFile::getBackgroundColor()
{
    return bgColor;
}

int ofxGifFile::getNumFrames()
{
    return gifFrames.size();
}

ofxGifFrame* ofxGifFile::getFrameAt(int _index)
{
    if (_index < getNumFrames()) {
        return &(gifFrames[_index]); //??
    } else {
        ofLogWarning() << "ofxGifFile:: trying to get frame that doesn't exist, returning NULL!";
        return NULL;
    }
}

vector<ofxGifFrame>& ofxGifFile::getFrames()
{
    return gifFrames;
}

vector<ofColor> ofxGifFile::getPalette()
{
    return globalPalette;
}

void ofxGifFile::clear()
{
    gifFrames.clear();
    globalPalette.clear();
}
