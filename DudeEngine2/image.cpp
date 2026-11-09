// =================================================================================
// Filename:  Image.cpp
//
// Created:   24.05.2025  by DimaSkup
// =================================================================================
#include "image.h"

#include "log.h"
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#pragma warning (disable : 4996)


//---------------------------------------
// helper to delete arrays
//---------------------------------------
template <typename T>
inline void SafeDeleteArr(T*& p)
{
    if (p) { delete[](p); p = nullptr; }
}

// --------------------------------------------------------
// default constructor
// --------------------------------------------------------
Image::Image()
{
}

// --------------------------------------------------------
// Desc:  create an image loading its data from file
// --------------------------------------------------------
Image::Image(const char* filepath)
{
    Load(filepath);
}

// --------------------------------------------------------
// destructor
// --------------------------------------------------------
Image::~Image()
{
    Shutdown();
}

// --------------------------------------------------------
// Desc:   create empty RGB image filling it with white
// Args:   - width, height: dimensions of the new image
//         - bpp:           bits per pixel
// --------------------------------------------------------
bool Image::CreateEmpty(const uint width, const uint height, const uint bpp)
{
    Shutdown();

    const int numBytes = width * height * (bpp >> 3);
    pixels_ = new uint8[numBytes];
    memset(pixels_, 255, numBytes);

    // set the member variables
    width_         = width;
    height_        = height;
    bpp_           = bpp;
    bytesPerPixel_ = bpp >> 3;
    isLoaded_      = true;

    return true;
}

// --------------------------------------------------------
// Desc:   unload the texture that is currently loaded
// --------------------------------------------------------
void Image::Shutdown()
{
    SafeDeleteArr(pixels_);

    width_ = 0;
    height_ = 0;
    bpp_ = 0;

    isLoaded_ = false;
}

//-----------------------------------------------------
// Desc:  extract file extension from input path
//-----------------------------------------------------
void Image::GetImageExt(const char* path, char* outExt) const
{
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, "empty path");
        return;
    }

    const char* dotPos = strrchr(path, '.');

    if (!dotPos)
    {
        strcpy(outExt, "invalid");
        return;
    }
     
    strcpy(outExt, dotPos);
    outExt[strlen(dotPos)] = '\0';                     // put extra NULL
}


// ----------------------------------------------------
// Desc:   get a color (RGB triplet) from a texture
// Args:   - x, y:     position to get color from
// Output: - r, g, b:  value of each channel
// ----------------------------------------------------
void Image::GetPixelColor(const uint x, const uint y, 
                                        uint8& r, uint8& g, uint8& b) const
{
    assert(bpp_ == 24 || bpp_ == 32);

    const uint idx = ((y * height_) + x) * bytesPerPixel_;

    r = pixels_[idx + 0];
    g = pixels_[idx + 1];
    b = pixels_[idx + 2];
}

// ----------------------------------------------------
// Desc:   get a color (RGBA) from a texture
// Args:   - x, y:        position to get color from
// Output: - r, g, b, a:  value of each channel
// ----------------------------------------------------
void Image::GetPixelColor(const uint x, const uint y,
                          uint8& r, uint8& g, uint8& b, uint8& a) const
{
    assert(bpp_ == 32);

    const uint idx = ((y * height_) + x) * bytesPerPixel_;

    r = pixels_[idx + 0];
    g = pixels_[idx + 1];
    b = pixels_[idx + 2];
    a = pixels_[idx + 3];
}

// ----------------------------------------------------
// Desc:   set the color (RGB triplet) for a texture pixel
// Args:   - x, y:     pixel position
//         - r, g, b:  color to set
// ----------------------------------------------------
void Image::SetPixelColor(const uint x, const uint y, 
                                 const uint8 r, const uint8 g, const uint8 b)
{
    assert(bpp_ == 24 || bpp_ == 32);

    if ((x < width_) && (y < height_))
    {
        const uint idx = ((y * height_) + x) * bytesPerPixel_;

        pixels_[idx + 0] = r;
        pixels_[idx + 1] = g;
        pixels_[idx + 2] = b;
    }
}

// --------------------------------------------------------
// save raw input pixels into the disk as BMP image 
// --------------------------------------------------------
void Image::SaveAsBMP(
    const char* filepath,
    const uint8* pixels,
    const uint w,
    const uint h,
    const uint bpp)
{
    assert(filepath && filepath[0] != '\0');
    assert(pixels);
    assert(w > 0 && h > 0);
    assert(bpp > 0);

    // give ownership
    width_ = w;
    height_ = h;
    bytesPerPixel_ = bpp >> 3;
    bpp_ = bpp;
    pixels_ = (uint8*)pixels;

    SaveAsBMP(filepath);

    // remove ownership
    pixels_ = nullptr;
}

// --------------------------------------------------------
// Desc:   load only the data for a new image
// Args:   filename: the file to load in
// --------------------------------------------------------
bool Image::Load(const char* filepath)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    // get file extension
    char ext[8]{ '\0' };
    GetImageExt(filepath, ext);


    // read BMP image
    if (strcmp(ext, ".bmp") == 0)
    {
        if (!LoadRgbBMP(filepath))
        {
            LogErr(LOG, "can't load BMP file: %s", filepath);
            Shutdown();
            return false;
        }
    }

    // we read TGA image
    else if (strcmp(ext, ".tga") == 0)
    {
        LogErr(LOG, "tga loading is broken :(");
        return false;

#if 0
        if (!LoadTGA(filepath))
        {
            LogErr(LOG, "can't load TGA file: %s", filepath);
            Shutdown();
            return false;
        }
#endif
    }
     
    // the file is not supported by our image loader
    else
    {
        LogErr(LOG, "a file has a NOT supported image type: %s", filepath);
        Shutdown();
        return false;
    }


    isLoaded_ = true;
    LogMsg(LOG, "file is loaded: %s", filepath);
    return true;
}

// --------------------------------------------------------
// Desc:   save image into file (supported formats: BMP, RAW)
// Args:   - filepath: where to store the image
// --------------------------------------------------------
bool Image::Save(const char* filepath) const
{
    // check input params
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    // check if we have proper data to save
    if (!width_ || !height_ || !pixels_ || !isLoaded_)
    {
        LogErr(LOG, "can't save: image is empty or corrupted (%s)", filepath);
        return false;
    }

    char ext[8]{'\0'};
    GetImageExt(filepath, ext);


    // store height map into BMP file
    if (strcmp(ext, ".bmp") == 0)
    {
        if (!SaveAsBMP(filepath))
        {
            LogErr(LOG, "can't save .bmp file: %s", filepath);
            return false;
        }
    }

    else
    {
        LogErr(LOG, "can't save image into file: unsupported format (%s)", ext);
        return false;
    }

    LogMsg(LOG, "An image was saved successfully: %s", filepath);
    return true;
}



//==================================================================================
// 
//                            SPECIFIC LOADERS
// 
//==================================================================================

//--------------------------------------------------------------
// Desc:    load bmp image data
// Output:  bmp file header, info header
// Return:  ptr to array of pixels or nullptr if we didn't manage to load data
//--------------------------------------------------------------
uint8* LoadBmpData(
    const char* filepath,
    BMPFileHeader& fileHeader,
    BMPInfoHeader& infoHeader)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return nullptr;
    }


    uint8* pixels = nullptr;
    size_t retCode = 0;

    // open file in binary and read in headers
    FILE* pFile = fopen(filepath, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for reading in binary: %s", filepath);
        return nullptr;
    }

    retCode = fread(&fileHeader, sizeof(BMPFileHeader), 1, pFile);
    if (retCode != 1)
    {
        LogErr(LOG, "error reading a bitmap file header: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    // check for BM reversed...
    if (fileHeader.type != 'MB')
    {
        LogErr(LOG, "not a bitmap file: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    retCode = fread(&infoHeader, sizeof(BMPInfoHeader), 1, pFile);
    if (retCode != 1)
    {
        LogErr(LOG, "error reading a bitmap info header: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    if (infoHeader.compression != 0)
    {
        LogErr(LOG, "only non-compressed bitmap images are supported: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    if (infoHeader.bitCount != 24 && infoHeader.bitCount != 32)
    {
        LogErr(LOG, "unsupported bit count (%u), it must be 24 or 32 (for file: %s)", infoHeader.bitCount, filepath);
        fclose(pFile);
        return nullptr;
    }


    // calculate a size of image pixels data
    uint bitSize = 0;
    
    if (infoHeader.sizeImage != 0)
        bitSize = infoHeader.sizeImage;
    else
        bitSize = (infoHeader.width * infoHeader.bitCount + 7) / 8 * abs(infoHeader.height);

    // allocate memory
    pixels = new uint8[bitSize];

    // read in the bitmap image data
    retCode = fread(pixels, 1, bitSize, pFile);
    if (retCode != bitSize)
    {
        LogErr(LOG, "can't read the bitmap image data: %s", filepath);
        fclose(pFile);
        SafeDeleteArr(pixels);
		return nullptr;
    }

    fclose(pFile);
    return pixels;
}

//--------------------------------------------------------------
// Desc:  RGB bitmap (BMP) image loader
//--------------------------------------------------------------
bool Image::LoadRgbBMP(const char* filepath)
{
    BMPFileHeader fileHeader = {};
    BMPInfoHeader infoHeader = {};

    uint8* tmpPixels = LoadBmpData(filepath, fileHeader, infoHeader);
    if (!tmpPixels)
    {
        LogErr(LOG, "can't load bitmap image data");
        return false;
    }

    if (!CreateEmpty(infoHeader.width, infoHeader.height, infoHeader.bitCount))
    {
        LogErr(LOG, "can't create an empty space for bmp image: %s", filepath);
        SafeDeleteArr(tmpPixels);
        return false;
    }


    // initialize the position in the final image data buffer
    uint k = 0;

    const uint width           = infoHeader.width;
    const uint height          = infoHeader.height;
    const uint bytesPerPixel   = infoHeader.bitCount >> 3;

    const uint bytesInScanline = width * bytesPerPixel;
    const uint padding         = (width % 2 != 0) * (bytesPerPixel == 3);


    // read the image data into the final pixels array
    for (uint j = 0; j < height; ++j)
    {
        // bitmaps are upside down so load bottom to top into the array
        const uint rowIdx = (height - 1 - j) * bytesInScanline;

        for (uint i = 0; i < width; ++i)
        {
            const uint baseIdx = rowIdx + i * bytesPerPixel;

            // swap the R and B values to get RGB since the bitmap color format is in BGR
            pixels_[baseIdx + 0] = tmpPixels[k + 2];
            pixels_[baseIdx + 1] = tmpPixels[k + 1];
            pixels_[baseIdx + 2] = tmpPixels[k + 0];

            k += bytesPerPixel;
        }

        // compensate for the extra byte at end of each line in 
        // non-divide by 2 bitmaps (eg. 257x257) with 24 bpp 
        k += padding;
    }

    // release temp buffer
    SafeDeleteArr(tmpPixels);

    SetName(filepath);

    return true;
}

//---------------------------------------------------------
// Desc:   to minimize memory usage we load grayscale bmp images
//         in a specific way: just get only R-channel of each pixel
//         and set bits per pixel (bpp) value == 8 for this image instance
//---------------------------------------------------------
bool Image::LoadGrayscaleBMP(const char* filepath)
{
    size_t        retCode = 0;
    BMPFileHeader fileHeader = {};
    BMPInfoHeader infoHeader = {};
    const uint8   bpp = 8;

    uint8* tmpPixels = LoadBmpData(filepath, fileHeader, infoHeader);
    if (!tmpPixels)
    {
        LogErr(LOG, "can't load bitmap image data");
        return false;
    }

    if (!CreateEmpty(infoHeader.width, infoHeader.height, bpp))
    {
        LogErr(LOG, "can't create an empty space for bmp image: %s", filepath);
        SafeDeleteArr(tmpPixels);
        return false;
    }

    // initialize the position in the image data buffer
    int k = 0;
    
    const int width         = infoHeader.width;
    const int height        = infoHeader.height;

    const int bytesPerPixel = infoHeader.bitCount >> 3;
    const int padding       = (width % 2 != 0) * (bytesPerPixel == 3);

    // read the image data into the final pixels array
    for (int j = 0; j < height; ++j)
    {
        // bitmaps are upside down so load bottom to top into the array
        const int rowIdx = (height - 1 - j) * width;

        for (int i = 0; i < width; ++i)
        {
            // get the grey scale pixel value from the bitmap image data at this location
            pixels_[rowIdx + i] = tmpPixels[k];

            // increment the bitmap image data idx (skip G and B channels, and also alpha if bpp == 32)
            k += bytesPerPixel;
        }

        // compensate for the extra byte at end of each line in 
        // non-divide by 2 bitmaps (eg. 257x257) with 24 bpp 
        k += padding;
    }

    // release temp buffer
    SafeDeleteArr(tmpPixels);

    SetName(filepath);

    return true;
}

//==================================================================================
// 
//                           SPECIFIC SAVERS
// 
//==================================================================================

// --------------------------------------------------------
// Desc:   save the current image info to a BMP file.
// Args:   - filename:  where to save
// --------------------------------------------------------
bool Image::SaveAsBMP(const char* filepath) const
{
    BMPFileHeader  bitmapFileHeader;
    BMPInfoHeader  bitmapInfoHeader;

    const uint bytesPerPixel = 3;
    const uint headersOffset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    
    uint bytesInScanline   = (width_ * bytesPerPixel);
   
    // compensate for the extra bytes at end of each line (to make boundary 4 bytes aligned)
    const uint overFour    = (bytesInScanline & 3);
    const uint paddingSize = (overFour == 0) ? 0 : 4-overFour ;
    const uint imageSize   = height_ * (width_ * bytesPerPixel + paddingSize);

    // account compensation
    bytesInScanline += paddingSize;


    // check input params
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filename");
        return false;
    }

    // open a file that we can write to
    FILE* pFile = fopen(filepath, "wb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file to save BMP image: %s", filepath);
        return false;
    }

    // define the BMP file header
    bitmapFileHeader.type            = 0x4D42;// BITMAP_ID;
    bitmapFileHeader.size            = headersOffset + imageSize;
    bitmapFileHeader.reserved1       = 0;
    bitmapFileHeader.reserved2       = 0;
    bitmapFileHeader.offBits         = headersOffset;
  
    // define the BMP info header
    bitmapInfoHeader.size            = sizeof(BMPInfoHeader);
    bitmapInfoHeader.width           = width_;
    bitmapInfoHeader.height          = height_;
    bitmapInfoHeader.planes          = 1;
    bitmapInfoHeader.bitCount        = 24;
    bitmapInfoHeader.compression     = 0;          // no compression
    bitmapInfoHeader.sizeImage       = imageSize;
    bitmapInfoHeader.xPixelsPerMeter = 0;
    bitmapInfoHeader.yPixelsPerMeter = 0;
    bitmapInfoHeader.colorUsed       = 0;
    bitmapInfoHeader.colorImportant  = 0;


    //
    // copy pixels into temp buffer
    //
    uint8* bmpPixels = new uint8[imageSize];
    memset(bmpPixels, 0, imageSize);


    // if the original image is grayscale
    if (bpp_ == 8)
    {
        int k = 0;
        const int width  = width_;
        const int height = height_;

        for (int j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            const int rowIdx = (height - 1 - j) * bytesInScanline;

            for (int i = 0; i < width; ++i)
            {
                bmpPixels[rowIdx + i*3 + 0] = pixels_[k];
                bmpPixels[rowIdx + i*3 + 1] = pixels_[k];
                bmpPixels[rowIdx + i*3 + 2] = pixels_[k];
                k++;
            }
        }
    }

    // if original image is already 24 bits per pixel
    else if (bpp_ == 24)
    {
        uint k = 0;
        const uint height = height_;
        const uint width  = width_;

        for (uint j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            uint rowIdx = (height - 1 - j) * bytesInScanline;

            for (uint i = 0; i < width; ++i)
            {
                uint base = rowIdx + i*3;

                // swap the image bit order (RGB to BGR)
                bmpPixels[base + 0] = pixels_[k + 2];
                bmpPixels[base + 1] = pixels_[k + 1];
                bmpPixels[base + 2] = pixels_[k + 0];

                k += 3;
            }
        }
    }

    // convert original 32 bits image into 24 bits image
    else
    {
        uint k = 0;
        const uint height = height_;
        const uint width = width_;

        for (uint j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            uint rowIdx = (height - 1 - j) * bytesInScanline;

            for (uint i = 0; i < width; ++i)
            {
                uint base = rowIdx + i*3;

                // swap the image bit order (RGB to BGR)
                bmpPixels[base + 0] = pixels_[k + 2];
                bmpPixels[base + 1] = pixels_[k + 1];
                bmpPixels[base + 2] = pixels_[k + 0];

                k += 4;
            }
        }
    }


    // write the .bmp file header to the file
    fwrite(&bitmapFileHeader, 1, sizeof(BMPFileHeader), pFile);

    // write the .bmp file info header to the file
    fwrite(&bitmapInfoHeader, 1, sizeof(BMPInfoHeader), pFile);

    // now write the actual image data
    fwrite(bmpPixels, 1, imageSize, pFile);

    // release the pixels temp buffer
    SafeDeleteArr(bmpPixels);

    // the file has been successfully saved
    fclose(pFile);

    return true;
}

//---------------------------------------------------------
// Desc:  setup a name for this image (usually it is a path to this image)
//---------------------------------------------------------
void Image::SetName(const char* name)
{
    if (!name || name[0] == '\0')
    {
        LogErr(LOG, "empty name");
        return;
    }

    size_t maxLen = sizeof(name_);
    size_t len = strlen(name);

    if (len > maxLen) 
        len = maxLen;

    strncpy(name_, name, len);
}
