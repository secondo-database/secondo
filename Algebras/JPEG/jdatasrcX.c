/******************************************************************************

//[->] [$\rightarrow $]

1 Libjpeg-Extension for use with FLOBs - Part 1: Decompression

  jdatasrc.c  eXtended to jdatasrcX.c  (to be placed in directory JPEG)

  Copyright (C) 2004, University in Hagen, Department of Computer Science, 
  Database Systems for New Applications.

  Copyright (C) 1994-1996, Thomas G. Lane.
  This file is part of the Independent JPEG Group's software.
  For conditions of distribution and use, see the accompanying README file.

  This file contains decompression data source routines for the case of
  reading JPEG data from a file (or any stdio stream).  While these routines
  are sufficient for most applications, some will want to use a different
  source manager.
  IMPORTANT: we assume that fread() will correctly transcribe an array of
  JOCTETs from 8-bit-wide elements on external storage.  If char is wider
  than 8 bits on your machine, you may need to do some tweaking.

*/

/*
(C-left) January 2004, Ulrich Neumann (eXt)

eXtended for supporting buffers (not well testet nor well implemented)
and for Secondo-FLOBs (expected to work without bugs) as datasources
instead of the (OS-) file-stream originally expected by jpeg\_stdio\_src

original fn()s: jpeg\_stdio\_src() and related (for comparison)
buffer-version: jpeg\_buffer\_src() using same fn()s for stdio\_src
flob-version:   jpeg\_flob\_src() using adapted fn()s

*/

// this is not a core library module,  so it doesn't define JPEG_INTERNALS

// unneccessary to include jinclude.h only for the following 3 defines:
#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)   ((size_t)  \
  fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

#define SIZEOF(object)	((size_t) sizeof(object))


//          jinclude: see 3 defines above.
//#include "jinclude.h"
// (standard include directory, $HOME/secondo-sdk/include:)
#include "jpeglib.h"
#include "jerror.h"


// eXt:
// INPUT_BUF_SIZE  is a configuration issue: possible big performance
// improvements  with values  adapted to FLOB::Get()  (which ones ???)
// => to do: experimenting  INPUT_BUF_SIZE <-> Image-Parse-Performance

#define INPUT_BUF_SIZE  1024

// INPUT_BUF_SIZE :
// possibly take pageSize of FLOB, which is most efficient, ...
// ... if the image is bigger then FLOB::THRESHOLD == actually 1024

// 4096  is the original value for fill_input_buffer() from TH. G. Lane

// choose an efficiently fread'able size  (Lane)
//                       FLOB.Get_able size


// Expanded data source object for stdio input (Lane)

typedef struct {
  //struct
  jpeg_source_mgr pub; // public fields

  FILE * infile;		// source stream
  JOCTET * buffer;		// start of buffer
  boolean start_of_file;	// have we gotten any data yet?
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;


// (eXt) eXtension for FLOB:

typedef struct {
  //struct
  jpeg_source_mgr pub;	// public fields

  FLOB * fromFLOB;		// source 'stream'
  JOCTET * buffer;		// start of buffer
  boolean start_of_file;	// have we gotten any data yet?
                          // eXt: attr. not really needed. for compatibility.
                          // instead:
  size_t offset;  // eXt: keep track of buffer-location for FLOB::Get()-call

} my_source_flob_mgr;

typedef my_source_flob_mgr * my_src_flob_ptr;


// Initialize source --- called by jpeg_read_header
// before any data is actually read.   (Lane)

static void
init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  // We reset the empty-input-file flag for each image,
  // but we don't clear the input buffer.
  // This is correct behavior for reading a series of images from one source.
  //
  src->start_of_file = TRUE;
}



// eXt:
// Initialize source --- called by jpeg_read_header
// before any data is actually read.   (Lane)

static void
init_source_flob (j_decompress_ptr cinfo)
{
  // (eXt) (as in init_source) the following cast works because
  // my_source_(flob_)mgr starts with a jpeg_source_mgr-Attribute

  my_src_flob_ptr src = (my_src_flob_ptr) cinfo->src;

  // "We reset the empty-input-file flag for each image,
  // but we don't clear the input buffer. " -> eXt: bytes of last FLOB
  // read remain in the buffer allocated (outside the FLOB) by libjpeg.
  // "This is correct behavior for reading a series of images from one source."
  // -> eXt: FLOB can be read several times ...
  //
  src->start_of_file = TRUE;
  src->offset = 0; // first byte of FLOB  is the next to fetch with FLOB::Get(...)
}


/**
(Lane:)
Fill the input buffer --- called whenever buffer is emptied.

In typical applications, this should read fresh data into the buffer
(ignoring the current state of next\_input\_byte and bytes\_in\_buffer),
reset the pointer and count to the start of the buffer, and return TRUE
indicating that the buffer has been reloaded.  It is not necessary to
fill the buffer entirely, only to obtain at least one more byte.

There is no such thing as an EOF return.  If the end of the file has been
reached, the routine has a choice of ERREXIT() or inserting fake data into
the buffer.  In most cases, generating a warning message and inserting a
fake EOI marker is the best course of action --- this will allow the
decompressor to output however much of the image is there.  However,
the resulting error message is misleading if the real problem is an empty
input file, so we handle that case specially.

In applications that need to be able to suspend compression due to input
not being available yet, a FALSE return indicates that no more data can be
obtained right now, but more may be forthcoming later.  In this situation,
the decompressor will return to its caller (with an indication of the
number of scanlines it has read, if any).  The application should resume
decompression after it has loaded more data into the input buffer.  Note
that there are substantial restrictions on the use of suspension --- see
the documentation.

When suspending, the decompressor will back up to a convenient restart point
(typically the start of the current MCU). next\_input\_byte and bytes\_in\_buffer
indicate where the restart point will be if the current call returns FALSE.
Data beyond this point must be rescanned after resumption, so move it to
the front of the buffer rather than discarding it.

*/

static boolean
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

  nbytes = JFREAD(src->infile, src->buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0) {
    if (src->start_of_file)	// Treat empty input file as fatal error
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}


/**
1.1 Essential Method for Performance

(eXt)  -- this is the essential fn() for performance improovements.

*/
static boolean
fill_input_buffer_fromFLOB (j_decompress_ptr cinfo)
{
  my_src_flob_ptr src = (my_src_flob_ptr) cinfo->src;
  size_t nbytes;
  int input_buf_size = INPUT_BUF_SIZE; // just for seeing it in GNU-Debugger

  // to do: assure offset + INPUT_BUF_SIZE <= FLOB::Size()
  size_t numFlobBytes = src->fromFLOB->Size();
  assert(src->offset <= numFlobBytes);

  // read from FLOB
  if ((src->offset + input_buf_size) > numFlobBytes)
  {
    src->fromFLOB->Get(src->offset, numFlobBytes - src->offset, src->buffer);
        // not - (offset - 1), since buffer starts with 0
   nbytes = numFlobBytes - src->offset;
    // Caution: FLOB::Get() actually DOESN'T check the
                             // NUMBER of bytes really copied ...
  }
  else
  {  src->fromFLOB->Get(src->offset, input_buf_size ,src->buffer);
     nbytes = input_buf_size;
  }

  // update offset to read next time from FLOB
  src->offset += nbytes;

  // (eXt)     nbytes < 0 ??? =>
  // error in FLOB::Get()? ...actually never occurs: Get assumes ALL bytes
  // correctly memcpy'd, if none of its assertions fail. one of them asserts:
  // enough bytes left.     *** But nbytes may be 0. *** => wrong usage?
  if (nbytes <= 0) {
    if (src->start_of_file)	/* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);

    WARNMS(cinfo, JWRN_JPEG_EOF);

    // Insert a fake EOI marker  (Lane, working with stream!)

    // eXt: possibly not used, but not removed for compatibility
    // to do: own resync_to_start-fn(), which obsoltes following lines.
    // ... but on the other side we should never arrive here!

    src->buffer[0] = (JOCTET) 0xFF;       // this is set in buffer,
    src->buffer[1] = (JOCTET) JPEG_EOI;   // not in the FLOB! (eXt)
    nbytes = 2;                           // ... not used ... return false!

    // nothing to resume, since FLOB is NOT a stream which could
    // be waiting  but   IF nbytes <= 0  nothing more can be got: Get()-Bug
    cerr << "fill_input_buffer_fromFLOB(): nbytes = " << nbytes
         << " src->pub.unread_marker = " << cinfo->unread_marker
         << " ... returning false instead of letting resynch try." << endl;

    assert(FALSE);  // eXt: don't try to correct error which
  }
  // not any error occurred

  src->pub.next_input_byte = src->buffer; // buffer[0], since newly filled
  src->pub.bytes_in_buffer = nbytes; // we could also use count == FLOB::Size()
                                     // instead of INPUT_BUF_SIZE
  src->start_of_file = FALSE;

  return TRUE;
}


/**
(Lane:)
Skip data. Used to skip over a potentially large amount of
uninteresting data (such as an APPn marker).

Writers of suspendable-input applications must note that skip\_input\_data
is not granted the right to give a suspension return.  If the skip extends
beyond the data currently in the buffer, the buffer can be marked empty so
that the next read will cause a fill\_input\_buffer call that can suspend.
Arranging for additional bytes to be discarded before reloading the input
buffer is the application writer's problem.

*/

static void
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  // Just a dumb implementation for now.  Could use fseek() except
  // it doesn't work on pipes.  Not clear that being smart is worth
  // any trouble anyway --- large skips are infrequent.
  //
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      // note we assume that fill_input_buffer will never return FALSE,
      // so suspension need not be handled.
      //
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


// (eXt) eXtension for FLOB: should be faster then Lane, since
//       seeking of position is possible.

static void
skip_input_flob_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_flob_ptr src = (my_src_flob_ptr) cinfo->src;

  // eXt: cf. comment in skip_input_data() :
  // "Just a dumb implementation for now.  Could use fseek() except
  // it doesn't work on pipes.  Not clear that being smart is worth
  // any trouble anyway --- large skips are infrequent."
  //
  // eXt: no fseek since no stream. Meantheless no simple dump. Large
  // skips also treated efficiently.
  //

  assert(num_bytes > 0); // if caller wants to skip a non-positive number of
                         // bytes then it's HIS/HER code to be investigated!

  if (num_bytes < (long)src->pub.bytes_in_buffer) // only 'inside-buffer-jump'
  {
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
    // src->offset remains same, since skipped bytes allready loaded
  }
  else // 'beyond-buffer-jump' to a FLOB-position not yet loaded
  {
    // in FLOB source skip only FLOB-bytes not yet loaded to buffer
    // (may be += 0, if skipped exactly the remaining buffer-bytes)
    src->offset += num_bytes - src->pub.bytes_in_buffer;

    // fetch the next bytes not skipped
    assert( fill_input_buffer_fromFLOB(cinfo)); //caller shouldn't skip beyond!

    // now: buffer ([0...]) filled from the first byte after the skipped ones

    // src->pub.next_input_byte   and   src->pub.bytes_in_buffer  ...
    // ... allready set by fill_input_buffer_fromFLOB
  }
}


//   R e s y n c h r o n i z a t i o n   f r o m   M a r k e r E r r o r s  ???

/**
(Lane:)
An additional method that can be provided by data source modules is the
resync\_to\_restart method for error recovery in the presence of RST markers.
For the moment, this source module just uses the default resync method
provided by the JPEG library.  That method assumes that no backtracking
is possible.

*/

/**
eXt:
Backtracking IS possible with FLOBs. But how to understand a RST marker found??
Since FLOB, this would not be a time-caused stream-error, but instead corrupt
data in the picture itself...  Really try to repair?

*/

/*
(eXt:) following the comment of (Lane)  regarding the default resynch\_
       to\_restart()   copied from  jdmarker.c
       Below in jpeg\_flob\_src we set up a dummy-failure-exit() instead.

*/
/**
(Lane:)
This is the default resync\_to\_restart method for data source managers
to use if they don't have any better approach.  Some data source managers
may be able to back up, or may have additional knowledge about the data
which permits a more intelligent recovery strategy; such managers would
presumably supply their own resync method.

read\_restart\_marker calls resync\_to\_restart if it finds a marker other than
the restart marker it was expecting.  (This code is *not* used unless
a nonzero restart interval has been declared.)  cinfo[->]unread\_marker is
the marker code actually found (might be anything, except 0 or FF).
The desired restart marker number (0..7) is passed as a parameter.
This routine is supposed to apply whatever error recovery strategy seems
appropriate in order to position the input stream to the next data segment.
Note that cinfo[->]unread\_marker is treated as a marker appearing before
the current data-source input point; usually it should be reset to zero
before returning.
Returns FALSE if suspension is required.

This implementation is substantially constrained by wanting to treat the
input as a data stream; this means we can't back up.  Therefore, we have
only the following actions to work with:
  1. Simply discard the marker and let the entropy decoder resume at next
     byte of file.
  2. Read forward until we find another marker, discarding intervening
     data.  (In theory we could look ahead within the current bufferload,
     without having to discard data if we don't find the desired marker.
     This idea is not implemented here, in part because it makes behavior
     dependent on buffer size and chance buffer-boundary positions.)
  3. Leave the marker unread (by failing to zero cinfo[->]unread\_marker).
     This will cause the entropy decoder to process an empty data segment,
     inserting dummy zeroes, and then we will reprocess the marker.

2. is appropriate if we think the desired marker lies ahead, while 3. is
appropriate if the found marker is a future restart marker (indicating
that we have missed the desired restart marker, probably because it got
corrupted).
We apply 2. or 3. if the found marker is a restart marker no more than
two counts behind or ahead of the expected one.  We also apply 2. if the
found marker is not a legal JPEG marker code (it's certainly bogus data).
If the found marker is a restart marker more than 2 counts away, we do 1.
(too much risk that the marker is erroneous; with luck we will be able to
resync at some future point).
For any valid non-restart JPEG marker, we apply 3.  This keeps us from
overrunning the end of a scan.  An implementation limited to single-scan
files might find it better to apply 2. for markers other than EOI, since
any other marker would have to be bogus data in that case.

*/

// (eXt): no default resynch-method ... with flobs
//        'eXtension': if resynching required then not the image or
//        time is to be debugged, but the source code, either ours
//        or Lanes. =>

boolean
jpeg_flob_no_resync (j_decompress_ptr cinfo, int desired)
{
  // Always put up a warning.
  // WARNMS2(cinfo, JWRN_MUST_RESYNC, marker, desired);
  cerr << "jpeg_flob_no_resynch(): if code arrives here, debug it.";

  assert(false);
  return true;  // pro forma ... you may continue but you can't
}




/**
(Lane:)
Terminate source --- called by jpeg\_finish\_decompress
after all data has been read.  Often a no-op.

NB: *not* called by jpeg\_abort or jpeg\_destroy; surrounding
application must deal with any cleanup that should happen even
for error exit.

*/

static void
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here (Lane) */
}



// (eXt) (not-yet-)eXtension

static void
term_source_flob (j_decompress_ptr cinfo)
{
  // For only-one-time read of FLOB and no further use, we could
  // help Secondo-Cache-Manager by calling :
  //

  //  scr->fromFLOB->SaveToLob(); // if Size() > THRESHOLD

}


/**
(Lane)     (Lanes main fn()  for  preparing  the parsing process )

Prepare for input from a stdio stream.
The caller must have already opened the stream, and is responsible
for closing it after finishing decompression.

*/

void
jpeg_stdio_src (j_decompress_ptr cinfo, FILE * infile)
{
  my_src_ptr src;

  // The source object and input buffer are made permanent so that a series
  // of JPEG images can be read from the same file by calling jpeg_stdio_src
  // only before the first one.  (If we discarded the buffer at the end of
  // one image, we'd likely lose the start of the next one.)
  // This makes it unsafe to use this manager and a different source
  // manager serially with the same JPEG object.  Caveat programmer.
  //
  if (cinfo->src == NULL) { // first time for this JPEG object?
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  INPUT_BUF_SIZE * SIZEOF(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read
  src->pub.next_input_byte = NULL; // until buffer loaded
}


/**
1.2 libjpeg-decompression-eXtension:  m a i n  method

(eXt:) eXtension  m a i n  method: prepare for parsing the FLOB bytes

Prepare for piece-wise input of bytes from FLOB.
FLOB must be valid: FLOB::Get(0,..) must return more than 0 Bytes.
FLOB must not be destructed as long as used by src. caveat: src is able to
read FLOB more than once by resetting its read-counters.

Receiving piece by piece may be an advantage over all-once-approach
in jpeg\_buffer\_src(), especially with very, very large JPEG-pictures.

*/

void
jpeg_flob_src (j_decompress_ptr cinfo, FLOB * flob)
{
  my_src_flob_ptr src;
  int input_buf_size = INPUT_BUF_SIZE;

  // "The source object and input buffer are made permanent so that a series
  // of JPEG images can be read from the same file by calling jpeg_stdio_src
  // only before the first one.  (If we discarded the buffer at the end of
  // one image, we'd likely lose the start of the next one.)
  // This makes it unsafe to use this manager and a different source
  // manager serially with the same JPEG object.  Caveat programmer."
  //
  // eXt: ?These problems should only emerge with streams? ?Is Problem, exactly
  // stated == 'loss of stream-bytes arriving DURING replacement of buffer'? ??
  //
  // It seems so. But there is no time-CAUSED loss in the source, if a FLOB.
  // There may only be a loss OF time if FLOB was removed from main memory, and
  // Get() must first bring it up. But FLOB::Get() and caller are on same
  // thread. ... => caveat does not apply.
  //
  if (cinfo->src == NULL) { // first time for this JPEG object?

    // (Permanently) allocate and set Source-Manager-Struct
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_source_flob_mgr));
    src = (my_src_flob_ptr) cinfo->src;

    // (Permanently) allocate buffer receiving the FLOB-copy piece-wise.
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  input_buf_size * SIZEOF(JOCTET));
  }

  // Set init-values and user-defined (= our) functions
  src = (my_src_flob_ptr) cinfo->src; // (redundancy needed for first call)
  // .. user-fns()
  src->pub.init_source = init_source_flob;
  src->pub.fill_input_buffer = fill_input_buffer_fromFLOB;
  src->pub.skip_input_data = skip_input_flob_data;
  // to do for next call: improove default function ??? see comment above:
  // "Backtracking ...". next call equal to default jpeg_resync_to_restart
  src->pub.resync_to_restart = jpeg_flob_no_resync;
                                                    //
                          // = ; // default fn()
  src->pub.term_source = term_source_flob;
  // .. init-values
  src->fromFLOB = flob;
  src->pub.bytes_in_buffer = 0; // same as in jpeg_stdio_src: fill_input_buffer
                                // at start
  src->pub.next_input_byte = NULL; // same as in jpeg_stdio_src:
                                   // not yet initialized
}


// (eXt) quick and dirty  solution  with intermediate buffer.  deprecated.
//
//          ... simply replaces stream from jpeg_stdio_src by the ready buffer.
//             not a very proper solution to the 'workaround' problem, neither:
//( this is for the C-style solution; C++-supported solution: jpeg_flob_src() )

void
jpeg_buffer_src (j_decompress_ptr cinfo, char* bytes, size_t count)
{
  my_src_ptr src;

  // inpropernesses (1): still permanent JPool
  //                (2): explizit JOCTED->char cast
  //                (3): unused / unusefull fn()s
  //                (4): no appropriate failure mechanisms
  //                (5): whole array bytes... no streaming for very large bytes
  // shorty tested with infile = 0  and  infile = stdin: it works both.

  if (cinfo->src == NULL) { // first time for this JPEG object?
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  SIZEOF(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET*) bytes;
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = 0;// fill_input_buffer; will not be called with
                                 // bytes[count]      !!! it is crucial to set
                                 // bytes_in_buffer <= count !!!
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method;
                                                    // never used if bytes ok.
  src->pub.term_source = term_source;
  src->infile = 0;// since fill_input_buffer not called => stream not utilized
  src->pub.bytes_in_buffer = count; // > 0 inhibits fill_input_buffer on first
                                    // read == 0  should crash (not tested)
  src->pub.next_input_byte = (JOCTET*) &(bytes[0]);
}
