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

This module contains the JPEG memory manager. This manager uses the jpeg lib 
to compress the
image data and to decompress jpeg data to a given memory buffer.

2 General definitions

*/

/*
defines the memory source manager

*/
GLOBAL(void) jpeg_mem_src (j_decompress_ptr cinfo, 
			   unsigned char *JPEGBuffer, 
			   unsigned long JPEGBufferSize);

/*
defines the memory destination manager

*/
GLOBAL(void) jpeg_mem_dest (j_compress_ptr cinfo, 
			    unsigned char **JPEGBuffer, 
			    unsigned long *JPEGBufferSize);

/*
release buffer allocated by the destination manager

*/
GLOBAL(void) jpeg_finish_mem_dest (j_compress_ptr cinfo);

