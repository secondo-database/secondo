/*
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
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[->] [$\rightarrow $]

\pagebreak

1 Implementation of MidiTrack

This class its purpose is to evaluate the meta informations provided by
the tracks meta messages

2 Imports

*/
package viewer.midi;

import javax.sound.midi.*;
import javax.sound.midi.Track.*;
import java.util.Vector;

/*
3 Class ~MidiTrack~

*/
public class MidiTrack
{
/*
3.1 private components of a MidiTrack

The name found by analyzing the tracks meta events

*/
private String name;
/*
The track number of the current track

*/
private int index;
/*
The vector contains all found meta informations. Used by the MidiViewer for creating the tab ~MetaInformations~.

*/
private Vector metaInfos = new Vector();
/*
If a possible name (assumedName) has been found, the foundName flag will
be set

*/
private boolean foundName = false;
/*
3.1 Constructor

The constructoer is called with two arguments: the tracknumber and the
track itself. It checks the whole track for any meta messages and
evaluates them. By assuming the name of the sequence, the meta message
has the type ~03~. As this message could also describe the tracks name
we assume that the track, to which the sequence name belongs, has a length
of zero ticks. This seems to be the only practicable approach.

*/
MidiTrack(int i, Track track)
{
  name = "";
  MidiEvent   midiEvent;
  MetaMessage metaMessage;
  MidiMessage midiMessage;
  String s;
  // a simple reference to a String to get the messages
  metaInfos.addElement(("Track Number: "+(i+1)));

  for (int j = 0; j < track.size(); j++)
  {
    midiEvent   = track.get(j);
    midiMessage = midiEvent.getMessage();

    if (midiMessage.getStatus() == 255) // that is a MetaMessage
    {
      metaMessage = (MetaMessage) midiEvent.getMessage();

      if (metaMessage.getType() == 3) // Type 3 = Track or Sequence name
      {
        s     = new String(metaMessage.getData());
        name += s;
        metaInfos.addElement("Name : " + name);

        if (track.ticks() == 0)
        {
          foundName = true;
        }
      }

      if (metaMessage.getType() == 1) // Type 1 = Text Event
      {
        s = new String(metaMessage.getData());
        metaInfos.addElement(s);
      }

      if (metaMessage.getType() == 2) // Type 2 = copyright info
      {
        s = new String(metaMessage.getData());
        metaInfos.addElement("Copyright: " + s);
      }

      if (metaMessage.getType() == 4) // Type 4 = Track instrument name
      {
        s = new String(metaMessage.getData());
        metaInfos.addElement("Instrument: " + s);
      }
    }
    s           = null;
    midiEvent   = null;
    metaMessage = null;
  }
  index = i;
  metaInfos.addElement(("Tracklength in MidiTicks: " + track.ticks()));
  metaInfos.addElement(("Number of Events: " + track.size()));
  metaInfos.addElement("--------");
  metaInfos.addElement("");
  name = i+1 + "  " + name;
}
/*
3.3 getMethods

3.3.3 getName

Returns the assumedName

*/
public String getName()
{
  return name;
}
/*
3.3.3 getIndex

Returns the tracknumber of the current track

*/
public int getIndex()
{
  return index;
}
/*
4.4.4 getMeta

Returns the generated meta informations as a vector of ~Strings~

*/
public Vector getMeta()
{
  return metaInfos;
}
/*
5.5.5 foundPossibleName

Returns true, if the sequence name was found

*/
public boolean foundPossibleName()
{
  return foundName;
}

}
