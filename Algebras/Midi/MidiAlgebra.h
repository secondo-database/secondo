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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[->] [$\rightarrow $]

\pagebreak

1 Header file of MidiAlgebra

This header file essentially contains the definition of the classes ~Midi~, ~Event~ and ~Track~ used in the Midi Algebra. Only the class ~Midi~ respectively correspond to the memory representation for the type constructor ~Midi~.

2 Defines and includes

*/

#ifndef __MIDI_ALGEBRA_H__
#define __MIDI_ALGEBRA_H__

#include "StandardAttribute.h"
#include "DBArray.h"
#include <string>
#include <vector>

namespace midialgebra {

class Track;
class Event;
/*
Forward declaration for classes ~Track~ and ~Event~ .

*/

/*
2.1 Constants

Several values are fixed inside a Midi object according to the specification by MMA.

*/
static const int MIN_TRACKS_MIDI   = 1;
/*
At least one track must be inside a Midi object.

*/
static const int MAX_TRACKS_MIDI   = 65536;
/*
Maximum numbers of tracks inside a Midi object.

*/
static const int MAX_CHANNELS_MIDI = 16;
/*
Maximum numbers of channels inside a track object.

*/
static const int MIN_FORMAT_MIDI   = 0;
static const int MAX_FORMAT_MIDI   = 2;
/*
The Midi format is only valid between 0 and 2.

*/
static const string MIDI_STRING    = "midi";
/*
Constant string for Midi.

*/
static const string MIDI_HEADER    = "MThd";
/*
Constant string for a Midi object header.

*/
static const string TRACK_STRING   = "track";
/*
Constant string for Track.

*/
static const string TRACK_HEADER   = "MTrk";
/*
Constant string for a Midi track header.

*/
static const unsigned int LENGTH_OF_SECONDO_STRING = 49;
/*
Constant length of SECONDO its internal string representation.

*/
static const int ERROR_INT = -1;
/*
Constant integer e.g. for wrong input values.

*/
/*
3 Class Midi

This class implements the memory representation of the ~Midi~ type constructor.

*/

enum EventEntryType {shortmessageEntry, shortmessageRSEntry,
                     metamessageEntry, sysexmessageEntry};
/*
The types of events which can be stored in the element listOfEvents

*/

struct TrackEntry
{
    int noOfEvents;
    int eventPtr;
};
/*
Objects of type TrackEntry can be stored in listOfElements. The element noOfEvents contains the count of elements in this track, and eventPtr references the first element entry of this track in the DBArray listOfElements.

*/

struct EventEntry
{
    int dataPtr;
    int size;
    EventEntryType type;
};
/*
Objects of the type EventEntry can be stored in listOfElements. The element dataPtr references the first byte of the event data in
the DBArray eventData. The element size represents the entire size of this event in bytes, the element type contains the type information of this event entry.

*/

class Midi: public StandardAttribute
{
  public:

/*
3.1 Constructors and Destructor

There are two ways of constructing a ~Midi~:

*/
    Midi();
/*
This constructor should not be used.

*/
    Midi( const bool defined, const unsigned char divisionMSB,
          const unsigned char divisionLSB );
/*
The first one receives a boolean value ~d~ indicating if the Midi is defined and two values for ~divisionMSB~ and ~divisionLSB~. Note that this constructor cannot be called without arguments.

*/
    Midi( const Midi& p );
/*
The second one receives a ~Midi~ object ~p~ as argument and creates a ~Midi~ that is a copy of ~p~.

*/
    ~Midi();
/*
The destructor.

*/
    Midi& operator=(const Midi&);
/*
Overloading the assignment operator.

*/

/*
3.2 Member functions

*/
    Track* GetTrack(int index) const;
/*
Returns the by index selected track of this ~Midi~ object.

*/
    void Append( const Track *inTrack );
/*
Appends the by index selected track of this ~Midi~ object. The caller of this method is responsible for destroying the passed Track object after using.

*/
    const int GetNumberOfTicks() const;
/*
Returns the number of ticks per quarter note of the ~Midi~. If IsDivisionInFramesFormat() returns true, the value of this function
will be undefined.

*/
    const int GetNumberOfTracks() const;
/*
Returns the number of tracks of the ~Midi~ object.

*/
    void SetFormat (const int format);
/*
Sets the format of this ~Midi~ object.

*/
    const int GetFormat () const;
/*
Gets the ~Midi~ object`s format.

*/
    void SetHeaderLength (const int lengthOfHeader);
/*
Sets the length of the ~Midi~ header. At the moment the value will be always 6.

*/
    const int GetHeaderLength () const;
/*
Returns the ~Midi~ object`s header length.

*/
    const string GetHeader () const;
/*
Returns the ~Midi~ object`s header. At the moment it is the constant string ''MThd''.

*/
    const int GetFileSize() const;
/*
Returns the size in bytes of the input ~Midi~ file.

*/
    Midi* Clone(const bool copyTracks) const;
/*
Overloads the Clone method. If copyTracks is set false, it returns a new Midi object with copied attributes, but without any event and track data. Otherwise it works like Clone().

*/
    void AppendEmptyTrack();
/*
Appends an empty track to this ~Midi~ object.

*/
    void Destroy();
/*
If this method is called, the destructor will destroy the physical representation of all DBArrays.

*/
    bool IsDivisionInFramesFormat() const;
/*
Returns true if the division of this Midi object is stored in frames format.

*/
    unsigned char GetDivisionMSB() const;
/*
Returns the division's most significant byte.

*/
    unsigned char GetDivisionLSB() const;
/*
Returns the division's least significant byte.

*/
    void  SetDivisionMSB(const unsigned char msb);
/*
Sets  the division's most significant byte.

*/
    void SetDivisionLSB(const unsigned char lsb);
/*
Returns the division's least significant byte.

*/
    void GetLyrics(string& result, bool all, bool lyr, bool any) const;
/*
Utility for operators extract\_lyrics and contains\_words

It puts the data of all lyric and titel textmessages into a result string.
For the meaning of the bool parameters see the description of extract\_lyrics
operator value mapping

*/

/*
3.3 Operations

*/

/*
3.3.11 Functions needed to import the ~Midi~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need to be defined here in order for the Midi data type to be used in Tuple definition as an attribute.

*/
    bool     IsDefined() const;
    void     SetDefined(bool Defined);
    size_t   HashValue() const;
    void     CopyFrom(const StandardAttribute* right);
    int      Compare(const Attribute * arg) const;
    bool     Adjacent(const Attribute * arg) const;
    Midi*    Clone() const;
    ostream& Print( ostream &os ) const;
    int      NumOfFLOBs() const;
    FLOB*    GetFLOB(const int);
    size_t   Sizeof() const;

  private:
/*
3.4 Attributes

*/
    bool defined;
/*
A flag that tells if the Midi is defined or not.

*/
    DBArray<TrackEntry> listOfTracks;
/*
Data structure for saving all tracks of a Midi object.

*/
    DBArray<EventEntry> listOfEvents;
/*
Stores the EventEntry objects.

*/
    DBArray<unsigned char> eventData;
/*
Stores the data of the events.

*/
    unsigned char divisionMSB;
/*
Stores the most significant byte of the division of a Midi file.

*/
    unsigned char divisionLSB;
/*
Stores the least significant byte of the division of a Midi file.

*/
    int lengthOfHeader;
/*
Specifies the length of the header information.

*/
    int format;
/*
specifies the format

[->] 0 = single-track

[->] 1 = multiple tracks, synchronous

[->] 2 = multiple tracks, asynchronous

*/
    bool isDeletable;
/*
True, if all DBArrarys of this object can be destroyed.

*/
};

/*
4 Class Track

This class implements a track of a Midi object.

*/

class Track
{
  public:
/*
4.1 Constructors and Destructor

*/
    Track();
/*
This constructor is empty.

*/
    ~Track();
/*
The destructor.

*/
/*
4.2 Member functions

*/
    const Event* GetEvent(int index) const;
/*
Returns the by index selected event of this track.

*/
    void Append( Event* inEvent );
/*
Appends the by index selected track of this Midi object

*/
    const int GetNumberOfEvents() const;
/*
Returns the number of all currently stored events for this track.

*/
    const string GetHeader () const;
/*
Returns the track`s header signature. At the moment it is the constant string ''MTrk''.

*/
   void Transpose(bool inPerc, int hfTnSt, int& errorval );
/*
utilty for operators transpose\_track and transpose\_midi

transposes the calling track the given number of half\_tone\_steps.
The parameter inPerc decides whether percussion channel should be include or
exclude in transposing process

*/

  private:
/*
4.4 Attributes

*/
    vector<Event*> listOfEvents;
/*
Data structure for saving all events of a Midi object.

*/

};

enum EventType {shortmessage, metamessage, sysexmessage};
/*
In this context a shortmessage is the same as a MidiEvent.

*/

/*
5 Class Event

This class includes all kind of events like MidiEvent, MetaEvent and SysEx.

*/
class Event
{
  public:
/*
5.1 Constructors and Destructor

*/
    Event();
/*
This constructor should not be used.

*/
    Event(EventType eventType, unsigned int deltaTime);
/*
This one receives two values for ~eventType~ and ~deltaTime~. The value ~eventType~ specifies the used kind of event. Note that this constructor cannot be called without arguments.

*/
    ~Event();
/*
The destructor.

*/

/*
5.2 Member functions

*/
    EventType GetEventType() const;
/*
Returns the kind of event.

*/
    void SetDeltaTime(unsigned int deltaTime);
/*
Sets the length of this event.

*/
    unsigned int GetDeltaTime() const;
/*
Returns the length of this event.

*/
    void SetShortMessageType(unsigned char shortMessageType);
/*
Stores MidiEvents.

*/
    unsigned char GetShortMessageType() const;
/*
Returns the command of a MidiEvent. The returned value will be undefined if GetShortMessageRunningStauts() returns true.

*/
    void SetShortMessageRunningStatus(bool b);
/*
Sets the ''running status'' of this Midi (channel) event. Default is false.

*/
    bool GetShortMessageRunningStatus() const;
/*
Returns the ''running status'' of this Midi (channel) event. If the status is set to true, this Midi (channel) event has the same
ShortMessageType like the preceding event. In this case the return value of GetShortMessageType will be undefined.

*/
    void SetShortMessageData(int index, unsigned char data);
/*
Stores the data of a MidiEvent.

*/
    unsigned char GetShortMessageData(int index) const;
/*
Return the data of a MidiEvent.

*/
    void SetShortMessageDataLength(int length);
/*
Specifies the length of a MidiEvent.

*/
    int GetShortMessageDataLength() const;
/*
Returns the length of a MidiEvent.

*/
    void SetMetaData(vector<unsigned char>* data);
/*
Stores MetaEvents.

*/
    vector<unsigned char>* GetMetaData(vector<unsigned char>* result) const;
/*
Returns the data of MetaEvents.

*/
    int GetMetaDataLength() const;
/*
Returns the length of a MetaEvent. The returned value is the length of the entire Metamessage. Example: The Metamessage 0xFF 0x01 0x01 0x4D returns the value 4.

*/
    unsigned char GetMetaMessageType() const;
/*
Returns the kind of event.

*/
    void SetSysexData(vector<unsigned char>* data);
/*
Stores the data of a SysexMessage.

*/
    vector<unsigned char>* GetSysexData(vector<unsigned char>* result) const;
/*
Returns the data of a SysexMessage.

*/
    int GetSysexDataLength() const;
/*
Returns the length of a SysexMessage.

*/
    const int GetCommand () const;
/*
Returns the MidiEvent`s command.

*/
    const int GetChannel() const;
/*
Returns the MidiEvent`s channel. To call this method is prohibited if GetShortMessageRunningStatus() returns true. In this case this method will abort with an assertion.

*/
    void GetTextFromMetaEvent(string& result) const;
/*
Returns the text content from a meta event which contains text data of variable size. Some of the meta events between 0x01 and 0x58 are storing text in this way.

*/
    void SetChannel(const int inChannel);
/*
Sets the MidiEvent`s channel.

*/

    static const unsigned char ACTIVE_SENSING        = 0xFE;
    static const unsigned char CHANNEL_PRESSURE      = 0xD0;
    static const unsigned char CONTINUE              = 0xFB;
    static const unsigned char CONTROL_CHANGE        = 0xB0;
    static const unsigned char END_OF_EXCLUSIVE      = 0xF7;
    static const unsigned char MIDI_TIME_CODE        = 0xF1;
    static const unsigned char NOTE_OFF              = 0x80;
    static const unsigned char NOTE_ON               = 0x90;
    static const unsigned char PITCH_BEND            = 0xE0;
    static const unsigned char POLY_PRESSURE         = 0xA0;
    static const unsigned char PROGRAM_CHANGE        = 0xC0;
    static const unsigned char SONG_POSITION_POINTER = 0xF2;
    static const unsigned char SONG_SELECT           = 0xF3;
    static const unsigned char START                 = 0xFA;
    static const unsigned char STOP                  = 0xFC;
    static const unsigned char SYSTEM_RESET          = 0xF3;
    static const unsigned char TIMING_CLOCK          = 0xF8;
    static const unsigned char TUNE_REQUEST          = 0xF6;
/*
These constants represents all commands of a MidiEvent.

*/
    static const unsigned char SEQUENCE_NUMBER       = 0x00;
    static const unsigned char ANY_TEXT              = 0x01;
    static const unsigned char COPYRIGHT             = 0x02;
    static const unsigned char TRACK_NAME            = 0x03;
    static const unsigned char INSTRUMENT_NAME       = 0x04;
    static const unsigned char LYRIC                 = 0x05;
    static const unsigned char CHANNEL_PREFIX        = 0x20;
    static const unsigned char END_OF_TRACK          = 0x2F;
    static const unsigned char TEMPO                 = 0x51;
    static const unsigned char SMPTE_OFFSET          = 0x54;
    static const unsigned char TIME_SIGNATURE        = 0x58;
    static const unsigned char KEY_SIGNATURE         = 0x59;
    static const unsigned char SEQ_SPECIFIC          = 0x7F;
/*
These constants represents some MetaEvents.

*/

/*
5.4 Static methods

*/
static vector<unsigned char>* ComputeIntToBytes( unsigned int arg,
                              vector<unsigned char>* result);
/*
Utility method for computing an integer into a form called ''Variable Length Quantity''. The resulting bytes are stored in the vector result. For using in an expression, the result is also returned as the return value of this function. Explanation of ''Variable Length Quantity'': ''Some numbers in MIDI Files are represented in a form called VARIABLE-LENGTH QUANTITY. These numbers are represented 7 bits per byte, most significant bits first. All bytes except the last have bit 7 set, and the last byte has bit 7 clear. If the number is between 0 and 127, it is thus represented exactly as one byte.''

*/
static unsigned int ComputeBytesToInt(vector<unsigned char>* arg);
/*
Utility method for computing a number which is represented in ''Variable Length Quantity''. See Event::ComputeIntToBytes for details.

*/
 static string FilterString(string textEvents );
/*
Utilty method for extract\_lyrics operator. The pure ASCI data in the
metaevents are not well readable because of several ''break'' characters like "/"
and "\".Moreover some control characters causes overwriting stringparts in the
outstream an have to be replaced as well .This is done here.

*/

/*
5.5 Attributes

*/
  private:
    EventType eventType;
/*
Specifies the kind of event ( MidiEvent, MetaEvent or SysexMessage ). See also definition of EventType.

*/
    unsigned int deltaTime;
/*
Specifies the length of this event.

*/
    unsigned char shortMessageType;
/*
Specifies the MidiEvent`s command.

*/
    bool shortMessageRunningStatus;
/*
Stores the ''running status'' of a Midi (channel) event.

*/
    int shortMessageDataLength;
/*
Specifies the length of a MidiEvent`s command.

*/
    unsigned char shortMessageData[2];
/*
Stores data of MidiEvents.

*/
    vector<unsigned char> dataList;
/*
Stores data of MetaEvents and SysexMessages.

*/
};

/*
6 Utitlity classes

*/

/*
6.1 Class SequenceParser

Helper class for parsing the input of the contains\_sequence operators.
The input string with notes of these operators must be following
the syntax below.

$<input\_list> := <note>,<note\_list>$

$<note\_list>  := <note> | <note>,<note\_list>$

$<note>        := <note\_char><note\_No> | <note\_char><flat\_sharp\_ident><note\_No>$

$<note\_char>  := C | D | E | F | G | A | G | A | B$

$<note\_No>    := -1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9$

$<flat\_sharp\_ident> := b | \#$

*/
class NoteStringToListParser
{
  public:

    NoteStringToListParser();

    bool ParseNoteString(char* inputStr, vector<int>* resultList);
/*
Parses the given string and returns a vector with note numbers. If the
parsing is succesfull the function returns true. Occurs a syntax
error during the parsing the function returns false and the value of
the result vector is undefined.

*/

  private:
    char GetNextChar(char** inputStrPtr);
    int ComputeBaseNoteNo(char currentChar);
};

} // namespace midialgebra

#endif

/*
\pagebreak

8 Operations of the Midi Algebra

In the following you can see the overall list of all implemented operations. Operators already defined in other algebras but also implemented in the MidiAlgebra are cleared out.

*/
