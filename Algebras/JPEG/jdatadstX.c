/**
1 Libjpeg-Extension for use with FLOBs - Part 2: Compression

jdatadstX.c

Copyright (C) 1994-1996, Thomas G. Lane.
This file is part of the Independent JPEG Group's software.

eXtended for use with flobs by Ulrich Neumann.

*/

//#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include "LogMsg.h"

// Expanded data destination object for FLOB output
typedef struct {
  struct jpeg_destination_mgr pub;

  FLOB * flob; // destination
  JOCTET * buffer;
  int offset;  // eXt: keep track of buffer-location for FLOB::Put()-call
} my_destination_flob_mgr;

typedef my_destination_flob_mgr * my_dest_flob_ptr;

#define OUTPUT_BUF_SIZE  1024 // FLOB::SWITCH_THRESHOLD should be optimal

static void
init_flob_destination (j_compress_ptr store_info)
{
  my_dest_flob_ptr dest = (my_dest_flob_ptr) store_info->dest;

  dest->buffer = (JOCTET *)
      (*store_info->mem->alloc_small) ((j_common_ptr) store_info, JPOOL_IMAGE,
        OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
  dest->offset = 0;
}

static boolean
empty_toFlob_output_buffer (j_compress_ptr store_info)
{
  // fetch/set parameters
  my_dest_flob_ptr dest = (my_dest_flob_ptr) store_info->dest;

  // write to FLOB ... there shouldn't be more than this in the buffer
  if ((dest->offset + OUTPUT_BUF_SIZE) > dest->flob->Size())
    dest->flob->Resize(dest->offset + OUTPUT_BUF_SIZE);

  assert((dest->offset + OUTPUT_BUF_SIZE) <= dest->flob->Size());
      
  dest->flob->Put(dest->offset, OUTPUT_BUF_SIZE, dest->buffer);

  // change status-info: inc flob position, reset buffer position
  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE ;
  dest->offset = dest->offset + OUTPUT_BUF_SIZE;

  return TRUE;
}

static void
term_flob_destination (j_compress_ptr store_info)
{
  my_dest_flob_ptr dest = (my_dest_flob_ptr) store_info->dest;
  int datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

  if ((dest->offset + datacount) > dest->flob->Size())
    dest->flob->Resize(dest->flob->Size() + datacount);


  // Write any data remaining in the buffer
  if (datacount > 0)
  {
    if ( RTFlag::isActive(JPEG_RT_DEBUG) ) {
      cout << endl << "term_flob_destination(), writing " << datacount
           << " additional bytes." << endl;
    }

    my_dest_flob_ptr dest = (my_dest_flob_ptr) store_info->dest;
    dest->flob->Put(dest->offset, datacount, dest->buffer);

    dest->offset = dest->offset + datacount;
  }
 
  if ( RTFlag::isActive(JPEG_RT_DEBUG) )
    cout << endl << "Correcting flobsize" << endl;

  dest->flob->Resize(dest->offset);
  
  // Make sure we wrote the output file OK
  // => done automatically for flobs ?
}                     

void
jpeg_flob_dest (j_compress_ptr store_info, FLOB * preparedFlob)
{
  my_dest_flob_ptr dest;

  if (store_info->dest == NULL) {
    store_info->dest = (struct jpeg_destination_mgr *)
      (*store_info->mem->alloc_small) ((j_common_ptr) store_info,
        // JPOOL_PERMANENT,
        JPOOL_IMAGE,                 // only needed once
        SIZEOF(my_destination_flob_mgr));
  }

  dest = (my_dest_flob_ptr) store_info->dest;
  dest->pub.init_destination = init_flob_destination;
  dest->pub.empty_output_buffer = empty_toFlob_output_buffer;
  dest->pub.term_destination = term_flob_destination;
    // .. init-values
  dest->flob = preparedFlob;
  dest->offset = 0;
}
