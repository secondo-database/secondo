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

This module contains the JPEG memory manager. This manager uses the jpeg lib to compress the
image data and to decompress jpeg data to a given memory buffer.

2 Includes and other preparations

*/
//#include "stdafx.h"

#include <jinclude.h>
extern "C" {
#include <jpeglib.h>
}
#include <jerror.h>

/* 
Expanded data source object for mem input

*/

typedef struct
{
	struct jpeg_source_mgr pub;	
	/*
	public fields
	*/
	unsigned long src_pos;
	unsigned long src_size;
	unsigned char *src_buffer;
	JOCTET * buffer;
	boolean start_of_file;
} 
mem_source_mgr;

typedef mem_source_mgr * mem_src_ptr;

#define INPUT_BUF_SIZE  4096

/*
 Initialize source --- called by ~jpeg\_read\_header~
 before any data is actually read.

*/

METHODDEF(void) mem_init_source (j_decompress_ptr cinfo)
{
	mem_src_ptr src = (mem_src_ptr) cinfo->src;

/*
  We reset the empty-input-file flag for each image,
  but we don't clear the input buffer.
  This is correct behavior for reading a series of images from one source.

*/
	src->start_of_file = TRUE;
}


/*
 Fill the dest buffer with the next ~INPUT\_BUF\_SIZE~ block from the source 
 buffer

*/
METHODDEF(boolean) mem_fill_input_buffer (j_decompress_ptr cinfo)
{
	mem_src_ptr src = (mem_src_ptr) cinfo->src;
	size_t nbytes;

	nbytes = INPUT_BUF_SIZE;
	if (src->src_pos + INPUT_BUF_SIZE > src->src_size)
	{
		nbytes = src->src_size - src->src_pos;
	}

	memcpy(src->buffer, 
	       (unsigned char*)((unsigned long)src->src_buffer + src->src_pos),
	       nbytes);
	src->src_pos += nbytes;

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}


/*
 Skip data --- used to skip over a potentially large amount of
 uninteresting data (such as an APPn marker).
 
 Writers of suspendable-input applications must note that ~skip\_input\_data~
 is not granted the right to give a suspension return.  If the skip extends
 beyond the data currently in the buffer, the buffer can be marked empty so
 that the next read will cause a ~fill\_input\_buffer~ call that can suspend.
 Arranging for additional bytes to be discarded before reloading the input
 buffer is the application writer's problem.

*/

METHODDEF(void) mem_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  mem_src_ptr src = (mem_src_ptr) cinfo->src;

 /*
   Just a dumb implementation for now.  Could use ~fseek()~ except
   it doesn't work on pipes.  Not clear that being smart is worth
   any trouble anyway --- large skips are infrequent.

 */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) mem_fill_input_buffer(cinfo);
/*
  note we assume that ~fill\_input\_buffer~ will never return ~FALSE~,
  so suspension need not be handled.

*/
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


/*
 An additional method that can be provided by data source modules is the
 ~resync\_to\_restart~ method for error recovery in the presence of RST 
 markers.
 For the moment, this source module just uses the default resync method
 provided by the JPEG library.  That method assumes that no backtracking
 is possible.

*/


/*
 Terminate source --- called by ~jpeg\_finish\_decompress~
 after all data has been read.  Often a no-op.
 
 NB: *not* called by ~jpeg\_abort~ or ~jpeg\_destroy~; surrounding
 application must deal with any cleanup that should happen even
 for error exit.

*/

METHODDEF(void) mem_term_source (j_decompress_ptr cinfo)
{
  /*
  no work necessary here
  */
}


/*
 Prepare for input from a stdio stream.
 The caller must have already opened the stream, and is responsible
 for closing it after finishing decompression.

*/

GLOBAL(void) jpeg_mem_src (j_decompress_ptr cinfo, 
			   unsigned char *JPEGBuffer, 
			   unsigned long JPEGBufferSize)
{
	mem_src_ptr src;

/*
  The source object and input buffer are made permanent so that a series
  of JPEG images can be read from the same file by calling ~jpeg\_stdio\_src~
  only before the first one.  (If we discarded the buffer at the end of
  one image, we'd likely lose the start of the next one.)
  This makes it unsafe to use this manager and a different source
  manager serially with the same JPEG object.  Caveat programmer.

*/
	if (cinfo->src == NULL)
	{	
/*
  first time for this JPEG object?

*/
		cinfo->src = 
		    (struct jpeg_source_mgr *)
		    (*cinfo->mem->alloc_small) (
			(j_common_ptr) cinfo, 
			JPOOL_PERMANENT, 
			SIZEOF(mem_source_mgr));
		
		src = (mem_src_ptr) cinfo->src;
		src->buffer = 
		    (JOCTET *)
		    (*cinfo->mem->alloc_small) (
			(j_common_ptr) cinfo, 
			JPOOL_PERMANENT, 
			INPUT_BUF_SIZE * SIZEOF(JOCTET));
	}
	
	src				= (mem_src_ptr) cinfo->src;
	src->pub.init_source		= mem_init_source;
	src->pub.fill_input_buffer	= mem_fill_input_buffer;
	src->pub.skip_input_data	= mem_skip_input_data;
	src->pub.resync_to_restart	= jpeg_resync_to_restart;
	src->pub.term_source		= mem_term_source;
	src->src_buffer			= JPEGBuffer;
	src->src_size			= JPEGBufferSize;
	src->src_pos			= 0;
	src->pub.bytes_in_buffer	= 0;
	src->pub.next_input_byte	= NULL;
}

/*
Expanded data destination object for stdio output

*/

typedef struct 
{
	struct jpeg_destination_mgr pub;

	unsigned char *dest_buffer;
	unsigned long dest_size;

	unsigned char **ret_buffer;
	unsigned long *ret_size;
	JOCTET * buffer;
}
mem_destination_mgr;

typedef mem_destination_mgr * mem_dest_ptr;

#define OUTPUT_BUF_SIZE  4096


/*
 Initialize destination --- called by ~jpeg\_start\_compress~
 before any data is actually written.

*/

METHODDEF(void) mem_init_destination (j_compress_ptr cinfo)
{
  mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;

 /*
   Allocate the output buffer --- it will be released when done with image

 */
  dest->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
/*
  size of the unpacked picture
  
*/
  int size = cinfo->image_width * cinfo->image_height * 3;

  if (size < OUTPUT_BUF_SIZE)
  {
	  size = OUTPUT_BUF_SIZE;
  }
  dest->dest_buffer = new unsigned char[size];
  dest->dest_size = 0;
}


/*
 Empty the output buffer --- called whenever buffer fills up.
 
 In typical applications, this should write the entire output buffer
 (ignoring the current state of ~next\_output\_byte~ and ~free\_in\_buffer~),
 reset the pointer and count to the start of the buffer, and return TRUE
 indicating that the buffer has been dumped.
 
 In applications that need to be able to suspend compression due to output
 overrun, a ~FALSE~ return indicates that the buffer cannot be emptied now.
 In this situation, the compressor will return to its caller (possibly with
 an indication that it has not accepted all the supplied scanlines).  The
 application should resume compression after it has made more room in the
 output buffer.  Note that there are substantial restrictions on the use of
 suspension --- see the documentation.
 
 When suspending, the compressor will back up to a convenient restart point
 (typically the start of the current MCU). ~next\_output\_byte~ and 
 ~free\_in\_buffer~
 indicate where the restart point will be if the current call returns ~FALSE~.
 Data beyond this point will be regenerated after resumption, so do not
 write it out when emptying the buffer externally.

*/

METHODDEF(boolean) mem_empty_output_buffer (j_compress_ptr cinfo)
{
  mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;

/*
  copies the buffer in the dest buffer

*/
  memcpy((unsigned char*)((unsigned long)dest->dest_buffer + dest->dest_size), 
	 dest->buffer, 
	 OUTPUT_BUF_SIZE);
  dest->dest_size += OUTPUT_BUF_SIZE;

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

  return TRUE;
}


/*
 Terminate destination --- called by ~jpeg\_finish\_compress~
 after all data has been written.  Usually needs to flush buffer.
 
 NB: *not* called by ~jpeg\_abort~ or ~jpeg\_destroy~; surrounding
 application must deal with any cleanup that should happen even
 for error exit.

*/

METHODDEF(void) mem_term_destination (j_compress_ptr cinfo)
{
  mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
  size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

/*
  Write any data remaining in the buffer

*/
  if (datacount > 0)
  {
	memcpy((unsigned char*)((unsigned long)dest->dest_buffer + dest->dest_size), dest->buffer, datacount);
	dest->dest_size += datacount;
  }

  *dest->ret_buffer = dest->dest_buffer;
  *dest->ret_size = dest->dest_size;
}


/*
 Prepare for output to a stdio stream.
 The caller must have already opened the stream, and is responsible
 for closing it after finishing compression.

*/

GLOBAL(void) jpeg_mem_dest (j_compress_ptr cinfo, unsigned char **JPEGBuffer, unsigned long *JPEGBufferSize)
{
  mem_dest_ptr dest;

/*
  The destination object is made permanent so that multiple JPEG images
  can be written to the same file without re-executing ~jpeg\_stdio\_dest~.
  This makes it dangerous to use this manager and a different destination
  manager serially with the same JPEG object, because their private object
  sizes may be different.  Caveat programmer.

*/
  if (cinfo->dest == NULL) {
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(mem_destination_mgr));
  }

  dest = (mem_dest_ptr) cinfo->dest;
  dest->pub.init_destination = mem_init_destination;
  dest->pub.empty_output_buffer = mem_empty_output_buffer;
  dest->pub.term_destination = mem_term_destination;
  dest->ret_buffer = JPEGBuffer;
  dest->ret_size = JPEGBufferSize;
}

GLOBAL(void) jpeg_finish_mem_dest (j_compress_ptr cinfo)
{
	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;

	delete dest->dest_buffer;
}
