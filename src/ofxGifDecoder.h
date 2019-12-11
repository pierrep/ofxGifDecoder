//
//  ofxGifDecoder.h
//
//  Created by Jesus Gollonet on 5/14/11.
//  Edited by Pierre Proske on 11/12/19
//

#pragma once
#include "ofMain.h"
#include "ofxGifFile.h"
#include "FreeImage.h"

class ofxGifDecoder : protected ofThread {
public:
    ofxGifDecoder();
    ~ofxGifDecoder();
    
    // blocking load: may cause your app to hang a little
    bool decode(string fileName, bool useTextures = true);
    
    // threaded load: you'll need to check if loaded via
    // isLoaded() or catch load onGifLoaded
    void decodeThreaded(string fileName);
    
    ofxGifFile & getFile();
    
    ofEvent<ofxGifFile> onGifLoaded;
    bool isLoaded();
    
protected:
    
    vector<ofPixels *> pxs;
    vector <ofColor> palette;
    
    string currentFile;
    bool bNeedToUpdate;
    void update( ofEventArgs & e );
    void threadedFunction();
    
private :
    
    ofxGifFile  gifFile;
    
    void reset();
    void createGifFile(FIBITMAP * bmp, int _nPages);
    void processFrame(FIBITMAP * bmp, int _frameNum, bool useTexture = false);
    
    int globalPaletteSize;
    RGBQUAD * globalPalette;
    float defaultFrameDuration;

};
