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

2 Includes and other preparations

*/

//#include "StdAfx.h"
extern "C" {
#include <jinclude.h>
#include <jpeglib.h>
}
#include "JPEGPicture.h"
#include "JPEGMemManager.h"
#include "ImageConverter.h"

#include <string.h>

#define RGB_BYTES	3

// Definition of the header structures of the image types
#pragma pack(1)
typedef struct
{
	unsigned char	id_len;
	unsigned char	color_map_typ;
	unsigned char	type;
	short	colormap;
	short	color_map_len;
	unsigned char    color_map_entry_size;
	short	xorigin;
	short	yorigin;
	short	xsize;
	short	ysize;
	unsigned char	bpp;
	unsigned char	descriptor;
} targa_head;


typedef struct
{
	unsigned char	id;
	unsigned char	version;
	unsigned char	compress;
	unsigned char	bpp;
	unsigned short	xmin, ymin, xmax, ymax;
	unsigned short	x_dpi;
	unsigned short	y_dpi;
	unsigned char	colormap[16][3];
	unsigned char	reserved;
	unsigned char	colorplanes;
	unsigned short	linelen;
	unsigned short	paletteinfo;
	unsigned short	xsize;
	unsigned short	ysize;
	unsigned char	fillbytes[54];
} pcx_head;

typedef struct
{
	unsigned short	id;
	int		fsize;
	unsigned short	dummy1,
			dummy2;
	int		offset;
	int		header_size;
	int		width;
	int		height;
	unsigned short	planes;
	unsigned short	bpp;
	int		compression;
	int		image_size;
	int		biXPels;
	int		biYPels;
	int		ColorUsed;
	int		imp_Colors;
} BMP_header;

#pragma pack()

/*

3 Implementation of class ~ImageConverter~

See the documentation of ~ImageConverter.h~ for details on the behaviour
of the methods implemented here.

*/

CImageConverter::CImageConverter(ImageType Type, unsigned char *PictureBuffer, unsigned long PictureSize)
{
	bool rval = false;

	m_pucImageBuffer	= 0;
	m_ulImageSize		= 0;
	m_eType				= Type;

	// load the given type
	switch (Type)
	{
		case IMAGE_JPEG :
			rval = LoadJPG(PictureBuffer, PictureSize);
			break;
		case IMAGE_TGA	:
			rval = LoadTGA(PictureBuffer, PictureSize);
			break;
		case IMAGE_BMP	:
			rval = LoadBMP(PictureBuffer, PictureSize);
			break;
		case IMAGE_PCX	:
			rval = LoadPCX(PictureBuffer, PictureSize);
			break;
	}

	m_bDefined = rval;
}

CImageConverter::~CImageConverter(void)
{
	if (m_pucImageBuffer != 0)
	{
		delete m_pucImageBuffer;
		m_pucImageBuffer = 0;
	}
}

unsigned char *CImageConverter::GetJpegData(unsigned long &Size)
{
	Size = 0;

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

	Size = jpg_buffer_size;

	jpeg_destroy_compress(&cinfo);

	return jpg_buffer;
}

unsigned char *CImageConverter::GetImageData(unsigned long& size)
{
	size = m_ulImageSize;

	return m_pucImageBuffer;
}


void CImageConverter::ConvertToTrueColor(BPP bpp, unsigned char *Buffer, unsigned long BufferSize, unsigned char *Palette)
{
	m_ulImageSize		= m_ulWidth * m_ulHeight * RGB_BYTES;
	m_pucImageBuffer	= new unsigned char[m_ulImageSize];

	// convert the graphic data to 24 bit rgb buffer
	switch (bpp)
	{
		case BPP_8 :
			{
				unsigned long len		= m_ulWidth*m_ulHeight;
				unsigned long src_pos	= 0;
				unsigned long dest_pos	= 0;
				
				// read palette
				for (unsigned long i=0; i<len; i++)
				{
					src_pos = Buffer[i] * 3;

					m_pucImageBuffer[dest_pos++] = Palette[src_pos+2];	// red
					m_pucImageBuffer[dest_pos++] = Palette[src_pos+1];	// green
					m_pucImageBuffer[dest_pos++] = Palette[src_pos];	// blue
				}
			}
			break;
		case BPP_16 :
			{
				unsigned short *buffer = (unsigned short*)Buffer;
				unsigned long size = BufferSize / 2;
				unsigned long pos = 0;
				unsigned char r,g,b;

				// convert from 16 to 24 bit
				for (unsigned long i = 0; i<size; ++i)
				{
					b = (buffer[i]&0x001f)<<3;
					g = (buffer[i]&0x03e0)>>2;
					r = (buffer[i]&0x7c00)>>7;

					m_pucImageBuffer[pos++] = r;
					m_pucImageBuffer[pos++] = g;
					m_pucImageBuffer[pos++] = b;
				}
			}
			break;
		case BPP_24 :
			{
				unsigned char blue;

				// copy buffer
				memcpy(m_pucImageBuffer, Buffer, m_ulImageSize);
				// change red and blue
				for (unsigned long y = 0; y < m_ulImageSize; y += 3)
				{
					blue					= m_pucImageBuffer[y];
					m_pucImageBuffer[y]		= m_pucImageBuffer[y + 2];
					m_pucImageBuffer[y+2]	= blue;
				}
			}
			break;
	}
}

bool CImageConverter::LoadJPG(unsigned char *PictureBuffer, unsigned long PictureSize)
{
	JPEGPicture	*pic = new JPEGPicture(PictureBuffer, PictureSize);

	m_ulWidth	= pic->GetWidth();
	m_ulHeight	= pic->GetHeight();

	unsigned char *buffer = pic->GetImageData(m_ulImageSize);

	m_pucImageBuffer	= new unsigned char[m_ulImageSize];
	memcpy(m_pucImageBuffer, buffer, m_ulImageSize);

	delete pic;

	return true;
}

bool CImageConverter::LoadBMP(unsigned char *PictureBuffer, unsigned long PictureSize)
{
	bool			compressed;
	int				lang;
	unsigned long	y,size;
	unsigned char	*dest_buffer, *ptr;
#define				cachesize	1024
	int				len;
	BMP_header		bmp;
	unsigned char	*src_buffer = PictureBuffer;
	unsigned char	colordepth;
	unsigned char	bytes;

	memcpy(&bmp, src_buffer, sizeof(bmp));
	src_buffer += sizeof(bmp);

	if (bmp.id != 'MB') // ID: BM
	{
		return false;
	}

	compressed = false;
	if (bmp.compression == 1)
	{
		compressed = true;
	}

	m_ulWidth   = bmp.width;
	m_ulHeight	= bmp.height;
	colordepth	= (unsigned char)bmp.bpp;

	// wird nicht unterstützt
	if (bmp.bpp<8)
	{
		return false;
	}

	if (colordepth==8)
		bytes=1;
	else
	if (colordepth<=24)
		bytes=3;
	else
	{
		return false;
	}

	size		= m_ulWidth*m_ulHeight*bytes;
	lang		= m_ulWidth*bytes;
	dest_buffer	= new unsigned char[size];

	if (compressed == false)
	{
		if (bmp.bpp==24)
		{
			len = m_ulWidth*3;
			len = len + ((4-(len&3))&3);
			len -= m_ulWidth*3;

			for (y=0;y<m_ulHeight;y++)
			{
				ptr = (unsigned char *)(dest_buffer + 3* m_ulWidth * (m_ulHeight-y-1));

				memcpy(ptr, src_buffer, m_ulWidth * 3);
				src_buffer += (m_ulWidth * 3);
				src_buffer += len;
			}

			ConvertToTrueColor(BPP_24, dest_buffer, size, 0);
		}
		else
		if (bmp.bpp==8)
		{
			unsigned char	spritepal[1024];
			int				src, dest;

			memset(spritepal, 0, sizeof(spritepal));

			memcpy(spritepal, src_buffer, 4*bmp.ColorUsed);
			src_buffer += (4*bmp.ColorUsed);

			src  = 3;
			dest = 4;
			for (len=1;len<bmp.ColorUsed;len++)
			{
				spritepal[src  ] = spritepal[dest];
				spritepal[src+1] = spritepal[dest+1];
				spritepal[src+2] = spritepal[dest+2];
				src  += 3;
				dest += 4;
			}

			len  = m_ulWidth;
			len  = len + ((4-(len&3))&3);
			len -= m_ulWidth;

			for (y=0;y<m_ulHeight;y++)
			{
				ptr = (unsigned char *)(dest_buffer + m_ulWidth * (m_ulHeight-y-1));
				
				memcpy(ptr, src_buffer, m_ulWidth);
				src_buffer += m_ulWidth;
				src_buffer += len;
			}

			ConvertToTrueColor(BPP_8, dest_buffer, size, spritepal);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	delete dest_buffer;

	return true;
}

bool CImageConverter::LoadTGA(unsigned char *PictureBuffer, unsigned long PictureSize)
{
	targa_head		header;
	bool			compressed;
	int				lang, x, y, counter, color, c;
	unsigned char	*dest_buffer, *ptr;
	bool			mirror;
	unsigned char	spritepal[768];
#define				cachesize	1024
	unsigned char	cache[cachesize];
	int				cachepos;
	unsigned char	*src_buffer = PictureBuffer;
	int				size;
	unsigned char	colordepth;
	unsigned char	bytes;

	memcpy( &header, src_buffer, sizeof(header));

	compressed = (header.type==9 || header.type==10 || header.type==11);
	mirror = ((header.descriptor&32)==32);

	m_ulWidth	= header.xsize;
	m_ulHeight	= header.ysize;
	colordepth	= header.bpp;

	if (colordepth<=8)
		bytes=1;
	else
	if (colordepth<=16)
		bytes=2;
	else
	if (colordepth<=24)
		bytes=3;
	else
	{
		return false;
	}


	src_buffer += (0x12 + header.id_len);

	if (header.color_map_typ==1)	// Palette vorhanden
	{
		lang = header.color_map_entry_size*header.color_map_len>>3;
		memset(spritepal, 0, 768);
		memcpy(spritepal, src_buffer, lang);
		src_buffer += lang;
	}
	else
	{
		if (header.type==3 || header.type==11)	// Graustufenbild
		{
			y = 0;
			for (unsigned char id=0;id<256;id++)
			{
				spritepal[y++] = id;
				spritepal[y++] = id;
				spritepal[y++] = id;
			}
		}
	}

	size		= m_ulWidth*m_ulHeight*bytes;
	lang		= m_ulWidth*bytes;
	dest_buffer	= new unsigned char[size];

	if (compressed==false)
	{
		for (y=0;y<(int)m_ulHeight;y++)
		{
			if (mirror)
				ptr = dest_buffer + y*lang;
			else
				ptr = dest_buffer + (m_ulHeight-y-1)*lang;

			memcpy(ptr, src_buffer, lang);
			src_buffer += lang;
		}
	}
	else
	{
		y=m_ulHeight-1;
		x=0;
		if (mirror)
			ptr = dest_buffer + (m_ulHeight-y-1)*lang;
		else
			ptr = dest_buffer + y*lang;

		memcpy(cache, src_buffer, cachesize);
		src_buffer += cachesize;
		cachepos = 0;

		while (true)
		{
			counter = cache[cachepos++];
			if (cachepos==cachesize)
			{
				memcpy(cache, src_buffer, cachesize);
				src_buffer += cachesize;
				cachepos = 0;
			}

			if (counter>127) // Compressed
			{
				counter &= 0x7f;
				color = 0;

				if (bytes+cachepos>=cachesize)
				{
					memcpy(cache, cache+cachepos, cachesize-cachepos);
					cachepos = cachesize-cachepos;

					memcpy(cache+cachepos, src_buffer, cachesize-cachepos);
					src_buffer += (cachesize-cachepos);

					cachepos = 0;
				}

				memcpy(&color, cache+cachepos, bytes);
				cachepos += bytes;
					
				for (c=0;c<counter+1;c++)
				{
					memcpy( ptr, &color, bytes);
					ptr+=bytes;
					x++;
					if ((unsigned long)x==m_ulWidth)
					{
						x=0;
						y--;

						if (y<0)
							break;
						if (mirror)
							ptr = dest_buffer + (m_ulHeight-y-1)*lang;
						else
							ptr = dest_buffer + y*lang;
					}
				}
			}
			else
			{
				for (c=0;c<counter+1;c++)
				{
					color = 0;

					if (bytes+cachepos>=cachesize)
					{
						memcpy(cache, cache+cachepos, cachesize-cachepos);
						cachepos = cachesize-cachepos;

						memcpy(cache+cachepos, src_buffer, cachesize-cachepos);
						src_buffer += (cachesize-cachepos);

						cachepos = 0;
					}

					memcpy(ptr, cache+cachepos, bytes);
					cachepos += bytes;

					ptr+=bytes;
					x++;
					if ((unsigned long)x==m_ulWidth)
					{
						x=0;
						y--;

						if (y<0)
							break;
						if (mirror)
							ptr = dest_buffer + (m_ulHeight-y-1)*lang;
						else
							ptr = dest_buffer + y*lang;
					}
				}
			}
			if (y<0)
				break;
		}
	}

	if (bytes == 1)
	{
		ConvertToTrueColor(BPP_8, dest_buffer, size, spritepal);
	}
	else
	if (bytes == 2)
	{
		ConvertToTrueColor(BPP_16, dest_buffer, size, 0);
	}
	else
	if (bytes == 3)
	{
		ConvertToTrueColor(BPP_24, dest_buffer, size, 0);
	}

	delete dest_buffer;

	return true;
}

bool CImageConverter::LoadPCX(unsigned char *PictureBuffer, unsigned long PictureSize)
{
	pcx_head		header;
	int				x, y, plane, b, anzahl;
	unsigned char	*ptr;
	int				skip;
	unsigned char	spritepal[768];
	unsigned char	*src_buffer = PictureBuffer;
	unsigned char	*dest_buffer;
	int				size;
	unsigned char	colordepth;
	unsigned char	bytes;


	memcpy( &header, src_buffer, sizeof(header));
	src_buffer += sizeof(header);

	m_ulWidth	= header.xmax - header.xmin + 1;
	m_ulHeight	= header.ymax - header.ymin + 1;


	if ((header.colorplanes!=1 && header.colorplanes!=3) || header.bpp!=8)
	{
		return false;
	}

	if (header.colorplanes==1)
	{
		colordepth = 8;
		bytes = 1;
	}
	else
	{
		colordepth = 24;
		bytes = 3;
	}

	size		= m_ulWidth*m_ulHeight*bytes;
	skip		= header.linelen - m_ulWidth;	// Zeile-Overhead
	dest_buffer	= new unsigned char[size + 1];

	for (y=0;y<(int)m_ulHeight;y++)
	{
		for (plane=0;plane<bytes;plane++)
		{
			ptr = (unsigned char *)dest_buffer + y*m_ulWidth*bytes + (bytes-1) - plane;

			x=0;
			while (x<header.linelen && plane<bytes)
			{
				int	c;

				memcpy(&b, src_buffer, sizeof(unsigned char));
				src_buffer += sizeof(unsigned char);

				c = b&0xc0;

				if ( (b&0xc0)==0xc0)
				{
					anzahl = b & 0x3f;

					memcpy(&b, src_buffer, sizeof(unsigned char));
					src_buffer += sizeof(unsigned char);

					while (anzahl>0)
					{
						if (x<(int)m_ulWidth)
							*ptr = b;
						ptr+=bytes;
						x++;
						anzahl--;
						if (x==header.linelen)
						{
							plane++;
							x = 0;
							ptr = (unsigned char *)dest_buffer + y*m_ulWidth*bytes + (bytes-1) - plane;
						}
					}
				}
				else
				{
					if (x<(int)m_ulWidth)
						*ptr = b;
					ptr+=bytes;
					x++;
				}
			}
		}
	}

	if (colordepth==8) // Palette vorhanden ...
	{
		src_buffer ++;
		memcpy(spritepal, src_buffer, 768);
		src_buffer += 768;

		ptr = spritepal;
		for (x=0;x<256;x++)
		{
			y = ptr[2];
			ptr[2] = ptr[0];
			ptr[0] = y;
			ptr+=3;
		}
	}

	if (colordepth == 8)
	{
		ConvertToTrueColor(BPP_8, dest_buffer, size, spritepal);
	}
	else
	{
		ConvertToTrueColor(BPP_24, dest_buffer, size, 0);
	}

	delete dest_buffer;

	return true;
}
