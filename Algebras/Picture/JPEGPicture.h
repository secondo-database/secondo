/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[TOC] [\tableofcontents]

[1] Picture Algebra: Class Definitions

Dezember 2004 Christian Bohnbuck, Uwe Hartmann, Marion Langen and Holger 
M[ue]nx during Prof. G[ue]ting's practical course 
'Extensible Database Systems' at Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module contains the raw JPEG routines like width, height and functions
to manipulate the Image like cut, scale, mirror and flipleft.

2 General definitions

*/

#ifndef _JPEGPICTURE_H_

#define _JPEGPICTURE_H_



class JPEGPicture

{

public:

	/*
	create object from a JPEG File
	*/
	JPEGPicture(char *filename);

	/*
	create object from compressed JPEG data provided in buf
	*/
	JPEGPicture(unsigned char *JPEGBuffer, unsigned long JPEGBufferSize);



	~JPEGPicture();


	/*
	scales image to have specified width and height
	*/
        JPEGPicture* scale(unsigned long w, unsigned long h);



	/*
	cut from image so that result fits in bounding box defined
	by parameters
	*/
        JPEGPicture* cut(unsigned long x,

	                 unsigned long y,

			 unsigned long w,

			 unsigned long h );



	/*
	turn image left by n*90 degrees (avoid unneccessary
	operations, reduce to n=1,2,3)
	*/

        JPEGPicture* flipleft(unsigned long n);



	/*
	mirror image horizontally, if dir=false, and vertically otherwise
	*/
        JPEGPicture* mirror(bool dir);



	/*
	returns raw and decompressed image data in format RGBRGBRGB...
	size will be set to size of image data returned by GetImageData()
	in bytes
	*/

	unsigned char* GetImageData(unsigned long& size);



	/*
	returns compress JPEG data; if possible, try to use quality as
	in originally provided JPEG data; if this is not possible, use
	constant for quality
	size will be set to size of image data returned by GetJpegData()
	*/
	unsigned char* GetJpegData(unsigned long& size);





	/*
	return parameters without creating object
	attribute access functions
	*/
        static unsigned long GetWidth(unsigned char* JPEGBuffer, unsigned long JPEGBufferSize);

        static unsigned long GetHeight(unsigned char* JPEGBuffer, unsigned long JPEGBufferSize);



	/*
	based on color space defined in JPEG
	*/
        static bool IsGrayScale(unsigned char* JPEGBuffer, unsigned long JPEGBufferSize);



	/*
	attribute access functions
	*/


	/*
	gets the Width of the Object
	*/
        inline unsigned long GetWidth()

	{

		return m_ulWidth;

	}



	/*
	gets the Height of the Object
	*/
	inline unsigned long GetHeight()

	{

		return m_ulHeight;

	}



	/*
	based on color space defined in JPEG
	return true if the Image is a gray scale
	*/
        inline bool IsGrayScale()

	{

		return m_bGrayScale;

	}



private:

	JPEGPicture();


	/*
	reads the JPEG Header Information from the buffer
	*/
	static void ReadJPEGHeader(	unsigned char *JPEGBuffer, unsigned long JPEGBufferSize,

					unsigned long *Width,

					unsigned long *Height,

					bool *GrayScale);

	/*
	decompress the JPEG Image
	*/
	void	CreateRGBBuffer(unsigned char *JPEGBuffer, unsigned long JPEGBufferSize);

	/*
	Initalize the member variables
	*/
	void	InitMembers();

	/*
	destroy the object
	*/
	void	Destroy();



	/*
	uncompressed rgb Image buffer of the JPEG Object
	*/

	unsigned char	*m_pucImageBuffer;

	/*
	size of the uncompressed Image buffer
	*/
	unsigned long	m_ulImageBufferSize;


	/*
	Width of the Image
	*/
	unsigned long	m_ulWidth;

	/*
	Height of the Image
	*/
	unsigned long	m_ulHeight;

	/*
	Flag for grayscale Image
	*/
	bool		m_bGrayScale;



};



#endif

