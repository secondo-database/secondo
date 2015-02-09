/*
----
This file is part of SECONDO.

Copyright (C) 2014,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

extern "C" {
#include <jinclude.h>
#include <jpeglib.h>
}
#include <stdio.h>


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





unsigned char* getBuffer( char* filename, size_t& size ){
  FILE *f;
  f = fopen(filename, "rb");
  if(f==0){ // file could not be opered
    return 0;
  }
  fseek(f, 0 ,SEEK_END);
  size = ftell(f);
  fseek(f, 0 ,SEEK_SET);
  unsigned char* buffer = new unsigned char[size];
  fread(buffer, size, 1, f);
  fclose(f);
  return buffer;
}

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

METHODDEF(void) mem_term_source (j_decompress_ptr cinfo)
{
  /*
  no work necessary here
  */
}



void jpeg_mem_src (j_decompress_ptr cinfo, 
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
     
      src                        = (mem_src_ptr) cinfo->src;
      src->pub.init_source            = mem_init_source;
      src->pub.fill_input_buffer      = mem_fill_input_buffer;
      src->pub.skip_input_data      = mem_skip_input_data;
      src->pub.resync_to_restart      = jpeg_resync_to_restart;
      src->pub.term_source            = mem_term_source;
      src->src_buffer                  = JPEGBuffer;
      src->src_size                  = JPEGBufferSize;
      src->src_pos                  = 0;
      src->pub.bytes_in_buffer      = 0;
      src->pub.next_input_byte      = NULL;
}


int ReadJPEGHeader(unsigned char *JPEGBuffer, 
                                 unsigned long JPEGBufferSize, 
                                 unsigned long *Width, 
                                 unsigned long *Height, 
                                 bool *GrayScale) {
      jpeg_decompress_struct      cinfo;
      jpeg_error_mgr              errorMgr;

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
      }  else {
         return 1;
      } 

      if (Height != 0)
      {   
            *Height = cinfo.image_height;
      }   else {
         return 2;
      }


      if (GrayScale != 0)
      {   
            *GrayScale = false;
            if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
            {   
                  *GrayScale = true;
            }   
      }   
      return 0;
}


void CreateRGBBuffer(unsigned char *JPEGBuffer, 
                                  unsigned long JPEGBufferSize)
{
      unsigned long                  buffer_pos;
      JSAMPARRAY                        line;
      int                                    row_stride;
      jpeg_decompress_struct      cinfo;
      jpeg_error_mgr                  errorMgr;

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
     
      buffer_pos                  = 0;
      unsigned long m_ulImageBufferSize = cinfo.output_width * 
                                          cinfo.output_height * 
                                         cinfo.output_components;
      unsigned char* m_pucImageBuffer = 
                          new unsigned char[m_ulImageBufferSize + 1]; 
     
      line = (*cinfo.mem->alloc_sarray)  ((j_common_ptr) &cinfo, 
                                          JPOOL_IMAGE, row_stride, 1); 

      while (cinfo.output_scanline < cinfo.output_height)
      {   
            jpeg_read_scanlines(&cinfo, line, 1); 

            // die einzelnen Blöcke speichern
            memcpy((unsigned char*)((unsigned long)m_pucImageBuffer + 
                                     buffer_pos), line[0], row_stride);
            buffer_pos += row_stride;
      }   

      jpeg_finish_decompress(&cinfo);

      jpeg_destroy_decompress(&cinfo);
      delete[] m_pucImageBuffer;
}



int main(int argc , char** argv){

   if(argc < 1){ // wrong number of arguments
      return -1;
   }

   size_t size;
   unsigned char* buffer = getBuffer(argv[1], size);
   if(buffer==0){ // file could not be read
      return 2;
   }
   unsigned long w;
   unsigned long h;
   bool gray;

   ReadJPEGHeader(buffer, size, &w, &h, &gray);

  CreateRGBBuffer(buffer,size);    

  delete[] buffer;

}


