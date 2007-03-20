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

2 Includes and other preparations

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "stdafx.h"

extern "C" {
#include <jinclude.h>
#include <jpeglib.h>
}
#include "JPEGPicture.h"
#include "JPEGMemManager.h"

#define RGB_BYTES	3

/*

3 Implementation of class ~JPEGPicture~

See the documentation of ~JPEGPicture.h~ for details on the behaviour
of the methods implemented here.

*/

// create object from a JPEG File
JPEGPicture::JPEGPicture(char *filename)
{
	unsigned char	*JPEGBuffer;
	FILE		*fp;
	size_t		size;

	InitMembers();

	fp = fopen(filename, "rb");
	if (fp != 0)
	{
		fseek(fp, 0 ,SEEK_END);
		size = ftell(fp);
		fseek(fp, 0 ,SEEK_SET);

		JPEGBuffer = new unsigned char[size];
		fread(JPEGBuffer, size, 1, fp);
		fclose(fp);

		ReadJPEGHeader(JPEGBuffer, size, &m_ulWidth, &m_ulHeight, &m_bGrayScale);

		CreateRGBBuffer(JPEGBuffer, size);

		delete JPEGBuffer;
	}
}

JPEGPicture::~JPEGPicture()
{
	Destroy();
}

void JPEGPicture::Destroy()
{
	if (m_pucImageBuffer != 0)
	{
		delete m_pucImageBuffer;
		m_pucImageBuffer = 0;
	}
}

// create object from compressed JPEG data provided in buf
JPEGPicture::JPEGPicture(unsigned char *JPEGBuffer, unsigned long JPEGBufferSize)
{
	InitMembers();

	ReadJPEGHeader(JPEGBuffer, JPEGBufferSize, &m_ulWidth, &m_ulHeight, &m_bGrayScale);

	CreateRGBBuffer(JPEGBuffer, JPEGBufferSize);
}

JPEGPicture::JPEGPicture()
{
	InitMembers();
}

void JPEGPicture::InitMembers()
{
	m_pucImageBuffer	= 0;
	m_ulImageBufferSize	= 0;
	m_ulWidth		= 0;
	m_ulHeight		= 0;
	m_bGrayScale		= false;
}

void JPEGPicture::CreateRGBBuffer(unsigned char *JPEGBuffer, unsigned long JPEGBufferSize)
{
	unsigned long			buffer_pos;
	JSAMPARRAY				line;
	int						row_stride;
	jpeg_decompress_struct	cinfo;
	jpeg_error_mgr			errorMgr;

	memset(&cinfo, 0, sizeof(jpeg_decompress_struct));
	memset(&errorMgr, 0, sizeof(jpeg_error_mgr));

	cinfo.err = jpeg_std_error(&errorMgr);

	jpeg_create_decompress(&cinfo);

	// Mem Manager initialisieren
	jpeg_mem_src(&cinfo, JPEGBuffer, JPEGBufferSize);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	// Anzahl der Spalten
	row_stride = cinfo.output_width * cinfo.output_components;
	
	Destroy();

	buffer_pos			= 0;
	m_ulImageBufferSize	= cinfo.output_width * cinfo.output_height * cinfo.output_components;
	m_pucImageBuffer	= new unsigned char[m_ulImageBufferSize + 1];
	
	line = (*cinfo.mem->alloc_sarray)	((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, line, 1);

		// die einzelnen Blöcke speichern
		memcpy((unsigned char*)((unsigned long)m_pucImageBuffer + buffer_pos), line[0], row_stride);
		buffer_pos += row_stride;
	}

	jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);
}

void JPEGPicture::ReadJPEGHeader(unsigned char *JPEGBuffer, unsigned long JPEGBufferSize, unsigned long *Width, unsigned long *Height, bool *GrayScale)
{
	jpeg_decompress_struct	cinfo;
	jpeg_error_mgr			errorMgr;

	memset(&cinfo, 0, sizeof(jpeg_decompress_struct));
	memset(&errorMgr, 0, sizeof(jpeg_error_mgr));

	cinfo.err = jpeg_std_error(&errorMgr);

	jpeg_create_decompress(&cinfo);

	// Mem Manager initialisieren
	jpeg_mem_src(&cinfo, JPEGBuffer, JPEGBufferSize);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_destroy_decompress(&cinfo);

	if (Width != 0)
	{
		*Width = cinfo.image_width;
	}

	if (Height != 0)
	{
		*Height = cinfo.image_height;
	}

	if (GrayScale != 0)
	{
		*GrayScale = false;
		if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
		{
			*GrayScale = true;
		}
	}
}

// scales image to have specified width and height
JPEGPicture* JPEGPicture::scale(unsigned long w, unsigned long h)
{
	unsigned long	len,x,y,new_x,new_y, xoff, yoff, src_pos, dest_pos;
	double			fx,fy,aspect;
	JPEGPicture		*pic;

	if (w<=0 || h<=0)
	{
		aspect = (float)m_ulWidth/(float)m_ulHeight;
		if (w>0)	// Da wurde eine X-Größe angegeben
		{
			new_x = w;
			new_y = (unsigned long)((float)w/aspect);
		}
		else
		{
			new_y = h;
			new_x = (unsigned long)((float)w*aspect);
		}
	}
	else
	{
		new_x=w;
		new_y=h;
	}

	if (new_x<=0)
	{
		new_x = m_ulWidth;
	}

	if (new_y<=0)
	{
		new_y = m_ulHeight;
	}

	fx			= (double)m_ulWidth/(double)new_x;
	fy			= (double)m_ulHeight/(double)new_y;
	len			= w*h;

	// Picture Objekt erzeugen.
	pic						= new JPEGPicture();
	pic->m_ulWidth			= w;
	pic->m_ulHeight			= h;
	pic->m_ulImageBufferSize= (len * RGB_BYTES);
	pic->m_pucImageBuffer	= new unsigned char[pic->m_ulImageBufferSize + 1];

	xoff		= (w-new_x)/2;
	yoff		= (h-new_y)/2;

	// und skalieren
	for ( y=0; y<new_y; y++ )
	{
		for ( x=0; x<new_x; x++ )
		{
			dest_pos = (y+yoff)*(w)+(x+xoff);
			src_pos = (unsigned long)(fx*(double)x)+((int)(fy*(double)y))*m_ulWidth;
			pic->m_pucImageBuffer[dest_pos * RGB_BYTES] = m_pucImageBuffer[src_pos * RGB_BYTES];
			pic->m_pucImageBuffer[(dest_pos * RGB_BYTES) + 1] = m_pucImageBuffer[(src_pos * RGB_BYTES) + 1];
			pic->m_pucImageBuffer[(dest_pos * RGB_BYTES) + 2] = m_pucImageBuffer[(src_pos * RGB_BYTES) + 2];
		}
	}

	return pic;
}

// cut from image so that result fits in bounding box defined
// by parameters
JPEGPicture* JPEGPicture::cut(unsigned long x, unsigned long y, unsigned long w, unsigned long h )
{
	unsigned char	*dest_buffer;
	unsigned long	dest_size, i, j, width, height;
	JPEGPicture		*pic;
	unsigned long	source_pos, dest_pos;

	width		= x + w;
	height		= y + h;
	if (width > m_ulWidth)
	{
		return 0;
	}

	if (height > m_ulHeight)
	{
		return 0;
	}

	// create buffer
	dest_size	= w * h * RGB_BYTES;
	dest_buffer	= new unsigned char[dest_size+1];

	// read image data
	dest_pos = 0;

	for (j = 0; j < h; ++j)
	{
		source_pos	= (((y + j) * m_ulWidth) - m_ulWidth + x) * RGB_BYTES;

		for (i = 0; i < w; ++i)
		{
			// red
			dest_buffer[dest_pos++] = m_pucImageBuffer[source_pos++];
			// green
			dest_buffer[dest_pos++] = m_pucImageBuffer[source_pos++];
			// blue
			dest_buffer[dest_pos++] = m_pucImageBuffer[source_pos++];
		}
	}

	// create Picture object
	pic				= new JPEGPicture();
	pic->m_pucImageBuffer		= dest_buffer;
	pic->m_ulImageBufferSize	= dest_size;

	pic->m_ulWidth			= w;
	pic->m_ulHeight			= h;
	pic->m_bGrayScale		= m_bGrayScale;

	return pic;
}

// turn image left by n*90 degrees (avoid unneccessary
// operations, reduce to n=1,2,3)
JPEGPicture* JPEGPicture::flipleft(unsigned long n)
{
	int			mode = n % 4;
	JPEGPicture	*pic;
	int			x,y,src_pos,dest_pos;

	// create image
	pic = new JPEGPicture();
	pic->m_bGrayScale			= false;
	pic->m_ulImageBufferSize	= m_ulImageBufferSize;
	pic->m_pucImageBuffer		= new unsigned char[m_ulImageBufferSize];

	switch (mode)
	{
		// 0 and 180 °
		case 0 :
		case 2 :
			pic->m_ulWidth	= m_ulWidth;
			pic->m_ulHeight	= m_ulHeight;
			break;

		// 90 and 270 ° 
		case 1 :
		case 3 :
			pic->m_ulWidth	= m_ulHeight;
			pic->m_ulHeight	= m_ulWidth;
			break;
	}

	src_pos	= 0;

	for (y = 0; y < (int)m_ulHeight; ++y)
	{
		for (x = 0; x < (int)m_ulWidth; ++x)
		{
			switch (mode)
			{
				// simple copy
				case 0 :
					dest_pos = src_pos;
					break;
				
				// 90 ° left
				case 1 :
					dest_pos	= (y + ((m_ulWidth - 1 - x) * m_ulHeight)) * RGB_BYTES;
					break;

				// 180 ° left
				case 2 :
					dest_pos	= m_ulImageBufferSize - src_pos - RGB_BYTES;
					break;

				// 270 ° left
				case 3 :
					dest_pos	= (((m_ulHeight - 1) - y) + (x * m_ulHeight)) * RGB_BYTES;
					break;
			}
			
			pic->m_pucImageBuffer[dest_pos]		= m_pucImageBuffer[src_pos];
			pic->m_pucImageBuffer[dest_pos+1]	= m_pucImageBuffer[src_pos+1];
			pic->m_pucImageBuffer[dest_pos+2]	= m_pucImageBuffer[src_pos+2];
			src_pos += RGB_BYTES;
		}
	}

	return pic;
}

// mirror image horizontally, if dir=false, and vertically otherwise
JPEGPicture* JPEGPicture::mirror(bool dir)
{
	JPEGPicture	*pic;
	int			x,y,src_pos,dest_pos;

	// create image
	pic = new JPEGPicture();
	pic->m_bGrayScale			= false;
	pic->m_ulImageBufferSize	= m_ulImageBufferSize;
	pic->m_pucImageBuffer		= new unsigned char[m_ulImageBufferSize];
	pic->m_ulWidth				= m_ulWidth;
	pic->m_ulHeight				= m_ulHeight;

	src_pos	= 0;

	for (y = 0; y < (int)m_ulHeight; ++y)
	{
		if (!dir)
		{
			// horizontal
			dest_pos = ((m_ulHeight - y) * m_ulWidth * RGB_BYTES) - (m_ulWidth * RGB_BYTES);
		}

		for (x = 0; x < (int)m_ulWidth; ++x)
		{
			if (dir)
			{
				// vertical
				dest_pos = ((m_ulWidth * y) + (m_ulWidth - x - 1)) * RGB_BYTES;
			}

			pic->m_pucImageBuffer[dest_pos]		= m_pucImageBuffer[src_pos];
			pic->m_pucImageBuffer[dest_pos+1]	= m_pucImageBuffer[src_pos+1];
			pic->m_pucImageBuffer[dest_pos+2]	= m_pucImageBuffer[src_pos+2];
			src_pos		+= RGB_BYTES;
			dest_pos	+= RGB_BYTES;
		}
	}

	return pic;
}

// returns raw and decompressed image data in format RGBRGBRGB...
// size will be set to size of image data returned by GetImageData()
// in bytes
unsigned char* JPEGPicture::GetImageData(unsigned long& size)
{
	size	= m_ulImageBufferSize;

	return m_pucImageBuffer;
}

// returns compress JPEG data; if possible, try to use quality as
// in originally provided JPEG data; if this is not possible, use
// constant for quality
// size will be set to size of image data returned by GetJpegData()
unsigned char* JPEGPicture::GetJpegData(unsigned long& size)
{
	size = 0;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */
	unsigned char *jpg_buffer = 0;
	unsigned long jpg_buffer_size = 0;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

	jpeg_mem_dest(&cinfo, &jpg_buffer, &jpg_buffer_size);

	cinfo.image_width = m_ulWidth; 	/* image width and height, in pixels */
	cinfo.image_height = m_ulHeight;
	cinfo.input_components = RGB_BYTES;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

	jpeg_set_defaults(&cinfo);

	//  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);


	jpeg_start_compress(&cinfo, TRUE);

	row_stride = m_ulWidth * RGB_BYTES;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
	row_pointer[0] = & m_pucImageBuffer[cinfo.next_scanline * row_stride];
	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	size = jpg_buffer_size;

	jpeg_destroy_compress(&cinfo);

	return jpg_buffer;
}


// return parameters without creating object

// attribute access functions
unsigned long JPEGPicture::GetWidth(unsigned char* JPEGBuffer, unsigned long JPEGBufferSize)
{
	unsigned long width;

	ReadJPEGHeader(JPEGBuffer, JPEGBufferSize, &width, 0, 0);

	return width;
}

unsigned long JPEGPicture::GetHeight(unsigned char* JPEGBuffer, unsigned long JPEGBufferSize)
{
	unsigned long height;

	ReadJPEGHeader(JPEGBuffer, JPEGBufferSize, 0, &height, 0);

	return height;
}

// based on color space defined in JPEG
bool JPEGPicture::IsGrayScale(unsigned char* JPEGBuffer, unsigned long JPEGBufferSize)
{
	bool grayscale;

	ReadJPEGHeader(JPEGBuffer, JPEGBufferSize, 0, 0, &grayscale);

	return grayscale;
}
