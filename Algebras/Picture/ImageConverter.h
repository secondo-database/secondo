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

This module contains functions to load the image data from a given image type
and converts it in a jpeg data buffer.

2 General definitions

*/
#ifndef _CIMAGECONVERTER_H_
#define _CIMAGECONVERTER_H_

// define the type of image
enum ImageType
{
	IMAGE_JPEG,
	IMAGE_TGA,
	IMAGE_BMP,
	IMAGE_PCX,
};

class CImageConverter
{
public:
	/*
	
	create a converted rgb image object

	*/
	CImageConverter(ImageType Type, unsigned char *PictureBuffer, unsigned long PictureSize);
	~CImageConverter(void);

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
	returns if the image could be readed
	*/
	inline bool Defined()
	{
		return m_bDefined;
	}

private:
	/*
	define the pixel type
	*/
	enum BPP
	{
		BPP_8,
		BPP_16,
		BPP_24,
	};

	/*
	converts the given image buffer into a 24 bit rgb buffer
	*/
	void	ConvertToTrueColor(BPP bpp, unsigned char *Buffer, unsigned long BufferSize, unsigned char *Palette);

	/*
	load a Bitmap Image
	*/
	bool	LoadBMP(unsigned char *PictureBuffer, unsigned long PictureSize);
	/*
	load a JPEG Image
	*/
	bool	LoadJPG(unsigned char *PictureBuffer, unsigned long PictureSize);
	/*
	load a Targa Image
	*/
	bool	LoadTGA(unsigned char *PictureBuffer, unsigned long PictureSize);
	/*
	load a PCX Image
	*/
	bool	LoadPCX(unsigned char *PictureBuffer, unsigned long PictureSize);

	ImageType		m_eType;
	unsigned char	*m_pucImageBuffer;
	unsigned long	m_ulImageSize;
	unsigned long	m_ulWidth;
	unsigned long	m_ulHeight;
	bool		m_bDefined;
};

#endif
