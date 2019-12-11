//
//  ofxGifDecoder.cpp
//
//  Created by Jesus Gollonet on 5/14/11.
//  Edited by Pierre Proske on 11/12/19
//

#include "ofxGifDecoder.h"

ofxGifDecoder::ofxGifDecoder()
{
    globalPalette = NULL;
    globalPaletteSize = 0;
    bNeedToUpdate = false;
    defaultFrameDuration = 0.04f;
    ofAddListener(ofEvents().update, this, &ofxGifDecoder::update);
}

ofxGifDecoder::~ofxGifDecoder()
{
    ofRemoveListener(ofEvents().update, this, &ofxGifDecoder::update);
    waitForThread();
}

// required to update textures with threaded version
void ofxGifDecoder::update(ofEventArgs& e)
{
    if (bNeedToUpdate) {
        // gif is loaded, switch all textures on
        for (auto& frame : gifFile.getFrames()) {
            frame.setUseTexture(true);
        }

        bNeedToUpdate = false;
        ofNotifyEvent(onGifLoaded, gifFile, this);
    }
}

// return a bool if succesful
bool ofxGifDecoder::decode(string fileName, bool useTextures)
{
    reset();
    int width, height, bpp;
    fileName = ofToDataPath(fileName);
    bool bDecoded = false;
    FIMULTIBITMAP* multiBmp = NULL;

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(fileName.c_str(), 0);

    if (fif != FIF_GIF) {
        ofLog(OF_LOG_WARNING, "ofxGifDecoder::decode. this is not a gif file. not processing");

        return bDecoded;
    }

    multiBmp = FreeImage_OpenMultiBitmap(fif, fileName.c_str(), false, false, true, GIF_LOAD256);

    if (multiBmp) {

        // num frames
        int nPages = FreeImage_GetPageCount(multiBmp);
        ofLogNotice() << "Gif frames: " << nPages;

        // here we process the first frame
        for (int i = 0; i < nPages; i++) {
            FIBITMAP* dib = FreeImage_LockPage(multiBmp, i);
            if (dib) {
                if (i == 0) {
                    createGifFile(dib, nPages);
                    bDecoded = true; // we have at least 1 frame
                }
                processFrame(dib, i, useTextures);
                FreeImage_UnlockPage(multiBmp, dib, false);
            } else {
                ofLog(OF_LOG_WARNING, "ofxGifDecoder::decode. problem locking page");
            }
        }
        FreeImage_CloseMultiBitmap(multiBmp, 0);
    } else {
        ofLog(OF_LOG_WARNING, "ofxGifDecoder::decode. there was an error processing.");
    }
    return bDecoded;
}

void ofxGifDecoder::decodeThreaded(string fileName)
{
    // will smash if inside thread
    reset();

    if (isThreadRunning()) {
        ofLogError() << "ofxGifDecoder::Thread already running! Please wait till loaded";
        return;
    }
    currentFile = fileName;
    startThread();
}

void ofxGifDecoder::threadedFunction()
{
    bool success = decode(currentFile, false);
    if (success) {
        bNeedToUpdate = true;
    }
}

void ofxGifDecoder::createGifFile(FIBITMAP* bmp, int _nPages)
{

    FITAG* tag;

    int logicalWidth, logicalHeight;

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "LogicalWidth", &tag)) {
        logicalWidth = *(unsigned short*)FreeImage_GetTagValue(tag);
    }

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "LogicalHeight", &tag)) {
        logicalHeight = *(unsigned short*)FreeImage_GetTagValue(tag);
    }

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "GlobalPalette", &tag)) {
        globalPaletteSize = FreeImage_GetTagCount(tag);
        if (globalPaletteSize >= 2) {
            globalPalette = (RGBQUAD*)FreeImage_GetTagValue(tag);
            for (int i = 0; i < globalPaletteSize; i++) {
                ofColor c;
                c.set(globalPalette[i].rgbRed, globalPalette[i].rgbGreen, globalPalette[i].rgbBlue);
                palette.push_back(c);
            }
        }
    }

    gifFile.setup(logicalWidth, logicalHeight, palette, _nPages);

    RGBQUAD bgColor;
    if (FreeImage_GetBackgroundColor(bmp, &bgColor)) {
        gifFile.setBackgroundColor(ofColor(bgColor.rgbRed, bgColor.rgbGreen, bgColor.rgbBlue));
    }
}

void ofxGifDecoder::processFrame(FIBITMAP* bmp, int _frameNum, bool useTexture)
{
    FITAG* tag = nullptr;
    ofPixels pix;

    unsigned int frameLeft, frameTop;
    float frameDuration;
    GifFrameDisposal disposal_method = GIF_DISPOSAL_BACKGROUND;

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameLeft", &tag)) {
        if (tag != nullptr) {
            frameLeft = *(unsigned short*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Frame Left:" << frameLeft;
        }
    }

    tag = nullptr;
    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTop", &tag)) {
        if (tag != nullptr) {
            frameTop = *(unsigned short*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Frame Top:" << frameTop;
        }
    }

    tag = nullptr;
    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTime", &tag)) {
        if (tag != nullptr) {
            ofLogVerbose() << "tag type: " << FreeImage_GetTagType(tag);
            ofLogVerbose() << "tag count: " << FreeImage_GetTagCount(tag);
            const unsigned long frameTime = *(long*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Frame Time: " << frameTime;
            if (frameTime < 1000000) {
                frameDuration = (float)(frameTime) / 1000.0f; // convert to milliseconds
                defaultFrameDuration = frameDuration;
            } else {
                frameDuration = defaultFrameDuration;
            }
            ofLogVerbose() << "Got GIF frame duration: " << frameDuration;
        } else {
            ofLogVerbose() << "GIF frame duration tag not found ";
        }
    }

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "DisposalMethod", &tag)) {
        if (tag != nullptr) {
            disposal_method = (GifFrameDisposal) * (unsigned char*)FreeImage_GetTagValue(tag);
        }
    }

    FIBITMAP* bmpConverted = NULL;
    if (FreeImage_GetColorType(bmp) == FIC_PALETTE || FreeImage_GetBPP(bmp) < 8) {
        // Force all incoming frames to be RGBA
        bmpConverted = FreeImage_ConvertTo32Bits(bmp);
        bmp = bmpConverted;
    }

    unsigned int width = FreeImage_GetWidth(bmp);
    unsigned int height = FreeImage_GetHeight(bmp);
    unsigned int bpp = FreeImage_GetBPP(bmp);
    unsigned int channels = (bpp / sizeof(unsigned char)) / 8;
    unsigned int pitch = FreeImage_GetPitch(bmp);
    ofLogVerbose() << "bpp: " << bpp << " channels: " << channels;

    FreeImage_FlipVertical(bmp);  // ofPixels are top left, FIBITMAP is bottom left

    unsigned char* bmpBits = FreeImage_GetBits(bmp);

    ofPixelFormat pixFormat;
    if (channels == 3)
        pixFormat = OF_PIXELS_BGR;
    if (channels == 4)
        pixFormat = OF_PIXELS_BGRA;

    if (bmpBits != NULL) {
        pix.setFromAlignedPixels(bmpBits, width, height, pixFormat, pitch);

#ifdef TARGET_LITTLE_ENDIAN
        if (channels >= 3) {
            pix.swapRgb();
        }
#endif

        gifFile.addFrame(pix, frameLeft, frameTop, useTexture, disposal_method, frameDuration);
    } else {
        ofLogError() << "ofImage::putBmpIntoPixels() unable to set ofPixels from FIBITMAP";
    }
}

void ofxGifDecoder::reset()
{
    gifFile.clear();
    pxs.clear();
    palette.clear();
}

// should I return a pointer of a copy?
ofxGifFile& ofxGifDecoder::getFile()
{
    return gifFile;
}
