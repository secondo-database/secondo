/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] MP3 Algebra

December 2003 R. Bozic, D. Riewenherm, M. Guenster

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

This algebra provides operators which deals with MP3, ID3 and Lyrics data.
Although an MP3 file contains ID3 and Lyrics this algebra provides three 
type constructors to be able to handle these informations seperated from
mp3s.

i) mp3

ii) id3

iii) lyrics

Furthermore it provides the following operators:

i) ~savemp3to~ - saves an MP3 object into an MP3 file.

ii) ~version~ - returns the MP3 version.

iii) ~frequency~ - returns the used frequency of an MP3.

iv) ~framecount~ - returns the number of frames in an MP3.

v) ~length~ - returns the length of a song in seconds.

vi) ~bitrate~ - returns the bitrate of an MP3.

vii) ~concatmp3~ - concats two MP3 objects.

viii) ~submp3~ - returns a subset of an M3 object.

ix) ~putid3~ - put a new ID3 tag into an MP3.

x) ~removeid3~ - removes an ID3 tag from an MP3.

xi) ~getid3~ - get the ID3 tag from an MP3.

xii) ~author~ - returns the author of an ID3.

xiii) ~titleof~ - returns the title of an ID3.

xiv) ~album~ - returns the album of an ID3.

xv) ~comment~ - returns the comment of an ID3.

xvi) ~track~ - returns the track number of an ID3 (V1.1)

xvii) ~songyear~ - returns the year of an ID3.

xviii) ~genre~ - returns the genre of an ID3.

xix) ~words~ - returns the lyrics line at a given point a time.

xx) ~getlyrics~ - returns the lyrics from an MP3.

xxi) ~putlyrics~ - put lyrics into an MP3.

xxii) ~removelyrics~ - remove a lyrics into an MP3.

[TOC]

1 Defines and includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "FLOB.h"
#include "DBArray.h"
#include "Base64.h"
#include <string.h>
#include <stdio.h>

extern NestedList* nl;
extern QueryProcessor *qp;

/*
2 Type Constructor ~mp3~

2.1 Class ~MP3~

*/
class MP3 : public StandardAttribute {
  public:
    /* Constructor. Initalizes the DBArray mp3Data. */
    MP3() {};
    MP3( const int size );
    /* Destructor. Destroys the DBArray. */
    ~MP3();
    /* This object can be deleted by the Secondo system. */
    void Destroy();
    /* Returns whether this object is defined or not. */
    bool IsDefined() const;
    /* Sets this object as defined or undefined. */
    void SetDefined( bool Defined);
    /* Calcules a hash value for an MP3. */
    size_t HashValue() const;
    /* Copy the data of another MP3 object into this object. */
    void CopyFrom(const StandardAttribute* right);
    /* Compare this MP3 with another MP3 object. */
    int Compare(const Attribute * arg) const;
    /* Adjacent is not useful for MP3 */
    bool Adjacent(const Attribute * arg) const;
    /* Clones this MP3 object. */
    MP3* Clone() const;
    /* Prints a textual representation of an MP3 file. */
    ostream& Print( ostream &os ) const;
    /* returns the number of FLOBs. Always 1. */
    int NumOfFLOBs() const;
    /* Get the FLOB (DBArray) */
    FLOB *GetFLOB(const int i);
    /* Returns the size of a class instance */
    size_t Sizeof() const;
    /* Returns a new MP3 object which contains the frames with indices 
       between begin and begin + size */
    MP3* SubMP3(int begin, int size) const;
    /* Returns the concatination of this MP3 and the other MP3 object. */
    MP3* Concat (const MP3* other) const;
    /* This methods writes the Base64 string of this MP3 Object into
       the string textBytes. This string textBytes must be provided by
       the caller. */
    void Encode( string& textBytes ) const;
    /* Gets the Base64 string and reconstruct the nessesary
       DBArray for our MP3 song. Afterwards some attributes 
       of the MP3 song will be recalculated. */
    void Decode( const string& textBytes );
    /* Saves this MP3 song into the file with name filename. */
    bool SaveMP3ToFile( const char *fileName ) const;
    /* Returns whether this MP3 has an ID3 tag. */
    bool ExistsID() const;
    /* Removes the ID3 tag from this MP3 if exists. */
    void RemoveID();
    /* Stores a new ID3 tag into this mp3. An older version will be
       replaced. */
    void PutID(char *iddump);
    /* Returns whether this MP3 has lyrics. */
    bool ExistsLyrics() const;
    /* Returns the beginning of the first frame of the MP3
       which is in the buffer. */
    int FindFirstHeader(const char *buffer,int size) const;
    /* Returns the beginning of the next frame of the MP3 
       which is in the buffer. The beginning of the previous
       frame has to be provided in prevHeader. */
    int FindNextHeader(int prevHeader, const char *buffer, int size) const;
    /* Returns the length of the frame which begins at position
       prevHeader in bytes. */
    int FrameLength (int prevHeader, const char* buffer) const;
    /* Calculates the MP3 version of the song which is in
       the buffer. */
    int CalcMP3Version(int Header, const char *buffer) const;
    /* Calculates the bitrate of the song which is in
       the buffer. */
    int CalcBitrate(int Header, const char *buffer) const;
    /* Calculates the frequency of the song which is in
       the buffer. */
    int CalcFrequency(int Header, const char *buffer) const;
    /* Stores the ID3 dump (128 bytes) into buffer.
       The buffer has to be provided by the caller. */
    void GetID3Dump (const char **buffer) const;
    /* Stores the Lyrics dump into a buffer. The size
       of the lyrics is unknown before and is returned
       in the output argument ~lyricssize~. */
    void GetLyricsDump (const char **buffer, int &size) const;
    /* Removes the lyrics from this object if Lyrics exists. */
    void RemoveLyrics();
    /* Stores the given lyrics into this MP3 object. 
       If this object has already lyrics this lycris will be
       overwritten. */
    void PutLyrics(char *lyricsdump, int size);
    /* Returns the MP3 version of this object. (MPEG1 or MPEG2) */
    int GetMP3Version() const;
    /* Returns the bitrate of this MP3. */
    int GetBitrate() const;
    /* Returns the frequency of this MP3. */
    int GetFrequency() const;
    /* Returns the number of frames of this MP3. */
    int GetFrameCount() const;
    /* Returns the length of this MP3 in seconds. */
    int GetLength() const;

  private:
    FLOB mp3Data;
    int version;
    int bitrate;
    bool canDelete;
    int length;
    int framecount;
    int frequency;
    bool defined;
};

/* 
2.1.1 Constructor. 

Initalizes the DBArray mp3Data. 

*/
MP3::MP3(const int size) : mp3Data(size), canDelete(false) {
    // empty.
}

/* 
2.1.2 Destructor. 

Destroys the DBArray. 

*/
MP3::~MP3() {
    if( canDelete ) {
        mp3Data.Destroy();
    }
}

/* 
2.1.3 Destroy

This object can be deleted by the Secondo system. 

*/
void MP3::Destroy() {
    canDelete = true;
}

/* 
2.1.4 IsDefined

Returns whether this object is defined or not. 

*/
bool MP3::IsDefined() const {
    return defined;
}

/* 
2.1.5 SetDefined

Sets this object as defined or undefined. 

*/
void MP3::SetDefined( bool def) {
    defined = def;
}

/* 
2.1.7 HashValue

Calcules a hash value for an MP3. 

*/
size_t MP3::HashValue() const {
    if (!defined)
        return 0;
    else
        return mp3Data.Size();
}

/* 
2.1.8 CopyFrom

Copy the data of another MP3 object into this object. 

*/
void MP3::CopyFrom(const StandardAttribute* right) {
    const MP3 *r = (const MP3 *)right;

    if (r->mp3Data.Size() > 0){
        mp3Data.Resize( r->mp3Data.Size() );
        const char *bin;
        r->mp3Data.Get( 0, &bin );
        mp3Data.Put( 0, r->mp3Data.Size(), bin );
    }

    version = r->version;
    bitrate = r->bitrate;
    canDelete = r->canDelete;
    length = r->length;
    framecount = r->framecount;
    frequency = r->frequency;
    defined = r->defined;
}

/* 
2.1.9 Compare

Compare this MP3 with another MP3 object. 

*/
int MP3::Compare(const Attribute * arg) const {
    return 0;
}

/* 
2.1.10 Adjacent

Adjacent is not useful for MP3 

*/
bool MP3::Adjacent(const Attribute * arg) const {
    return false;
}

/* 
2.1.11 Clone

Clones this MP3 object. 

*/
MP3* MP3::Clone() const {
    MP3 *newMP3 = new MP3( 0 );
    newMP3->CopyFrom( this );
    return newMP3;
}

/* 
2.1.12 Print

prints a textual representation of an MP3 file. 

*/
ostream& MP3::Print( ostream &os ) const {
    return os << "MP3 Algebra" << endl;
}

/* 
2.1.13 NumOfFLOBs

returns the number of FLOBs. Always 1. 

*/
int MP3::NumOfFLOBs() const {
    return 1;
}

/* 
2.1.14 GetFLOB

Get the FLOB (DBArray) 

*/
FLOB *MP3::GetFLOB(const int i) {
    assert( i >= 0 && i < NumOfFLOBs() );
    return &mp3Data;
}

/*
2.1.14 Sizeof

Returns the size of a class instance

*/
size_t MP3::Sizeof() const {
    return sizeof(*this);
}

/*
2.1.15 SubMP3

Returns a new MP3 object which contains the frames with indices
between begin and begin + size. 

*/
MP3* MP3::SubMP3(int beginframe, int size) const {
    MP3 *newmp3;

    if (!this->IsDefined()){
        /* If this MP3 object is undefined all parts of this
           MP3 are also undefined. Therefore we return a
           clone of this. */
        newmp3 = this->Clone();
        return newmp3;
    }

    if (beginframe >= this->GetFrameCount()){
        // the beginning is behind the end, an undefined mp3 
        // can be returnend
        newmp3 = new MP3(0);
        newmp3->SetDefined (false);
        return newmp3;
    }

    if (beginframe < 0){
        // beginframe was in front of the beginning of a song, 
        // the parameters have to be adjusted
        size = size + beginframe;
        beginframe = 0;
    }

    if ( (beginframe + size) > GetFrameCount()){
        size = GetFrameCount() - beginframe;
    }

    if (size <= 0){
        // size was <=0, an undefined mp3 can be returned
        newmp3 = new MP3(0);
        newmp3->SetDefined (false);
        return newmp3;
    }

    newmp3 = new MP3(0);
    
    int sizeofoldflob = mp3Data.Size();
    const char *bin;
    
    mp3Data.Get(0, &bin);
    
    int beginheaderpos = FindFirstHeader (bin,sizeofoldflob);
    int sizeofid3v2 = beginheaderpos;
    int counter=0;

    /* We have to determine the first cut position. */
    while ((beginheaderpos >= 0) 
            && (beginheaderpos < (sizeofoldflob-4)) 
            && (counter < beginframe ) ) {
        /* 4 = size of the header */
        beginheaderpos = FindNextHeader(beginheaderpos,bin,sizeofoldflob);
        counter++;
    }
    
    counter=0;
    int endheaderpos=beginheaderpos;
    /* We have to determine the second cut position */
    while ((endheaderpos < (sizeofoldflob - 4)) && (counter < size)) {
        /* 4 = size of the header */
        endheaderpos=FindNextHeader (endheaderpos,bin,sizeofoldflob);         
        counter++;
    }
    
    int newsize= endheaderpos - beginheaderpos 
        + sizeofid3v2 + FrameLength (endheaderpos,bin);
    if (ExistsID()) {
        newsize= newsize + 128;
    }
    
    newmp3->mp3Data.Resize (newsize);
    
    /* copy the bytes of ID3 V2-Tag */
    newmp3->mp3Data.Put (0,sizeofid3v2,bin);
    
    /* copy the mp3-frames */
    newmp3->mp3Data.Put (sizeofid3v2,
                         endheaderpos - beginheaderpos 
                         + FrameLength(endheaderpos,bin),
                         bin+beginheaderpos);
    
    /* copy the ID3 V1-Tag */
    if (ExistsID()) {
        newmp3->mp3Data.Put (newsize - 128,
                             128,
                             bin + sizeofoldflob -128);
    }
        
    /* recalculate the attributes of our new mp3. */
    newmp3->version = this->version;
    newmp3->bitrate = this->bitrate;
    newmp3->canDelete = this->canDelete;
    newmp3->length = (int) (size * 1152.0 / ((float) this->frequency));
    newmp3->framecount = size;
    newmp3->frequency = this->frequency;
    
    newmp3->SetDefined (true);
    return newmp3;
}

/* 
2.1.16 Concat

Returns the concatination of this MP3 and the other MP3 object. 

*/
MP3* MP3::Concat (const MP3* other) const {
    MP3* newmp3 = new MP3 (0);
    
    /* if the other mp3 is undefined we can just clone 
       this object and return the result */
    if ( !other->IsDefined()){
        newmp3 = this->Clone();
        return newmp3;
    }
        
    /* if this object is not defined we can just return 
       the clone of the other object*/
    if ( !this->IsDefined()){
        newmp3 = other->Clone();
        return newmp3;
    }

    /* First we copy both MP3s into the memory. */
    int sizeofthisflob = mp3Data.Size(),
        sizeofotherflob = other->mp3Data.Size();
    const char *thisbin, *otherbin;
    mp3Data.Get( 0, &thisbin );
    other->mp3Data.Get( 0, &otherbin );
    
    /* We have to check weather the first MP3 has an ID3 tag. This 
       ID3 tag will be the one of the result. */
    int sizeofid3v2 = FindFirstHeader (thisbin,sizeofthisflob);

    /* The second MP3 might also contain an ID3v2 tag. Therefore
       we have to find the first frame header. */
    int beginotherheaderpos = FindFirstHeader (otherbin,sizeofotherflob);
    
    int iterator = sizeofid3v2;
    int endofthisflob;
    /* After this while loop has terminated endofthisflob contains the
       address of the last header in the first MP3. */
    while ((iterator >= 0) && (iterator < (sizeofthisflob-4) ) ) {
        endofthisflob = iterator;
        iterator = FindNextHeader (iterator, thisbin, sizeofthisflob);
    }
    
    int newsize 
        = endofthisflob 
        // All frames except of the last one plus header
        + FrameLength(endofthisflob, thisbin) 
        // The length of the last frame
        + sizeofotherflob 
        // Just the size of the whole MP3 song
        - beginotherheaderpos;
    // except the size of the second header.
    
    newmp3->mp3Data.Resize(newsize);
    /* Copy the first MP3 into the result MP3. */
    newmp3->mp3Data.Put(0,
                        endofthisflob + FrameLength(endofthisflob, thisbin),
                        thisbin);
    /* Copy the second MP3 into the result MP3. */
    newmp3->mp3Data.Put(endofthisflob + FrameLength(endofthisflob, thisbin),
                        sizeofotherflob - beginotherheaderpos,
                        otherbin + beginotherheaderpos);
    
    /* recalculate the attributes of our new mp3. */
    newmp3->version = this->version;
    newmp3->bitrate = this->bitrate;
    newmp3->canDelete = this->canDelete;
    newmp3->length = this->length + other->length;
    newmp3->framecount = this->framecount + other->framecount;
    newmp3->frequency = this->frequency;
    
    newmp3->SetDefined (true);
    return newmp3;
}


/* 
2.1.17 Encode 

This methods writes the Base64 string of this MP3 Object into
the string textBytes. This string textBytes must be provided by
the caller. 

*/
void MP3::Encode(string& textBytes) const {
    Base64 b;
    /* allocate as many bytes as the size of the FLOB has 
       for the data of the DBArray. */
    const char *bytes;
    /* load the contents of the DBArray mp3Data into above
       byte array. */
    mp3Data.Get( 0, &bytes );
    /* Write the encoded string into textBytes */
    b.encode( bytes, mp3Data.Size(), textBytes );
}

/* 
2.1.18 Decode

Gets the Base64 string and reconstruct the nessesary
DBArray for our MP3 song. Afterwards some attributes 
of the MP3 song will be recalculated. 

*/
void MP3::Decode( const string& textBytes ) {
    Base64 b;
    int previousheaderpos;
    int sizeDecoded = b.sizeDecoded( textBytes.size() );
    char *bytes = (char *)malloc( sizeDecoded );
    
    int result = b.decode( textBytes, bytes );
    
    assert( result <= sizeDecoded );
    
    int headerpos = FindFirstHeader (bytes,result);
    
    bitrate = CalcBitrate(headerpos, bytes);
    frequency = CalcFrequency(headerpos, bytes);
    version = CalcMP3Version(headerpos, bytes);
    
    framecount = 1;
    /* We have to scan through the MP3 file in order to
       calculate some general value like length, number
       of frames, etc. */
    while ((headerpos >= 0) && (headerpos < (result - 4))) {
        previousheaderpos = headerpos;
        headerpos=FindNextHeader(headerpos, bytes, result);
        if (headerpos >= 0) {
            framecount++;
        }
    }
    length = (int)(framecount * 1152.0 / ((float) frequency));
        
    mp3Data.Resize( result );
    mp3Data.Put( 0, result, bytes );
        
    free( bytes );
}

/* 
2.1.19 GetID3Dump

Stores the ID3 dump (128 bytes) into buffer.
The buffer has to be provided by the caller. 

*/
void MP3::GetID3Dump (const char **buffer) const {
    mp3Data.Get(mp3Data.Size() - 128, buffer);
}

/* 
2.1.20 ExistsID

Returns whether this MP3 has an ID3 tag. 

*/
bool MP3::ExistsID() const {
    const char *idbytes;
    mp3Data.Get (mp3Data.Size()-128,&idbytes);
    char tag [4] = "TAG";
    if (strncmp (idbytes,tag,3) == 0) {
        return true;
    }
    else {
        return false;
    }  
}

/* 
2.1.21 GetLyricsDump

Stores the Lyrics dump into a buffer. The size 
of the lyrics is unknown before and is returned
in the output argument ~lyricssize~. 

*/
void MP3::GetLyricsDump (const char **buffer, int &lyricssize) const {
    /* The last 9 bytes of a Lyrics tag have to be 
       "LYRICS200" (= 9 bytes). The preceding six bytes
       before contain the size of the whole lyrics tag
       as a char array of decimals. */
    const char *auxlyricssize_;
    bool idexists=ExistsID();
    /* The position of the lyrics tag depends on the existence 
       of the ID3 tag. */
    if (idexists) {
        /* length of ID3 = 128 */
        /* length("LYRICS200") + length(lyricssize_) = 15 */
        mp3Data.Get (mp3Data.Size() - 128 - 15, &auxlyricssize_);
    }
    else {
        mp3Data.Get (mp3Data.Size() - 15, &auxlyricssize_);
    }

    char lyricssize_[7]; 
    memcpy( lyricssize_, auxlyricssize_, 6 );
    lyricssize_[6] = 0;
    lyricssize = atoi(lyricssize_);

    /* Now we can copy the content of the lyrics into buffer. 
       Again this operation depends on the existence of an ID3 tag. */
    if (idexists) {
        /* length of ID3 = 128 */
        /* length("LYRICS200") + length(lyricssize_) = 15 */
        mp3Data.Get (mp3Data.Size() - 128 - 15 - lyricssize, buffer);
    }
    else {
        mp3Data.Get (mp3Data.Size() - 15 - lyricssize, buffer);
    }
}

/* 
2.1.22 ExistsLyrics

Returns whether this MP3 has lyrics. 

*/
bool MP3::ExistsLyrics() const {
    /* The last nine bytes of the lyrics always contain "LYRICS200". */
    const char *bytes;

    /* The position of the lyrics depends on the existence of an ID3 tag. */
    if (ExistsID() ) {
        /* length of ID3 = 128 */
        mp3Data.Get (mp3Data.Size()-128-9,&bytes);
    }
    else {
        /* length of ID3 = 128 */
        mp3Data.Get (mp3Data.Size()-9,&bytes); 
    }

    if (strncmp (bytes,"LYRICS200",9) == 0 ) {
        return true;
    }
    else {
        return false;
    }
}

/* 
2.1.23 RemoveLyrics

Removes the lyrics from this object if Lyrics exists. 

*/
void MP3::RemoveLyrics() {
    if (!ExistsLyrics()) {
        return;
    }

    int oldmp3size=mp3Data.Size();
    const char *bytes;
    mp3Data.Get (0,&bytes);

    /* The last 9 bytes of a Lyrics tag have to be 
       "LYRICS200" (= 9 bytes). The preceding six bytes
       before contain the size of the whole lyrics tag
       as a char array of decimals. */

    bool idexists=ExistsID();

    /* The position of the lyrics tag depends on the existence 
       of the ID3 tag. */
    const char *auxlyricssize_;
    if (idexists) {
        /* length of ID3 = 128 */
        /* length("LYRICS200") + length(lyricssize_) = 15 */
        mp3Data.Get (oldmp3size-128-15, &auxlyricssize_);
    }
    else {
        mp3Data.Get (oldmp3size-15, &auxlyricssize_);
    }
    char lyricssize_[7];
    strncpy( lyricssize_, auxlyricssize_, 6 ); 
    lyricssize_[6]=0;
    int lyricssize = atoi (lyricssize_) + 15;


    int newsize=oldmp3size-lyricssize;

    mp3Data.Resize (newsize);
    if (idexists) {
        /* The ID3 tag exists. Therefor we have to copy
           the proper MP3 data and the ID3 data into the 
           new MP3 object. */
        mp3Data.Put (0,newsize-128,bytes); /* proper music data */
        mp3Data.Put (newsize-128,128,bytes+oldmp3size-128); /* ID3 */
    }
    else {
        mp3Data.Put (0,newsize,bytes); /* proper music data */
    }
}

/* 
2.1.24 PutLyrics

Stores the given lyrics into this MP3 object. 
If this object has already lyrics this lycris will be
overwritten. 

*/
void MP3::PutLyrics(char *lyricsdump, int size) {
    if (ExistsLyrics()) {
        /* Remove the old Lyrics tag if exists. */
        RemoveLyrics();
    }

    int oldmp3size = mp3Data.Size();
    bool existsid = ExistsID();
    const char *bytes;

    mp3Data.Get(0,&bytes);

    mp3Data.Resize (oldmp3size+size);
    
    if (existsid) {
        /* Store the music data into mp3Data without ID3 and lyrics
           tag. The lyrics was deleted before! */
        /* length of ID3 = 128 */
        mp3Data.Put (0, oldmp3size-128,bytes);
        /* Store the (new) lyrics into mp3Data. */
        mp3Data.Put (oldmp3size-128, size, lyricsdump);
        /* Store the ID3 tag. */
        mp3Data.Put (oldmp3size-128+size, 128, bytes+oldmp3size-128);
    }
    else {
        /* Store the music data into mp3Data without ID3 and lyrics
           tag. The lyrics was deleted before! */
        mp3Data.Put (0,oldmp3size,bytes);
        /* Store the (new) lyrics into mp3Data. */
        mp3Data.Put (oldmp3size, size, lyricsdump);
        /* no ID3 information available. */
    }
}

/* 
2.1.25 RemoveID

Removes the ID3 tag from this MP3 if exists. 

*/
void MP3::RemoveID () {
    if ( !ExistsID() ) {
        return;
    }
    /* copy mp3Data into bytes */
    const char *bytes;
    mp3Data.Get( 0, &bytes );

    /* length of ID3 = 128 */
    int newsize = mp3Data.Size()-128;
    mp3Data.Resize (newsize);

    /* Write the music data into mp3Data. */
    mp3Data.Put (0,newsize,bytes);
    return;
}

/* 
2.1.26 PutID

Stores a new ID3 tag into this mp3. An older version will be
replaced. 

*/
void MP3::PutID (char *iddump) {
    if ( !ExistsID()) {    
        /* length of ID3 = 128 */
        mp3Data.Resize (mp3Data.Size()+128);
    }
    mp3Data.Put(mp3Data.Size()-128,128,iddump);   
}

/* 
2.1.27 SaveMP3ToFile

Saves this MP3 song into the file with name filename. 

*/
bool MP3::SaveMP3ToFile( const char *fileName ) const {
    FILE *f = fopen( fileName, "wb" );
    
    if( f == NULL )
        return false;
    
    const char *bytes;
    mp3Data.Get( 0, &bytes );
    
    if( fwrite( bytes, 1, mp3Data.Size(), f ) != mp3Data.Size() )
        return false;
    
    fclose( f );
    return true;
}


/* 
2.1.28 FindFirstHeader

Returns the beginning of the first frame of the MP3
which is in the buffer. 

*/
int MP3::FindFirstHeader(const char *buffer, int size) const {
    /*  search for the beginning of the header... */

    /* First we have to search for 0xFF, the synchronization
       byte. This byte must be followed by one which highest
       nibble is also F. To ensure that we catched a real
       beginning of a frame we assume that this is correct
       and calculate the beginning of the next header.
       Only if this potential next header begins again with 
       FF this header is accepted. Otherwise we continue the 
       search. */

    for (int i = 0; i < size - 3; i++) {
        if ((buffer[i] == (char)0xFF) && ((buffer[i+1] & 0xF0) == 0xF0)) {
            int offset2 = FindNextHeader (i, buffer, size);
            if ( offset2 >=0  &&  buffer [offset2] == (char) 0xFF) {
                return i;
            }
        }
    }

    return -1;
}

/* 
2.1.29 FrameLength

Returns the length of the frame which begins at position
prevHeader in bytes. 

*/
int MP3::FrameLength (int prevHeader, const char *buffer) const {
    int bitrate = CalcBitrate (prevHeader, buffer)*1000;
    int frequ = CalcFrequency (prevHeader, buffer);
    int padding = (buffer[prevHeader + 2] & 0x02) >> 1;  
    return (144* bitrate / frequ + padding);
}

/* 
2.1.30 FindNextHeader

Returns the beginning of the next frame of the MP3 
which is in the buffer. The beginning of the previous
frame has to be provided in prevHeader. 

*/
int MP3::FindNextHeader(int prevHeader, const char *buffer, int size) const {
    int length = FrameLength (prevHeader, buffer);

    /* The maximum frame length is 1440. */
    if ((length < 0) || (length > 1440) || (length + prevHeader >= size)  )
        // length is out of range, so there is no next header
        return -1;
    else if ( buffer [prevHeader+length] != (char) 0xFF)
        // the sync-Tag 00xFF was not found, so there is no next Header
        return -1;
    return prevHeader+length;
}

/* 
2.1.31 CalcMP3Version

Calculates the MP3 version of the song which is in
the buffer. 

*/
int MP3::CalcMP3Version(int Header, const char *buffer) const {
    byte versionCode = buffer[Header + 1] & 0X08;
    return (versionCode == 8) ? 1 : 2;
}

/* 
2.1.32 CalcBitrate

Calculates the bitrate of the song which is in
the buffer. 

*/
int MP3::CalcBitrate(int Header, const char *buffer) const {
    byte bitrateCode = (buffer[Header + 2] & 0xF0) >> 4;

    switch(bitrateCode) {
        case 0: return -1;
        case 1: return 32;
        case 2: return 40;
        case 3: return 48;
        case 4: return 56;
        case 5: return 64;
        case 6: return 80;
        case 7: return 96;
        case 8: return 112;
        case 9: return 128;
        case 10: return 160;
        case 11: return 192;
        case 12: return 224;
        case 13: return 256;
        case 14: return 320;
        case 15: return -1;
        default: return -1;
    }
}

/* 
2.1.33 CalcFrequency

Calculates the frequency of the song which is in
the buffer. 

*/
int MP3::CalcFrequency(int Header, const char *buffer) const {
    int frequencyCode = buffer[Header + 2] & 0x0C; 

    switch(frequencyCode) {
        case 0: return 44100;
        case 1: return 32000;
        case 2: return 48000;
        case 3: return -1;
        default: return -1;
    }
}

/* 
2.1.34 GetBitrate

Returns the bitrate of this MP3. 

*/
int MP3::GetBitrate() const {
    return bitrate;
}

/* 
2.1.35 GetMP3Version

Returns the MP3 version of this object. (MPEG1 or MPEG2) 

*/
int MP3::GetMP3Version() const {
    return version;
}

/* 
2.1.36 GetFrequency

Returns the frequency of this MP3. 

*/
int MP3::GetFrequency() const {
    return frequency;
}

/* 
2.1.37 GetFrameCount

Returns the number of frames of this MP3. 

*/
int MP3::GetFrameCount() const{
    return framecount;
}

/* 
2.1.38 GetLength

Returns the length of this MP3 in seconds. 

*/
int MP3::GetLength() const {
    return length;
}


/*
2.2 List Representation

The list representation of a ~mp3~ are

----        ( <file>filename</file---> )
----

and

----        ( <text>BASE64-Coding</text---> )
----

If first representation is used, then the contents of a file is read
into the second representation. This is done automatically by the
Secondo parser.

2.3 ~Out~-Function

*/
ListExpr OutMP3( ListExpr typeInfo, Word value ) {
    ListExpr result = nl->TextAtom();
    
    MP3 *mp3 = (MP3 *)value.addr;
    string encoded;

    if (!mp3->IsDefined()){
        /* the mp3 is not defined, so we have to return a nested list 
           with the symbol atom "undef"*/
         return (nl->SymbolAtom("undef"));
    }
        
    mp3->Encode( encoded );
    nl->AppendText( result, encoded );    
    return result;
}

/*
2.4 ~In~-Function

*/
Word InMP3(const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct) {
    MP3 *mp3 = new MP3( 0 );
    
    
    /* we have to check whether the nested list contains 
       a valid mp3 object or the symbol atom "undef"*/
    if (nl->IsAtom( instance ) 
        && nl->AtomType( instance ) == SymbolType 
        && nl->SymbolValue( instance ) == "undef" )
    {
        correct = true;
        mp3->SetDefined (false);
        return SetWord (mp3);
    }    

    if (nl->IsAtom( instance ) &&
        nl->AtomType( instance ) == TextType ) {
        string encoded;
        nl->Text2String( instance, encoded );
        
        /* encoded contains now the Base64 Data of this MP3 object. */
        
        mp3->Decode( encoded );
        
        mp3->SetDefined (true);
        correct = true;
        return SetWord( mp3 );
    }
    correct = false;
    return SetWord( Address(0) );
}

/*
2.5 The ~Property~-function
  
*/
ListExpr MP3Property() {
    return 
        (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("mp3"),
                             nl->StringAtom("( <file>filename</file---> )"),
                             nl->StringAtom("( <file>song.mp3</file---> )"),
                             nl->StringAtom(""))));
}

/*
2.6 ~Create~-function

*/
Word CreateMP3( const ListExpr typeInfo ) {
    return SetWord( new MP3( 0 ) );
}

/*
2.7 ~Delete~-function

*/
void DeleteMP3( const ListExpr typeInfo, Word& w ) {
    MP3 *mp3 = (MP3 *)w.addr;
    mp3->Destroy();
    delete mp3;
    w.addr = 0;
}

/*
2.8 ~Close~-function

*/
void CloseMP3( const ListExpr typeInfo, Word& w ) {
    delete (MP3 *)w.addr;
    w.addr = 0;
}

/*
2.9 ~Clone~-function

*/
Word CloneMP3( const ListExpr typeInfo, const Word& w ) {
    return SetWord( ((MP3 *)w.addr)->Clone() );
}

/*
2.10 ~SizeOf~-function

*/
int SizeOfMP3() {
    return sizeof(MP3);
}

/*
2.11 ~Cast~-function

*/
void* CastMP3( void* addr ) {
    return new (addr) MP3;
}

/*
2.12 Kind Checking Function
  
This function checks whether 
the type constructor 
is applied correctly. Since
type constructor ~mp3~ does not 
have arguments, this is trivial.

*/
bool CheckMP3( ListExpr type, ListExpr& errorInfo ) {
    return (nl->IsEqual( type, "mp3" ));
}

/*
2.13 Creation of the Type Constructor Instance
  
*/
TypeConstructor mp3(
    // name
    "mp3",
    // property function describing signature
    MP3Property,
    // out function
    OutMP3,
    // in function
    InMP3,
    // SaveToList function
    0,
    // RestoreFromList function
    0,
    // Object creation
    CreateMP3,
    // Object deletion
    DeleteMP3,        
    // object open
    0, 
    // object save
    0, 
    // object close
    CloseMP3,
    // object clone
    CloneMP3,
    // cast function
    CastMP3, 
    // sizeof function
    SizeOfMP3, 
    // kind checking function
    CheckMP3 );

/* 
3 Type Constructor ~ID3~

3.1 Class ~ID3~

The class ID3 encapulates an ID3 tag. Although an ID3 tag is a
part of an MP3 file this algebra provides a seperate storage of
such ID3 tags.

*/
class ID3 : StandardAttribute {
public:
    /* Standard constructor. */
    ID3();
    /* This object can be deleted by the Secondo system. */
    void Destroy();
    /* Returns whether this object is defined or not. */
    bool IsDefined() const;
    /* Sets this object as defined or undefined. */
    void SetDefined( bool Defined);
    /* Calcules a hash value for an ID3. */
    size_t HashValue() const;
    /* Copy the data of another ID3 object into this object. */
    void CopyFrom(const StandardAttribute* right);
    /* Compare this ID3 with another ID3 object. */
    int Compare(const Attribute * arg) const;
    /* Adjacent is not useful for ID3 */
    bool Adjacent(const Attribute * arg) const;
    /* Clones this ID3 object. */
    ID3* Clone() const;
    /* Prints a textual representation of an ID3 tag. */
    ostream& Print( ostream &os ) const;
    /* Returns the size of a class instance */
    size_t Sizeof() const;
    /* Creates an ID3 dump (128 bytes) from this object
       and stores it into iddump. The memory has to be 
       provided by the caller. */
    void MakeID3Dump (char *iddump);
    /* Returns the author. The memory has to be 
       provided by the caller. */
    void GetAuthor (char *authorname);
    /* Returns the title. The memory has to be 
       provided by the caller. */
    void GetTitle (char *titlename);
    /* Returns the album name The memory has to be 
       provided by the caller.*/
    void GetAlbum(char *albumname);
    /* Returns the comment. The memory has to be 
       provided by the caller.*/
    void GetComment(char *commentname);
    /* Returns the track number. */
    int GetTrack ();
    /* Returns the year. */
    int GetYear ();
    /* Retuns the genre. The memory has to be 
       provided by the caller. */
    void GetGenre(char *genrename);
    /* Extracts the genre name from the genre code. */
    char *GetGenreName(byte nr);

    char songname [31];
    char author [31];
    char album [31];
    int year;
    char comment [31];
    int track;
    char genre [21];
    int version;
    
    bool canDelete;
    bool defined;
};

/* 
3.1.1 Constructor

Standard constructor.

*/
ID3::ID3() : canDelete(false) {
    //songname[0] = 01;
    //author[0] = 0;
    //album[0] = 0;
    //year = 0;
    //comment[0] = 0;
    //track = 0;
    //genre[0] = 0;
    //version = 0;
    //canDelete = false;
    //defined = false;
}

/* 
3.1.2 Destroy

This object can be deleted by the Secondo system. 

*/
void ID3::Destroy() {
    canDelete = true;
}

/* 
3.1.3 IsDefined

Returns whether this object is defined or not. 

*/
bool ID3::IsDefined() const {
    return defined;
}

/* 
3.1.4 SetDefined

Sets this object as defined or undefined. 

*/
void ID3::SetDefined (bool Defined) {
    defined = Defined;
}

/* 
3.1.5 HashValue

Calcules a hash value for an ID3.

*/
size_t ID3::HashValue () const { 
    if (!defined)
        return 0;
    else {
        size_t result = 0;
        
        for (int i = 0; i < 31; i++) result = result + (size_t)songname[i];
        for (int i = 0; i < 31; i++) result = result + (size_t)author[i];
        for (int i = 0; i < 31; i++) result = result + (size_t)album[i];
        result = result + (size_t)year;
        return result;
    }
}

/* 
3.1.6 CopyFrom

Copy the data of another ID3 object into this object. 

*/
void ID3::CopyFrom(const StandardAttribute* right) {
    const ID3* id = (const ID3*) right;
    *this = *id;
}

/* 
3.1.7 Compare

Compare this ID3 with another ID3 object. 

*/
int ID3::Compare (const Attribute * arg) const {
    return 0;
}

/* 
3.1.8 Adjacent

Adjacent is not useful for ID3 

*/
bool ID3::Adjacent (const Attribute * arg) const {
    return 0;
}

/* 
3.1.9 Clone

Clones this ID3 object. 

*/
ID3* ID3::Clone() const {
    ID3 *newID3 = new ID3();
    newID3->CopyFrom(this);
    return newID3;
}

/* 
3.1.10 Print

Prints a textual representation of an ID3 file. 

*/
ostream& ID3::Print( ostream &os ) const {

    return os << "ID3 tag." << endl;
}

/*
3.1.11 Sizeof

Returns the size of a class instance

*/
size_t ID3::Sizeof() const {
    return sizeof(*this);
}


/* 
3.1.11 MakeID3Dump

Creates an ID3 dump (128 bytes) from this object
and stores it into iddump. The memory has to be 
provided by the caller. 

*/
void ID3::MakeID3Dump (char *iddump) {
    char tag[4] ="TAG";
    
    strncpy (iddump,tag,3);
    strncpy (iddump + 3, songname,30);
    strncpy (iddump + 33, author,30);
    strncpy (iddump + 63, album, 30);
    strncpy (iddump + 97, comment,30);

    if (version == 1)
    {
        // ID3-Tag Version 1.1, Byte before Tracknumber has to zero
        *(iddump+125) = (byte) 0x00;
        *(iddump+126) = (byte) track;
    }
        
    char year_ [6];
    sprintf (year_, "%d",year);
    strncpy (iddump + 93, year_, 4);


    bool found = false;
    byte genrenumber = 127;
    for (int i=0; i<= 127; i++)
    {
        if  (strncmp (genre, GetGenreName (i),21) == 0 )
        {
            found = true;
            genrenumber = i;
        }
    }

    *(iddump + 127) = (byte) genrenumber;
}

/* 
3.1.12 GetAuthor

Returns the author. The memory has to be 
provided by the caller. 

*/
void ID3::GetAuthor(char *authorname) {
    strncpy (authorname,author,31);
}

/* 
3.1.13 GetTitle

Returns the title. The memory has to be 
provided by the caller. 

*/
void ID3::GetTitle(char *titlename) {
    strncpy (titlename,songname,31);
}

/* 
3.1.14 GetAlbum

Returns the album name. The memory has to be 
provided by the caller.

*/
void ID3::GetAlbum(char *albumname) {
    strncpy (albumname,album,31);
}

/* 
3.1.15 GetComment

Returns the comment. The memory has to be 
provided by the caller. 

*/
void ID3::GetComment(char *commentname) {
    if (version == 0){
        strncpy (commentname,comment,31);
    }
    else
        strncpy (commentname,comment,29);
}

/* 
3.1.16 GetGenre

Returns the genre. The memory has to be 
provided by the caller. 

*/
void ID3::GetGenre(char *genrename) {
    strncpy (genrename,genre,21);
}

/* 
3.1.17 GetTrack

Returns the track number. 

*/
int ID3::GetTrack() {
    return track; 
}

/* 
3.1.18 GetYear

Returns the year. 

*/
int ID3::GetYear() { 
    return this->year;
}

/*
3.1.19 GetGenreName

Extracts the genre name from the genre code. 

*/
char *ID3::GetGenreName(byte nr) {
    switch(nr) {
        case 0: return "Blues";
        case 1: return "Classic Rock";
        case 2: return "Country";
        case 3: return "Dance";
        case 4: return "Disco";
        case 5: return "Funk";
        case 6: return "Grunge";
        case 7: return "Hip-Hop";
        case 8: return "Jazz";
        case 9: return "Metal";
        case 10: return "New Age";
        case 11: return "Oldies";
        case 12: return "Other";
        case 13: return "Pop";
        case 14: return "R&B";
        case 15: return "Rap";
        case 16: return "Raggae";
        case 17: return "Rock";
        case 18: return "Techno";
        case 19: return "Industrial";
        case 20: return "Alternative";
        case 21: return "Ska";
        case 22: return "Death Metal";
        case 23: return "Pranks";
        case 24: return "Soundtrack";
        case 25: return "Euro-Techno";
        case 26: return "Ambient";
        case 27: return "Trip-Hop";
        case 28: return "Vocal";
        case 29: return "Jazz+Funk";
        case 30: return "Fusion";
        case 31: return "Trance";
        case 32: return "Classical";
        case 33: return "Instrumental";
        case 34: return "Acid";
        case 35: return "House";
        case 36: return "Game";
        case 37: return "Sound Clip";
        case 38: return "Gospel";
        case 39: return "Noise";
        case 40: return "Alt. Rock";
        case 41: return "Bass";
        case 42: return "Soul";
        case 43: return "Punk";
        case 44: return "Space";
        case 45: return "Meditative";
        case 46: return "Instrumental Pop";
        case 47: return "Instrumental Rock";
        case 48: return "Ethnic";
        case 49: return "Gothie";
        case 50: return "Darkwave";
        case 51: return "Techno-Industrial";
        case 52: return "Electronic";
        case 53: return "Pop-Folk";
        case 54: return "Eurodance";
        case 55: return "Dream";
        case 56: return "Southern Rock";
        case 57: return "Comedy";
        case 58: return "Cult";
        case 59: return "Gangsta";
        case 60: return "Top 40";
        case 61: return "Christian Rap";
        case 62: return "Pop/Funk";
        case 63: return "Jungle";
        case 64: return "Native American";
        case 65: return "Cabaret";
        case 66: return "New Wave";
        case 67: return "Psychadelic";
        case 68: return "Rave";
        case 69: return "Showtunes";
        case 70: return "Trailer";
        case 71: return "Lo-Fi";
        case 72: return "Tribal";
        case 73: return "Acid Punk";
        case 74: return "Acid Jazz";
        case 75: return "Polka";
        case 76: return "Retro";
        case 77: return "Musical";
        case 78: return "Rock & Roll";
        case 79: return "Hard Rock";
        case 80: return "Folk";
        case 81: return "Folk/Rock";
        case 82: return "National Folk";
        case 83: return "Swing";
        case 84: return "Fusion";
         case 85: return "Bebop";
        case 86: return "Latin";
        case 87: return "Revival";
        case 88: return "Celtic";
        case 89: return "Bluegrass";
        case 90: return "Avantgarde";
        case 91: return "Gothic Rock";
        case 92: return "Progressive Rock";
        case 93: return "Psychedelic Rock";
        case 94: return "Symphonic Rock";
        case 95: return "Slow Rock";
        case 96: return "Big Band";
        case 97: return "Chorus";
        case 98: return "Easy Listening";
        case 99: return "Acoustic";
        case 100: return "Humour";
        case 101: return "Speech";
        case 102: return "Chanson";
        case 103: return "Opera";
        case 104: return "Chamber Music";
        case 105: return "Sonata";
        case 106: return "Symphony";
        case 107: return "Booty Bass";
        case 108: return "Primus";
        case 109: return "Porn Groove";
        case 110: return "Satire";
        case 111: return "Slow Jam";
        case 112: return "Club";
        case 113: return "Tango";
        case 114: return "Samba";
        case 115: return "Folklore";
        default: return "--Unknown--";
    }
}

/*
3.2 List Representation

The list representation of an ~id3~ are

----        ( string string string int string string ) (ID3v1.0)
----

or

----        ( string string string int string int string ) (ID3v1.1)
----

3.3 ~Out~-Function

*/

ListExpr OutID3( ListExpr typeInfo, Word value ) {
    ID3 *id3 = (ID3 *)(value.addr);
    
    ListExpr result;
    string songName(id3->songname);
    string authorName(id3->author);
    string albumName(id3->album);
    string commentName(id3->comment);
    string genreName(id3->genre);


    if (!id3->IsDefined())    {
        /* the ID3 tag is not defined, so we have to return 
           a nested list with the symbol atom "undef"*/
         return (nl->SymbolAtom("undef"));
    }

    /* If ID3 tag version is 1.0 the ID3 information is coded into
       six elements. The ID3 tag version 1.1 has one field more namely 
       one for the track number. */
    else if (id3->version == 0) {
        result = nl->SixElemList(
            nl->StringAtom(songName),
            nl->StringAtom(authorName),
            nl->StringAtom(albumName),
            nl->IntAtom(id3->year),
            nl->StringAtom(commentName),
            nl->StringAtom(genreName));
    } else {
        /* we build up a seven element list here. */
        result = nl->SixElemList(
            nl->StringAtom(authorName),
            nl->StringAtom(albumName),
            nl->IntAtom(id3->year),
            nl->StringAtom(commentName),
            nl->IntAtom(id3->track),
            nl->StringAtom(genreName));
        result = nl->Cons (nl->StringAtom (songName), result);
    }
    return result;
}

/*
3.4 ~In~-Function

*/
Word InID3( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct ){  
    ID3 *id3 = new ID3 ();


    /* we have to check whether the nested list contains a valid 
       id3 tag or the symbol atom "undef"*/
    if (nl->IsAtom( instance ) 
        && nl->AtomType( instance ) == SymbolType 
        && nl->SymbolValue( instance ) == "undef" ) {
        correct = true;
        id3->SetDefined (false);
        return SetWord (id3);
    }    

    /* If ID3 tag version is 1.0 the ID3 information is coded into
       six elements. The ID3 tag version 1.1 has one field more namely 
       one for the track number. */
    else if (nl->ListLength (instance) == 6
        && nl->IsAtom (nl->First( instance ))
        && nl->AtomType (nl->First( instance )) == StringType 
        && nl->IsAtom (nl->Second( instance ))
        && nl->AtomType (nl->Second( instance )) == StringType
        && nl->IsAtom (nl->Third( instance ))
        && nl->AtomType (nl->Third( instance )) == StringType
        && nl->IsAtom (nl->Fourth( instance ))
        && nl->AtomType (nl->Fourth( instance )) == IntType
        && nl->IsAtom (nl->Fifth( instance ))
        && nl->AtomType (nl->Fifth( instance )) == StringType
        && nl->IsAtom (nl->Sixth( instance ))
        && nl->AtomType (nl->Sixth( instance )) == StringType) {


        id3->version = 0;
        string songName = nl->StringValue(nl->First(instance));
        string authorName = nl->StringValue(nl->Second(instance));
        string albumName = nl->StringValue(nl->Third(instance));
        char* dummy = (char*) songName.c_str();
        strncpy (id3->songname,dummy,30);
        *(id3->songname+30) = (byte) 0x00;
        dummy = (char*) authorName.c_str();
        strncpy (id3->author,dummy,30);
        *(id3->author+30) = (byte) 0x00;
        dummy = (char*) albumName.c_str();
        strncpy (id3->album,dummy,30);
        *(id3->album+30) = (byte) 0x00;
        
        id3->year = nl->IntValue(nl->Fourth(instance));
        
        string commentName = nl->StringValue(nl->Fifth(instance));
        dummy = (char*) commentName.c_str();
        strncpy (id3->comment,dummy,30);
        *(id3->comment+30) = (byte) 0x00;
        
        string genreName = nl->StringValue(nl->Sixth(instance));
        dummy = (char*) genreName.c_str();
        strncpy (id3->genre,dummy,20);
        *(id3->genre+20) = (byte) 0x00;

        id3->SetDefined(true);

        correct = true;
        return SetWord (id3);
    } else if (nl->ListLength (instance) == 7
               && nl->IsAtom (nl->First( instance )) 
               && nl->AtomType (nl->First( instance )) == StringType
               && nl->IsAtom (nl->Second( instance )) 
               && nl->AtomType (nl->Second( instance )) == StringType
               && nl->IsAtom (nl->Third( instance )) 
               && nl->AtomType (nl->Third( instance )) == StringType
               && nl->IsAtom (nl->Fourth( instance )) 
               && nl->AtomType (nl->Fourth( instance )) == IntType
               && nl->IsAtom (nl->Fifth( instance )) 
               && nl->AtomType (nl->Fifth( instance )) == StringType
               && nl->IsAtom (nl->Sixth( instance )) 
               && nl->AtomType (nl->Sixth( instance )) == IntType
               && nl->IsAtom (nl->Sixth (nl->Rest (instance)) ) 
               && nl->AtomType (nl->Sixth( nl->Rest (instance) )) 
               == StringType) {

               id3->version = 1;
        string songName = nl->StringValue(nl->First(instance));
        string authorName = nl->StringValue(nl->Second(instance));
        string albumName = nl->StringValue(nl->Third(instance));
        char* dummy = (char*) songName.c_str();
        strncpy (id3->songname,dummy,30);
        *(id3->songname+30) = (byte) 0x00;
        dummy = (char*) authorName.c_str();
        strncpy (id3->author,dummy,30);
        *(id3->author+30) = (byte) 0x00;
        dummy = (char*) albumName.c_str();
        strncpy (id3->album,dummy,30);
        *(id3->album+30) = (byte) 0x00;
        
        id3->year = nl->IntValue(nl->Fourth(instance));
        
        string commentName = nl->StringValue(nl->Fifth(instance));
        dummy = (char*) commentName.c_str();
        strncpy (id3->comment,dummy,28);
        *(id3->comment+28) = (byte) 0x00;
        
        id3->track = nl->IntValue(nl->Sixth(instance));
        
        string genreName = nl->StringValue(nl->Sixth(nl->Rest (instance)));
        
        dummy = (char*) genreName.c_str();
        strncpy (id3->genre,dummy,20);
        *(id3->genre+20) = (byte) 0x00;

        id3->SetDefined(true);
        
        correct = true;
        return SetWord (id3);
    } else {
        /* other versions of ID3 tags are not supported. */
        correct = false;
        return SetWord( Address(0));
    }
}


/*
3.5 The ~Property~-function

*/
ListExpr ID3Property() {
    return (nl->TwoElemList
            (nl->FiveElemList
             (nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              nl->StringAtom("List Rep"),
              nl->StringAtom("Example List"),
              nl->StringAtom("Remarks")),
             nl->FiveElemList
             (nl->StringAtom("-> DATA"),
              nl->StringAtom("id3"),
              nl->TextAtom("( string string string int string string )"),
              nl->TextAtom(
                  "( 'songname' 'author' 'album' 1984 'comment' 'Rock') "),
              nl->StringAtom(""))));
}

/*
3.6 ~Create~-function

*/
Word CreateID3( const ListExpr typeInfo ) {
    return SetWord( new ID3( ) );
}

/*
3.7 ~Delete~-function

*/
void DeleteID3( const ListExpr typeInfo, Word& w ) {
    ID3 *id3 = (ID3 *)w.addr;
    id3->Destroy();
    delete id3;
    w.addr = 0;
}

/*
3.8 ~Close~-function

*/
void CloseID3( const ListExpr typeInfo, Word& w ) {
    delete (ID3 *)w.addr;
    w.addr = 0;
}

/*
3.9 ~Clone~-function

*/
Word CloneID3( const ListExpr typeInfo, const Word& w ) {
    return SetWord( ((ID3 *)w.addr)->Clone() );
}

/*
3.10 ~SizeOf~-function

*/
int SizeOfID3() {
    return sizeof(ID3);
}


/*
3.11 ~Cast~-function

*/
void* CastID3( void* addr ) {
    return new (addr) ID3;
}

/*
3.12 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~id3~ does not have arguments, this is trivial.

*/
bool CheckID3( ListExpr type, ListExpr& errorInfo ) {
    return (nl->IsEqual( type, "id3" ));
}

/*
3.13 Creation of the Type Constructor Instance

*/
TypeConstructor id3(
    // name
    "id3",
    // property function describing the signature
    ID3Property,
    // Out function
    OutID3,
    // In function
    InID3,      
    // SaveToList
    0,                
    // RestoreFromList
    0,
    // object creation
    CreateID3,          
    // object deletion
    DeleteID3,
    // object open
    0, 
    // object save
    0,
    // object close
    CloseID3,
    // object clone
    CloneID3,
    // cast function
    CastID3,
    // sizeof function
    SizeOfID3,
    // kind checking function
    CheckID3 );


/*
4 Type Constructor ~Lyrics~

4.1 struct Line

This class encapsulates a single line of song lyrics. 

*/
struct Line {
    Line () {}
    Line (int sec, char* text) {
        seconds = sec;
    }
    int seconds;
    char textline[255];

    size_t HashValue() const {
        size_t result = 0;
        for (int i = 0; i < 255; i++) result = result + textline[i];
        return result;
    }
};

enum LyricsState { partial, complete};

/*

4.2 Class ~Lyrics~

The class Lyrics encapulates the lyrics of an MP3. Although the lyrics are a
part of an MP3 file this algebra provides a seperate storage of
such lyrics lines.

*/
class Lyrics : StandardAttribute {
  public:
    /* Constructor */
    Lyrics() {};
    /* Constructor with size */
    Lyrics( const int size);
    /* Destructor */
    ~Lyrics();
    /* Clones this Lyrics object. */
    Lyrics *Clone() const;
    /* Returns whether this object is defined or not. */ 
    bool IsDefined() const;
    /* Sets this object as defined or undefined. */
    void SetDefined( bool Defined);
    /* Calcules a hash value for an Lyrics. */
    size_t HashValue() const;
    /* Copy the data of another Lyrics object into this object. */
    void CopyFrom(const StandardAttribute* right);
    /* Returns the number of FLOBs : 1 */
    int NumOfFLOBs() const;
    /* Returns the line array. */
    FLOB *GetFLOB(const int i);
    /* Compare this Lyrics with another Lyrics object. */
    int Compare(const Attribute * arg) const;
    /* Adjacent is not useful for Lyrics */
    bool Adjacent(const Attribute * arg) const;
    /* Returns the size of a class instance */
    size_t Sizeof() const;
    /* Appends a new line to the lines array. */
    void Append( Line oneline);
    /* This object can be deleted by the secondo system. */
    void Destroy();
    /* Returns the number of lines of the lyrics. */
    int NoLines() const;
    /* Returns the ith line of the lyrics. */
    Line GetLine( int i ) const;

  private:
    bool defined;
    DBArray<Line> linearray;
    bool canDelete;
};


/*
4.2.1 Constructors.

Constructor

*/
Lyrics::Lyrics(const int size) : linearray (size), canDelete (false) {
    //canDelete = false;
    //defined = false;
}

/*

4.2.2 Destructor.

*/
Lyrics::~Lyrics() {
    if (canDelete) {
        linearray.Destroy();
    }
}

/*
4.2.3 Clone

Clones this Lyrics object.

*/
Lyrics *Lyrics::Clone() const {
    Lyrics *lyrics = new Lyrics(0);
    lyrics->CopyFrom(this);
    return lyrics;
}


/*
4.2.4 IsDefined

Returns whether this object is defined or not. 

*/
bool Lyrics::IsDefined() const {
    return defined;
}

/*
4.2.5 SetDefined 

Sets this object as defined or undefined.

*/
void Lyrics::SetDefined (bool Defined) {
    defined = Defined;
}

/* 
4.2.6 HashValue

Calcules a hash value for an Lyrics. 

*/
size_t Lyrics::HashValue () const {  
    size_t result = 0;
    for (int i = 0; i < linearray.Size(); i++) {
        const Line *aLine;
        linearray.Get(i, aLine);
        result = result + aLine->HashValue();
    }
    return 0;
}

/* 
4.2.7 CopyFrom

Copy the data of another Lyrics object into this object. 

*/
void Lyrics::CopyFrom(const StandardAttribute* right) {
    const Lyrics* lyrics = (const Lyrics*) right;
    
    for(int i = 0; i < lyrics->NoLines(); i++ ) {
        Append(lyrics->GetLine(i));
    }
    defined = lyrics->defined;
    canDelete = lyrics->canDelete;
}

/*
4.2.8 NumOfFLOBs.

Returns the number of FLOBs : 1

*/
int Lyrics::NumOfFLOBs() const {
    return 1;
}

/*
4.2.9 GetFLOB

Returns the line array.

*/
FLOB *Lyrics::GetFLOB(const int i) {
    return &linearray;
}


/* 
4.2.10 Compare

Compare this Lyrics with another Lyrics object. 

*/
int Lyrics::Compare (const Attribute * arg) const {
    return 0;
}

/* 
4.2.11 Adjacent 

Adjacent is not useful for Lyrics

*/
bool Lyrics::Adjacent (const Attribute * arg) const {
    return 0;
}

/*
4.2.12 Sizeof

Returns the size of a class instance

*/
size_t Lyrics::Sizeof() const {
    return sizeof(*this);
}

/*
4.2.13 Append

Appends a new line to the lines array.

*/
void Lyrics::Append(Line oneline ) {
    linearray.Append (oneline );
}

/*
4.2.14 Destroy

This object can be deleted by the secondo system.

*/
void Lyrics::Destroy() {
    canDelete = true;
}


/*
4.2.15 NoLines

Returns the number of lines of the lyrics.

*/
int Lyrics::NoLines() const {
    return linearray.Size();
}

/*
4.2.16 GetLine

Returns a Line indexed by ~i~.

*/
Line Lyrics::GetLine( int i ) const {
    const Line *l;
    linearray.Get( i, l );
    return *l;
}


/*
4.3 List Representation

The list representation of a ~lyrics~ are

----        ( int string int string ... int string )
----

4.4 ~Out~-Function

*/
ListExpr OutLyrics( ListExpr typeInfo, Word value ) {
    Lyrics *lyrics = (Lyrics *)(value.addr);
    
    ListExpr result=nl->TheEmptyList();

    if (!lyrics->IsDefined())    {
        /* the lyrics tag is not defined, so we have to return 
           a nested list with the symbol atom "undef"*/
         return (nl->SymbolAtom("undef"));
    }

    /* The nested list is build up beginning from the end. 
       This is done because the append operator does not seem 
       to be very reliable. */
    for (int i = lyrics->NoLines() - 1; i >= 0; i--) {
        int sec = lyrics->GetLine(i).seconds;
        string tline = lyrics->GetLine(i).textline;
        
        result = nl->Cons(nl-> StringAtom (tline ), result);
        result = nl->Cons(nl-> IntAtom (sec ), result);
    }
    return result;
}

/*
4.5 ~In~-Function

*/
Word InLyrics( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct ){  
    Lyrics *lyrics = new Lyrics(0);
    

    /* we have to check whether the nested list contains 
       valid lyrics or the symbol atom "undef"*/
    if (nl->IsAtom(instance) 
        && nl->AtomType(instance) == SymbolType 
        && nl->SymbolValue(instance) == "undef") {
        correct = true;
        lyrics->SetDefined (false);
        return SetWord (lyrics);
    }    


    /* The nested list of a lyrics object consists of alternating 
       elements for the time stamps and the text. Therefore the number
       of elements must be even. */
    int nolines = nl->ListLength (instance) / 2;

    if (!(nl->ListLength (instance) % 2 == 0) ) {
        /* The number of elements is odd. Create an incorrect
           lyrics object. */
        correct = false;
        return SetWord( Address(0));
    }

    /* Now we have to check whether all element pairs in the list
       have the correct type namely IntType resp. StringType. */
    bool typeOk = true;
    ListExpr iter=instance;
    for (int i = 1; i <= nolines ; i++) {
        if ( nl->IsAtom (nl->First(iter))
             && nl->AtomType (nl->First(iter)) == IntType 
             && nl->IsAtom (nl->Second(iter))
             && nl->AtomType (nl->Second(iter)) == StringType) {       
            // empty.
        }
        else { 
            typeOk=false;
        }
       iter = nl->Rest(nl->Rest(iter));
    }
    
    /* Now we can copy each element of the nested list into the
       lyrics object. */
    iter = instance;
    if (typeOk ) {
        for (int i = 1; i <= nolines; i++) {
            int timer = nl->IntValue (nl->First (iter));
            string text = nl->StringValue (nl->Second (iter));
            
            Line oneline;
            oneline.seconds = timer;
            char *dummy = (char*) text.c_str();
            
            strncpy (oneline.textline,dummy,255);

            lyrics->Append (oneline);
            
            iter = nl->Rest(nl->Rest(iter));
        }
        correct = true;
        lyrics->SetDefined(true);
        return SetWord (lyrics);
    }
    else {
        correct = false;
        return SetWord( Address(0));
    }
}

/*

4.6 The ~Property~-function

*/
ListExpr LyricsProperty() {
    return (nl->TwoElemList
            (nl->FiveElemList
             (nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              nl->StringAtom("List Rep"),
              nl->StringAtom("Example List"),
              nl->StringAtom("Remarks")),
             nl->FiveElemList
             (nl->StringAtom("-> DATA"),
              nl->StringAtom("lyrics"),
              nl->StringAtom("( int string int string ... )"),
              nl->StringAtom("( 7 'first line' 11 'second line' )"),
              nl->StringAtom(""))));
}

/*
4.7 ~Create~-function

*/
Word CreateLyrics( const ListExpr typeInfo ) {
    return SetWord(new Lyrics(0));
}

/*
4.8 ~Delete~-function

*/
void DeleteLyrics( const ListExpr typeInfo, Word& w ) {
    Lyrics *lyrics = (Lyrics *)w.addr;
    lyrics->Destroy();
    delete lyrics;
    w.addr = 0;
}

/*
4.9 ~Close~-function

*/
void CloseLyrics( const ListExpr typeInfo, Word& w ) {
    delete (Lyrics *)w.addr;
    w.addr = 0;
}

/*
4.10 ~Clone~-function

*/
Word CloneLyrics( const ListExpr typeInfo, const Word& w ) {
    return SetWord( ((Lyrics *)w.addr)->Clone() );
}

/*
4.11 ~SizeOf~-function

*/
int SizeOfLyrics() {
    return sizeof(Lyrics);
}
/*
4.12 ~Cast~-function

*/
void* CastLyrics( void* addr ) {
    return new (addr) Lyrics;
}

/*
4.13 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~lyrics~ does not have arguments, this is trivial.

*/
bool CheckLyrics( ListExpr type, ListExpr& errorInfo ) {
    return (nl->IsEqual( type, "lyrics" ));
}

/*
4.14 Creation of the Type Constructor Instance

*/
TypeConstructor lyrics(
    // name
    "lyrics",
    // property function describing signature
    LyricsProperty,
    // Out function
    OutLyrics,
    // In function
    InLyrics,      
    // SaveToList function
    0,
    // RestoreFromList function
    0,      
    // object creation
    CreateLyrics,
    // object deletion
    DeleteLyrics,
    // object open
    0,
    // object save
    0,
    // object close
    CloseLyrics, 
    // object clone
    CloneLyrics,
    // cast function
    CastLyrics,
    // sizeof function
    SizeOfLyrics,
    // kind checking function
    CheckLyrics );




/*
5 Operators

5.1 Operator ~savemp3to~

Saves the binary contents of an mp3 into a file.

5.1.1 Type mapping function of operator ~savemp3to~

Operator ~savemp3to~ accepts a mp3 object and a string representing
the name of the file, and returns a boolean meaning success or not.

----    (mp3 string)               -> bool
----

*/
ListExpr SaveMP3ToTypeMap( ListExpr args ) {
    ListExpr arg1, arg2;
    if ( nl->ListLength(args) == 2 ) {
        arg1 = nl->First(args);
        arg2 = nl->Second(args);
        if (nl->IsEqual(arg1, "mp3") && nl->IsEqual(arg2, "string")) {
            return nl->SymbolAtom("bool");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.1.2 Value mapping functions of operator ~savemp3to~

*/
int SaveMP3ToFun(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    CcString *fileName = (CcString*)args[1].addr;
    
    if( mp3->SaveMP3ToFile( *(fileName->GetStringval()) ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );
    
    return 0;
}

/*
  
5.1.3 Specification of operator ~savemp3to~

*/
const string SaveMP3ToSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3 string) -> bool"
"</text--->"
"<text>_ savemp3to _</text--->"
"<text>Saves the contents of the mp3 object into a "
"file.</text--->"
"<text>query song savemp3to \"song.mp3\"</text--->"
") )";

/*

5.1.4 Definition of operator ~savemp3to~

*/
Operator savemp3to (
    // name
    "savemp3to",
    // specification
    SaveMP3ToSpec,
    // value mapping
    SaveMP3ToFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    SaveMP3ToTypeMap
);


/*
5.2 Operator ~removelyrics~

Removes the Lyrics-Tag of an MP3-File.

5.2.1 Type mapping function of operator ~removelyrics~

----    (mp3)               -> mp3
----

*/
ListExpr RemoveLyricsTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("mp3");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.2.2 Value mapping functions of operator ~removelyrics~

*/
int RemoveLyricsFun(Word* args, Word& result, int message, 
                    Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    MP3 *newmp3 = mp3->Clone();
    newmp3->RemoveLyrics();
    result.addr = newmp3;
    return 0;
}

/*
  
5.2.3 Specification of operator ~removelyrics~

*/
const string RemoveLyricsSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> mp3"
"</text--->"
"<text>_ removelyrics </text--->"
"<text>Removes the lyrics of a mp3 object"
".</text--->"
"<text>query song removelyrics</text--->"
") )";

/*
  
5.2.4 Definition of operator ~removelyrics~

*/
Operator removelyrics (
    // name
    "removelyrics",
    // specification
    RemoveLyricsSpec,
    // value mapping
    RemoveLyricsFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    RemoveLyricsTypeMap
    );

/*

5.3 Operator ~removeid3~

Removes the ID3-Tag of a MP3-File.

5.3.1 Type mapping function of operator ~removeid3~

----    (mp3)               -> mp3
----

*/
ListExpr RemoveID3TypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("mp3");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.3.2 Value mapping functions of operator ~removeid3~

*/
int RemoveID3Fun(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    MP3 *newmp3 = mp3->Clone();
    newmp3->RemoveID();
    
    result.addr = newmp3;
    
    return 0;
}

/*
  
5.3.3 Specification of operator ~removeid3~

*/
const string RemoveID3Spec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> mp3"
"</text--->"
"<text>_ removeid3</text--->"
"<text>Removes the ID3-Tag of a mp3 objects"
".</text--->"
"<text>query song removeid3</text--->"
") )";

/*
  
5.3.4 Definition of operator ~removeid3~

*/
Operator removeid3 (
    // name
    "removeid3",
    // specification
    RemoveID3Spec,
    // value mapping
    RemoveID3Fun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    RemoveID3TypeMap
    );



/*

5.4 Operator ~submp3~

Extracts frames from an MP3 file object and generate a new MP3
file object which contains specific parts of the song. 
The first integer parameter specifies the start frame and the
second integer parameter specifies the length of the fragment. 

5.4.1 Type mapping function of operator ~submp3~

----    (mp3, int, int)               -> mp3
----

*/
ListExpr SubMP3TypeMap( ListExpr args ) {
    ListExpr arg1,arg2,arg3;
    if (nl->ListLength(args) == 3) {
        arg1 = nl->First(args);
        arg2 = nl->Second(args);
        arg3 = nl->Third(args);
        if (nl->IsEqual(arg1, "mp3") &&
            nl->IsEqual(arg2, "int") &&
            nl->IsEqual(arg3, "int")) {
            return nl->SymbolAtom("mp3");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.4.2 Value mapping functions of operator ~submp3~

*/
int SubMP3Fun(Word* args, Word& result, int message, 
              Word& local, Supplier s) {
    result = qp->ResultStorage( s );

    int beginframe = ((CcInt*)args[1].addr)->GetIntval();
    int size = ((CcInt*)args[2].addr)->GetIntval();
    
    MP3 *newmp3 = ((MP3*)args[0].addr)->SubMP3 (beginframe, size);
    result.addr = newmp3;
    
    return 0;
}

/*

5.4.3 Specification of operator ~submp3~

*/
const string SubMP3Spec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3 int int) -> mp3"
"</text--->"
"<text> _ _ _ submp3</text--->"
"<text>Extracts frames from mp3 objects and"
" saves them into a new mp3 object.</text--->"
"<text>query song 12 100 submp3</text--->"
") )";

/*

5.4.4 Definition of operator ~submp3~

*/
Operator submp3 (
    // name
    "submp3",
    // specification
    SubMP3Spec,
    // value mapping
    SubMP3Fun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    SubMP3TypeMap
    );


/*

5.5 Operator ~concatmp3~

Returns the bitrate of an MP3-File.

5.5.1 Type mapping function of operator ~concatmp3~

----    (mp3, mp3)               -> mp3
----

*/
ListExpr ConcatMP3TypeMap(ListExpr args) {
    ListExpr arg1,arg2;
    if (nl->ListLength(args) == 2) {
        arg1 = nl->First(args);
        arg2 = nl->Second(args);
        if (nl->IsEqual(arg1, "mp3") &&
            nl->IsEqual(arg2, "mp3")) {
            return nl->SymbolAtom("mp3");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*
  
5.5.2 Value mapping functions of operator ~concatmp3~

*/
int ConcatMP3Fun(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    
    MP3 *firstmp3 = (MP3*)args[0].addr;
    MP3 *secondmp3 = (MP3*)args[1].addr;
    
    MP3 *newmp3 = firstmp3->Concat (secondmp3);
    result.addr = newmp3;
    
    return 0;
}

/*
  
5.5.3 Specification of operator ~concatmp3~

*/
const string ConcatMP3Spec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3 , mp3) -> mp3"
"</text--->"
"<text> _ _ concatmp3</text--->"
"<text>Concats two mp3 objects"
".</text--->"
"<text>query song1 song2 concatmp3</text--->"
") )";

/*

5.5.4 Definition of operator ~concatmp3~

*/
Operator concatmp3 (
    // name
    "concatmp3",
    // Specification 
    ConcatMP3Spec,
    // value mapping
    ConcatMP3Fun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    ConcatMP3TypeMap
    );


/*

5.6 Operator ~bitrate~

Returns the bitrate of an MP3-File.

5.6.1 Type mapping function of operator ~bitrate~

----    (mp3)               -> int
----

*/
ListExpr BitrateTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.6.2 Value mapping functions of operator ~bitrate~

*/
int BitrateFun(Word* args, Word& result, int message, 
               Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    ((CcInt *)result.addr)->Set(mp3->IsDefined(), mp3->GetBitrate());
    return 0;
}

/*

5.6.3 Specification of operator ~bitrate~

*/
const string BitrateSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> int"
"</text--->"
"<text>_ bitrate </text--->"
"<text>Returns the bitrate of the mp3 object"
".</text--->"
"<text>query song bitrate</text--->"
") )";

/*

5.6.4 Definition of operator ~bitrate~

*/
Operator bitrate (
    // name
    "bitrate",
    // specification
    BitrateSpec,
    // value mapping
    BitrateFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    BitrateTypeMap
);

/*

5.7 Operator ~version~

Returns the version of an MP3-File.

5.7.1 Type mapping function of operator ~version~

----    (mp3)               -> int
----

*/
ListExpr VersionTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.7.2 Value mapping functions of operator ~version~

*/
int VersionFun(Word* args, Word& result, int message, 
               Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    
    ((CcInt *)result.addr)->Set(mp3->IsDefined(), mp3->GetMP3Version());
    
    return 0;
}

/*

5.7.3 Specification of operator ~version~

*/
const string VersionSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> int"
"</text--->"
"<text>_ version</text--->"
"<text>Returns the version of the mp3 object"
".</text--->"
"<text>query song version</text--->"
") )";

/*

5.7.4 Definition of operator ~version~

*/
Operator version (
    // name
    "version",
    // specification
    VersionSpec,
    // value mapping
    VersionFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    VersionTypeMap
    );

/*

5.8 Operator ~frequency~

Returns the frequency of an MP3-File.

5.8.1 Type mapping function of operator ~frequency~

----    (mp3)               -> int
----

*/
ListExpr FrequencyTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.8.2 Value mapping functions of operator ~frequency~

*/
int FrequencyFun(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    
    ((CcInt *)result.addr)->Set(mp3->IsDefined(), mp3->GetFrequency());

    return 0;
}

/*

5.8.3 Specification of operator ~frequency~

*/
const string FrequencySpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> int"
"</text--->"
"<text>_ frequency</text--->"
"<text>Returns the frequency of the mp3 object "
".</text--->"
"<text>query song frequency</text--->"
") )";

/*
  
5.8.4 Definition of operator ~frequency~

*/
Operator frequency (
    // name
    "frequency",
    // specification
    FrequencySpec,
    // value mapping
    FrequencyFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    FrequencyTypeMap
    );


/*

5.9 Operator ~framecount~

Returns the number of frames of an MP3-File.

5.9.1 Type mapping function of operator ~framecount~

----    (mp3)               -> int
----

*/
ListExpr FrameCountTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.9.2 Value mapping functions of operator ~framecount~

*/
int FrameCountFun(Word* args, Word& result, int message, 
                  Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    
    ((CcInt *)result.addr)->Set(mp3->IsDefined(), mp3->GetFrameCount());
    
    return 0;
}

/*
  
5.9.3 Specification of operator ~framecount~

*/
const string FrameCountSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> int"
"</text--->"
"<text>_ framecount</text--->"
"<text>Returns the number of frames within the mp3"
"object.</text--->"
"<text>query song framecount</text--->"
") )";

/*

5.9.4 Definition of operator ~framecount~

*/
Operator framecount (
    // name
    "framecount",
    // specification
    FrameCountSpec,
    // value mapping
    FrameCountFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    FrameCountTypeMap
    );


/*

5.10 Operator ~length~

Returns the length of an MP3-File (seconds).

5.10.1 Type mapping function of operator ~length~

----    (mp3)               -> int
----

*/
ListExpr LengthTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.10.2 Value mapping functions of operator ~length~

*/
int LengthFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    
    ((CcInt *)result.addr)->Set(mp3->IsDefined(), mp3->GetLength());
    
    return 0;
}

/*

5.10.3 Specification of operator ~length~

*/
const string LengthSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> int"
"</text--->"
"<text>_ songlength</text--->"
"<text>Returns the length of a song (seconds) "
".</text--->"
"<text>query song songlength</text--->"
") )";

/*

5.10.4 Definition of operator ~length~

*/
Operator lengthsong (
    // name
    "songlength",
    // specification
    LengthSpec,
    // value mapping
    LengthFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    LengthTypeMap
    );

/*

5.11 Operator ~getid3~

Returns the id3-tag of an MP3-File.

5.11.1 Type mapping function of operator ~getid3~

----    (mp3)               -> id3
----

*/
ListExpr GetID3TypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("id3");
        }
    }
    return nl->SymbolAtom("typeerror");
}



/*

5.11.2 Value mapping functions of operator ~getid3~

*/
int GetID3Fun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    ID3* id3= new ID3();

    /* is the mp3 object defined? */
    if (!mp3->IsDefined()){
        id3->SetDefined(false);
        result.addr=id3;
        return 0;
    }
    
    const char *bytes;
    char tag [4] = "TAG";
    mp3->GetID3Dump (&bytes);
        
    if (!(strncmp (bytes,tag,3) == 0)) {
        /* The first three characters were not "TAG". 
           This means there was no ID3 tag in the MP3 song.
           An undefined object is created. */
        id3->SetDefined (false);
        result.addr=id3;
        return 0;
    }
        
    /* copy the songname into the new id3 object. */
    strncpy (id3->songname,bytes+3,30);
    *(id3->songname + 30) = (byte) 0x00;

    /* copy the author into the new id3 object. */
    strncpy (id3->author,bytes+33,30);
    *(id3->author + 30) = (byte) 0x00;
    /* copy the album name into the new id3 object. */
    strncpy (id3->album,bytes+63,30);
    *(id3->album + 30) = (byte) 0x00;
    /* copy the year into the new ID3 object. The year is
       not encoded as a number but as a string. Therefore
       we have to use the atoi function. */
    char year_[5];
    strncpy (year_,bytes+93,4);
    *(year_ + 4) = (byte) 0x00;
    id3->year = atoi (year_);

    if ( bytes[125] == 0) {
        // the zero byte indicates that we have Version 1.1
        id3->version = 1;
        // copy the comment
        strncpy (id3->comment,bytes+97,28);
        *(id3->comment + 28) = (byte) 0x00;
        /* copy the track number into the new id3 object. 
           The track number is encoded as word. (two bytes). */
        id3->track = (byte) (*(bytes+126));
    }
    else {
        id3->version = 0;
        // copy the comment
        strncpy (id3->comment,bytes+97,30);
        *(id3->comment + 30) = (byte) 0x00;
        // since there is no track-number information we can set it to 0
        id3->track = 0;
    }

    /* copy the genre name into the new id3 object. */
    strncpy (id3->genre,id3->GetGenreName((byte) (*(bytes+127)) ),20);
    *(id3->genre + 21) = (byte) 0x00;
    
    id3->SetDefined(true);

    result.addr=id3;
    
    return 0;
}

/*

5.11.3 Specification of operator ~getid3~

*/
const string GetID3Spec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> id3"
"</text--->"
"<text>_ getid3</text--->"
"<text>Extracts an id3 object from a song "
".</text--->"
"<text>query song getid3</text--->"
") )";

/*

5.11.4 Definition of operator ~getid3~

*/
Operator getid3 (
    // name
    "getid3",
    GetID3Spec,         //specification
    // value mapping
    GetID3Fun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    GetID3TypeMap
    );



/*

5.12 Operator ~putid3~

Adds an ID3-Tag to the mp3 object.

5.12.1 Type mapping function of operator ~putid3~

----    (mp3, id3)               -> mp3
----

*/
ListExpr PutID3TypeMap(ListExpr args) {
    ListExpr arg1;
    ListExpr arg2;
    if (nl->ListLength(args) == 2) {
        arg1 = nl->First(args);
        arg2 = nl->Second(args);
        if (nl->IsEqual(arg1, "mp3")  && nl->IsEqual (arg2,"id3")) {
            return nl->SymbolAtom("mp3");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.12.2 Value mapping functions of operator ~putid3~

*/
int PutID3Fun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    MP3 *mp3 = (MP3*)args[0].addr;
    ID3 *id3 = (ID3*)args[1].addr;
    
    MP3 *newmp3 = mp3->Clone();
    
    if (id3->IsDefined()){
        char idbytes [128];
        id3->MakeID3Dump (idbytes);  
        newmp3->PutID(idbytes);
    }
    
    result.addr = newmp3;
    return 0;
}

/*

5.12.3 Specification of operator ~putid3~

*/
const string PutID3Spec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3 id3) -> mp3"
"</text--->"
"<text>_ _ putid3</text--->"
"<text>Adds an id3 Tag to the mp3 song"
".</text--->"
"<text>query song id3 putid3</text--->"
") )";

/*

5.12.4 Definition of operator ~putid3~

*/
Operator putid3 (
    // name
    "putid3",
    // specification
    PutID3Spec,
    // value mapping
    PutID3Fun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    PutID3TypeMap
    );


/*

5.13 Operator ~author~

Gets the author of an ID3-Tag.

5.13.1 Type mapping function of operator ~author~

----    (id3)               -> string
----

*/
ListExpr AuthorTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
            return nl->SymbolAtom("string");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.13.2 Value mapping functions of operator ~author~

*/
int AuthorFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;

    if (id3->IsDefined()){
        char newStr[256];
        id3->GetAuthor (newStr);
        ((CcString *)result.addr)->Set( true, (STRING*) &newStr );
        return 0;
    }
    else {
        ((CcString *)result.addr)->Set( false, (STRING*) "");
        return 0;
    }
}

/*

5.13.3 Specification of operator ~author~

*/
const string AuthorSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(id3) -> string"
"</text--->"
"<text>_ author</text--->"
"<text>Extracts the author`s name from an id3-object"
".</text--->"
"<text>query id3 author</text--->"
") )";

/*

5.13.4 Definition of operator ~author~

*/
Operator author (
    // name 
    "author",
    // specification
    AuthorSpec,
    // value mapping
    AuthorFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    AuthorTypeMap
    );


/*

5.14 Operator ~titleof~

Gets the title of an ID3-Tag.

5.14.1 Type mapping function of operator ~titleof~

----    (id3)               -> string
----

*/
ListExpr TitleTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
            return nl->SymbolAtom("string");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.14.2 Value mapping functions of operator ~titleof~

*/
int TitleFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;

    if (id3->IsDefined()){    
        char newStr[256];
        id3->GetTitle (newStr);
    
        ((CcString *)result.addr)->Set( true, (STRING*) &newStr );
        return 0;
    }  
    else {
        ((CcString *)result.addr)->Set( false, (STRING*) "");
        return 0;
    }
}

/*

5.14.3 Specification of operator ~title~

*/
const string TitleSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>id3 -> string"
"</text--->"
"<text>_ titleof</text--->"
"<text>Extracts the title from an id3 object"
".</text--->"
"<text>query id3 titleof</text--->"
") )";

/*

5.14.4 Definition of operator ~titleof~

*/
Operator cctitle (
    // name
    "titleof",
    // specification
    TitleSpec,
    // value mapping
    TitleFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    TitleTypeMap
    );


/*

5.15 Operator ~album~

Gets the album name of an ID3-Tag.

5.15.1 Type mapping function of operator ~album~

----    (id3)               -> string
----

*/
ListExpr AlbumTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
             return nl->SymbolAtom("string");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.15.2 Value mapping functions of operator ~album~

*/
int AlbumFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;

    if (id3->IsDefined()){
        char newStr[256];
        id3->GetAlbum (newStr);
    
        ((CcString *)result.addr)->Set( true, (STRING*) &newStr );
        return 0;
    }
    else {
        ((CcString *)result.addr)->Set( false, (STRING*) "");
        return 0;
    }
}

/*

5.15.3 Specification of operator ~album~

*/
const string AlbumSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(id3) -> string"
"</text--->"
"<text>_ album</text--->"
"<text>Extracts the albumname from an id3 object "
".</text--->"
"<text>query id3 album</text--->"
") )";

/*

5.15.4 Definition of operator ~album~

*/
Operator album (
    // name
    "album",
    // specification
    AlbumSpec,
    // value mapping
    AlbumFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    AlbumTypeMap
    );


/*

5.16 Operator ~comment~

Gets the comment of an ID3-Tag.

5.16.1 Type mapping function of operator ~comment~

----    (id3)               -> string
----

*/
ListExpr CommentTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
            return nl->SymbolAtom("string");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.16.2 Value mapping functions of operator ~comment~

*/
int CommentFun(Word* args, Word& result, int message, 
               Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;

    if (id3->IsDefined()){
        char newStr[256];
        id3->GetComment (newStr);
        
        ((CcString *)result.addr)->Set( true, (STRING*) &newStr );
        return 0;
    }
    else {
        ((CcString *)result.addr)->Set( false, (STRING*) "");
        return 0;
    }
    
}

/*

5.16.3 Specification of operator ~comment~

*/
const string CommentSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> bool"
"</text--->"
"<text>_ comment</text--->"
"<text>Extracts the comment of an id3 object "
".</text--->"
"<text>query id3 comment</text--->"
") )";

/*

5.16.4 Definition of operator ~comment~

*/
Operator comment (
    // name
    "comment",
    // specification
    CommentSpec,
    // value mapping
    CommentFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    CommentTypeMap
    );


/*

5.17 Operator ~genre~

Gets the genre of an ID3-Tag.

5.17.1 Type mapping function of operator ~genre~

----    (id3)               -> string
----

*/
ListExpr GenreTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
            return nl->SymbolAtom("string");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.17.2 Value mapping functions of operator ~genre~

*/
int GenreFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;
 
    if (id3->IsDefined()){
        char newStr[256];
        id3->GetGenre (newStr);
    
        ((CcString *)result.addr)->Set( true, (STRING*) &newStr );
        return 0;
    }
    else {
        ((CcString *)result.addr)->Set( false, (STRING*) "");
        return 0;
    }
}

/*

5.17.3 Specification of operator ~genre~

*/
const string GenreSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(id3) -> string"
"</text--->"
"<text>_ genre</text--->"
"<text>Extracts the genre of an id3 object"
".</text--->"
"<text>query id3 genre</text--->"
") )";

/*

5.17.4 Definition of operator ~genre~

*/
Operator genre (
    // name
    "genre",
    // specification
    GenreSpec,
    // value mapping
    GenreFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    GenreTypeMap
    );


/*

5.18 Operator ~track~

Gets the track-no of an ID3-Tag.

5.18.1 Type mapping function of operator ~track~

----    (id3)               -> int
----

*/
ListExpr TrackTypeMap(ListExpr args) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.18.2 Value mapping functions of operator ~track~

*/
int TrackFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;

    if (id3->IsDefined())
        ((CcInt *)result.addr)->Set( true, id3->GetTrack() );
    else
        ((CcInt *)result.addr)->Set( false, 0 );        
    return 0;
}

/*
  
5.18.3 Specification of operator ~track~

*/
const string TrackSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(id3) -> int"
"</text--->"
"<text>_ track</text--->"
"<text>Extracts the track-no from an id3-object "
".</text--->"
"<text>query id3 track</text--->"
") )";

/*

5.18.4 Definition of operator ~track~

*/
Operator track (
    // name
    "track",
    // specification
    TrackSpec,
    // value mapping
    TrackFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    TrackTypeMap
    );

/*

5.19 Operator ~songyear~

Gets the year of an ID3-Tag.

5.19.1 Type mapping function of operator ~songyear~

----    (id3)               -> int
----

*/
ListExpr YearTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "id3")) {
            return nl->SymbolAtom("int");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.19.2 Value mapping functions of operator ~songyear~

*/
int YearFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    ID3 *id3 = (ID3*)args[0].addr;
    
    if (id3->IsDefined())
        ((CcInt *)result.addr)->Set( true, id3->GetYear() );
    else
        ((CcInt *)result.addr)->Set( false, 0 );
    return 0;
}

/*

5.19.3 Specification of operator ~songyear~

*/
const string YearSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(id3) -> int"
"</text--->"
"<text>_ songyear</text--->"
"<text>Extracts the year from an id3-object"
".</text--->"
"<text>query id3 songyear</text--->"
") )";

/*

5.19.4 Definition of operator ~songyear~

*/
Operator songyear (
    // name
    "songyear",
    // specification
    YearSpec,
    // value mapping
    YearFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    YearTypeMap
    );

/*

5.20 Operator ~lyricswords~

Gets the corresponding text line to the provided point of time. 

5.20.1 Type mapping function of operator ~lyricswords~

----    (lyrics int)               -> string
----

*/
ListExpr WordsTypeMap( ListExpr args ) {
    ListExpr arg1;
    ListExpr arg2;
    if (nl->ListLength(args) == 2) {
        arg1 = nl->First(args);
        arg2 = nl->Second(args);
        if (nl->IsEqual(arg1, "lyrics") &&
            nl->IsEqual(arg2, "int")) {
            return nl->SymbolAtom("string");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.20.2 Value mapping functions of operator ~lyricswords~

This function returns the text line of the lyrics that corresponds to the
given point of time.

*/
int WordsFun(Word* args, Word& result, int message, Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    Lyrics *lyrics = (Lyrics*)args[0].addr;
    int secs = ((CcInt*)args[1].addr)->GetIntval();

    /* First we have to check whether the lyrics are defined*/
    if (!lyrics->IsDefined() ){
        ((CcString *)result.addr)->Set( false, (STRING*) "" );        
        return 0;
    }

    /* We have to check whether the point of time is in the intervall 
       from the beginning of one line to the beginning of the next
       line. */
    bool found=false;
    char newStr[256];
    for (int i = 0; i < lyrics->NoLines()-1; i++) {
        /* time when ith line begins. */
        int beginsecs = lyrics->GetLine (i).seconds;
        /* time when ith line ends. This is equal to the time
           whean the (i+1)th line begins. */
        int endsecs   = lyrics->GetLine (i+1).seconds;
        
        if ((secs >= beginsecs) && (secs < endsecs) && (!found)) {
            /* we have found the correct interval. So the string
             has to be copied to newStr. */
            found = true;
            strncpy (newStr,lyrics->GetLine (i).textline,255);
        }
    }

    /* Now we have to check whether the given time stamp is inside the 
       last intervall. Because the length of an intervall (specially 
       the last interval) is not available we offer the last line even
       if the time stamp is "behind" the song. */
    if ((!found) && (secs >= lyrics->GetLine(lyrics->NoLines()-1).seconds)) {
        found =true;
        strncpy (newStr,lyrics->GetLine (lyrics->NoLines()-1).textline,255);
    }

    if (found) {
        ((CcString *)result.addr)->Set( true, (STRING*) &newStr );
    }
    else
        /* This case can occur if the given time stamp is lower than
           the beginning of the text. */
        ((CcString *)result.addr)->Set( false, (STRING*) &newStr );
    return 0;
}

/*

5.20.3 Specification of operator ~lyricswords~

*/
const string WordsSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(lyrics int) -> string"
"</text--->"
"<text>_ _ lyricswords</text--->"
"<text>Returns the corresponding text line of "
"a lyrics.</text--->"
"<text>query songlyrics 10 lyricswords</text--->"
") )";

/*

5.20.4 Definition of operator ~words~

*/
Operator words (
    // name
    "lyricswords",
    // specification
    WordsSpec,
    // value mapping
    WordsFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    WordsTypeMap
    );

/*

5.21 Operator ~getlyrics~

Gets the lyrics of an MP3-Song.

5.21.1 Type mapping function of operator ~getlyrics~

----    (mp3)               -> lyrics
----

*/
ListExpr GetLyricsTypeMap( ListExpr args ) {
    ListExpr arg1;
    if (nl->ListLength(args) == 1) {
        arg1 = nl->First(args);
        if (nl->IsEqual(arg1, "mp3")) {
            return nl->SymbolAtom("lyrics");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.21.2 Value mapping functions of operator ~getlyrics~

Returns a lyrics object from an MP3 object with lyrics. If the
given MP3 object has no lyrics an undefined lyrics object is returned.

*/
int GetLyricsFun(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {
    result = qp->ResultStorage(s);
    MP3 *mp3 = (MP3*)args[0].addr;
    
    Lyrics* newlyrics = new Lyrics(0);
    
    if  (!mp3->IsDefined()) {
        newlyrics->SetDefined (false);
        result.addr = newlyrics;
        return 0;
    }

    if  (!mp3->ExistsLyrics()) {
        newlyrics->SetDefined (false);
        result.addr = newlyrics;
        return 0;
    }

    /* buffer for the lyrics */
    const char* buffer;
    /* size of the lyrics dump */
    int size;
    /* iterator for scanning the lyrics text lines. */
    char* iter; 
    
    /* get the lyrics dump from the MP3 object. */
    mp3->GetLyricsDump (&buffer,size);

    /* The proper lyrics text data begin after the "LYR" tag. 
     So we have to look for the first occurance of "LYR" 
     in the dump. */
    char *lyrptr = strstr(buffer+1,"LYR"); 

    /* The text length is encoded in 
       a 5-byte-char-array behind the "LYR". */
    char textsize_ [6]; 
    strncpy (textsize_, lyrptr + 3, 5);
    textsize_[5] = 0;
    int textsize = atoi(textsize_);

    Line oneline;
    
    /* mulitiline is a pointer to the fist multiline item 
       (multiline = [time stamp] + text) 
       Example: [00:02]first text line */
    char *multiline = lyrptr + 8;
    iter = multiline;

    /* Append a [ to make the handling easier. */
    *(multiline + textsize) = *"[";

    /* scan through all multilines and build up the lyrics object. */
    while ((!(iter == 0)) && (iter < (multiline + textsize))) {
        /* find the next [. This is the beginning of the multiline or
         the end of the multiline text. */
        iter = strstr (iter, "[" );

        if ((!(iter == 0)) && (iter < (multiline + textsize)) ) {
            /* A multiline was found. */

            /* digitbuffer is used to convert a string representation
               of a number to int. 
               Read the minute information. */
            char digitbuffer[3];
            strncpy (digitbuffer,iter + 1, 2);
            digitbuffer[2] = 0;
            int min = atoi(digitbuffer);

            /* Read the second information */
            strncpy(digitbuffer, iter + 4, 2);
            digitbuffer[2]=0;
            int sec = atoi(digitbuffer);

            /* calculate the time stamp for the current line. */
            oneline.seconds = min * 60 + sec;

            /* extract the text line excl. time information and 
               store it into oneline. */
            char * endofchars = strstr(iter + 7, "[");
            int linelength = endofchars-(iter + 7);
            strncpy (oneline.textline, iter + 7, linelength);
            *(oneline.textline + linelength) = 0;

            /* Append oneline to the new lyrics object. */
            newlyrics->Append (oneline);

            /* overread the time stamp. */
            iter = iter + 7;
        }
    }
    newlyrics->SetDefined (true);
    result.addr = newlyrics;
    return 0;
}

/*

5.21.3 Specification of operator ~getlyrics~

*/
const string GetLyricsSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> bool"
"</text--->"
"<text>_ getlyrics </text--->"
"<text>Saves the lyrics of an MP3 object into a "
"lyrics object.</text--->"
"<text>query song getlyrics</text--->"
") )";

/*

5.21.4 Definition of operator ~getlyrics~

*/
Operator getlyrics (
    // name
    "getlyrics",
    // specification
    GetLyricsSpec,
    // value mapping
    GetLyricsFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    GetLyricsTypeMap
    );

/*

5.22 Operator ~putlyrics~

Removes the ID3-Tag of an MP3-File.

5.22.1 Type mapping function of operator ~putlyrics~

----    (mp3, lyrics)               -> mp3
----

*/
ListExpr PutLyricsTypeMap(ListExpr args) {
    ListExpr arg1;
    ListExpr arg2;
    if (nl->ListLength(args) == 2) {
        arg1 = nl->First(args);
        arg2 = nl->Second(args);
        if (nl->IsEqual(arg1, "mp3")  && nl->IsEqual (arg2,"lyrics")) {
            return nl->SymbolAtom("mp3");
        }
    }
    return nl->SymbolAtom("typeerror");
}

/*

5.22.2 Value mapping functions of operator ~putlyrics~

This function creates and returns a new MP3 objects that contains
the same song and same ID3 tag but with given lyrics.

*/
int PutLyricsFun(Word* args, Word& result, int message, 
                 Word& local, Supplier s) {
    result = qp->ResultStorage( s );
    /* old MP3 */
    MP3 *mp3 = (MP3*)args[0].addr; 
    /* new lyrics */
    Lyrics *lyrics = (Lyrics*)args[1].addr; 
    /* clone the MP3 song to put the lyrics later in. */
    MP3 *newmp3 = mp3->Clone(); // 


    /* First we have to check whether lyrics are defined, if not we can 
       just return the clone of the old mp3*/
    if (!lyrics->IsDefined()){
        result.addr = newmp3;
        return 0;
    }

    /* Calculate the length of the lyrics multiline text. 
     This length has to be provided at the top of the 
     multiline part. */
    int textlength=0;
    for (int i = 0; i <lyrics->NoLines(); i++) {
        // dont forget 7 bytes for the time stamp
        textlength = textlength + strlen (lyrics->GetLine(i).textline) + 7;
    }

    /* Calculate the length which has to be provided before "LYRICS200".
     29 is the length of a minimalized header. */
    int endlength=textlength + 29;

    /* this is the real length of the whole Lyrics tag in bytes. 
     At the end of the lyrics data "LYRICS200" (9 bytes) has to 
     be added. Furthermore above length has to be provided in a
     textual representation (= 6 bytes). */
    int blocklength = endlength + 9 + 6;
    
    /* Allocate memory for the dump of the lyrics */
    char* lyricbytes = (char*) malloc (blocklength);

    
    char dummy[255];

    /* Copy a temporary lyrics header into lyricsbytes. The text length
       is inserted later. */
    memcpy (lyricbytes,"LYRICSBEGININD0000211LYR00000",29);

    /* write the text length to the end of the lyrics header. 
       Now the header is finished. */
    sprintf (dummy, "%d",textlength);
    /* 24 = position of the 00000 */
    /* 5 = sizeof("00000") */
    memcpy (lyricbytes + 24 + 5 - strlen(dummy), dummy, strlen(dummy));
    
    /* Now we can write the multilines into lyricsbytes. */
    char* position= lyricbytes + 29;
    for (int i = 0; i < lyrics->NoLines(); i++) {
        /* write the time stamp */
        memcpy (position,"[00:00]", 7); // modified later.
        int timesecs = lyrics->GetLine(i).seconds;
        int min = timesecs / 60;
        int secs = timesecs % 60;
        /* writes the minute information into the time stamp. */
        sprintf(dummy,"%d",min);
        memcpy(position + 1 + 2 - strlen(dummy), dummy, strlen(dummy));
        /* writes the second information into the time stamp. */
        sprintf(dummy,"%d",secs);
        memcpy(position + 4 + 2 - strlen (dummy), dummy, strlen (dummy));
        position = position + 7;
        /* Add the text line. */
        char* textlinepointer = lyrics->GetLine(i).textline;
        memcpy (position, textlinepointer, strlen (textlinepointer));
        position = position + strlen (textlinepointer);
        //printf ("%d  : %d !\n",min,secs);
        
    }
    /* Add the length of the whole lyrics (multilines + header but 
       without "LYRICS200" at the end) in bytes. */
    memcpy (position, "000000",6);
    sprintf (dummy, "%d", endlength);
    memcpy (position + 6 - strlen (dummy), dummy, strlen(dummy));
    position = position + 6;
    memcpy (position, "LYRICS200",9);

    /* insert (replace) the lyrics */
    newmp3->PutLyrics(lyricbytes, blocklength);

    free (lyricbytes);
    
    result.addr = newmp3;
    return 0;
}

/*

5.22.3 Specification of operator ~putlyrics~

*/
const string PutLyricsSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(mp3) -> bool"
"</text--->"
"<text>_ _ putlyrics </text--->"
"<text>put the lyrics into an MP3 "
"file.</text--->"
"<text>query song lyrics putlyrics</text--->"
") )";

/*

5.22.4 Definition of operator ~putlyrics~

*/
Operator putlyrics (
    // name
    "putlyrics",
    // specification
    PutLyricsSpec,
    // value mapping
    PutLyricsFun,
    // trivial selection function
    Operator::SimpleSelect,
    // type mapping
    PutLyricsTypeMap
    );


/*

6 Creating the Algebra

*/

class MP3Algebra : public Algebra {
public:
    MP3Algebra() : Algebra() {
        AddTypeConstructor( &mp3 );
        AddTypeConstructor( &id3 );
        AddTypeConstructor( &lyrics);
        
        mp3.AssociateKind("DATA");
        mp3.AssociateKind("FILE");
        id3.AssociateKind("DATA");
        id3.AssociateKind("FILE");
        lyrics.AssociateKind("DATA");
        lyrics.AssociateKind("FILE");

        AddOperator(&savemp3to);
        AddOperator(&version);
        AddOperator(&frequency);
        AddOperator(&framecount);
        AddOperator(&lengthsong);
        AddOperator(&bitrate);
        AddOperator(&concatmp3);
        AddOperator(&submp3);
        AddOperator(&putid3);
        AddOperator(&removeid3);
        AddOperator(&getid3);
        AddOperator(&author);
        AddOperator(&cctitle);
        AddOperator(&album);
        AddOperator(&comment);
        AddOperator(&track);
        AddOperator(&songyear);
        AddOperator(&genre);
        AddOperator(&words);
        AddOperator(&getlyrics);
        AddOperator(&removelyrics);
        AddOperator(&putlyrics);

  }
  ~MP3Algebra() {};
};

MP3Algebra mp3Algebra;

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeMP3Algebra( NestedList* nlRef, QueryProcessor* qpRef ) {
    nl = nlRef;
    qp = qpRef;
    return (&mp3Algebra);
}


