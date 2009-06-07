//
// Copyright 2002,2003 Sony Corporation
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef BMP_h_DEFINED
#define BMP_h_DEFINED

#include <stdio.h>
#include <OPENR/ODataFormats.h>
#include "CommunicateObj.h"

struct BMPHeader {
    byte      magic[2];         // Magic number 'BM'
    longword  size;             // File size
    word      reserved1;        // Reserved
    word      reserved2;        // Reserved
    longword  offset;           // Offset to image data

    BMPHeader() {
        magic[0] = 'B';
        magic[1] = 'M';
        size = 0;
        reserved1 = 0;
        reserved2 = 0;
        offset = 14 + 40;
    }
};

const longword bmpcompressionRGB      = 0; // No compression - RGB bitmap
const longword bmpcompressionRLE8     = 1; // 8-bit run-length compression
const longword bmpcompressionRLE4     = 2; // 4-bit run-length compression
const longword bmpcompressionRGB_MASK = 3; // RGB bitmap with RGB masks

struct BMPInfoHeader {          // 40 bytes (total)
    longword  size;             // Info header size
    slongword width;            // Width of image
    slongword height;           // Height of image
    word      planes;           // Number of color planes
    word      bits;             // Bits per pixel
    longword  compression;      // Compression type
    longword  imagesize;        // Image size
    slongword xresolution;      // X pixels per meter
    slongword yresolution;      // Y pixels per meter
    longword  ncolors;          // Number of colors
    longword  nimportantcolors; // Number of important colors

    BMPInfoHeader() {
        size             = 40;  // sizeof(BMPInfoHeader)
        width            = 0;
        height           = 0;
        planes           = 1;
        bits             = 24;
        compression      = bmpcompressionRGB;
        imagesize        = 0;
        xresolution      = 5706; // 144dpi
        yresolution      = 5706; // 144dpi
        ncolors          = 0;
        nimportantcolors = 0;
    }
};

class BMP {
public:
    BMP() {}
    ~BMP() {}

    bool SaveYCrCb2RGB(char* path,
                       OFbkImageVectorData* imageVec, OFbkImageLayer layer);

    bool SaveLayerC(char* basepath, OFbkImageVectorData* imageVec);
    bool SaveLayerCmod(char* basepath, OFbkImageVectorData* imageVec, int count);
    bool SaveLayerCcolors(char* basepath, OFbkImageVectorData* imageVec, int count);
    int CountPixel( OFbkImageVectorData* imageVec);
    bool SaveRaw2Gray(char* path,
                      byte* image, int width, int height, int skip);
    bool from_matrix_to_BMP(unsigned int** mat,int count);

	bool creaBMP(unsigned int** matptrR,unsigned int** matptrG,unsigned int** matptrB,int countx) ;

	void SaveBMPHeader(FILE* fp, const BMPHeader& header);
	void SaveBMPInfoHeader(FILE* fp, const BMPInfoHeader& infoheader);

private:
    //
    // Image pixels are ordered B,G,R,B,G,R,... instead of R,G,B,R,G,B,...
    //
    static const int B_PIXEL = 0;
    static const int G_PIXEL = 1;
    static const int R_PIXEL = 2;

    void YCrCb2RGB(byte y, byte cr, byte cb, byte* r, byte* g, byte* b);

    void write_word(FILE* fp, word w);
    void write_longword(FILE* fp, longword l);
    void write_slongword(FILE* fp, slongword sl);
};

#endif // BMP_h_DEFINED
