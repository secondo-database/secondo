
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

//paragraph    [1]     Title:         [{\Large \bf \begin {center}]        [\end {center}}]
//paragraph    [21]    table1column:  [\begin{quote}\begin{tabular}{l}]    [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns: [\begin{quote}\begin{tabular}{ll}]   [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns: [\begin{quote}\begin{tabular}{lll}]  [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns: [\begin{quote}\begin{tabular}{llll}] [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters [1] verbatim:   [$]    [$]
//characters [2] formula:    [$]    [$]
//characters [3] capital:    [\textsc{]    [}]
//characters [4] teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[->] [$\rightarrow $]
//[TOC] [\tableofcontents]

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]


1 Implementation of the Midi Algebra

This Algebra provides support for the MIDI music format inside SECONDO.
MIDI songs are using the binary interface via BASE64 coding. Several operators
are available to manipulate tracks, search for content and so on. For more
detailed information look inside the header file. A list containing all
operators you can easily find inside the SPEC file. Detailed information
concerning the operators you can find beside the implementation of each
operator.

The class design follows not directly the hierachical structure of a Midi file. Normally you will create a class ~Midi~ storing references to the included tracks. This class ~track~ would store the references to all included events. Later you would design inherited classes ( from ~Event~ ) for ~MidiEvent~, ~MetaEvent~ and ~SystemEvent~. Due to the restrictions of SECONDO you cannot work with references.

The used design is completely different.

The main class is ~Midi~. It is the only persistent class. ~Midi~ includes infomation about all used types. That means that this class stores information about ~tracks~, ~events~ and ~eventData~ inside separated DBArrays. By that were are able to store Midi files into SECONDO.

The also used classes ~Track~ and ~Event~ are only transient. They are not inherited from ~StandardAttribute~. We use them to follow the object-oriented approach. ~Track~ ( for events ) and ~Event~ ( for event data ) includes their own structures for holding data. These structures are filled on the fly e.g. running SECONDO operators.

2 Defines and Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "FLOB.h"
#include "Base64.h"
#include "MidiAlgebra.h"
#include "FTextAlgebra.h"
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;

/*
The using of a namespace avoids the modifier ''static'' in function
declarations.

*/
namespace midialgebra {

/*
2 Implementation of class ~Midi~

*/

/*
2.1 Basic Constructor

This constructor is not used but necessary

*/
Midi::Midi()
{ }

/*
2.1 Standard Constructor I

This one receives a boolean value ~d~ indicating if the Midi is defined and two
values for ~divisionMSB~ and ~divisionLSB~. Note that this constructor cannot
be called without arguments.

*/
Midi::Midi( const bool defined, const unsigned char divisionMSB,
            const unsigned char divisionLSB ) :
defined      ( defined ),
listOfTracks ( 0 ),
listOfEvents ( 0 ),
eventData    ( 0 ),
divisionMSB  ( divisionMSB ),
divisionLSB  ( divisionLSB )
{
  lengthOfHeader = 6;
  format         = 0;
  isDeletable    = false;
}

/*
2.1 Standard Constructor II

This one receives a ~Midi~ object ~p~ as argument and creates a ~Midi~ that is
a copy of ~p~.

*/
Midi::Midi(const Midi& midi):
listOfTracks ( 0 ),
listOfEvents ( 0 ),
eventData    ( 0 )
{
  defined        = midi.defined;
  divisionMSB    = midi.divisionMSB;
  divisionLSB    = midi.divisionLSB;
  lengthOfHeader = midi.lengthOfHeader;
  format         = midi.format;
  isDeletable    = midi.isDeletable;

  for (int i = 0; i < midi.listOfTracks.Size(); i++)
  {
    const TrackEntry *trackEntry;
    midi.listOfTracks.Get(i, trackEntry);
    listOfTracks.Append(*trackEntry);
  }

  for (int i = 0; i < midi.listOfEvents.Size(); i++)
  {
    const EventEntry *eventEntry;
    midi.listOfEvents.Get(i, eventEntry);
    listOfEvents.Append(*eventEntry);
  }

  for (int i = 0; i < midi.eventData.Size(); i++)
  {
    const unsigned char *data;
    midi.eventData.Get(i, data);
    eventData.Append(*data);
  }
}

/*
2.1 The destructor

*/
Midi::~Midi()
{
  if (isDeletable)
  {
    listOfTracks.Destroy();
    listOfEvents.Destroy();
    eventData.Destroy();
  }
}

/*
2.1 Overloading equals

Overloading the assignment operator.

*/
Midi& Midi::operator=(const Midi& midi)
{
  if (this == &midi)
  {
    return *this;
  }

  defined        = midi.defined;
  divisionMSB    = midi.divisionMSB;
  divisionLSB    = midi.divisionLSB;
  lengthOfHeader = midi.lengthOfHeader;
  format         = midi.format;
  isDeletable    = midi.isDeletable;

  if (midi.listOfTracks.Size() == 0)
  {
    listOfTracks.Clear();
  }
  else
  {
    listOfTracks.Resize(midi.listOfTracks.Size());
  }

  for (int i = 0; i < midi.listOfTracks.Size(); i++)
  {
    const TrackEntry *trackEntry;
    midi.listOfTracks.Get(i, trackEntry);
    listOfTracks.Append(*trackEntry);
  }

  if (midi.listOfEvents.Size() == 0)
  {
    listOfEvents.Clear();
  }
  else
  {
    listOfEvents.Resize(midi.listOfEvents.Size());
  }

  for (int i = 0; i < midi.listOfEvents.Size(); i++)
  {
    const EventEntry *eventEntry;
    midi.listOfEvents.Get(i, eventEntry);
    listOfEvents.Append(*eventEntry);
  }

  if (midi.eventData.Size() == 0)
  {
    eventData.Clear();
  }
  else
  {
    eventData.Resize(midi.eventData.Size());
  }

  for (int i = 0; i < midi.eventData.Size(); i++)
  {
    const unsigned char *data;
    midi.eventData.Get(i, data);
    eventData.Append(*data);
  }

  return *this;
}

/*
2.1 GetTrack

Returns the by index selected track of this ~Midi~ object.

*/
Track* Midi::GetTrack(int index) const
{
  assert( listOfTracks.Size() && index < MAX_TRACKS_MIDI );
  Track* track = new Track();
  const TrackEntry *trackEntry;
  listOfTracks.Get(index, trackEntry);
  int eventPtr = trackEntry->eventPtr;

  for (int i = 0; i < trackEntry->noOfEvents; i++)
  {
    const EventEntry *auxEventEntry;
    listOfEvents.Get(eventPtr++, auxEventEntry);
    EventEntry eventEntry( *auxEventEntry );
    int dataPtr = eventEntry.dataPtr;
    vector<unsigned char> deltaTimeBytes;
    const unsigned char *currentByte;

    do
    {
      eventData.Get(dataPtr++, currentByte);
      deltaTimeBytes.push_back(*currentByte);
      eventEntry.size--;
    } while ((*currentByte & 0x80));

    unsigned int deltaTime = Event::ComputeBytesToInt(&deltaTimeBytes);

    if (eventEntry.type == shortmessageEntry ||
        eventEntry.type == shortmessageRSEntry)
    {
      Event* event = new Event(shortmessage, deltaTime);

      if (eventEntry.type == shortmessageEntry)
      {
        event->SetShortMessageRunningStatus(false);
        const unsigned char *c;
        eventData.Get(dataPtr++, c);
        event->SetShortMessageType(*c);
        eventEntry.size--;
      }
      else
      {
        event->SetShortMessageRunningStatus(true);
      }

      event->SetShortMessageDataLength(eventEntry.size);
      for (int j = 0; j < eventEntry.size; j++)
      {
        const unsigned char *c;
        eventData.Get(dataPtr++, c);
        event->SetShortMessageData(j, *c);
      }
      track->Append(event);
    }
    else
    {
      Event* event;

      if (eventEntry.type == metamessageEntry)
      {
        event = new Event(metamessage, deltaTime);
      }
      else
      {
        event = new Event(sysexmessage, deltaTime);
      }

      vector<unsigned char> messageData;
      for (int j = 0; j < eventEntry.size; j++)
      {
        const unsigned char *c;
        eventData.Get(dataPtr++, c);
        messageData.push_back(*c);
      }

      event->SetMetaData(&messageData);
      track->Append(event);
    }
  }
  return track;
}

/*
2.1 Append

Appends the by index selected track of this ~Midi~ object. The caller of this
method is responsible for destroying the passed Track object after using.

*/
void Midi::Append(const Track *inTrack)
{
  TrackEntry trackEntry;
  trackEntry.noOfEvents = inTrack->GetNumberOfEvents();
  trackEntry.eventPtr   = listOfEvents.Size();
  listOfTracks.Append(trackEntry);

  for (int i = 0; i < inTrack->GetNumberOfEvents(); i++)
  {
    EventEntry eventEntry;
    eventEntry.size    = 0;
    eventEntry.dataPtr = eventData.Size();
    const Event* event = inTrack->GetEvent(i);

    unsigned int deltaTime = event->GetDeltaTime();
    vector<unsigned char> deltaTimeBytes;
    Event::ComputeIntToBytes(deltaTime, &deltaTimeBytes);

    for (unsigned int j = 0; j < deltaTimeBytes.size(); j++)
    {
      unsigned char c = deltaTimeBytes[j];
      eventData.Append(c);
      eventEntry.size++;
    }

    if (event->GetEventType() == shortmessage)
    {
      if (event->GetShortMessageRunningStatus())
      {
        eventEntry.type = shortmessageRSEntry;
      }
      else
      {
        eventEntry.type = shortmessageEntry;
        unsigned char c = event->GetShortMessageType();
        eventData.Append(c);
        eventEntry.size++;
      }

      for (int j = 0; j < event->GetShortMessageDataLength(); j++)
      {
        unsigned char c = event->GetShortMessageData(j);
        eventData.Append(c);
        eventEntry.size++;
      }
    }
    else
    {
      eventEntry.type = event->GetEventType() == metamessage ?
                        metamessageEntry : sysexmessageEntry;
      vector<unsigned char> messageData;
      event->GetMetaData(&messageData);

      for (unsigned int j = 0; j < messageData.size(); j++)
      {
        unsigned char c = messageData[j];
        eventData.Append(c);
        eventEntry.size++;
      }
    }
    listOfEvents.Append(eventEntry);
  }
}

/*
2.1 AppendEmptyTrack

Appends an empty track to this ~Midi~ object.

*/
void Midi::AppendEmptyTrack()
{
  Track* emptyTrack = new Track();

  vector<unsigned char> metaData(14);
  metaData[0]  = 0xFF;
  metaData[1]  = Event::TRACK_NAME;
  metaData[2]  = 11;
  metaData[3]  = 'E';
  metaData[4]  = 'm';
  metaData[5]  = 'p';
  metaData[6]  = 't';
  metaData[7]  = 'y';
  metaData[8]  = ' ';
  metaData[9]  = 'T';
  metaData[10] = 'r';
  metaData[11] = 'a';
  metaData[12] = 'c';
  metaData[13] = 'k';
  Event* trackNameEvent = new Event(metamessage, 0);
  trackNameEvent->SetMetaData(&metaData);
  emptyTrack->Append(trackNameEvent);

  metaData.resize(3);
  metaData[0] = 0xFF;
  metaData[1] = Event::END_OF_TRACK;
  metaData[2] = 0;
  Event* endOfTrackEvent = new Event(metamessage, 0);
  endOfTrackEvent->SetMetaData(&metaData);
  emptyTrack->Append(endOfTrackEvent);

  Append(emptyTrack);
  delete emptyTrack;
}

/*
2.1 GetLyrics

Extract lyrics for operators and concatenats it into a single string

*/
void Midi::GetLyrics(string& result ,bool all, bool lyr,  bool any) const
{
  int nrTrcks = GetNumberOfTracks();
  string eventdata;
  int lastType = -1;
  int currType;

  for (int j = 0; j < nrTrcks; j++)
  {
    Track* currentTrack = GetTrack(j);
    int nrEvts = currentTrack->GetNumberOfEvents();

    for ( int k = 0; k < nrEvts; k++)
    {
      const Event* currentEvent = currentTrack-> GetEvent(k);

      if (currentEvent->GetEventType() == metamessage)
      {
        currType = currentEvent->GetMetaMessageType();

        if (( all & (currType > 0x00 ) & (currType<= 0xFF ))  ||
            ( lyr & currType == Event::LYRIC )                ||
            ( any & currType == Event::ANY_TEXT ))
        {
          if(lastType != currType)
          {
            result += (char)10;
            // newline added if kind of text event changes
          }
          currentEvent-> GetTextFromMetaEvent (result);
          lastType = currType;
        }
      }
    }
  }
  result = Event::FilterString(result);
}

/*
2.1 Implementation of SECONDO`s virtual fucntions

2.1.1 HashValue

Returns a hashvalue for a Midi reducing the complexity to a simple value

*/
size_t Midi::HashValue() const
{
  if(!defined)
  {
    return (0);
  }
  else
  {
    double long h;
    int val = eventData.Size() / 10;
    const unsigned char *ch;

    for (int k = 1; k < 10; k++)
    {
      eventData.Get((val* k),ch);
      double z = *ch *pow((double)2,(double) k-1);
      h += z;
    }
    return size_t(h);
  }
}

/*
2.1.1 CopyFrom

Takes any object of kind StandardAttribute and copies all information from it

*/
void Midi::CopyFrom(const StandardAttribute* right)
{
  const Midi* midi = (const Midi*) right;
  *this = *midi;
}

/*
2.1.1 Compare

Returns always 0 because two Midis are not comparable concerning $''>'' or ''<''$

*/
int Midi::Compare(const Attribute * arg) const
{
  return 0;
}

/*
2.1.1 Adjacent

Returns always false because two Midis cannot be put into an order

*/
bool Midi::Adjacent(const Attribute * arg) const
{
  return false;
}

/*
2.1.1 Standard Clone

Returns a copy of a Midi

*/
Midi* Midi::Clone() const
{
  return new Midi(*this);
}

/*
2.1.1 Extended Clone

Returns a copy of a Midi without tracks if ~copyTracks~ = false, works like
Clone if ~copyTracks~ = true

*/
Midi* Midi::Clone(const bool copyTracks) const
{
  if (copyTracks)
  {
    return this->Clone();
  }
  else
  {
    Midi *newMidi           = new Midi( true, 0, 0);
    newMidi->defined        = this->defined;
    newMidi->divisionMSB    = this->divisionMSB;
    newMidi->divisionLSB    = this->divisionLSB;
    newMidi->format         = this->format;
    newMidi->isDeletable    = this->isDeletable;
    newMidi->lengthOfHeader = this->lengthOfHeader;
    return newMidi;
  }
}

/*
2.1.1 Print

Returns a string including the description ''midi Algebra''

*/
ostream& Midi::Print( ostream &os ) const
{
  return os << MIDI_STRING << " Algebra" << endl;
}

/*
2.1.1 NumOfFLOBs

Returns the number of used DBArrays for Midi

*/
int Midi::NumOfFLOBs() const
{
  return 3;
}

/*
2.1.1 GetFLOB

Returns the address of the required DBArray. Calling this method for the address of tracks you need to put in a 0, a 1 for events`s address and a 2 for eventData`s address.

*/
FLOB* Midi::GetFLOB(const int i)
{
  assert(i >= 0 && i < NumOfFLOBs());
  switch (i)
  {
    case 0:
      return &listOfTracks;
    case 1:
      return &listOfEvents;
    case 2:
      return &eventData;
  }
  assert(false); // may not happen
}

/*
2.1.1 Sizeof

Returns the size of a class instance.

*/
size_t Midi::Sizeof() const
{
  return sizeof(*this);
}

/*
2.2 Destroy

Sets the attribute ~isDeletable~ to true for further destroying by SECONDO

*/
void Midi::Destroy()
{
  isDeletable = true;
}

/*
2.1 Get methods for private attributes

They return the content of an attribute or calculate the selected value on the fly

*/
bool Midi::IsDefined() const
{
  return defined;
}

const int Midi::GetNumberOfTracks () const
{
  return listOfTracks.Size();
}

const int Midi::GetNumberOfTicks () const
{
  return divisionMSB * 256 + divisionLSB;
}

const int Midi::GetFormat () const
{
  return format;
}

const int Midi::GetHeaderLength () const
{
  return lengthOfHeader;
}

const string Midi::GetHeader () const
{
  return MIDI_HEADER;
}

const int Midi::GetFileSize() const
{
  return 14 + listOfTracks.Size() * 8 + eventData.Size();
}

unsigned char Midi::GetDivisionMSB() const
{
  return divisionMSB;
}

unsigned char Midi::GetDivisionLSB() const
{
  return divisionLSB;
}

bool Midi::IsDivisionInFramesFormat() const
{
  return divisionMSB & 0x80;
}

/*
2.2 Set methods for private attributes

They set the attribute to the specified value

*/
void Midi::SetDefined( bool Defined)
{
  this->defined = Defined;
}

void Midi::SetFormat (const int inFormat)
{
  assert( inFormat >= MIN_FORMAT_MIDI && inFormat <= MAX_FORMAT_MIDI );
  this->format = inFormat;
}

void Midi::SetHeaderLength (const int inLength)
{
  this->lengthOfHeader = inLength;
}

void Midi::SetDivisionLSB(const unsigned char lsb)
{
  this-> divisionLSB = lsb;
}

void Midi::SetDivisionMSB(const unsigned char msb)
{
  this-> divisionMSB = msb;
}
/*
2.2 List Representation

The list representation of a ~Midi~ are

----  ( <file>filename.mid</file---> )
----

and

----  ( <text><Base64 decoded file content></text---> )
----

If first representation is used, then the contents of a file is read into the
second representation. This is done automatically by the Secondo parser.

2.3 ~Out~-Function

*/

/*
First some utility functions

*/

void computeShortIntToBytes(int arg, unsigned char& msb,
                            unsigned char& lsb)
/*
Computes the msb and lsb of an integer in the range 0 - 65535.

*/
{
  lsb = arg % 256;
  msb = (arg - lsb) / 256;
}

void computeIntToBytes(unsigned int arg, unsigned char& byte3,
                       unsigned char& byte2, unsigned char& byte1,
                       unsigned char& byte0)
/*
Separates the unsigned integer arg into four bytes. The most significant byte of
the reuslt is byte3, the least significant byte is byte0.

*/
{
  unsigned int rest;

  rest  = arg % 16777216; // 256 ^ 3 = 16777216
  byte3 = (arg - rest) / 16777216;
  arg  -= byte3 * 16777216;
  rest  = arg % 65536; // 256 ^ 2 = 65536
  byte2 = (arg - rest ) / 65536;
  arg  -= byte2 * 65536;
  byte0 = arg % 256;
  byte1 = (arg - byte0) / 256;
}

void midiToBytes(Midi* midi, vector<unsigned char>& resultVector)
{
/*
This functions returns a vector which contains the Midi file its bytes
representation of the passed Midi object. The result vector can be empty, it
will be resized by this function.

*/
  resultVector.resize(14);
  resultVector[0] = 0x4D;
  resultVector[1] = 0x54;
  resultVector[2] = 0x68;
  resultVector[3] = 0x64;
  resultVector[4] = 0;
  resultVector[5] = 0;
  resultVector[6] = 0;
  resultVector[7] = 6;
  resultVector[8] = 0;
  resultVector[9] = midi->GetFormat();
  int noOfTracks  = midi->GetNumberOfTracks();
  unsigned char lsbTracks;
  unsigned char msbTracks;
  computeShortIntToBytes(noOfTracks, msbTracks, lsbTracks);
  resultVector[10] = msbTracks;
  resultVector[11] = lsbTracks;
  int noOfTicks = midi->GetNumberOfTicks();
  unsigned char lsbTicks;
  unsigned char msbTicks;
  computeShortIntToBytes(noOfTicks, msbTicks, lsbTicks);
  resultVector[12] = msbTicks;
  resultVector[13] = lsbTicks;
/*
sets file header

*/
  for (int i = 0; i < noOfTracks; i++)
  {
    Track* track = midi->GetTrack(i);
    resultVector.push_back(0x4D);
    resultVector.push_back(0x54);
    resultVector.push_back(0x72);
    resultVector.push_back(0x6B);
    unsigned int trackSize    = 0;
    unsigned int trackSizePtr = resultVector.size();
    resultVector.push_back(0);
    resultVector.push_back(0);
    resultVector.push_back(0);
    resultVector.push_back(0);
/*
sets track header

*/
    for (int j = 0; j < track->GetNumberOfEvents(); j++)
    {
      // write delta time of event
      const Event* event = track->GetEvent(j);
      vector<unsigned char> deltaTimeBytes;
      Event::ComputeIntToBytes(event->GetDeltaTime(), &deltaTimeBytes);

      for (unsigned int k = 0; k < deltaTimeBytes.size(); k++)
      {
        resultVector.push_back(deltaTimeBytes[k]);
        trackSize++;
      }

      // write event data
      if (event->GetEventType() == shortmessage)
      {
        if (!event->GetShortMessageRunningStatus())
        {
          resultVector.push_back(event->GetShortMessageType());
          trackSize++;
        }

        for (int l = 0; l < event->GetShortMessageDataLength(); l++)
        {
          resultVector.push_back(event->GetShortMessageData(l));
          trackSize++;
        }
      } else if (event->GetEventType() == metamessage) {
          vector<unsigned char> data;
          event->GetMetaData(&data);
          for (unsigned int l = 0; l < data.size(); l++)
          {
            resultVector.push_back(data[l]);
            trackSize++;
          }
      } else {
          vector<unsigned char> data;
          event->GetSysexData(&data);

          for (unsigned int l = 0; l < data.size(); l++)
          {
            resultVector.push_back(data[l]);
            trackSize++;
          }
      }
    }

    unsigned char byte0, byte1, byte2, byte3;
    computeIntToBytes(trackSize, byte3, byte2, byte1, byte0);
    resultVector[trackSizePtr++] = byte3;
    resultVector[trackSizePtr++] = byte2;
    resultVector[trackSizePtr++] = byte1;
    resultVector[trackSizePtr++] = byte0;
    delete track;
  }
}

ListExpr OutMidi( ListExpr typeInfo, Word value )
/*
The real Out-function. Encodes the internal representation into Base64 format.

*/
{
  Midi* midi = (Midi*) value.addr;
  vector<unsigned char> byteVector;
  midiToBytes(midi, byteVector);
  char* bytes = new char[byteVector.size()];

  for (unsigned int i = 0; i < byteVector.size(); i++)
  {
    bytes[i] = byteVector[i];
  }

  Base64 b;
  string textBytes;
  b.encode(bytes, byteVector.size(), textBytes);
  delete[] bytes;

  ListExpr textAtom = nl->TextAtom();
  nl->AppendText(textAtom, textBytes);
  ListExpr result = nl->ThreeElemList(textAtom,
                    nl->IntAtom(midi->GetFileSize()),
                    nl->IntAtom(midi->GetNumberOfTracks()));
  return result;
}


/*
2.1 Utility functions used by infunction

The interaction between these functions works as follows:

The function ~InMidi~ constructs a Midi object with zero ticks and the
standard header. It takes the coded text representing the whole Midi file
and transforms it into a c++ string. Then it calls the ~InHeader~ function
with both Midi and string as parameters.

~InHeader~ puts the string into an stringstream and reads out all header
informations of the Midi file. Syntactical correctness is tested and
then all header informations are put in the Midi object.

Then ~InTracks~ is called with the Midi ( in construction ), the stream
( without header) and the number of tracks as parameters. ~InTracks~
realizes a loop over the number of tracks , counts the bytelength of
every track and reads the bytes according to one track out of the stream.

The track header information are read, checked and a Track object is
constructed and filled with Events ( which are in the current streampart )
by the ~InEvents~ function.

~InEvents~ reads out the stream event by event as long as the tracklength
allows. At first deltatime and the event type characterizing bytes are
read and than the corresponding Event object is constructed. For every
Event type there is a function ( ~InMetaEvent~, ~InSystemmessage~, ...) which
checks the syntax of this Event ( also some context checks are done ) and the
information of these events are put into corresponding structures. Then the
events are appended to the current track.

Given back this track to ~InTracks~ the
information of the whole track are put into the Midi object structures
( DBArrays ) by function ~AppendTrack~. Because these structures are
independend from track objects the track objects have to be deleted after
appending them.

Note that all deviation from Midi Standard found out in syntax or context
checks causes error messages and avoid constructing a Midi object. This
can lead to the situation that a  Midi is playable in usual Midiplayer
program but not loadable into SECONDO. This decision follows our
intention concerning consistence and uniqueness of database objects.

*/
bool InSystemmessage(Event*& actualEvent,stringstream& byteStream,
                     unsigned char& typebyte, unsigned char& namebyte,
                     bool& multisysex, bool& sequenceSpecified,
                     bool& runmodepossible, int tracknr)
{
  char ch;

  if (multisysex)
  {
    cout << "Error in track "<< tracknr << "." << endl;
    cout << "Sysexpackage interrupted | closed by F7 byte. ";
    cout << "Last 2 read bytes are: " << (unsigned int)typebyte;
    cout << ", " << (unsigned int)namebyte;
    return false;
  }
  else
  {
    runmodepossible   = false;
    sequenceSpecified = true;
    actualEvent->SetShortMessageType(typebyte);

    if(typebyte == Event::SONG_POSITION_POINTER)
    {
      actualEvent->SetShortMessageDataLength(2);
      actualEvent->SetShortMessageData(0,namebyte);
      byteStream.get(ch);
      actualEvent->SetShortMessageData(1, (unsigned char)ch);
    }

    if(typebyte == Event::SYSTEM_RESET)
    {
      actualEvent->SetShortMessageDataLength(1);
      actualEvent->SetShortMessageData(0, namebyte);
    }

    if((typebyte == Event::TUNE_REQUEST)  ||
       (typebyte == Event::TIMING_CLOCK)  ||
       (typebyte == Event::START)         ||
       (typebyte == Event::CONTINUE)      ||
       (typebyte == Event::ACTIVE_SENSING))
    {
      actualEvent-> SetShortMessageDataLength(0);
      byteStream.putback(namebyte);
    }

    return true;
  }
}



bool InChannelVoiceRun(Event*& actualEvent,stringstream& byteStream,
                       unsigned char& typebyte, unsigned char& namebyte,
                       bool& multisysex, bool& sequenceSpecified,
                       bool& runmodepossible,  int tracknr)
{
  int datalength;

  if (!runmodepossible)
  {
    cout << "Error in track " << tracknr << "." << endl;
    cout << "Error runmode event without ";
    cout << "previuos channelmessage " ;
    cout << (unsigned int)typebyte << "," << (unsigned int)namebyte;
    return false;
  }

  if(multisysex)
  {
    cout << "Error in track " << tracknr << "." << endl;
    cout << "Sysexpackage interupted or not closed ";
    cout << "by F7 byte. Last two read bytes are: ";
    cout << (unsigned int)typebyte << "," << (unsigned int)namebyte;
    return false;
  }

  sequenceSpecified = true;
  actualEvent->SetShortMessageRunningStatus(true);

  if (datalength == 1 )
  {
    actualEvent-> SetShortMessageDataLength(1);
    actualEvent-> SetShortMessageData(0, typebyte);
    byteStream.putback(namebyte);
  }
  else
  {
    actualEvent-> SetShortMessageDataLength(2);
    actualEvent-> SetShortMessageData(0, typebyte);
    actualEvent-> SetShortMessageData(1, namebyte);
  }
  return true;
}

bool InChannelVoiceNoRun(Event *& actualEvent,stringstream& byteStream,
                         unsigned char& typebyte, unsigned char& namebyte,
                         bool& multisysex, bool& sequenceSpecified,
                         bool& runmodepossible, int tracknr)
{
  int datalength;
  char ch;

  if (multisysex)
  {
    cout << "Error in track " << tracknr << "." << endl;
    cout << "Sysexpackage interrupted or closed by F7 byte. ";
    cout << "Last two read bytes are: " << (unsigned int)typebyte;
    cout << "," << (unsigned int)namebyte;
    return false;
  }
  else
  {
    runmodepossible   = true;
    sequenceSpecified = true;
    actualEvent->SetShortMessageType(typebyte);
    actualEvent->SetShortMessageRunningStatus(false);

    if((typebyte & 0xF0) == Event::PROGRAM_CHANGE ||
       (typebyte & 0xF0) == Event::CHANNEL_PRESSURE)
    {
      datalength = 1;
      actualEvent->SetShortMessageDataLength(1);
      actualEvent->SetShortMessageData(0, namebyte);
    }
    else
    {
      datalength = 2;
      actualEvent-> SetShortMessageDataLength(2);
      actualEvent-> SetShortMessageData(0, namebyte);
      byteStream.get(ch);
      actualEvent->SetShortMessageData(1, (unsigned char)ch);
    }

    return true;
  }
}

bool InSysex(Event*& actualEvent, stringstream& byteStream,
             vector<unsigned char>*& data, unsigned char& typebyte,
             unsigned char& secbyte, bool& multisysex,
             bool& sequenceSpecified, bool& runmodepossible, int tracknr)
{
  vector<unsigned char> * eventLengthVector = new vector<unsigned char>;
  char ch;

  if(typebyte == 0xF0)
  {
    if (multisysex)
    {
      cout << "Error in track " << tracknr << "." << endl;
      cout << "Sysexpackage interrupted or not closed by F7 byte. " ;
      cout << "Last two read bytes are: " << (unsigned int)typebyte;
      cout << "," << (unsigned int)secbyte;
      return false;
    }
    else
    {
      multisysex = true;
    }
  }

  if(typebyte == 0xF7)
  {
    if (!multisysex)
    {
      cout << "Error in track " << tracknr << endl;
      cout << "sysexpackage interrupted or not closed by F7 byte. " ;
      cout << "Last two read bytes are: "<< (unsigned int)typebyte;
      cout << "," << (unsigned int)secbyte;
      return false;
    }
  }

  runmodepossible = false;
  data->push_back(typebyte);
  data->push_back(secbyte);
  int bytecounter = 1;
  eventLengthVector->push_back(secbyte);

  if (secbyte > 128 )
  {
    do
    {
      byteStream.get(ch);
      eventLengthVector->push_back((unsigned char)ch);
      data->push_back((unsigned char)ch);
      bytecounter ++;
    } while ((unsigned char)ch > 128 && bytecounter <= 5);
  }

  if ( bytecounter > 4 )
  {
    cout << "Error in track " << tracknr << "." << endl;
    cout << "Eventlength longer than 4 bytes. ";
    cout << "Last two read bytes are: " << (unsigned int)typebyte;
    cout << "," << (unsigned int)secbyte;
    return false;
   }
   else
   {
     int eventLength =  Event::ComputeBytesToInt(eventLengthVector);
     delete eventLengthVector;
     int j = 0;

     while (j < eventLength)
     {
       byteStream.get(ch);
       data->push_back((unsigned char)ch);
       j++;
     }

     typebyte = (unsigned char)ch;

     if(!typebyte == 0xF7)
     {
       multisysex &= true;
     }
     else
     {
       multisysex =false;
     }

     sequenceSpecified = true;
     return true;
   }
}


bool InSequenceNumberEvent(stringstream& byteStream,
                           vector<unsigned char>* & data,
                           bool& deltagreaterO, bool& sequenceSpecified,
                           int tracknr, unsigned char cha,
                           unsigned char chb)
{
  char ch;
  byteStream.get(ch); // first byte of length

  if ((!sequenceSpecified) && (!deltagreaterO))
  {
    if ((unsigned int)ch == 0x02)
    {   // valid length, get data of meta event
      data->push_back(ch);

      for( int j = 1; j < 3; j++)
      {
        byteStream.get(ch);
        data->push_back((unsigned int)ch);
      }
    }
    else
    {
      cout << "Error in track " << tracknr << "." << endl;
      cout << "False length of sequence event." << endl ;
      cout << "Last two read bytes are: " << (unsigned int)cha;
      cout << "," << (unsigned int)chb;
      return  false;
    }
  }
  else
  {
    cout << "Error in track " << tracknr << "." << endl;
    cout << "Sequence event should be before any delta time > 0 ";
    cout << "and any other seq specifying event. ";
    cout << "Last two read bytes are: " << (unsigned int)cha;
    cout << "," << (unsigned int)chb << endl;
    return false;
  }

  return true;
}

bool InMetaEvent(Event*& actualEvent,stringstream& byteStream,
                 vector<unsigned char>*& data,
                 unsigned char& typebyte, unsigned char& namebyte,
                 bool& multisysex, bool& deltagreaterO,
                 bool& sequenceSpecified, bool& endOfTrackFound,
                 bool& runmodepossible, int tracknr)
{
  vector<unsigned char>* eventLengthVector = new vector<unsigned char>;
  char ch;

  if(multisysex)
  {
    cout << "Error in track " << tracknr << "." << endl;
    cout << "Sysexpackage interrupted or not closed by F7 byte.";
    cout << endl << "Last two read bytes are: ";
    cout << (unsigned int)typebyte << "," << (unsigned int) namebyte;
    return false;
  }

  runmodepossible = false;
  data->push_back(typebyte);
  data->push_back(namebyte);

  if( namebyte == 0x00)     // sequenceNumberEvent found
  {
    if( !( InSequenceNumberEvent( byteStream, data,
           deltagreaterO, sequenceSpecified,
           tracknr, typebyte, namebyte)))
    {
      return false;
    }
  }
  else
  {
    int bytecounter = 0;

    do                 // counting of eventlength
    {
      byteStream.get(ch);
      eventLengthVector->push_back((unsigned char)ch);
      data->push_back((unsigned char) ch);
      bytecounter ++;
    } while ( (unsigned char)ch > 128 && bytecounter <= 5) ;

    if (bytecounter > 4)
    {
      cout << "Error in track " << tracknr << "." << endl;
      cout << "Event length > 4 bytes detected. ";
      cout << "Last two read bytes are: ";
      cout << (unsigned int)typebyte << ",";
      cout << (unsigned int)namebyte;
      return false;
    }

    int eventLength = Event::ComputeBytesToInt(eventLengthVector);
    delete eventLengthVector;

    if (namebyte == Event::END_OF_TRACK)
    {
      endOfTrackFound = true;
      byteStream.get(ch);
    }

    for( int j = 0; j < eventLength; j++ )
//put message in vektor
    {
      byteStream.get(ch);
      data->push_back((unsigned char)ch);
    }
  }
  sequenceSpecified = true;
  return true;
}


bool InEvents(Track* actualTrack, int nob, char* bytes, int tracknr )
{
  stringstream byteStream;
  byteStream.write(bytes,nob);
  char ch;
  unsigned char cha;
  unsigned char chb;
  int deltatime;

  bool sequenceSpecified = false;
  bool multisysex        = false;
  bool deltagreaterO     = false;
  bool runmodepossible   = false;
  bool endOfTrackFound   = false;
  int  eventcount        = 0;

  vector<unsigned char>* variableLength =
        new vector<unsigned char>;
  vector<unsigned char>* eventLengthVector =
      new vector<unsigned char>;
  vector<unsigned char>* data = new vector<unsigned char>;

  while (byteStream.good())
  {
    eventcount ++;
    if(endOfTrackFound)
    {
      cout <<" left bytes after EndofTrack message at last Track";
      return false;
    }
    int bytecounter = 0;

    do
    {
// counting of deltatime
      byteStream.get(ch);
      variableLength->push_back((unsigned int)ch);
      bytecounter ++;
      } while (((unsigned int) ch > 128) && (bytecounter < 6));

    if (bytecounter > 4)
    {
      cout << endl << "Error in Track " <<  tracknr << "." << endl;
      cout << "Deltatime > 4 bytes detected." << endl;
      cout << "Last read bytes are : fortype : " << (unsigned int) cha;
      cout << " forname: " << (unsigned int)chb;
      return false;
    }

    deltatime =  Event:: ComputeBytesToInt(variableLength);
    deltagreaterO |= ( deltatime >0 );
    byteStream.get(ch);

    cha = (unsigned char)ch;     // type of event
    byteStream.get(ch);
    chb = (unsigned char)ch;     // name of event

    if(cha == 0xFF)              // metaEvent found
    {
      Event* actualEvent= new Event(metamessage, deltatime);

      if(InMetaEvent(actualEvent, byteStream, data, cha, chb,
                     multisysex, deltagreaterO,
                     sequenceSpecified,endOfTrackFound,
                     runmodepossible,tracknr))
      {
        actualEvent->SetMetaData(data);
        actualTrack->Append(actualEvent);
        data->clear();
      }
      else
      {
        return false;
      }
    }
    else                         // end of metamessage
    {
      if(( cha == 0xF0 ) || (cha == 0xF7))  // sysEx message found
      {
        Event* actualEvent= new Event(sysexmessage, deltatime);

        if( InSysex(actualEvent, byteStream, data, cha, chb,
                    multisysex, sequenceSpecified,
                    runmodepossible, tracknr))
        {
          actualEvent->SetSysexData(data);
          actualTrack->Append(actualEvent);
          data->clear();
        }
        else
        {
          return false;
        }
      }
      else                       // end of sysEx message
      {
        if (((cha & 0xF0) >= 0x80) & ((cha & 0xF0) <= 0xE0))
        {
          Event* actualEvent = new Event(shortmessage, deltatime);

          if( InChannelVoiceNoRun( actualEvent, byteStream,
                                   cha, chb, multisysex,
                                   sequenceSpecified,
                                   runmodepossible, tracknr))
          {
            actualTrack->Append(actualEvent);
          }
          else
          {
            return false;
          }
        }
        else         // end of channelshortmessage - no runmode
        {
          if( cha < 128 )        // running mode found
          {
            Event* actualEvent = new Event(shortmessage, deltatime);

            if( InChannelVoiceRun( actualEvent,byteStream,
                                   cha, chb, multisysex,
                                   sequenceSpecified,
                                   runmodepossible,tracknr))
            {
              actualTrack->Append(actualEvent);
            }
            else
            {
              return false;
            }
          }
          else        // end of running mode
          {
            if ((cha & 0xF0 == 0xF0) &  // sysEx message found
                (cha & 0x0F != 0x0F))
            {
              Event* actualEvent = new Event(shortmessage, deltatime);
              if ( InSystemmessage(actualEvent, byteStream,cha, chb,
                                   multisysex, sequenceSpecified,
                                   runmodepossible, tracknr))
              {
                actualTrack->Append(actualEvent);
              }
              else
              {
                return false;
              }
            }
            else
            {
              cout << "error in track " << tracknr << ":";
              cout << endl << "Unknown event found. ";
              cout << "Last two bytes are: " << cha;
              cout << "," << chb;
              return false;
            }
          }
        }
      }
    }
    data->clear();
    eventLengthVector->clear();
    variableLength->clear();
  }

  if (!endOfTrackFound)
  {
    cout <<" endoftrack message missing after track " << tracknr;
    return false;
  }

  if (multisysex)
  {
    cout << "sysexpackage not closed in track " << tracknr;
    return false;
  }

  return true;
}

 bool InTracks(Midi*& actualMidi ,stringstream& byteStream,
               int nrtracks)
{
  char* longw = new char[5];
  unsigned int tracklen;

  for (int k = 1; k <= nrtracks; k++)
  {
    byteStream.read(longw, 4);
    longw[4] = '\0';

    if(((string(longw).find(TRACK_HEADER))) == 0 )
// track header found
    {
      int exp  = 3;
      tracklen = 0;

      for ( int j = 0; j <= 3; j++)
// count bytelength of track
      {
        byteStream.get(longw[j]);
        tracklen +=  (unsigned char)longw[j] *
          ((unsigned int) pow((double)256, exp));
        exp--;
      }

      char* evsOfTrack = new char[tracklen];
      byteStream.read(evsOfTrack, tracklen);
      Track* actualTrack = new Track() ;

      if(!(InEvents(actualTrack, tracklen,evsOfTrack,k)))
      {
        return false;
      }
      else
      {
        delete evsOfTrack;
        actualMidi->Append(actualTrack);
        delete actualTrack;
      }
    }
    else
    {
      cout << "Error: " << TRACK_HEADER << " missing after Track ";
      cout << (k-1);
      return false;
    }
  }
  char ch;
  byteStream.get(ch);

  if(byteStream.good())
  {
    cout << "bytes left after last track";
    return false;
  }
  else
  {
    return true;
  }
}

 bool InHeader (Midi*& actualmidi, string& b64s)
{
  Base64 b;
  char* longw  = new char[5];
  char* shortw = new char[2];
  unsigned int nrtracks;
  int frmt;
  int headerlen = 0;
  stringstream byteStream;
  int sizeDecoded = b.sizeDecoded( b64s.size() );
  char *bytes = new char[ sizeDecoded ];
  int bytelength =  b.decode(b64s, bytes);
  byteStream.write( bytes, bytelength);
  delete bytes;
  byteStream.read(longw,4);
  longw[4] = '\0';

  if ( string(longw).find(MIDI_HEADER) == 0 )   // Midi header found
  {
    int exp = 3;
    for (int j = 0; j <= 3 ;j++)
    {
      byteStream.get(longw[j]);
      headerlen +=
        (longw[j]) * ((unsigned int) pow ((double) 256, exp));
      exp--;
    }

    if(headerlen == 6)
    {
      shortw = new char[2];
      byteStream.read( shortw,2 );
      frmt = shortw[1]+shortw[0]* 128;

      if(frmt == 1 || frmt == 2 || frmt == 0)   // used types
      {
        actualmidi->SetFormat( frmt);
        byteStream.read( shortw, 2);
        nrtracks = (unsigned char)shortw[1] +
                   (unsigned char)shortw[0] * 256;

        if((frmt ==1) || (((frmt ==2) || frmt == 0) & nrtracks == 1 ))
        {
          // correct type -nrtr combination
          byteStream.read( shortw, 2);

          if ((unsigned int)shortw[0] < 128)
          {
            actualmidi->SetDivisionLSB((unsigned char)shortw[1]);
            actualmidi->SetDivisionMSB((unsigned char)shortw[0]);
          }
          else
          {
            // framrate = shortw[0];
            actualmidi->SetDivisionLSB((unsigned char)shortw[1]);
            actualmidi->SetDivisionMSB((unsigned char)shortw[0]);
          }
        }
        else
        {
          cout << "Error: invalid relation between ";
          cout << "format and numberOfTracks.";
          return false;
        }
      }
      else
      {
        cout << "Error: unsupported number of format.";
        return false;
      }
    }
    else
    {
      cout << "Error: not supported length of header.";
      return false ;
    }
  }
  else
  {
    cout << "Error: " << MIDI_HEADER << " missing.";
    return false ;
  }

  if ( InTracks( actualmidi, byteStream , nrtracks))
  {
    return true ;
  }
  else
  {
    return false;
  }
}


/*
2.4 ~In~-Function

*/
static Word InMidi(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if (((nl->IsAtom(instance) && nl->AtomType(instance) == TextType )) ||
     ((nl->ListLength(instance) == 3 ) && nl->IsAtom(nl->First(instance))
      && (nl->AtomType( nl->First(instance))  == TextType)
      && (nl->IsAtom(nl->Second(instance)))
      && (nl->AtomType( nl->Second(instance)) == IntType)
      && (nl->IsAtom(nl->Third(instance)))
      && (nl->AtomType( nl->Third(instance))  == IntType)))
  {
    Midi * midi = new Midi( true, 0, 0  );
    string encoded;

    if(nl->IsAtom(instance))
    {
      nl->Text2String( instance, encoded );
    }
    else
    {
      nl->Text2String( (nl->First( instance )), encoded );
    }

    if(InHeader(midi, encoded ))
    {
      correct = true;
      return SetWord( midi);
    }

    correct = false;
    errorInfo = nl->Append(errorInfo,nl->FourElemList(
                nl->IntAtom(70), nl->SymbolAtom(MIDI_STRING),
                nl->IntAtom(1), nl->IntAtom(19)));
    return SetWord(Address(0));
  }

  correct = false;
  return SetWord(Address(0));
}


/*
2.5 The ~Property~-function

*/
ListExpr MidiProperty()
{
  return (nl->TwoElemList(
          nl->FiveElemList(nl->StringAtom("Signature"),
          nl->StringAtom("Example Type List"),
          nl->StringAtom("List Rep"),
          nl->StringAtom("Example List"),
          nl->StringAtom("Remarks")),
          nl->FiveElemList(nl->StringAtom("-> DATA"),
          nl->StringAtom(MIDI_STRING),
          nl->StringAtom("( <file>filename</file---> )"),
          nl->StringAtom("( <file>Document.mid</file---> )"),
          nl->StringAtom(""))));
}

/*
2.6 ~Create~-function

*/
Word CreateMidi( const ListExpr typeInfo )
{
  return SetWord( new Midi( true, 0, 0 ) );
}

/*
2.7 ~Delete~-function

*/
void DeleteMidi( const ListExpr typeInfo, Word& w )
{
  Midi *midi = (Midi *)w.addr;
  midi->Destroy();
  delete midi;
  w.addr = 0;
}

/*
2.8 ~Close~-function

*/
void CloseMidi( const ListExpr typeInfo, Word& w )
{
  delete (Midi *)w.addr;
  w.addr = 0;
}

/*
2.9 ~Clone~-function

*/
Word CloneMidi( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((Midi *)w.addr)->Clone() );
}

/*
2.10 ~SizeOf~-function

*/
int SizeOfMidi()
{
  return sizeof(Midi);
}

/*
2.11 ~Cast~-function

*/
void* CastMidi( void* addr )
{
  return new (addr) Midi;
}


/*
2.14 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~Midi~ does not have arguments, this is trivial.

*/
bool CheckMidi( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MIDI_STRING ));
}

/*
2.15 Creation of the Type Constructor Instance

*/
TypeConstructor midi(
        MIDI_STRING,            //name
        MidiProperty,           //property funct. describing signature
        OutMidi,    InMidi,     //Out and In functions
        0,          0,          //list functions
        CreateMidi, DeleteMidi, //object creation and deletion
        0,   0,                 //object open and save
        CloseMidi,  CloneMidi,  //object close and clone
        CastMidi,               //cast function
        SizeOfMidi,             //sizeof function
        CheckMidi );            //kind checking function

/*
2 Implementation of class ~Track~

2.1 The constructor.

*/
Track::Track()
{ }

/*
2.1 The destructor.

*/
Track::~Track()
{
  for (unsigned int i = 0; i < listOfEvents.size(); i++)
  {
    delete listOfEvents[i];
  }
}

/*
2.1 GetEvent

Returns the by index selected event of this track.

*/
const Event* Track::GetEvent(int index) const
{
  return listOfEvents[index];
}

/*
2.1 Append

Appends the by index selected track of this Midi object

*/
void Track::Append(Event* inEvent)
{
  listOfEvents.push_back(inEvent);
}

/*
2.1 Get methods for private attributes

They return the content of an attribute or calculate the selected value on the fly

*/
const int Track::GetNumberOfEvents () const
{
  return listOfEvents.size();
}

const string Track::GetHeader () const
{
  return TRACK_HEADER;
}

/*
2.2 Utilty methods for operators inside class ~Track~

*/

/*
2.2.1 transpose

This is an utility for operator transpose\_midi implicit track given
number of halfsteps tones including/excluding transposing percussion channel

*/

void Track::Transpose( bool inPerc, int hfTnSt, int& errorval )
{
  int  note;
  bool runmode    = false;
  bool perChannel = false;
  int  nrEvts     = GetNumberOfEvents();
  unsigned char currenttype;

  for ( int k = 0; k < nrEvts; k++)
  {
    const Event* auxCurrentEvent = GetEvent(k);
    Event *currentEvent = const_cast<Event*>( auxCurrentEvent );
    if (currentEvent->GetEventType() == shortmessage)
    {
      runmode = currentEvent->GetShortMessageRunningStatus();

      if(!runmode)
      {
        if((currentEvent->GetChannel() != 9)|| inPerc )
        {
          currenttype = currentEvent-> GetShortMessageType();

          if((0xF0 & currenttype) == Event:: NOTE_ON ||
            ((0xF0 & currenttype) == Event:: NOTE_OFF))
          {
            note = (unsigned int)currentEvent->GetShortMessageData(0);

            if ((note + hfTnSt)< 128 & (note + hfTnSt) > -1)
            {
              currentEvent-> SetShortMessageData(0,
                (unsigned char)(note + hfTnSt));
              perChannel = false;
            }
            else
            {
              errorval = 1;
              cout << "note " << note << " step " << hfTnSt;
              return;
            }
          }
        }
        else     // percussion channel and excluding them demanded
        {
          perChannel = true;
        }
      }
      else      // runmode
      {
        if(!perChannel)
        {
          note = (unsigned int)currentEvent->GetShortMessageData(0);
          if ((note + hfTnSt) < 128  & (note + hfTnSt) > -1)
          {
            currentEvent->SetShortMessageData(0,
              (unsigned char)(note + hfTnSt));
            perChannel = false;
          }
          else
          {
            errorval = 1;
            return;
          }
        }
      }
    }
  }
}

/*
2 Implementation of class ~Event~

2.1 Standard constructor I

This constructor should not be used but it is necessary.

*/
Event::Event()
{ }

/*
2.1 Standard constructor II

This one receives two values for ~eventType~ and ~deltaTime~. The value ~eventType~ specifies the used kind of event. Note that this constructor cannot be called without arguments.

*/
Event::Event(EventType eventType, unsigned int deltaTime)
{
  this->eventType = eventType;
  this->deltaTime = deltaTime;
  shortMessageRunningStatus = false;
}

/*
2.1 The destructor

*/
Event::~Event()
{ }

/*
2.1 GetTextFromMetaEvent

Returns the text content from a meta event which contains text data of variable size. Some of the meta events between 0x01 and 0x58 are storing text in this way.

*/
void Event::GetTextFromMetaEvent(string& result) const
{
  assert(eventType == metamessage);
  int metaEvent = GetMetaMessageType();
  assert(metaEvent >= 0x01 && metaEvent <= 0xFF );

  vector<unsigned char> data;
  vector<unsigned char> lengthBytes;
  int dataPtr = 2;

  do
  {
    lengthBytes.push_back(dataList[dataPtr++]);
  } while ((dataList[dataPtr - 1] & 0x80));

  int dataLength      = Event::ComputeBytesToInt(&lengthBytes);
  unsigned char* cstr = new unsigned char[dataLength + 1];
  int cstrPtr         = 0;

  for (int k = 0; k < dataLength; k++)
  {
    cstr[cstrPtr] = dataList[dataPtr++];
    cstrPtr++;
  }
  cstr[cstrPtr] = 0;
  result +=(char*)cstr;
  delete[] cstr;
}


/*
2.1 Get methods for private attributes

They return the content of an attribute or calculate the selected value on the fly

Returns the data of a SysexMessage.

*/
vector<unsigned char>* Event::GetSysexData(vector<unsigned char>*result) const
{
  assert(eventType == sysexmessage || eventType == metamessage);
  for (unsigned int i = 0; i < dataList.size(); i++)
  {
    result->push_back(dataList[i]);
  }
  return result;
}

/*
Returns the length of a SysexMessage.

*/
int Event::GetSysexDataLength() const
{
  assert(eventType == sysexmessage || eventType == metamessage);
  return dataList.size();
}

/*
Returns the data of MetaEvents.

*/
vector<unsigned char>* Event::GetMetaData(vector<unsigned char>* result) const
{
  assert(eventType == metamessage || eventType == sysexmessage);
  for (unsigned int i = 0; i < dataList.size(); i++)
  {
    result->push_back(dataList[i]);
  }
  return result;
}

/*
Returns the length of a MetaEvent. The returned value is the length of the entire Metamessage. Example: The Metamessage 0xFF 0x01 0x01 0x4D returns the value 4.

*/
int Event::GetMetaDataLength() const
{
  assert(eventType == metamessage || eventType == sysexmessage);
  return dataList.size();
}

/*
Returns the kind of event.

*/
unsigned char Event::GetMetaMessageType() const
{
  assert(eventType == metamessage && dataList.size() > 2);
  return dataList[1];
}

/*
Returns the length of a MidiEvent.

*/
int Event::GetShortMessageDataLength() const
{
  assert(eventType == shortmessage);
  return shortMessageDataLength;
}

/*
Return the data of a MidiEvent.

*/
unsigned char Event::GetShortMessageData(int index) const
{
  assert(eventType == shortmessage && index >= 0 && index < 2);
  return shortMessageData[index];
}

/*
Returns the command of a MidiEvent. The returned value will be undefined if GetShortMessageRunningStauts() returns true.

*/
unsigned char Event::GetShortMessageType() const
{
  assert(shortMessageRunningStatus == false);
  assert(eventType == shortmessage);
  return shortMessageType;
}

/*
Returns the kind of event.

*/
EventType Event::GetEventType() const
{
  return eventType;
}

/*
Returns the length of this event.

*/
unsigned int Event::GetDeltaTime() const
{
  return deltaTime;
}

/*
Returns the MidiEvent`s channel. To call this method is prohibited if GetShortMessageRunningStatus() returns true. In this case this method will abort with an assertion.

*/
const int Event::GetChannel () const
{
  assert(shortMessageRunningStatus == false);
  assert(eventType == shortmessage);
  assert((shortMessageType & 0xF0) != 0xF0);
  int channel = shortMessageType & 0x0F;
  return channel;
}

/*
Returns the ''running status'' of this Midi (channel) event. If the status is set to true, this Midi (channel) event has the same
ShortMessageType like the preceding event. In this case the return value of GetShortMessageType will be undefined.

*/
bool Event::GetShortMessageRunningStatus() const
{
  return shortMessageRunningStatus;
}

/*
2.2 Set methods for private attributes

They sets the attribute to the specified value.

*/
void Event::SetDeltaTime(unsigned int deltaTime)
{
  this->deltaTime = deltaTime;
}

void Event::SetShortMessageType(unsigned char shortMessageType)
{
  assert(eventType == shortmessage);
  this->shortMessageType = shortMessageType;
}

void Event::SetShortMessageData(int index, unsigned char data)
{
  assert(eventType == shortmessage);
  assert(index >= 0 && index < 2);
  shortMessageData[index] = data;
}

void Event::SetShortMessageDataLength(int length)
{
  assert(eventType == shortmessage && length >= 0 && length < 3);
  shortMessageDataLength = length;
}

void Event::SetMetaData(vector<unsigned char>* data)
{
  assert(eventType == metamessage || eventType == sysexmessage);
  for (unsigned int i = 0; i < data->size(); i++)
  {
    dataList.push_back((*data)[i]);
  }
}

void Event::SetSysexData(vector<unsigned char>* data)
{
  assert(eventType == sysexmessage || eventType == metamessage);
  for (unsigned int i = 0; i < data->size(); i++)
  {
    dataList.push_back((*data)[i]);
  }
}

void Event::SetChannel(const int channel)
{
  assert(eventType == shortmessage);
  assert((shortMessageType & 0xF0) != 0xF0);
  unsigned char channelNo = channel;
  shortMessageType &= 0xF0;
  shortMessageType |= channelNo;
}

void Event::SetShortMessageRunningStatus(bool status)
{
  shortMessageRunningStatus = status;
}

/*
2.3 Static methods

2.1.1 ComputeIntToBytes

Transforms an integer value into the necessary byte representation used by Midi.

*/
vector<unsigned char>* Event::ComputeIntToBytes(unsigned int arg,
                              vector<unsigned char>* result)
{
  result->clear();
  if (!arg)
  {
    result->push_back(0);
    return result;
  }

  bool lastDigits = false;
  for (int i = 3; i >= 0; i--)
  {
    unsigned char rest    =
      (unsigned char)(arg % (unsigned int) pow((double) 128, i));
    unsigned char divisor =
      (unsigned char)((arg - rest) / (unsigned int) pow((double) 128, i));

    if (lastDigits || divisor != 0)
    {
      lastDigits = true;
      arg       -= (divisor * (unsigned int) pow((double) 128, i));
      divisor   |= 0x80;
      result->push_back(divisor);
    }
  }
  (*result)[result->size() - 1] &= 0x7F;
  return result;
}

/*
2.1.1 ComputeBytesToInt

Calculates the integer value out of an array of bytes.

*/
unsigned int Event::ComputeBytesToInt(vector<unsigned char>* arg)
{
  int exp = 0;
  unsigned int result = 0;

  for (int i = arg->size() - 1; i >= 0; i--)
  {
    unsigned char value = (*arg)[i];
    value &= 0x7F;
    result += value * ((unsigned int) pow((double) 128, exp));
    exp++;
  }
  return result;
}

/*
2.1.1 FilterString

Changes the content alculates the integer value out of an array of bytes.

*/
string Event::FilterString(string textEvents)
{
  stringstream in(textEvents);
  stringstream filtered;
  char ch;

  while (in)
  {
    in.get(ch);

    switch((unsigned int)ch)
    {
      case 0x0d:              // rollback
        filtered.put(0x0a);
        break;
     case 0x2f:               // "/"
        filtered.put(0x20);
        break;
     case 0x5c:               // "\"
        filtered.put(0x0a);
        break;
     default:
        filtered.put(ch);
    }
  }
  return filtered.str();
}

/*
3 Utility functions

3.1 ~FindMetaEvent~

Examines the selected track object until it finds the specified MetaEvent or
the track ends.

*/
CcString* FindMetaEvent(Midi* inMidi, unsigned char searchedValue, int number)
{
  const Event* currentEvent;
  string temp;

  temp.clear();
  CcString* result    = new CcString(false, (STRING*)temp.c_str());
  Track* currentTrack = inMidi->GetTrack(number);
  int numberEvents    = currentTrack->GetNumberOfEvents();
  bool found          = false;

  for ( int eventIndex = 0; eventIndex < numberEvents && !found; eventIndex++)
  {
    currentEvent = currentTrack->GetEvent(eventIndex);

    if ( currentEvent->GetEventType()       == metamessage &&
         currentEvent->GetMetaMessageType() == searchedValue )
    {
      currentEvent->GetTextFromMetaEvent(temp);
      result->Set(true, (STRING*)temp.c_str());
      found  = true;
    }
  }
  return result;
}

/*
3.2 ~IntToString~, ~StringToInt~

Converting functions for int to string and string to int.

*/
string IntToString (int i)
{
  ostringstream temp;
  temp << i;
  string result = temp.str();
  return result;
}

bool StringToInt(string s, int& result)
/*
Returns false if it's not possible to convert the given string in an integer
value.

*/
{
  stringstream sstr;
  sstr << s;
  sstr >> result;
  return sstr.eof();
}

/*
3.3 ~BytesToIntNoClearMSB~

Converts an binary value into an integer without clearing the msb

Higher and lower byte are calculated with identical exponent, because
conversion into integer value ''low'' and ''high'' includes already
multiplication with 16. Therefor it is not allowed to increase the exponent for
the higher byte.

*/
unsigned long int BytesToIntNoClearMSB(vector<unsigned char>* arg)
{
  int exp = 0;
  unsigned long result = 0;

  for (int i = arg->size() - 1; i >= 0; i--)
  {
    unsigned char value = (*arg)[i];
    unsigned int low    = value & 0x0F;
    unsigned int high   = value & 0xF0;
    unsigned long basis = (unsigned long) pow ((double) 16, exp);
    result += low * basis;
    basis   = (unsigned long) pow ((double) 16, exp);
    result += high * basis;
    exp += 2;
  }
  return result;
}

/*
3.4 Implementation of helper class ~NoteStringToListParser~

3.4.1 The constructor.

*/
NoteStringToListParser::NoteStringToListParser()
{ }

/*
3.4.1 ParseNoteString

*/
bool NoteStringToListParser::ParseNoteString(char* inputStr,
                                             vector<int>* resultList)
{
  resultList->clear();

  if ( !(*inputStr) )
  {
    return false;
  }

  char currentChar = GetNextChar(&inputStr);

  do
  {
    int baseNoteNo   = ComputeBaseNoteNo(currentChar);
    int octaveFactor = 0;

    if (baseNoteNo == -1)
    {
      return false;
    }

    currentChar = GetNextChar(&inputStr);
    if (currentChar == '#')
    {
      baseNoteNo++;
      currentChar = GetNextChar(&inputStr);
    }
    else if (currentChar == 'b') {
        baseNoteNo--;
        currentChar = GetNextChar(&inputStr);
      }
      else if (!currentChar) {
          return false;
      }

    stringstream sstr;
    while (currentChar && currentChar != ',')
    {
      sstr << currentChar;
      currentChar = GetNextChar(&inputStr);
    }

    string intString = sstr.str();
    bool conversionResult = StringToInt(intString, octaveFactor);

    if (!conversionResult || octaveFactor < -1 || octaveFactor > 9)
    {
      return false;
    }

    int noteNo = (octaveFactor + 1) * 12 + baseNoteNo;
    if (noteNo < 0 || noteNo > 127)
    {
      return false;
    }
    resultList->push_back(noteNo);

    if (!currentChar)
    {
      return true;
    }

    if (currentChar != ',')
    {
      return false;
    }
    currentChar = GetNextChar(&inputStr);
  } while (true);
}

/*
3.4.1 GetNextChar

*/
char NoteStringToListParser::GetNextChar(char** inputStrPtr)
{
  char result = *(*inputStrPtr);
  (*inputStrPtr)++;

  while (result == ' ')         // skip spaces
  {
    result = *(*inputStrPtr);
    (*inputStrPtr)++;
  }
  return result;
}

/*
3.4.1 ComputeBaseNoteNo

*/
int NoteStringToListParser::ComputeBaseNoteNo(char currentChar)
{
  switch (currentChar)
  {
    case 'C':
      return 0;
    case 'D':
      return 2;
    case 'E':
      return 4;
    case 'F':
      return 5;
    case 'G':
      return 7;
    case 'A':
      return 9;
    case 'B':
      return 11;
  }
  return ERROR_INT;
}

/*
4 Operators

4.1 Operator ~extract\_track~

Constructs a new Midi object which includes only the selected track.

4.1.1 Type mapping function of operator ~extract\_track~

Operator ~extract\_track~ accepts a Midi object and a integer representingthe
track number and returns a Midi object.

----    (midi int)               -> midi
----

*/
ListExpr genericTrackTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) && nl->IsEqual(arg2, "int") )
    {
      return arg1;
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
4.1.2 Value mapping functions of operator ~extract\_track~

*/
int extractTrackFun(Word* args, Word& result, int message, Word& local,
                    Supplier s)
{
  Midi *currentMidi  = (Midi*)args[0].addr;
  int extractedTrack = ((CcInt*)args[1].addr)->GetIntval();
  Midi *newMidi      = currentMidi->Clone(false);
/*
Gets the Midi object and the to be extracted track number out of the argument
list, then a new Midi instance is created with same content as currentMidi but
without tracks

*/
  if ( currentMidi->GetNumberOfTracks() < extractedTrack ||
       extractedTrack < MIN_TRACKS_MIDI )
  {
    cout << "Error in extract_track: Parameter is out of range, ";
    cout << "the operator's input is passed without changes." << endl;
    newMidi = currentMidi;
  }
  else
  {
    newMidi->Append(currentMidi->GetTrack(extractedTrack - 1));
  }
/*
If the user specifies a track number outside the valid range we give back the
original input Midi. So always we have a valid result.

*/
  result = qp->ResultStorage(s);
  *((Midi *)result.addr) = *newMidi;
/*
Deletion was always succesfull [->] return new Midi instance by storing their
reference

*/
  return 0;
}

/*
4.1.3 Specification of operator ~extract\_track~

*/
const string extractTrackSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int) -> midi"
    "</text--->"
    "<text>_ extract_track [_]</text--->"
    "<text>Constructs a new Midi object including "
    "only the selected track.</text--->"
    "<text>query midifile extract_track [2]</text--->"
    ") )";

/*
4.1.4 Definition of operator ~extract\_track~

*/
Operator extract_track (
        "extract_track",        //name
        extractTrackSpec,       //specification
        extractTrackFun,        //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericTrackTypeMap     //type mapping
);

/*
4.2 Operator ~delete\_track~

Deletes the selected track out of the specified Midi object.

4.2.1 Type mapping function of operator ~delete\_track~

Operator ~delete\_track~ accepts a Midi object and an integer value
representingthe number of the to be deleted track and returns a new Midi
object.

----    (midi int)               -> midi
----

( please refer to function genericTrackTypeMap )

*/

/*
4.2.2 Value mapping functions of operator ~delete\_track~

*/
int deleteTrackFun(Word* args, Word& result, int message, Word& local,
                   Supplier s)
{
  Midi *currentMidi = (Midi*)args[0].addr;
  int deletedTrack  = ((CcInt*)args[1].addr)->GetIntval() - 1;
  int currentTracks = currentMidi->GetNumberOfTracks();
  Midi *newMidi     = currentMidi->Clone(false);
/*
Gets the Midi object and the to be deleted track number out of the argument
list, then a new Midi instance is created with same content as currentMidi but
without tracks

*/
  result = qp->ResultStorage(s);

  if ( currentTracks >= MIN_TRACKS_MIDI && currentTracks >= deletedTrack + 1 )
  {
    for ( int index = 0; index < currentTracks; index++ )
    {
      if ( index != deletedTrack )
      {
        newMidi->Append(currentMidi->GetTrack(index));
      }
    }
/*
Deletes the specified track by taking over the other ones from the instance
currentMidi, only allowed with more than MIN\_TRACKS\_MIDI tracks

*/
    *((Midi *)result.addr) = *newMidi;
/*
Reduction was succesfull [->] return new Midi instance by storing their
reference

*/
  }
  else
  {
    cout << "Error in delete_track: Parameter is out of range, ";
    cout << "the operator's input is passed without changes." << endl;
    *((Midi *)result.addr) = *currentMidi;
/*
Error before track deletion ( too few remaining tracks inside the Midi Object )
[->] return original Midi object

*/
  }
  return 0;
}

/*
4.2.3 Specification of operator ~delete\_track~

*/
const string deleteTrackSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int) -> midi"
    "</text--->"
    "<text>_ delete_track [_]</text--->"
    "<text>Deletes the selected track out of the "
    "specified Midi object. If the selected track "
    "is lower or higher than the track`s valid "
    "range, the original Midi object will be "
    "returned</text--->"
    "<text>query midifile delete_track [3]</text--->"
    ") )";

/*
4.2.4 Definition of operator ~delete\_track~

*/
Operator delete_track (
        "delete_track",         //name
        deleteTrackSpec,        //specification
        deleteTrackFun,         //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericTrackTypeMap     //type mapping
);

/*
4.3 Operator ~expand\_track~

A track could consists of up to 16 channels. This operator creates one new
track per each channel inside the selected track number.

Example: a Midi object consists of 3 tracks. Track 1 and 2 consists of 4
channels each, track 3 consists of 5 channels. Now this Midi object shall be
expanded with track 3. After this operation the Midi object consists of 7
tracks ( old 1 and 2 and the new 5 ones ( because of 5 channels in track 3 )
makes 7 ).

4.3.1 Type mapping function of operator ~expand\_track~

Operator ~expand\_track~ accepts a Midi object and an integer value
representingthe number of the to be expanded track and returns the changed Midi
object.

----    (midi int)               ->  midi
----

( please refer to function genericTrackTypeMap )

*/

/*
4.3.2 Value mapping functions of operator ~expand\_track~

*/
int expandTrackFun(Word* args, Word& result, int message, Word& local,
                   Supplier s)
{
  Event *currentEvent;
  const Event* auxCurrentEvent;
  int currentChannel;
// number of current channel

  bool foundChannels[MAX_CHANNELS_MIDI];
// stores found channels of to be expanded track

  Track* newTracks[MAX_CHANNELS_MIDI];
  bool expand = false;
// controls the expansition of metaEvents and sysEx

  int runningChannel = 0;
// if expand == true, then it stores the MetaEvent`s channel

  Midi *currentMidi = (Midi*)args[0].addr;
  int expandedTrack = ((CcInt*)args[1].addr)->GetIntval() - 1;
  int currentTracks = currentMidi->GetNumberOfTracks();
  Midi *newMidi     = currentMidi->Clone(false);
/*
Gets the Midi object and the to be extracted track number out of the argument
list, then a new Midi instance is created with same content as currentMidi but
without tracks

*/
  for ( int index = 0; index < MAX_CHANNELS_MIDI; index++ )
  {
    foundChannels[index] = false;
    newTracks[index]     = NULL;
  }
/*
Initialises the array for found channels and new tracks

*/
  for ( int trackIndex = 0; trackIndex < currentTracks; trackIndex++ )
  {
    if ( trackIndex != expandedTrack )
    {
      newMidi->Append(currentMidi->GetTrack(trackIndex));
    }
/*
All tracks except the to be expanded one are copied into the new Midi instance

*/
    else
    {
      Track* oldTrack  = currentMidi->GetTrack(expandedTrack);
      int numberEvents = oldTrack->GetNumberOfEvents();
/*
First of all the number of different channels is counted, because we need to
know how many tracks needs to be created at first.

*/
      for ( int eventIndex = 0; eventIndex < numberEvents; eventIndex++ )
      {
        auxCurrentEvent = oldTrack->GetEvent(eventIndex);
        currentEvent = const_cast<Event*>( auxCurrentEvent );

        if ( currentEvent->GetEventType() == metamessage )
        {
          if (currentEvent->GetMetaMessageType() == Event::CHANNEL_PREFIX)
          {
            expand = true;
            string temp;
            currentEvent->GetTextFromMetaEvent(temp);
            runningChannel = temp[0];
/*
Normally SysEx and MetaEvents are not expanded, except MetaEvent
''CHANNEL\_PREFIX'' occured or is still valid inside the track. Then *all*
events are copied to the new track created by inside the MetaEvent mentioned
channel number

*/
          }
          else
          {
            expand = false;
            runningChannel = MAX_CHANNELS_MIDI + 1;
/*
A different meta message to CHANNEL\_PREFIX inside the to be expanded track
clears it

*/
          }
        }

        if ( currentEvent->GetEventType() == sysexmessage )
        {
          expand = false;
          runningChannel = MAX_CHANNELS_MIDI + 1;
/*
A sysex message inside the to be expanded track clears the MetaEvent
''CHANNEL\_PREFIX''

*/
        }

        if ( currentEvent->GetEventType() == shortmessage && expand &&
            !currentEvent->GetShortMessageRunningStatus() &&
             currentEvent->GetChannel() != runningChannel )
        {
          expand = false;
          runningChannel = MAX_CHANNELS_MIDI + 1;
/*
A new channel inside the to be expanded track clears the MetaEvent
''CHANNEL\_PREFIX''

*/
        }

        if (expand)
        {
          if ( !foundChannels[runningChannel] )
          {
            foundChannels[runningChannel] = true;
            Track* newTrack = new Track();
            newTrack->Append(currentEvent);
            newTracks[runningChannel] = newTrack;
/*
New channel found [->] new track created and current event appended

*/
          }
          else
          {
            Track* existingTrack = newTracks[runningChannel];
            existingTrack->Append(currentEvent);
            newTracks[runningChannel] = existingTrack;
/*
Add Event to newly created track for this specific channel

*/
          }
        }
        else  // expand modus is invalid
        {
          if ( currentEvent->GetEventType() == shortmessage )
          {
            if ( !currentEvent->GetShortMessageRunningStatus() )
            {
              currentChannel = currentEvent->GetChannel();

              if ( !foundChannels[currentChannel] )
              {
                foundChannels[currentChannel] = true;
                Track* newTrack = new Track();
                newTrack->Append(currentEvent);
                newTracks[currentChannel] = newTrack;
                runningChannel = currentChannel;
              }
              else
              {
                Track* existingTrack = newTracks[currentChannel];
                existingTrack->Append(currentEvent);
                newTracks[currentChannel] = existingTrack;
                runningChannel = currentChannel;
              }
            }
            else  // uupps running status
            {
              Track* existingTrack = newTracks[runningChannel];
              existingTrack->Append(currentEvent);
              newTracks[runningChannel] = existingTrack;
/*
If a MidiEvent occured and ~runningStatus~ is valid, then ~runningChannel~
should have a valid eventIndex for copying into a new track.

*/
            }
          }
        }
      }
    }
  }
/*
Whilst walking through the entire to be expanded track, for each channel a new
track will be created.

*/
  for ( int k = 0; k < MAX_CHANNELS_MIDI; k++ )
  {
    if ( foundChannels[k] )
    {
      Track* track     = newTracks[k];
      Event* lastEvent = new Event(metamessage, 0);
      vector<unsigned char> v(3);
      v[0] = 0xFF;
      v[1] = Event::END_OF_TRACK;
      v[2] = 0;
      lastEvent->SetMetaData(&v);
      track->Append(lastEvent);
      newMidi->Append(newTracks[k]);
    }
  }
/*
Before appending all new tracks to the new Midi instance, they needs to be
finished with an ''end of track'' event.

*/
  result = qp->ResultStorage(s);

  if ( newMidi->GetNumberOfTracks() >= MIN_TRACKS_MIDI )
  {
    *((Midi *)result.addr) = *newMidi;
/*
Expand was succesfull [->] return new Midi instance by storing their reference

*/
  }
  else
  {
    cout << "Error in expand_track: too few remaining tracks" << endl;
    *((Midi *)result.addr) = *currentMidi;
  }
/*
error during track expand ( too few remaining tracks inside the Midi Object )
[->] return original Midi object

*/
  return 0;
}

/*
4.3.3 Specification of operator ~expand\_track~

*/
const string expandTrackSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int) -> midi"
    "</text--->"
    "<text>_ expand_track [_]</text--->"
    "<text>Expands the selected track inside the "
    "specified Midi object. Each channel inside "
    "this track is copied into a new track "
    "of the output Midi. If MetaEvent "
    "CHANNEL_PREFIX is valid, then all "
    "MetaEvents and SystemEvents are copied."
    "Otherwise only MidiEvents are copied.</text--->"
    "<text>query midifile expand_track [4]</text--->"
    ") )";

/*

4.3.4 Definition of operator ~expand\_track~

*/
Operator expand_track (
        "expand_track",         //name
        expandTrackSpec,        //specification
        expandTrackFun,         //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericTrackTypeMap     //type mapping
);

/*
4.3 Operator ~merge\_tracks~

This operator creates a new Midi object which includes one track merged
together out of two tracks.

4.3.1 Type mapping function of operator ~merge\_tracks~

Operator ~merge\_tracks~ accepts a Midi object and two integer values
representingthe number of the to be merged tracks and returns a new Midi
object.

----    (midi int int bool)               -> midi
----

*/
ListExpr mergeTracksTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 4 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) &&
         nl->IsEqual(arg2, "int")       &&
         nl->IsEqual(arg3, "int")       &&
         nl->IsEqual(arg4, "bool"))
    {
      return arg1;
    }
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.3.2 Value mapping functions of operator ~merge\_tracks~

Utilities of the value mapping function.

*/

struct EventWithAbsTime
/*
Stores an event with the absolute time position.

*/
{
  unsigned int absTime;
  Event* event;
};

void computeEventsOfTrackWithAbsTime(Track* track,
                                          vector<EventWithAbsTime>& result)
/*
Computes the absolute time position of each event in a track.

*/
{
  unsigned int absPos = 0;
  unsigned char shortMessageType;

  for (int i = 0; i < track->GetNumberOfEvents(); i++)
  {
    EventWithAbsTime* eventStruct = new EventWithAbsTime;
    Event *event;
    const Event* auxEvent = track->GetEvent(i);
    event = const_cast<Event*>( auxEvent );

    if (event->GetEventType() == shortmessage &&
       !event->GetShortMessageRunningStatus())
    {
      shortMessageType = event->GetShortMessageType();
    }

    if (event->GetEventType() == shortmessage &&
        event->GetShortMessageRunningStatus())
    {
      event->SetShortMessageRunningStatus(false);
      event->SetShortMessageType(shortMessageType);
    }

    absPos += event->GetDeltaTime();
    eventStruct->absTime = absPos;
    eventStruct->event = event;
    result.push_back(*eventStruct);
  }
}

void appendEventMergedTrack(
            vector<EventWithAbsTime>::iterator eventStruct,
            vector<EventWithAbsTime>& mergedTrackList,
            bool destTrack)
/*
Append an event at the list of merged events and do some filtering.

*/
{
  if (eventStruct->event->GetEventType() == shortmessage)
  {
    unsigned char shortMessageType = eventStruct->event->GetShortMessageType();
    shortMessageType = shortMessageType & 0xF0;

    if (destTrack)
    {
// passes all events of the destination track without any filtering
      mergedTrackList.push_back(*eventStruct);
      return;
    }
// at the moment no filtering will be done
    mergedTrackList.push_back(*eventStruct);
    return;
  }

  if (eventStruct->event->GetEventType() == metamessage)
  {
    unsigned char metaMessageType =
      eventStruct->event->GetMetaMessageType();

    if (metaMessageType == Event::END_OF_TRACK ||
        metaMessageType == Event::TRACK_NAME)
    {
      return;
    }

    if (destTrack)
    {
// passes all events of the destination track without any filtering
      mergedTrackList.push_back(*eventStruct);
      return;
    }
// filter some meta messages of the source track
    if (metaMessageType == Event::INSTRUMENT_NAME)
    {
      return;
    }

    mergedTrackList.push_back(*eventStruct);
    return;
  }
  else
  {
// it is a sysex message
    mergedTrackList.push_back(*eventStruct);
    return;
  }
}

void getTrackFromEventStructList(string& mergedTrackName,
                                     vector<EventWithAbsTime>& mergedTrackList,
                                        Track* result)
/*
Create a track from a merged list with entries of type EventWithAbsTime.

*/
{
  unsigned int prevAbsTime = 0;
  Event* trackNameMetaMessage = new Event(metamessage, 0);
  vector<unsigned char> metaData(mergedTrackName.size() + 3);
  metaData[0] = 0xFF;
  metaData[1] = Event::TRACK_NAME;
  metaData[2] = mergedTrackName.size();
  const char* mergedTrackNamecstr = mergedTrackName.c_str();

  for (unsigned int i = 0; i < mergedTrackName.size(); i++)
  {
    metaData[3 + i] = mergedTrackNamecstr[i];
  }

  trackNameMetaMessage->SetMetaData(&metaData);
  result->Append(trackNameMetaMessage);

  for (unsigned int i = 0; i < mergedTrackList.size(); i++)
  {
    Event* event = new Event(*(mergedTrackList[i].event));
// Set the new delta time
    event->SetDeltaTime(mergedTrackList[i].absTime - prevAbsTime);
    result->Append(event);
    prevAbsTime = mergedTrackList[i].absTime;
  }

  Event* lastEvent = new Event(metamessage, 0);
  vector<unsigned char> v(3);
  v[0] = 0xFF;
  v[1] = Event::END_OF_TRACK;
  v[2] = 0;
  lastEvent->SetMetaData(&v);
  result->Append(lastEvent);
}

void constructMergedTrackName(int track1No, int track2No,
                                     string& result)
/*
Construct a string ''Track $<srcTrackNo>, <destTrackNo>$ merged''

*/
{
  result.clear();
  string track1NoStr = IntToString(track1No + 1);
  string track2NoStr = IntToString(track2No + 1);
  result.append("*Tracks ");
  result.append(track1NoStr);
  result.append(", ");
  result.append(track2NoStr);
  result.append(" merged*");
}

void mergeTracks(Track* track1, int track1No, Track* track2,
                        int track2No, Track* result)
/*
This function merges two tracks into one track.

*/
{
  vector<EventWithAbsTime> track1List;
  vector<EventWithAbsTime> track2List;

// Create lists of event objects with absolute time
  computeEventsOfTrackWithAbsTime(track1, track1List);
  computeEventsOfTrackWithAbsTime(track2, track2List);

  vector<EventWithAbsTime>::iterator it1 = track1List.begin();
  vector<EventWithAbsTime>::iterator it2 = track2List.begin();
  vector<EventWithAbsTime> mergedTrackList;

// A small mergesort implementation...
  while (it1 != track1List.end() && it2 != track2List.end())
  {
    if (it1->absTime <= it2->absTime)
    {
      appendEventMergedTrack(it1, mergedTrackList, false);
      it1++;
    }
    else
    {
      appendEventMergedTrack(it2, mergedTrackList, true);
      it2++;
    }
  }

  while (it1 != track1List.end())
  {
    appendEventMergedTrack(it1, mergedTrackList, false);
    it1++;
  }

  while (it2 != track2List.end())
  {
    appendEventMergedTrack(it2, mergedTrackList, true);
    it2++;
  }

  string mergedTrackName;
  constructMergedTrackName(track1No, track2No, mergedTrackName);
  getTrackFromEventStructList(mergedTrackName, mergedTrackList, result);
}

void replaceProgramChangeValuesForTrackMerging(Track* srcTrack,
                                               Track* destTrack)
/*
Replaces all ~program changes~ values in the source track with the value of the
first ~program change~ in the destination track.

*/
{
  const Event* firstPCOfDestTrack = 0;

  for (int i = 0; i < destTrack->GetNumberOfEvents(); i++)
  {
    const Event* currentEvent = destTrack->GetEvent(i);

    if (currentEvent->GetEventType() == shortmessage &&
       !currentEvent->GetShortMessageRunningStatus() &&
       (currentEvent->GetShortMessageType()&0xF0) == Event::PROGRAM_CHANGE)
    {
      firstPCOfDestTrack = currentEvent;
    }
  }

  if (!firstPCOfDestTrack)
  {
    return;
  }
/*
No program change in destination track! We hope the best for the result of
merging...

*/

  unsigned char programChangeValue =
    firstPCOfDestTrack->GetShortMessageData(0);
/*
Replace all program changes in the source track with the value of the found
program change's value

*/
  for (int i = 0; i < srcTrack->GetNumberOfEvents(); i++)
  {
    const Event* auxCurrentEvent = srcTrack->GetEvent(i);
    Event *currentEvent = const_cast<Event*>( auxCurrentEvent );

    if (currentEvent->GetEventType() == shortmessage &&
       !currentEvent->GetShortMessageRunningStatus() &&
       (currentEvent->GetShortMessageType()&0xF0) == Event::PROGRAM_CHANGE)
    {
      currentEvent->SetShortMessageData(0, programChangeValue);
    }
  }
}

int mergeTracksFun(Word* args, Word& result, int message,
                          Word& local, Supplier s)
/*
The value mapping function.

*/
{
  Midi* midiIn = (Midi*) args[0].addr;
  Midi* midiOut = midiIn->Clone(false);
  midiOut->Destroy();

  int track1No = ((CcInt*) args[1].addr)->GetIntval() - 1;
  int track2No = ((CcInt*) args[2].addr)->GetIntval() - 1;
  bool filterProgramChanges = ((CcBool*) args[3].addr)->GetBoolval();

  if (track1No != track2No &&
      track1No >= 0 && track1No < midiIn->GetNumberOfTracks() &&
      track2No >= 0 && track2No < midiIn->GetNumberOfTracks())
  {
    Track* track1 = midiIn->GetTrack(track1No);
    Track* track2 = midiIn->GetTrack(track2No);

    if (filterProgramChanges)
    {
      replaceProgramChangeValuesForTrackMerging(track1, track2);
    }

    Track* mergedTrack = new Track();
    mergeTracks(track1, track1No, track2, track2No, mergedTrack);

    // filterProgramChanges
    for (int i = 0; i < midiIn->GetNumberOfTracks(); i++)
    {
      if (i != track1No)
      {
        if (i == track2No)
        {
          midiOut->Append(mergedTrack);
        }
        else
        {
          midiOut->Append(midiIn->GetTrack(i));
        }
      }
    }
    delete track1;
    delete track2;
    delete mergedTrack;
  }
  else
  {
/*
''Official'' output of operator merge\_tracks. Please do not comment the
following two lines.

*/
    cout << "Operator merge_tracks: Parameters are out of range, ";
    cout << "the operator's input is passed without changes.\n";
    delete midiOut;
    midiOut = new Midi(*midiIn);
    midiOut->Destroy();
  }

  result = SetWord(midiOut);
  return 0;
}

/*
4.3.3 Specification of operator ~merge\_tracks~

*/
const string mergeTracksSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int int bool) -> midi"
    "</text--->"
    "<text>_ merge_tracks [_,_,_]</text--->"
    "<text>Merges the first specified track into the "
    "second track. If the boolean flag is set true, the notes of the "
    "first track will be played with the instrument's voice of the second"
    "track.</text--->"
    "<text>query mymidi merge_tracks [2,4,FALSE]</text--->"
    ") )";

/*
4.3.4 Definition of operator ~merge\_tracks~

*/
Operator merge_tracks (
        "merge_tracks",         //name
        mergeTracksSpec,        //specification
        mergeTracksFun,         //value mapping
        Operator::SimpleSelect, //trivial selection function
        mergeTracksTypeMap      //type mapping
);

/*
4.3 Operator ~transpose\_track~

This operator transposes the selected track inside the specified Midi object by
the number of specified halftones.

4.3.1 Type mapping function of operator ~transpose\_track~

Operator ~transpose\_track~ accepts a Midi object, an 2 integer values. It
returns a Midi object.

----    (midi int int               -> midi
----

*/
ListExpr transposeTrackTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 3 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) &&
         nl->IsEqual(arg2, "int")       &&
         nl->IsEqual(arg3, "int"))
    {
      return arg1;
    }
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.3.2 Value mapping functions of operator ~transpose\_track~

*/

int transposeTrackFun(Word* args, Word& result, int message, Word& local,
                      Supplier s)
{
  int errorinfo     = 0;
  Midi* currentMidi = (Midi*)args[0].addr;
  CcInt* tracknr    = (CcInt*)args[1].addr;
  CcInt* htsteps    = (CcInt*)args[2].addr;
  int stepval       = htsteps->GetIntval();

  Track* currentTrack;
  Midi* transposedMidi =  new Midi (true, currentMidi->GetDivisionMSB(),
    currentMidi->GetDivisionLSB());

  result = qp->ResultStorage(s);

  if(stepval < 128 && stepval > -128)
  {
    transposedMidi->SetFormat(currentMidi->GetFormat());

    if((currentMidi->GetNumberOfTracks() + 1) > tracknr->GetIntval())
    {
      int k = 1;
      do
      {
        currentTrack = currentMidi->GetTrack(k-1);

        if(k == (tracknr->GetIntval()))
        {
          currentTrack->Transpose(true,stepval,errorinfo);
        }

        transposedMidi->Append(currentTrack);
        k++;
      } while(!errorinfo && k < (currentMidi->GetNumberOfTracks()));

      if(errorinfo == 1 )
      {
        *(Midi *)result.addr = *currentMidi;
        cout << endl << "crossing range of playable values";
        cout << endl << "in Track " << k-1 << " !!";
        cout << endl<< "  nothing done ";
        return 0;
      }
    }
    else
    {
      cout << "tracknr too big or small  - nothing done";
     *(Midi *)result.addr = *currentMidi;
     return 0;
    }
  }
  else
  {
    cout << endl << "too many steps- nothing done";
    *(Midi *)result.addr = *currentMidi;
    return 0;
  }

  *(Midi *)result.addr = *transposedMidi;
  delete currentTrack;
  return 0;
}

/*
4.3.3 Specification of operator ~transpose\_track~

*/
const string transposeTrackSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int real) -> midi"
    "</text--->"
    "<text>_ transpose_track [_,_]</text--->"
    "<text>Transposes the selected track inside the "
    "specified Midi object by -second parameter-"
    "number of halftones.including perussion channel</text--->"
    "<text>query yoursong.mid transpose_track [2,1]</text--->"
    ") )";

/*
4.3.4 Definition of operator ~transpose\_track~

*/
Operator transpose_track (
        "transpose_track",      //name
        transposeTrackSpec,     //specification
        transposeTrackFun,      //value mapping
        Operator::SimpleSelect, //trivial selection function
        transposeTrackTypeMap   //type mapping
);

/*
4.4 Operator ~transpose\_midi~

This operator transposes the specified Midi object by the number of specified
halftones.(exluding the precussion/drumchannel !!)

4.3.1 Type mapping function of operator ~transpose\_midi~

Operator ~transpose\_midi~ accepts a Midi object and an integer value. It
returns a Midi object.

----    (midi int )               -> midi
----

( please refer to function genericTrackTypeMap )

*/

/*
4.3.2 Value mapping functions of operator ~transpose\_midi~

*/

int transposeMidiFun(Word* args, Word& result, int message, Word& local,
                     Supplier s)
{
  int errorinfo     = 0;
  Midi* currentMidi = (Midi*)args[0].addr;
  CcInt* htsteps    = (CcInt*)args[1].addr;
  int stepval       = htsteps->GetIntval();

  Track* currentTrack;
  Midi* transposedMidi =  new Midi (true, currentMidi->GetDivisionMSB(),
    currentMidi->GetDivisionLSB());
  result = qp->ResultStorage(s);

  if(stepval < 128 && stepval > -128)
  {
    transposedMidi->SetFormat(currentMidi->GetFormat());
    int k = 1;
    do
    {
      currentTrack = currentMidi->GetTrack(k-1);
      currentTrack->Transpose(false,stepval,errorinfo);
      transposedMidi->Append(currentTrack);
      k++;
    } while(!errorinfo && k< currentMidi->GetNumberOfTracks());

    if(errorinfo)
    {
      cout << "crossing limits of playable values";
      cout << endl << "in track " << k-1 << " !! ";
      *(Midi *)result.addr = *currentMidi;
      return 0;
    }
  }
  else
  {
    cout << endl << "too many steps";
    cout << endl << "no changes  done";
    *(Midi *)result.addr = *currentMidi;
    return 0;
  }
  *(Midi *)result.addr = *transposedMidi;
  delete currentTrack;
  return 0;
}

/*
4.4.3 Specification of operator ~transpose\_midi~

*/
const string transposeMidiSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int) -> midi"
    "</text--->"
    "<text>_ transpose_midi _ </text--->"
    "<text>transposes the specified Midi object the given "
    " parameter number of halftones, excluding percussion</text--->"
    "<text>query mysong.mid transpose_midi [2] </text--->"
    ") )";

/*
4.3.4 Definition of operator ~transpose\_track~

*/
Operator transpose_midi (
        "transpose_midi",       //name
        transposeMidiSpec,      //specification
        transposeMidiFun,       //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericTrackTypeMap     //type mapping
);


/*
4.3 Operator ~extract\_lyrics~

This operator filters out the lyrics/text hidden in MetaEvents and puts out
an quite pretty formated text object. Using bool flags you can decide which
kind of meta(text) events are included in search.

Flags have following meaning:

~all~

all text events will be included - independend from other flags
(that means e.g. copyright information, cuepointer,
instrument information, track and file names,
stupid advertisement and other surprices :))

~lyr~

lyric events will be included  (FF05)

~any~

anyText events will be included (FF01) (in karaoke files lyrics are mostly hidden here)

4.3.1 Type mapping function of operator ~extract\_lyrics~

Operator ~extract\_lyrics~ accepts a Midi object and returns a string object.

----    (midi, bool, bool, bool)               -> text
----

*/

ListExpr extractLyricsTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 4 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    ListExpr arg3 = nl->Third( args );
    ListExpr arg4 = nl->Fourth( args );

    if ( nl->IsEqual(arg1, MIDI_STRING) &&
         nl->IsEqual(arg2, "bool"   )   &&
         nl->IsEqual(arg3, "bool"   )   &&
         nl->IsEqual(arg4, "bool"   ))
    {
      return nl->SymbolAtom("text");
    }
  }

  return nl->SymbolAtom("typeerror");
}


/*
4.3.2 Value mapping functions of operator ~extract\_lyrics~

*/
int extractLyricsFun(Word* args, Word& result, int message, Word& local,
Supplier s)
{

  Midi* currentMidi = (Midi*)args[0].addr;
  bool  all         = ((CcBool*)args[1].addr)->GetBoolval();
  bool  lyr         = ((CcBool*)args[2].addr)->GetBoolval();
  bool  any         = ((CcBool*)args[3].addr)->GetBoolval();
  string lyS;
  currentMidi->GetLyrics( lyS, all, lyr, any );

  FText :: FText* outText = new FText::FText(false, NULL);
  outText->Set(true,lyS.c_str());

  result = qp->ResultStorage(s);
  *((FText*)result.addr) = * outText;

  return 0;
}

/*
4.3.3 Specification of operator ~extract\_lyrics~

*/
const string extractLyricsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi text) -> string"
    "</text--->"
    "<text>_ extract_lyrics (_, _, _)</text--->"
    "<text>Extracts the included text of the "
    "midi object into a text object. "
    "With 3 flags you "
    "can decide which kind of text events are "
    "included in extracting: "
    "1.Flag TRUE: all kind of Text(FF01 ..FF0F) is incl, "
    "2.Flag TRUE: all Lyric Events(FF05), "
    "3.Flag TRUE: Any_Text_ Events (FF01). "
    "If first flag is set the others do not matters "
    "</text--->"
    "<text>query hersong.mid extract_lyrics [FALSE,TRUE,FALSE)</text--->"
    ") )";

/*
4.3.4 Definition of operator ~extract\_lyrics~

*/
Operator extract_lyrics (
        "extract_lyrics",       //name
        extractLyricsSpec,      //specification
        extractLyricsFun,       //value mapping
        Operator::SimpleSelect, //trivial selection function
        extractLyricsTypeMap    //type mapping
);

/*
4.3 Operator ~contains\_words~

This operator examines an existing Midi object and looks for a specified text
if it is included in lyricsevents.

4.3.1 Type mapping function of operator ~contains\_words~

Operator ~contains\_words~ accepts a Midi object and the text sequence to be
searched for. With the 3 flags you can specify the search for different kind
of Text - the same way like in extract\_lyrics operators. True will be returned
if the search was succesfull, false if not.

----    (midi, string, bool, bool, bool)               -> bool
----

*/

ListExpr containsWordsTypeMap( ListExpr args )
{
if ( nl->ListLength(args) == 5 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    ListExpr arg3 = nl->Third( args );
    ListExpr arg4 = nl->Fourth( args );
    ListExpr arg5 = nl->Fifth( args );

    if ( nl->IsEqual(arg1, MIDI_STRING)  &&
         nl->IsEqual( arg2, "string" )   &&
         nl->IsEqual( arg3, "bool"   )   &&
         nl->IsEqual( arg4, "bool"   )   &&
         nl->IsEqual( arg5, "bool"   ))

    {
      return nl->SymbolAtom("bool");
    }
  }
 return nl->SymbolAtom("typeerror");
}

/*
4.3.2 Value mapping functions of operator ~contains\_words~

*/
int containsWordsFun(Word* args, Word& result, int message, Word& local,
                     Supplier s)
{
  Midi* currentMidi = (Midi*)args[0].addr;
  CcString * tofind = (CcString*)args[1].addr;
  bool all = ((CcBool*)args[2].addr)->GetBoolval();
  bool lyr = ((CcBool*)args[3].addr)->GetBoolval();
  bool any = ((CcBool*)args[4].addr)->GetBoolval();
  string lyS;
  currentMidi->GetLyrics( lyS, all,lyr,any );
  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->
    Set(true,(lyS.find(*(tofind->GetStringval())) != string :: npos));

  return 0;
}

/*
4.3.3 Specification of operator ~contains\_words~

*/
const string containsWordsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi string) -> bool"
    "</text--->"
    "<text>_ contains_words [_,_,_,_,_]</text--->"
    "<text>Examines the selected Midi object if it "
    "includes the searched word(string)."
    "with 3 flags you can decide which kind"
    " of text events are included in search: "
    "1.Flag TRUE: all kind of text(FF01..FF0F) is incl, "
    "2.Flag TRUE: all Lyric events(FF05), "
    "3.Flag TRUE: any_Text_events (FF01) "
    "If first flag is set the others no matters"
    "</text--->"
    "<text>query oursong.mid contains_words "
     "[""jens"",TRUE,FALSE,FALSE]</text--->"
    ") )";

/*
4.3.4 Definition of operator ~contains\_words~

*/
Operator contains_words (
        "contains_words",       //name
        containsWordsSpec,      //specification
        containsWordsFun,       //value mapping
        Operator::SimpleSelect, //trivial selection function
        containsWordsTypeMap    //type mapping
);

/*
4.3 Operator ~contains\_sequence~

This operator examines a track of an existing Midi object and looks for
a specified sequence of notes if it is included. The searched notes are
passed by a string in the syntax below.

$<input\_list> := <note>,<note\_list>$

$<note\_list>  := <note> | <note>,<note\_list>$

$<note>        := <note\_char><note\_No> |
<note\_char><flat\_sharp\_ident><note\_No>$

$<note\_char>  := C | D | E | F | G | A | G | A | B$

$<note\_No>    := -1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9$

$<flat\_sharp\_ident> := b | \#$

The next argument allows transposing. If is set TRUE, the input list of
notes will be interpreted as a sequence of halftone steps.
The first occurrence of the sequence will be found. The last argument
specifies the to be examined track. A searching for a sequence with
this operator is only useful in tracks with a monophonic voice.


4.3.2 Type mapping function of operator ~contains\_sequence~

Operator ~contains\_sequence~ accepts a Midi object, the string with
the searched notes, a boolean value for allowed transposition and an
integer value representing the to be examined track.
If the search was succesfull, the position will be returned, -1 otherwise.

----    (midi string bool int)               -> int
----

*/
ListExpr containsSequenceTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 4 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) &&
         nl->IsEqual(arg2, "string")    &&
         nl->IsEqual(arg3, "bool")      &&
         nl->IsEqual(arg4, "int") )
    {
      return  nl->SymbolAtom("int");
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
4.3.3 Value mapping functions of operator ~contains\_sequence~

Utility functions of operator ~contains\_sequence~

*/

void computeNoteOnEventsWithAbsTime(Track* track,
                                    vector<EventWithAbsTime>& result)
/*
Computes the absolute time position of each note on event with a none zero
velocity and filters all other events of the given track.

*/
{
  unsigned int absPos = 0;
  unsigned char shortMessageType;

  for (int i = 0; i < track->GetNumberOfEvents(); i++)
  {
    const Event* auxEvent = track->GetEvent(i);
    Event *event = const_cast<Event*>( auxEvent );

    if (event->GetEventType() == shortmessage &&
       !event->GetShortMessageRunningStatus())
    {
      shortMessageType = event->GetShortMessageType();
    }

    if (event->GetEventType() == shortmessage &&
        event->GetShortMessageRunningStatus())
    {
      event->SetShortMessageRunningStatus(false);
      event->SetShortMessageType(shortMessageType);
    }

    absPos += event->GetDeltaTime();
    if (event->GetEventType() == shortmessage &&
       (event->GetShortMessageType() & 0xF0) == Event::NOTE_ON &&
        event->GetShortMessageData(1))
    {
      EventWithAbsTime* eventStruct = new EventWithAbsTime;
      eventStruct->absTime = absPos;
      eventStruct->event = event;
      result.push_back(*eventStruct);
    }
  }
}

int searchNoteSequenceAbsolute(vector<int>* searchedNotes,
                                      Track* track)
/*
Searches for the passed sequence of note numbers in the specified track.
Returns the position in ticks if the sequence is found, -1
otherwise.

*/
{
  vector<EventWithAbsTime> noteEvents;
  computeNoteOnEventsWithAbsTime(track, noteEvents);

  for (unsigned int i = 0; i < noteEvents.size() - searchedNotes->size(); i++)
  {
    bool found = true;
    for (unsigned int j = 0; j < searchedNotes->size(); j++)
    {
      if (noteEvents[i + j].event->GetShortMessageData(0) !=
          (*searchedNotes)[j])
      {
        found = false;
        break;
      }
    }
    if (found)
    {
      return noteEvents[i].absTime;
    }
  }
  return ERROR_INT;
}

int searchNoteSequenceRelative(vector<int>* searchedNotes, Track* track)
/*
Searches for the passed sequence of notes in the specified track. This sequence
is translated in a sequence of musical intervals
and the functions searches for this interval sequence. Returns the position in
ticks if the sequence is found, -1 otherwise.

*/
{
  unsigned int i, j, k;
  vector<int> intervals(searchedNotes->size() - 1);

  for ( i = 0; i < searchedNotes->size() - 1; i++)
  {
    intervals[i] = (*searchedNotes)[i + 1] - (*searchedNotes)[i];
  }

  vector<EventWithAbsTime> noteEvents;
  computeNoteOnEventsWithAbsTime(track, noteEvents);

  for ( j = 0; j < noteEvents.size() - searchedNotes->size(); j++)
  {
    bool found = true;
    for ( k = 0; k < intervals.size(); k++)
    {
      int currentInterval =
            noteEvents[j + k + 1].event->GetShortMessageData(0) -
            noteEvents[j + k].event->GetShortMessageData(0);
      if (currentInterval != intervals[k])
      {
        found = false;
        break;
      }
    }
    if (found)
    {
      return noteEvents[i].absTime;
    }
  }
  return ERROR_INT;
}

/*
The value mapping function

*/
int containsSequenceFun(Word* args, Word& result, int message, Word& local,
                        Supplier s)
/*
contains predicate for a sequence inside the Midi object on one track

*/
{
  Midi* midiIn          = (Midi*) args[0].addr;
  char* inputStr        = (char*) (((CcString*) args[1].addr)->GetStringval());
  bool transposeAllowed = ((CcBool*) args[2].addr)->GetBoolval();
  int trackNo           = ((CcInt*) args[3].addr)->GetIntval() - 1;

  result = qp->ResultStorage(s);

  if (trackNo < 1 || trackNo > midiIn->GetNumberOfTracks())
  {
/*
''Official'' output of operator contains\_sequence. Please do not comment the
following line.

*/
    cout << "Operator contains_sequence: Parameters are out of range.\n";
    ((CcInt*) result.addr)->Set(true, ERROR_INT);
    return 0;
  }

  vector<int> noteList;
  NoteStringToListParser noteParser;
  bool parseResult = noteParser.ParseNoteString(inputStr, &noteList);
  Track* track     = midiIn->GetTrack(trackNo);
  int resultValue  = ERROR_INT;

  if (parseResult && noteList.size() > 1)
  {
    if (transposeAllowed)
    {
      resultValue = searchNoteSequenceRelative(&noteList, track);
    }
    else
    {
      resultValue = searchNoteSequenceAbsolute(&noteList, track);
    }
  }
  else
  {
/*
''Official'' output of operator contains\_sequence. Please do not comment the
following line.

*/
    cout << "Operator contains_sequence: Syntax error in search string.\n";
  }

  delete track;
  ((CcInt*) result.addr)->Set(true, resultValue);
  return 0;
}

/*
4.3.4 Specification of operator ~contains\_sequence~

*/
const string containsSequenceSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi string bool int) -> int"
    "</text--->"
    "<text>_ contains_sequence [_,_,_]</text--->"
    "<text>Examines the selected Midi object if it "
    "contains the specified sequence of notes. The second argument "
    "allows transposing. If is set TRUE, the input list of notes will be "
    "interpreted as a sequence of halftone steps. The first occurrence of "
    "the sequence will be found. The third argument specifies the to be "
    "examined track. A searching for a sequence with this operator is "
    "only useful in tracks with a monophonic voice.</text--->"
    "<text>query mymidi contains_sequence [\"D4,F#4,A4\", TRUE, 2]</text--->"
    ") )";

/*
4.3.5 Definition of operator ~contains\_sequence~

*/
Operator contains_sequence (
        "contains_sequence",      //name
        containsSequenceSpec,     //specification
        containsSequenceFun,      //value mapping
        Operator::SimpleSelect,   //trivial selection function
        containsSequenceTypeMap   //type mapping
 );


/*
4.4 Operator ~saveto~

This operator saves a Midi object to a file.

4.4.1 Type mapping function of operator ~saveto~

The operator ~saveto~ accepts a Midi object and a string representation of the
file name. It will return a bool with the value true if the saving is
succesfull, false otherwise.

*/
ListExpr SavetoMidiTypeMap (ListExpr args)
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (nl->IsAtom(first)   && nl->IsEqual(first, MIDI_STRING) &&
        nl->IsAtom(second)  && nl->IsEqual(second, "string"))
    {
      return nl->SymbolAtom("bool");
    }
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.4.2 Value mapping function of operator ~saveto~

----    (midi string )               -> bool
----

*/
int SavetoMidiFun(Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result             = qp->ResultStorage(s);
  Midi* midi         = (Midi*) (args[0].addr);
  CcString* filename = (CcString*) args[1].addr;

  if (!midi->IsDefined())
  {
    ((CcBool*) result.addr)->Set(true, false);
  }

  vector<unsigned char> byteVector;
  midiToBytes(midi, byteVector);
  char* byteArray = new char[byteVector.size()];

  for (unsigned int i = 0; i < byteVector.size(); i++)
  {
    byteArray[i] = byteVector[i];
  }

  char* fname = (char*) (filename->GetStringval());
  ofstream out(fname, ios::out | ios::binary);
  bool writeResult = false;

  if (out)
  {
    writeResult = out.write(byteArray, byteVector.size());
  }
  else
  {
    writeResult = false;
  }

  delete[] byteArray;
  ((CcBool*) result.addr)->Set(true, writeResult);
  return 0;
}

/*
4.4.3 Specification of operator ~saveto~

*/
const string SavetoMidiSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi string) -> bool"
    "</text--->"
    "<text>_ saveto _</text--->"
    "<text>Saves the selected Midi object to a file.</text--->"
    "<text>query m saveto \"myfile.mid\"</text--->"
    ") )";

/*
4.4.4 Definition of operator ~saveto~

*/
Operator savetoMidi (
        "saveto",               //name
        SavetoMidiSpec,         //specification
        SavetoMidiFun,          //value mapping
        Operator::SimpleSelect, //trivial selection function
        SavetoMidiTypeMap       //type mapping
 );

/*
4.5 Operator ~tempo\_bpm~

This operator returns the tempo of a Midi object shown as the number beats per
minute.

4.5.1 Type mapping function of operator ~tempo\_bpm~

The operator ~tempo\_bpm~ accepts a Midi object and return an integer object.

----    (midi)               -> int
----

*/
ListExpr genericMidiTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if ( nl->IsEqual(arg1, MIDI_STRING))
    {
      return nl->SymbolAtom("int");
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
4.5.2 Value mapping functions of operator ~tempo\_bpm~

*/
template<bool isBpm> int tempoFun(Word* args, Word& result, int message,
                                  Word& local, Supplier s)
/*
Describes the velocity of a Midi object

*/
{
  unsigned long value;
  vector<unsigned char> meta;

  Midi *currentMidi = (Midi*)args[0].addr;
  int currentTracks = currentMidi->GetNumberOfTracks();
/*
Gets the Midi object out of the argument list

*/
  bool found                  = false;
  CcString* metaEventValue    = new CcString();

  for ( int i = 0; i < currentTracks && !found; i++ )
  {
    metaEventValue = FindMetaEvent(currentMidi,Event::TEMPO,i);
    if (metaEventValue->IsDefined())
    {
      found = true;
    }
  }
/*
Walks through all tracks and searches for the MetaEvent ''tempo'' from the Midi
object, the value ~defined~ from CcString
is used to declare a valid result ( ''undefined'' would say no result was
received ).
Normally the metaEvent ''Tempo'' is located inside the first track. To be sure
all tracks are examined.

*/
  result = qp->ResultStorage(s);

  if (found)
  {
    char* metaString = (char*)metaEventValue->GetStringval();

    for ( int i = 0; i < 3; i++)
    {
      unsigned char c = (unsigned char)metaString[i];
      meta.push_back(c);
    }

    if (isBpm)
    {
      value = 60 * 1000 * 1000 / BytesToIntNoClearMSB(&meta);
/*
Conversion from microseconds per quarter note into beats per minute

*/
    }
    else
    {
      value = BytesToIntNoClearMSB(&meta);
    }
  }
  else
  {
    value = 0;
  }

  ((CcInt*)result.addr)->Set(metaEventValue->IsDefined(), (int)value);
/*
The first argument says the integer value is defined, the second is the integer
value

*/
  return 0;
}

/*
4.5.3 Specification of operator ~tempo\_bpm~

*/
const string tempoBpmSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> int"
    "</text--->"
    "<text>_ tempo_bpm</text--->"
    "<text>Returns the tempo of the selected "
    "Midi object. It is shown as BPM "
    "(beats per minute ).</text--->"
    "<text>query midifile tempo_bpm</text--->"
    ") )";

/*
4.5.4 Definition of operator ~tempo\_bpm~

*/
Operator tempo_bpm (
        "tempo_bpm",            //name
        tempoBpmSpec,           //specification
        tempoFun<true>,         //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericMidiTypeMap      //type mapping
 );

/*
4.5 Operator ~tempo\_ms~

This operator returns the tempo of a Midi object shown as the number of
microseconds per quarter note.

4.5.1 Type mapping function of operator ~tempo\_ms~

The operator ~tempo\_ms~ accepts a Midi object and return an integer object.

----    (midi)               -> int
----

( please refer to function genericMidiTypeMap )

*/

/*
4.5.2 Value mapping functions of operator ~tempo\_ms~

( please refer to function tempoBpm )

*/

/*
4.5.3 Specification of operator ~tempo\_ms~

*/
const string tempoMsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> int"
    "</text--->"
    "<text>_ tempo_ms</text--->"
    "<text>Returns the tempo of the selected "
    "Midi object. It is shown as number of "
    "microseconds per quarter note.</text--->"
    "<text>query midifile tempo_ms</text--->"
    ") )";

/*
4.5.4 Definition of operator ~tempo\_ms~

*/
Operator tempo_ms (
        "tempo_ms",             //name
        tempoMsSpec,            //specification
        tempoFun<false>,        //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericMidiTypeMap      //type mapping
 );

/*
4.6 Operator ~format~

This operator returns the format of a Midi object.

4.6.1 Type mapping function of operator ~format~

The operator ~format~ accepts a Midi object and returns an integer object.

----    (midi)               -> int
----

( please refer to function genericTrackTypeMap )

*/

/*
4.6.2 Value mapping functions of operator ~format~

*/
int formatFun(Word* args, Word& result, int message, Word& local,
              Supplier s)
/*
Describes the format of a Midi object

*/
{
  Midi *currentMidi = (Midi*)args[0].addr;
  int format = currentMidi->GetFormat();
/*
Gets the Midi object out of the argument list and determines the format of this
Midi object

*/
  result = qp->ResultStorage(s);

  if ( format >= MIN_FORMAT_MIDI && format <= MAX_FORMAT_MIDI )
  {
    ((CcInt*)result.addr)->Set(true, format);
  }
  else
  {
    ((CcInt*)result.addr)->Set(false, format);
  }
/*
The first argument says if the integer value is defined or not, the second is
the integer value. The format will be returned as undefined, if the value is
not inside their specified range.

*/
  return 0;
}

/*
4.6.3 Specification of operator ~format~

*/
const string formatSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> int"
    "</text--->"
    "<text>_ format</text--->"
    "<text>Returns the format of the selected "
    "Midi object. It is shown as integer 0, "
    "1 or 2.</text--->"
    "<text>query midifile format</text--->"
    ") )";

/*
4.6.4 Definition of operator ~format~

*/
Operator format (
        "format",               //name
        formatSpec,             //specification
        formatFun,              //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericMidiTypeMap      //type mapping
 );

/*
4.7 Operator ~count\_tracks~

This operator returns the number of used tracks of a Midi object.

4.7.1 Type mapping function of operator ~count\_tracks~

The operator ~count\_tracks~ accepts a Midi object and returns an integer
object.

----    (midi)               -> int
----

( please refer to function genericMidiTypeMap )

*/

/*
4.7.2 Value mapping functions of operator ~count\_tracks~

*/
int countTracksFun(Word* args, Word& result, int message, Word& local,
                   Supplier s)
/*
Counts the number of tracks of a Midi object

*/
{
  Midi *currentMidi = (Midi*)args[0].addr;
  int tracks = currentMidi->GetNumberOfTracks();
/*
Gets the Midi object out of the argument list and determines the number of
tracks

*/
  result = qp->ResultStorage(s);
  ((CcInt*)result.addr)->Set(true, tracks);
/*
The first argument says the integer value is defined, the second is the integer
value

*/
  return 0;
}

/*
4.7.3 Specification of operator ~count\_tracks~

*/
const string countTracksSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> int"
    "</text--->"
    "<text>_ count_tracks</text--->"
    "<text>Returns the number of used tracks "
    "of the selected Midi object.</text--->"
    "<text>query midifile count_tracks</text--->"
    ") )";

/*
4.7.4 Definition of operator ~count\_tracks~

*/
Operator count_tracks (
        "count_tracks",         //name
        countTracksSpec,        //specification
        countTracksFun,         //value mapping
        Operator::SimpleSelect, //trivial selection function
        genericMidiTypeMap      //type mapping
 );

/*
4.8 Operator ~count\_channels~

This operator returns the number of used channels inside a Midi object.

4.8.1 Type mapping function of operator ~count\_channels~

The operator ~count\_channels~ accepts a Midi object and an integer object. It
returns an integer object.

----    (midi int)               -> int
----

*/

ListExpr countChannelsTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) && nl->IsEqual(arg2, "int"))
    {
      return arg2;
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
4.8.2 Value mapping functions of operator ~count\_channels~

*/
int countChannelsFun(Word* args, Word& result, int message, Word& local,
                     Supplier s)
/*
counts the number of channels of a Midi object

*/
{
  const Event* currentEvent;
  int foundChannel    = 0;
  int countedChannels = 0;
  bool channels[MAX_CHANNELS_MIDI];

  Midi *currentMidi = (Midi*)args[0].addr;
  int track         = ((CcInt*)args[1].addr)->GetIntval();
/*
gets the Midi object and the to be examined track number out of the argument
list

*/
  result = qp->ResultStorage(s);

  if ( track >= MIN_TRACKS_MIDI &&
       track <= currentMidi->GetNumberOfTracks() )
  {
    track = track - 1;
    Track* currentTrack = currentMidi->GetTrack(track);
    int numberEvents    = currentTrack->GetNumberOfEvents();

    for ( int j = 0; j < MAX_CHANNELS_MIDI; j++ )
    {
      channels[j] = false;
    }
/*
initialises the array for counted channels

*/
    for ( int eventIndex = 0; eventIndex < numberEvents; eventIndex++)
    {
      currentEvent = currentTrack->GetEvent(eventIndex);

      if ( currentEvent->GetEventType() == shortmessage &&
          !currentEvent->GetShortMessageRunningStatus())
      {
        foundChannel = currentEvent->GetChannel();

        if ( !channels[foundChannel] )
        {
          channels[foundChannel] = true;
          countedChannels++;
        }
      }
    }
/*
Walks through the whole track and counts the number of *different* channels
inside

*/
    ((CcInt*)result.addr)->Set(true, countedChannels);
/*
The first argument says the integer value is defined, the second is the integer
value

*/
  }
  else
  {
    cout << "Error in count_channels: Parameter is out of range, ";
    cout << "the result is set to 0." << endl;
    ((CcInt*)result.addr)->Set(true, 0);
/*
Error before track deletion ( too few remaining tracks inside the Midi Object )
[->] return original Midi object

*/
  }
  return 0;
}

/*
4.8.3 Specification of operator ~count\_channels~

*/
const string countChannelsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> int"
    "</text--->"
    "<text>_ count_channels[_]</text--->"
    "<text>Returns the number of used channels "
    "inside the specified track "
    "of the selected Midi object."
    "If the selected track is outside the "
    "valid range of the Midi file, the return "
    "value is set to 0.</text--->"
    "<text>query midifile count_channels[2]</text--->"
    ") )";

/*
4.8.4 Definition of operator ~count\_channels~

*/
Operator count_channels (
        "count_channels",       //name
        countChannelsSpec,      //specification
        countChannelsFun,       //value mapping
        Operator::SimpleSelect, //trivial selection function
        countChannelsTypeMap    //type mapping
 );

/*
4.9 Operator ~track\_name~

This operator returns the name of the specified track number.

4.9.1 Type mapping function of operator ~track\_name~

The operator ~track\_name~ accepts a Midi object and an integer object. It
returns a string object.

----    (midi int)               -> string
----

*/

ListExpr trackStringTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) && nl->IsEqual(arg2, "int") )
    {
      return nl->SymbolAtom("string");
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
4.9.2 Value mapping functions of operator ~track\_name~

*/
int trackNameFun(Word* args, Word& result, int message, Word& local,
                 Supplier s)
/*
Returns the name of the selected track out of a Midi object

*/
{
  Midi *currentMidi = (Midi*)args[0].addr;
  int namedTrack    = ((CcInt*)args[1].addr)->GetIntval() - 1;
/*
gets the Midi object and the to be examined track number out of the argument
list

*/
  CcString* metaEventValue = FindMetaEvent(currentMidi, Event::TRACK_NAME,
    namedTrack);
/*
Searches for the track name in the specified track

*/
  result = qp->ResultStorage(s);
  ((CcString*)result.addr)->Set(metaEventValue->IsDefined(),
    metaEventValue->GetStringval());
/*
Examination was succesfull [->] return text string

*/
  return 0;
}

/*
4.9.3 Specification of operator ~track\_name~

*/
const string trackNameSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int) -> string"
    "</text--->"
    "<text>_ track_name[_]</text--->"
    "<text>Returns the name of the selected "
    "track.</text--->"
    "<text>query midifile track_name [2]</text--->"
    ") )";

/*
4.9.4 Definition of operator ~track\_name~

*/
Operator track_name (
        "track_name",           //name
        trackNameSpec,          //specification
        trackNameFun,           //value mapping
        Operator::SimpleSelect, //trivial selection function
        trackStringTypeMap      //type mapping
 );

/*
4.10 Operator ~time\_signature~

This operator returns the time signature of a Midi object.

4.10.1 Type mapping function of operator ~time\_signature~

The operator ~time\_signature~ accepts a Midi object and returns a string
object.

----    (midi)               -> string
----

*/

ListExpr stringTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if ( nl->IsEqual(arg1, MIDI_STRING) )
    {
      return nl->SymbolAtom("string");
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
4.10.2 Value mapping functions of operator ~time\_signature~

*/
template<bool isBeat> int timeSignatureFun(Word* args, Word& result, int
                                           message, Word& local, Supplier s)
/*
Returns the time signature of a Midi object

*/
{
  string signature;

  Midi *currentMidi = (Midi*)args[0].addr;
  int currentTracks = currentMidi->GetNumberOfTracks();
/*
Gets the Midi object and the number of used tracks out of the argument list

*/
  bool found               = false;
  CcString* metaEventValue = new CcString();

  for ( int i = 0; i < currentTracks && !found; i++ )
  {
    metaEventValue = FindMetaEvent(currentMidi,Event::TIME_SIGNATURE,i);
    if (metaEventValue->IsDefined())
    {
      found = true;
    }
  }
/*
Walks through all tracks and searches for the MetaEvent ''time\_signature''
from the Midi object, the value ~defined~ from CcString
is used to declare a valid result ( undefined would say no result was received
)

*/
  signature.clear();
/*
An empty result string is always good, it is filled only if defined. If not
then it will be returned undefined and empty.

*/
  if (metaEventValue->IsDefined())
  {
    unsigned char* metaEventBytes = (unsigned char*)
      metaEventValue->GetStringval();
/*
Gets the defined result and moves it into a vector for further processing

*/
    vector<unsigned char> temp;
    temp.push_back(metaEventBytes[0]);
    unsigned short int numerator = Event::ComputeBytesToInt(&temp);
    temp.clear();
/*
Processes the numerator, together with the denominator they are representing
the time signature as the Midi would be notated

*/
    temp.push_back(metaEventBytes[1]);
    unsigned short int denominator = Event::ComputeBytesToInt(&temp);
    denominator = (unsigned int) pow((double)denominator, 2);
    temp.clear();
/*
Processes the denominator, inside a Midi it is stored as a negative power of 2
[->] a 3 means an eight-note

*/
    temp.push_back(metaEventBytes[2]);
    unsigned short int midiClocks = Event::ComputeBytesToInt(&temp);
    temp.clear();
/*
Processes the Midi clocks, a value expressing the number of Midi clocks in a
metronome click

*/
    temp.push_back(metaEventBytes[3]);
    unsigned short int notation = Event::ComputeBytesToInt(&temp);
    temp.clear();
/*
Processes the notation, a value expressing the number of notated 32nd-notes in
a Midi quarter-note ( 24 Midi clocks )

*/
    string c = IntToString(numerator);
    signature.assign(c);
    signature.append("/");
    c = IntToString(denominator);
    signature.append(c);

    if (!isBeat)
    {
      signature.append(" ");
      c = IntToString(midiClocks);
      signature.append(c);
      signature.append(" ");
      c = IntToString(notation);
      signature.append(c);
    }

    signature.append("\0");
/*
Return string is built out of numerator, denominator, midiClocks and notation
for TIME\_SIGNATURE. For BEAT only numerator and denominator are built
together.

*/
  }

  result = qp->ResultStorage(s);
  ((CcString*)result.addr)->Set(metaEventValue->IsDefined(),
    (STRING*)signature.c_str());
/*
Examination was succesfull [->] return text string

*/
  return 0;
}

/*
4.10.3 Specification of operator ~time\_signature~

*/
const string timeSignatureSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> string"
    "</text--->"
    "<text>_ time_signature</text--->"
    "<text>Returns the time signature "
    "of the selected Midi object.</text--->"
    "<text>query midifile time_signature</text--->"
    ") )";

/*
4.10.4 Definition of operator ~time\_signature~

*/
Operator time_signature (
        "time_signature",         //name
        timeSignatureSpec,        //specification
        timeSignatureFun<false>,  //value mapping
        Operator::SimpleSelect,   //trivial selection function
        stringTypeMap             //type mapping
 );

/*
4.11 Operator ~beat~

This operator returns the beat of the Midi.

4.11.1 Type mapping function of operator ~beat~

The operator ~beat~ accepts a Midi object and returns an string object.

----    (midi)               -> string
----

( please refer to function stringTypeMap )

*/

/*
4.11.2 Value mapping functions of operator ~beat~

( please refer to function timeSignatureFun )

*/

/*
4.11.3 Specification of operator ~beat~

*/
const string beatSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> string"
    "</text--->"
    "<text>_ beat</text--->"
    "<text>Returns the beat of the "
    "Midi object shown as a string."
    "Example of result : 3/4</text--->"
    "<text>query midifile beat</text--->"
    ") )";

/*
4.11.4 Definition of operator ~beat~

*/
Operator beat (
        "beat",                 //name
        beatSpec,               //specification
        timeSignatureFun<true>, //value mapping
        Operator::SimpleSelect, //trivial selection function
        stringTypeMap           //type mapping
 );

/*
4.11 Operator ~instrument\_name~

This operator returns the name of the instrument for a track.

4.11.1 Type mapping function of operator ~instrument\_name~

The operator ~instrument\_name~ accepts a Midi object and an integer value. It
returns an string object.

----    (midi int)               -> string
----

( please refer to function trackStringTypeMap )

*/

/*
4.11.2 Value mapping functions of operator ~instrument\_name~

*/
int instrumentNameFun(Word* args, Word& result, int message, Word& local,
                      Supplier s)
/*
Returns the name of the used instrument in a track

*/
{
  Midi *currentMidi = (Midi*)args[0].addr;
  int currentTrack  = ((CcInt*)args[1].addr)->GetIntval() - 1;
/*
Gets the Midi object out of the argument list and gets the number of used
tracks

*/
  CcString* metaEventValue = new CcString();
  metaEventValue = FindMetaEvent(currentMidi, Event::INSTRUMENT_NAME,
    currentTrack);
/*
Walks through all tracks and searches for the MetaEvent ''instrument\_name''
from the track object, the value ~defined~ from
CcString is used to declare a valid result ( undefined would say no result was
received )

*/
  result = qp->ResultStorage(s);
  ((CcString*)result.addr)->Set(metaEventValue->IsDefined(),
    metaEventValue->GetStringval());
/*
Examination was succesfull [->] return text string

*/
  return 0;
}

/*
4.11.3 Specification of operator ~instrument\_name~

*/
const string instrumentNameSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi int) -> string"
    "</text--->"
    "<text>_ instrument_name[_]</text--->"
    "<text>Returns the name of the used "
    "instrument inside the track.</text--->"
    "<text>query midifile instrument_name[3]</text--->"
    ") )";

/*
4.11.4 Definition of operator ~instrument\_name~

*/
Operator instrument_name (
        "instrument_name",      //name
        instrumentNameSpec,     //specification
        instrumentNameFun,      //value mapping
        Operator::SimpleSelect, //trivial selection function
        trackStringTypeMap      //type mapping
 );

/*
4.12 Operator ~get\_name~

This operator returns the name of a Midi object.

4.10.1 Type mapping function of operator ~get\_name~

The operator ~get\_name~ accepts a Midi object and returns a string object.

----    (midi)               -> string
----

( please refer to function stringTypeMap )

*/

/*
4.12.2 Value mapping functions of operator ~get\_name~

*/
int getNameMidiFun(Word* args, Word& result, int message, Word& local,
                   Supplier s)
/*
Returns the name of a Midi object

*/
{
  Midi *currentMidi = (Midi*)args[0].addr;
  CcString* seqName = FindMetaEvent(currentMidi, Event::TRACK_NAME, 0);
/*
Searches in track 1 for the MetaEvent ''track\_name'' from the Midi object.
According to the specification the name of format 0 and 1 Midi files is inside
track 1.

*/
  result = SetWord(seqName);
  return 0;
}

/*
4.12.3 Specification of operator ~get\_name~

*/
const string getNameMidiSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(midi) -> string"
    "</text--->"
    "<text>_ get_name</text--->"
    "<text>Returns the name of the "
    "selected Midi object. The name is taken "
    " out of track 1</text--->"
    "<text>query midifile get_name</text--->"
    ") )";

/*
4.12.4 Definition of operator ~get\_name~

*/
Operator get_name_midi (
        "get_name",             //name
        getNameMidiSpec,        //specification
        getNameMidiFun,         //value mapping
        Operator::SimpleSelect, //trivial selection function
        stringTypeMap           //type mapping
 );

} // namespace midialgebra

/*
5 Creating the Algebra

*/
using namespace midialgebra;

class MidiAlgebra : public Algebra
{
 public:
  MidiAlgebra() : Algebra()
  {
    AddTypeConstructor( &midi  );

    midi.AssociateKind( "DATA" );

    AddOperator( &extract_track         );
    AddOperator( &merge_tracks          );
    AddOperator( &transpose_track       );
    AddOperator( &transpose_midi        );
    AddOperator( &extract_lyrics        );
    AddOperator( &contains_words        );
    AddOperator( &contains_sequence     );
    AddOperator( &delete_track          );
    AddOperator( &expand_track          );
    AddOperator( &savetoMidi            );
    AddOperator( &tempo_ms              );
    AddOperator( &tempo_bpm             );
    AddOperator( &format                );
    AddOperator( &count_tracks          );
    AddOperator( &track_name            );
    AddOperator( &time_signature        );
    AddOperator( &beat                  );
    AddOperator( &instrument_name       );
    AddOperator( &count_channels        );
    AddOperator( &get_name_midi         );
  }
  ~MidiAlgebra() {};
};

MidiAlgebra midiAlgebra;

/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager has a
reference to this function if this algebra is included in the listof required
algebras, thus forcing the linker to include this module.
The algebra manager invokes this function to get a reference to the instanceof
the algebra class and to provide references to the globalnested listcontainer
(used to store constructor, type, operator and object information)and to the
query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeMidiAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&midiAlgebra);
}
