/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP_VariousBMPutilities.h   *
*    date added: 05-02-2005                      *
* date modified: 12-01-2006                      *
*       version: 1.06                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       * 
*                                                *
* description: Various utilities.                *
*                                                *
*************************************************/

#ifndef _EasyBMP_VariousBMPutilities_h_
#define _EasyBMP_VariousBMPutilities_h_

BMFH GetBMFH( const char* szFileNameIn );
BMIH GetBMIH( const char* szFileNameIn );
void DisplayBitmapInfo( const char* szFileNameIn );
int GetBitmapColorDepth( const char* szFileNameIn );
void PixelToPixelCopy( eBMP& From, int FromX, int FromY,  
                       eBMP& To, int ToX, int ToY);
void PixelToPixelCopyTransparent( eBMP& From, int FromX, int FromY,  
                                  eBMP& To, int ToX, int ToY,
                                  RGBApixel& Transparent );
void RangedPixelToPixelCopy( eBMP& From, int FromL , int FromR, int FromB, int FromT, 
                             eBMP& To, int ToX, int ToY );
void RangedPixelToPixelCopyTransparent( 
     eBMP& From, int FromL , int FromR, int FromB, int FromT, 
     eBMP& To, int ToX, int ToY ,
     RGBApixel& Transparent );
bool CreateGrayscaleColorTable( eBMP& InputImage );

bool Rescale( eBMP& InputImage , char mode, int NewDimension );

#endif
