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
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[->] [$\rightarrow $]

\pagebreak

1 Implementation of the JavaGUI

This class provides the SECODNO viewer for Midi type objects and relations
containing midis. Therefore this viewer consists of two single
viewers combined to a whole one, being capable of displaying both.

2 Imports

*/
package viewer;

import javax.swing.*;
import javax.swing.text.*;
import javax.swing.event.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.util.Vector;
import java.util.*;
import java.awt.*;
import java.lang.Long;
import java.awt.event.*;
import gui.SecondoObject;
import gui.*;
import sj.lang.*;
import tools.*;
import viewer.midi.*;
import javax.swing.Timer;
import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.io.*;
import javax.sound.midi.*;


/*
3 Class ~MidiViewer~

This class extends the class SecondoViewer and must therefore implement the
abstract methods inherited. Additionally it implements three
listener interfaces to act itself as ActionListener.

*/


public class MidiViewer extends SecondoViewer implements ChangeListener,
                                              ItemListener,ListSelectionListener
{
/*
3.1 private components

The name of the viewer including its version

*/
  private static final String name = "MidiViewer 1.22";

/*
*global swing-components*

A JTabbedPane with three tabs as the primary organising component

*/
  private JTabbedPane jtp = new JTabbedPane(JTabbedPane.TOP);

/*
The first tab is the playPanel, containing the Midi player

*/
  private JPanel  playPanel           = new JPanel();
  private JPanel  PlayerPanel         = new JPanel();
  private JPanel  InfoPanel           = new JPanel();
  private JButton play                = new JButton("play");
  private JButton halt                = new JButton("pause");
  private JButton stop                = new JButton("stop");
  private JButton ff                  = new JButton("ff");
  private JButton rew                 = new JButton("rew");
  private JButton export              = new JButton("export");
  private JButton skipP               = new JButton("skip +");
  private JButton skipM               = new JButton("skip -");
  private JButton setTickPosition     = new JButton("setTickPosition");
  private JList list;
  private JSlider slider              = new JSlider(JSlider.HORIZONTAL, 0, 100000, 0);
  private JLabel time                 = new JLabel("0:00 / 0:00");
  private JPanel TrackPanel           = new JPanel();
  private JLabel infoNameLabel        = new JLabel(("----- Welcome to  " + name +" -----"), JLabel.CENTER);
  private JLabel infoAssumedNameLabel = new JLabel();
  private JLabel infoTempoLabel       = new JLabel();
  private JLabel infoLengthLabel      = new JLabel();
  private JLabel infoTickLengthLabel  = new JLabel();
  private JLabel infoTrackLabel       = new JLabel();
  private JLabel infoResLabel         = new JLabel();
/*
The second tab is used to display the relations

*/
  private JPanel relPanel = new JPanel();
  private JComboBox ComboBox;
  private JScrollPane ScrollPane;
  private JTable CurrentTable;
  private JPanel dummy;
/*
The third tab displays the meta informations of the currently playing
Midi

*/
  private JPanel metaPanel = new JPanel();
/*
*global components of the adapted relation viewer*

*/
  private Vector Tables;
  private Vector midiTables = new Vector();
  private boolean listSelectionAllowed = true;
/*
*global components of the Midi player*

All midis of the player are stored in a vector, representing the
~playlist~, as there should be no limitation of the maximum of queried
midis

*/
  private Vector midiVector;
/*
The first approach to the MidiViewer project was to be as object
orientated as possible. Therefore every MidiFile had its own sequencer.
After some performance and threading problems we decided that there
should be only one sequencer managed by the viewer itself.

*/
  private static Sequencer sequencer = null;
/*
As it is not guaranteed that a sequencer provided by the system is an
instance of ~Synthesizer~ there must be a way to call a synthsizer, a
receiver and a transmitter

*/
  private Synthesizer synthesizer = null;
  private Receiver    receiver    = null;
  private Transmitter transmitter = null;

/*
The sequencer loads only the sequence from a MidiFile. That is why it is
necessary to have a reference to the currently loaded MidiFile.

*/
  private MidiFile thisMidiFile = null;

/*
The slider should capable to change its value as the Midi is playing,
so that its position must be updated continuesly. On the other hand,
every change made by the user should set the current position.
The problem was, that every change of the slider position
made by the player, invoked the ActionListner waiting for
user commands. This caused unfortunately some bad behavior of the
sequencer. Therefore a flag is used to indicate whether the change was
made by the user or the system.

*/
  private boolean mouseSliding = false;
/*
An array of the current tracks

*/
  private MidiTrack[] currentTracks;
/*
As a sequencer does not implement a pause function, it is neccassary to
know the position to relaunch after stopping the sequencer for pausing

*/
  private long startAt = 0;

/*
By using the skip buttons, the position shall be changed by ten
percent of the whole length

*/
  private long skippy = 0;
/*
Length of the sequence is handled as ~String~

*/
  private String length              = "0:00 / 0:00";
  private String lengthOfCurrentMidi = "0:00";
/*
During a Midi sequence the tempo might change. To display always the
correct tempo it's necessary to save the current displayed tempo to not
always update the displayed tempo when not needed

*/
private float displayedTempo = 0.0f;

/*
3.1 constructor

The constructor initializes the whole GUI in a couple of steps.

*/
  public MidiViewer()
  {
/*
Creates the GUI by calling the three methods to create and show the
three tabs of the viewer

*/
    createAndShowPlayerGUI();
    createAndShowRelationGUI();
    createAndShowMetaPanel();

/*
As swing is not threadsafe the two update methods ~refreshTime~
and ~refreshSlider~ are called from a timer

*/
    int delay = 50;
    ActionListener taskPerformer = new ActionListener() {
      public void actionPerformed(ActionEvent evt)
                {
                  refreshTime();
                  refreshSlider();
                }
            };
    new Timer(delay, taskPerformer).start();

    setLayout(new BorderLayout());

/*
As the viewer manages the one and only sequencer of the whole
implementation, the ~MidiSystem~ and expecially the sequencer needs to be
initialized.

*/
    try { sequencer = MidiSystem.getSequencer(); }
    catch (MidiUnavailableException e)
    {
      System.out.println("MidiFile 3: Midi unavailable");
    }

    if (sequencer == null)
    {
      System.out.println("MidiFile 4: can't get a sequencer");
    }

    if (! (sequencer instanceof Synthesizer))
    {
      try
      {
        synthesizer = MidiSystem.getSynthesizer();
        synthesizer.open();
        receiver = synthesizer.getReceiver();
        transmitter = sequencer.getTransmitter();
        transmitter.setReceiver(receiver);
      }
      catch (MidiUnavailableException e)
      {
        System.out.println("MidiFile 7: can't get a synthesizer");
      }
    }

/*
Getting the GUI together

*/
    jtp.addTab("Midi-Player", playPanel);
    jtp.add("Relation", relPanel);
    jtp.add("Meta-Informations", (new JScrollPane(metaPanel)));
    add(jtp, BorderLayout.CENTER);
  }

/*
3.2 inherited methods of SecondoViewer

3.3.3 getName

Returns the name of the viewer

*/
  public String getName()
  {
    return name;
  }

/*
3.2.3 addObject

Checks whether the given SecondoObject is a Midi or relation containing
midis. If it is a Midi, a MidiFile is created, automatically started and added to the
playlist. Otherwise a table is created containing the relation. From this table
it is possible to load the containing midis

*/
  public boolean addObject(SecondoObject o)
  {
    ListExpr LE = o.toListExpr();

    if (LE.first().isAtom() && LE.first().symbolValue().equals("midi"))
    {
      unload();

      if (isDisplayed(o))
      {
        for (int i = 0; i < midiVector.size(); i++)
        {
          if(((MidiFile)midiVector.elementAt(i)).getID().equals(
            o.getID().toString()))
          {
            thisMidiFile = (MidiFile)midiVector.elementAt(i);
            load();
            play();
            return true;
          }
        }
        return true;
      }
      else
      {
        Base64Decoder  b64;
        StringReader sr = new StringReader(LE.second().first().textValue());
        b64 = new Base64Decoder(sr);
        MidiFile midi = new MidiFile(b64.getInputStream(), o.getName(),
          o.getID().toString());
        midiVector.addElement(midi);
        list.setListData(midiVector);
        thisMidiFile = midi;
        load();
        long l = getLengthInTicks();
        length = Long.toString(l);
        play();
        slider.setMaximum((int) getLengthInTicks());
        currentTracks = thisMidiFile.getTrackArray();
        setTrackPanel();
        setInfoPanel();
        jtp.setSelectedIndex(0);
        return true;
      }
    }
    else
    {
      if (isDisplayed(o))
      {
        selectObject(o);
        return false;
      }
      else
      {
        JTable NTable = createTableFrom(o.toListExpr(), o.getName());
        if(NTable==null)
        {
          return false;
        }
        else
        {
          Tables.add(NTable);
          ComboBox.addItem(o.getName());
          selectObject(o);
          ScrollPane.setViewportView(NTable);
          jtp.setSelectedIndex(1);
          return true;
        }
      }
    }
  }

/*
3.3.3 removeObject

Removes the targeted object from the viewer. If a relation has to be
removed, so all objects are loaded from the relation into the player.

*/

  public void removeObject(SecondoObject o)
  {
    ListExpr LE = o.toListExpr();

    if (LE.first().isAtom() && LE.first().symbolValue().equals("midi"))
    {
      if (thisMidiFile.getID().equals(o.getID().toString()))
      {
        midiVector.remove(thisMidiFile);
        unload();
        setInfoPanel();
        setTrackPanel();
        setMetaPanel();
      }
      else
      {
        int finder = -1;
        // searches the right midi
        for (int i = 0; i < midiVector.size(); i++)
        {
          if (((MidiFile)midiVector.elementAt(i)).getID().equals(
              o.getID().toString()))
          {
            finder = i;
          }
        }
        if (finder != -1)
        {
          midiVector.remove(finder);
        }
      }
      list.setListData(midiVector);
    }
    else
    {
      int index = getIndexOf(o.getName());

      if( index >= 0 )
      {
        kickThem(o.getName());
        setInfoPanel();
        setTrackPanel();
        setMetaPanel();
        ComboBox.removeItemAt(index);
        Tables.remove(index);
      }
    }
  }

/*
3.3.3 removeAll

Simple and selfexplaining

*/
  public void removeAll()
  {

    unload();
    midiVector.removeAllElements();
    list.setListData(midiVector);
    ComboBox.removeAllItems();
    Tables.clear();
    jtp.setSelectedIndex(0);
    setTrackPanel();
    setMetaPanel();
  }

/*
3.3.3 canDisplay

Checks if SecondoObject can be displayed. There are two possible types
which can be displayed. On the one hand a Midi can be displayed,
on the other hand relations can be displayed. If the type is rel, it is
checked if the relation contains midis at all.

*/
  public boolean canDisplay(SecondoObject o)
  {
    ListExpr LE = o.toListExpr();

    if(LE.listLength()!=2) return false;

    if(LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
       LE.first().symbolValue().equals("midi"))
    {
      if (LE.second().first().atomType() == ListExpr.TEXT_ATOM)
        return true;
      else return false;
    }
    LE = LE.first();

    if(LE.isAtom()) return false;
    else
    {
      LE = LE.first();
      if(LE.isAtom() && LE.atomType()==ListExpr.SYMBOL_ATOM &&
        (LE.symbolValue().equals("rel") |
         LE.symbolValue().equals("mrel")) )
      {
        if (containsMidiObject(o)) return true;
      }
    }
    return false;
  }

/*
4.4.4 isDisplayed

Checks whether an object is already displayed or not.

*/
  public boolean isDisplayed(SecondoObject o)
  {
    ListExpr LE = o.toListExpr();
    if (LE.first().isAtom() && LE.first().symbolValue().equals("midi"))
    {
      for (int i = 0; i < midiVector.size(); i++)
      {
        if(((MidiFile)midiVector.elementAt(i)).getID().equals(
          o.getID().toString()))
        {
          return true;
        }
      }
      return false;
    }
    else
    {
      return getIndexOf(o.getName())>=0;
    }
  }

/*
3.3.3 selectObject

Selected Object is displayed

*/
  public boolean selectObject(SecondoObject o)
  {
    ListExpr LE = o.toListExpr();
    if (LE.first().isAtom() && LE.first().symbolValue().equals("midi"))
    {
      unload();
      for (int i = 0; i < midiVector.size(); i++)
      {
        if(((MidiFile)midiVector.elementAt(i)).getID().equals(
            o.getID().toString()))
        {
          thisMidiFile = (MidiFile)midiVector.elementAt(i);
          load();
          play();
          setInfoPanel();
          setTrackPanel();
          jtp.setSelectedIndex(0);
          return true;
        }
      }
      setTrackPanel();
      return false;
    }
    else
    {
      int index = getIndexOf(o.getName());
      if ( index < 0 )
      {
        return false;
      }
      else
      {
        ComboBox.setSelectedIndex(index);
        showSelectedObject();
        jtp.setSelectedIndex(1);
        return true;
      }
    }

  }

/*
2.2.2 getMenuVector

As there is no need for an extra menu on the GUI, null is returned

*/
  public MenuVector getMenuVector()
  {
    return null;
  }


/*
2.3.3 getDisplayQuality

If the SecondoObject is a Midi or a relation containing midis this
viewer shall be used to display it

*/
  public double getDisplayQuality(SecondoObject o)
  {
    if (canDisplay(o))
    {
      return 0.9;
    }
    else
    {
      return 0;
    }
  }

/*
3.2 the player

3.3.3 setTrackPanel

The track panel is located in the lower left corner of the Midi player.
It shall display all tracks and their names. Also it must be possible
to mute and to solo a single track.

*/
   void setTrackPanel()
  {
    TrackPanel.removeAll();

    if (sequencer == null || !sequencer.isOpen())
    {
      TrackPanel.add(new JLabel("Nothing to display"));
      TrackPanel.repaint();
    }
    else
    {
      int i;
      JPanel headline = new JPanel(new FlowLayout(FlowLayout.LEFT));
      headline.add(new JLabel(" Current Tracks - "));
      headline.add(new JLabel("monitoring states"));
      JButton playAll = new JButton("play all");
      playAll.addMouseListener(new MouseAdapter()
                              {
                                public void mouseClicked(MouseEvent e)
                                {
                                  unMuteAllTracks();
                                }
                              });
      TrackPanel.add(headline);
      JPanel playAllPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
      playAllPanel.add(playAll);
      TrackPanel.add(playAllPanel);

      for (i = 0; i < currentTracks.length; i++)
      {
        boolean checked = !sequencer.getTrackMute(i);
        JPanel    jp    = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JCheckBox cb    = new JCheckBox(currentTracks[i].getName(),
                                        checked);

        JButton jb = new JButton("Solo");
        jb.setName(i + "  ");
        cb.addItemListener(this);
        jb.addMouseListener(new MouseAdapter()
                            {
                              public void mouseClicked(MouseEvent e)
                              {
                                int i = getIndexNo(((JButton)
                                  e.getSource()).getName());
                                soloTrack(i);
                              }
                            });
        jp.add(jb);
        jp.add(cb);
        TrackPanel.add(jp);
      }
      TrackPanel.setLayout(new GridLayout(i+2, 1));
    }
  }


/*
2.3.2 soloTrack

If a solo button is hit, every track but one needs to be set to the mute state.

(As said in the Java documentation it is possible that not every system
supports the solo functionality provided by Java. Therefore we are just
using mute functions.)

*/
  void soloTrack(int trackSolo)
  {
    for (int i = 0; i < currentTracks.length; i++)
    {
      if (i != trackSolo)
      {
        sequencer.setTrackMute(i, true);
      }
      else
      {
        sequencer.setTrackMute(i, false);
      }
    }
    setTrackPanel();
  }

/*
3.3.3 unMuteAllTracks

Reverses the effect of soloTrack and unsets all mute states

*/
  void unMuteAllTracks()
  {
    for(int i = 0; i < currentTracks.length; i++)
    {
      sequencer.setTrackMute(i, false);
    }
    setTrackPanel();
  }

/*
The info panel is located right under the player controls in the lower
right corner. It is meant to give a quick overview of the current
playing Midi.

*/
  void setInfoPanel()
  {
    if (sequencer == null || !sequencer.isOpen())
    {
      infoAssumedNameLabel.setText("");
      infoTempoLabel.setText("");
      infoLengthLabel.setText("");
      infoTickLengthLabel.setText("");
      infoTrackLabel.setText("");
      infoResLabel.setText("");
    }
    else
    {
      infoAssumedNameLabel.setText("Assumed Name: "+thisMidiFile.getName());
      displayedTempo = getTempo();
      infoTempoLabel.setText("Tempo: " +
        getTempo() / sequencer.getTempoFactor()  + " bpm  x  " +
        getTempoFactor() + " = " + getTempo()+ " bpm");
      infoLengthLabel.setText("Length: "+ lengthOfCurrentMidi);
      infoTickLengthLabel.setText("Length in Ticks: " + sequencer.getTickLength());
      infoTrackLabel.setText("Tracks: "+ thisMidiFile.getNrTracks());
      if (thisMidiFile.getDivisionType() == Sequence.PPQ)
      {
        infoResLabel.setText("Resolution in ticks per beat: "
          + thisMidiFile.getRes());
      }
      else
      {
        infoResLabel.setText("Resolution in ticks per frame: "
          + thisMidiFile.getRes());
      }
    }
  }
/*
3.2.2 set MetaPanel

Calls the meta informations belonging to a MidiFile and displays them at
the third tab

*/
   void setMetaPanel()
  {
    int zaehler = 0;
    metaPanel.removeAll();

    if (thisMidiFile == null)
    {
      metaPanel.add(new JLabel("nothing to display..."));
    }
    else
    {
      Vector v;

      for (int i = 0; i < thisMidiFile.getNrTracks(); i++)
      {
        v = thisMidiFile.getMetaInfos(i);

        for (int j = 0; j < v.size(); j++)
        {
          String s  = (String) v.get(j);
          JLabel jl = new JLabel(s);
          metaPanel.add(jl);
          zaehler++;
        }
      }
      metaPanel.setLayout(new GridLayout(zaehler,1));
    }
  }

/*
3.4.3 createAndShowMetaPanel

Some kind of ~constructor~ for the meta panel. When it is called there
can be no Midi file loaded. So there is nothing to do.

*/
  public void createAndShowMetaPanel()
  {
    metaPanel.add(new JLabel("nothing to display"));
  }


/*
2.2.2 createAndShowPlayerGUI

In contrast to ~createAndShowMetaPanel~ this ~constructor~ of the player
is much more sophisticated as every button. ActionListener and all other
components needs to be initialized.

*/
  public void createAndShowPlayerGUI()
  {
    midiVector = new Vector();
/*
the buttons and their listener are linked

*/
    play.addMouseListener(new MouseAdapter()
                          {
                            public void mouseClicked (MouseEvent e)
                          {
                            if (sequencer.isOpen())
                              {
                                play();
                                setInfoPanel();
                              }
                          }
                          });

    halt.addMouseListener(new MouseAdapter()
                          {
                            public void mouseClicked(MouseEvent e)
                            {
                              if (sequencer.isOpen())
                              {
                                halt();
                                //setInfoPanel();
                              }
                            }
                          });

    ff.addMouseListener(new MouseAdapter()
                        {
                          public void mouseClicked(MouseEvent e)
                          {
                            if (sequencer.isOpen())
                              {
                                fastforward();
                              }
                          }
                        });

    rew.addMouseListener(new MouseAdapter()
                          {
                            public void mouseClicked(MouseEvent e)
                            {
                              if (sequencer.isOpen())
                              {
                                rewind();
                              }
                            }
                          });

    stop.addMouseListener(new MouseAdapter()
                          {
                            public void mouseClicked(MouseEvent e)
                            {
                              if (sequencer.isOpen())
                              {
                                stop();
                              }
                            }
                          });

    export.addMouseListener(new MouseAdapter()
                            {
                              public void mouseClicked(MouseEvent e)
                              {
                                if(sequencer.isOpen())
                                {
                                  thisMidiFile.export();
                                }
                              }
                            });

    skipM.addMouseListener(new MouseAdapter()
                            {
                              public void mouseClicked(MouseEvent e)
                              {
                                if (sequencer.isOpen())
                                {
                                  skipM();
                                }
                              }
                            });

    skipP.addMouseListener(new MouseAdapter()
                            {
                              public void mouseClicked(MouseEvent e)
                              {
                                if(sequencer.isOpen())
                                {
                                  skipP();
                                }
                              }
                            });
    setTickPosition.addMouseListener(new MouseAdapter()
    {
      public void mouseClicked(MouseEvent e)
      {
        if (sequencer.isOpen())
        {
          try
          {
            String s = (String)JOptionPane.showInputDialog(
              null, "Set the Tick Position","Enter the new Tick Position" ,
              JOptionPane.QUESTION_MESSAGE);
            long l = (Long.decode(s)).longValue();

            if (sequencer.getTickLength() >= l)
            {
              sequencer.setTickPosition(l);
            }
          }
          catch (Exception ede) {System.out.println("wrong Input");}
        }
        refreshSlider();
      }
    });

    slider.addChangeListener(this);
/*
The borders of the buttons are set

*/
    Border bd1 = BorderFactory.createEtchedBorder();
    play.setBorder(bd1);
    play.setForeground(new Color(0,0,255));
    halt.setBorder(bd1);
    ff.setBorder(bd1);
    rew.setBorder(bd1);
    stop.setBorder(bd1);
    export.setBorder(bd1);
    slider.setBorder(bd1);
    skipM.setBorder(bd1);
    skipP.setBorder(bd1);
/*
As layout manager GridBagLayout is used. All Components get their
constraints.

*/
    InfoPanel.removeAll();
    InfoPanel.add(infoNameLabel);
    InfoPanel.add(infoAssumedNameLabel);
    InfoPanel.add(infoTempoLabel);
    InfoPanel.add(infoLengthLabel);
    InfoPanel.add(infoTickLengthLabel);
    InfoPanel.add(infoTrackLabel);
    InfoPanel.add(infoResLabel);
    InfoPanel.add(setTickPosition);

    // The playlist is created
    list = new JList(midiVector);
    list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

    list.addMouseListener(new MouseAdapter()
    {
      public void mouseClicked(MouseEvent e)
      {
        if (e.getClickCount() == 2 && listSelectionAllowed)
        {
          listSelectionAllowed = false;
          unload();
          thisMidiFile = ((MidiFile) list.getSelectedValue());
          load();
          play();
          setInfoPanel();
          setTrackPanel();
          listSelectionAllowed = true;
        }
      }
    });

    JScrollPane listScrollPane = new JScrollPane(list);

    // Trackpanel is generated
    TrackPanel.setLayout(new GridLayout(1,1));
    JScrollPane trackScrollPane = new JScrollPane(TrackPanel);

    // playercontrols are generated
    GridBagLayout playLayout = new GridBagLayout();
    GridBagConstraints playConstraints = new GridBagConstraints();
    PlayerPanel.setLayout(playLayout);
    playConstraints.gridx      = 0;
    playConstraints.gridy      = 0;
    playConstraints.gridwidth  = 2;
    playConstraints.gridheight = 1;
    playConstraints.weightx    = 100;
    playConstraints.weighty    = 100;
    playConstraints.fill       = GridBagConstraints.BOTH;

    playLayout.setConstraints(slider, playConstraints);
    PlayerPanel.add(slider);
    playConstraints.gridx = 2;
    playLayout.setConstraints(time, playConstraints);
    PlayerPanel.add(time);
    playConstraints.gridwidth = 1;
    playConstraints.gridx     = 0;
    playConstraints.gridy     = 1;
    playLayout.setConstraints(play, playConstraints);
    PlayerPanel.add(play);
    playConstraints.gridx = 1;
    playLayout.setConstraints(halt, playConstraints);
    PlayerPanel.add(halt);
    playConstraints.gridx = 2;
    playLayout.setConstraints(stop, playConstraints);
    PlayerPanel.add(stop);
    playConstraints.gridx = 3;
    playLayout.setConstraints(export, playConstraints);
    PlayerPanel.add(export);
    playConstraints.gridx = 0;
    playConstraints.gridy = 2;
    playLayout.setConstraints(rew, playConstraints);
    PlayerPanel.add(rew);
    playConstraints.gridx = 1;
    playLayout.setConstraints(skipM, playConstraints);
    PlayerPanel.add(skipM);
    playConstraints.gridx = 2;
    playLayout.setConstraints(skipP, playConstraints);
    PlayerPanel.add(skipP);
    playConstraints.gridx = 3;
    playLayout.setConstraints(ff, playConstraints);
    PlayerPanel.add(ff);

    // InfoPanel is generated
    InfoPanel.setLayout(new GridLayout(8,1));
    InfoPanel.setBorder(bd1);

    playPanel.setLayout(new GridLayout(1,1));

    GridBagLayout layoutLeftTop = new GridBagLayout();
    GridBagConstraints constraintsLeftTop = new GridBagConstraints();
    GridBagLayout layoutLeftButtom  = new GridBagLayout();
    GridBagConstraints constraintsLeftButtom = new GridBagConstraints();
    GridBagLayout layoutRight = new GridBagLayout();
    GridBagConstraints constraintsRight  = new GridBagConstraints();

    JPanel left = new JPanel(new GridLayout(1,1));
    JPanel right = new JPanel(layoutRight);
    JPanel top = new JPanel(layoutLeftTop);
    JPanel buttom = new JPanel(layoutLeftButtom);

    // adding the player controls
    constraintsRight.gridx      = 0;
    constraintsRight.gridy      = 0;
    constraintsRight.gridwidth  = 1;
    constraintsRight.gridheight = 1;
    constraintsRight.weightx    = 100;
    constraintsRight.weighty    = 100;
    constraintsRight.fill = GridBagConstraints.BOTH;
    layoutRight.setConstraints(PlayerPanel, constraintsRight);
    right.add(PlayerPanel);

    // adding the InfoPanel
    constraintsRight.gridy = 1;
    layoutRight.setConstraints(InfoPanel, constraintsRight);
    right.add(InfoPanel);

    // adding the trackpanel
    constraintsLeftButtom.gridx      = 0;
    constraintsLeftButtom.gridy      = 0;
    constraintsLeftButtom.gridwidth  = 1;
    constraintsLeftButtom.gridheight = 1;
    constraintsLeftButtom.weightx    = 100;
    constraintsLeftButtom.weighty    = 100;
    constraintsLeftButtom.fill       = GridBagConstraints.BOTH;
    layoutLeftButtom.setConstraints(trackScrollPane, constraintsLeftButtom);
    buttom.add(trackScrollPane);

    // adding the list
    constraintsLeftTop.gridx      = 0;
    constraintsLeftTop.gridy      = 0;
    constraintsLeftTop.gridwidth  = 1;
    constraintsLeftTop.gridheight = 1;
    constraintsLeftTop.weightx    = 100;
    constraintsLeftTop.weighty    = 100;
    constraintsLeftTop.fill       = GridBagConstraints.BOTH;
    layoutLeftTop.setConstraints(listScrollPane, constraintsLeftTop);
    top.add(listScrollPane);

    JSplitPane splitPaneLeftVertical = new JSplitPane(JSplitPane.VERTICAL_SPLIT, top, buttom);
    splitPaneLeftVertical.setContinuousLayout(true);
    left.add(splitPaneLeftVertical);

    JSplitPane splitPaneHorizontal = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, left, right);
    splitPaneHorizontal.setContinuousLayout(true);
    splitPaneHorizontal.setDividerLocation(400);

    playPanel.add(splitPaneHorizontal);

    setInfoPanel();
    setTrackPanel();
  }

/*
2.2.2 stateChanged

Implementation of the ChangeListener belonging to the slider

*/
  public void stateChanged(ChangeEvent e)
  {
    if(slider.getValueIsAdjusting())
    {
      mouseSliding = true;
    }
    if(!slider.getValueIsAdjusting())
    {
      if(sequencer.isOpen() && mouseSliding)
      {
        setPosition(slider.getValue());
        mouseSliding = false;
      }
    }
  }
/*
2.3.2 refreshSlider

Refreshes the slider

*/
  private void refreshSlider()
  {
    if (sequencer.isOpen())
    {
      if (!mouseSliding)
      {
        slider.setValue((int) getPosition());
        if (displayedTempo != getTempo())
        {
          setInfoPanel();
        }
      }
    }
  }
/*
2.2.2 refreshTime

Refreshes the time display

*/
  private void refreshTime()
  {
    if (sequencer.isOpen())
    {
      time.setText(getCurrentTime() + " / " + lengthOfCurrentMidi);
    }
  }
/*
3.3.3 getIndexNo

Extracts the first three digits of a given ~String~ and returns them as
an ~Integer~ value. This function is used to get the track number when a
mute check box has changed its value by using only the check box name.

*/
  private int getIndexNo(String s)
  {
    // by constructing the strings it is guarenteed that there are at
    // least 3 Characters
    char c1 = s.charAt(0);
    char c2 = s.charAt(1);
    char c3 = s.charAt(2);
    int result = 0;
    int firstDigit = (int) '0';
    boolean b = Character.isDigit(c3);

    // indicates which of the three characters contains is relevant
    if (b)
    {
      result += (((int) c3) - firstDigit);
      result += (((int) c2) - firstDigit)*10;
      result += (((int) c1) - firstDigit)*100;
      return result;
    }

    b = Character.isDigit(c2);

    if (b)
    {
      result += (((int) c2) - firstDigit);
      result += (((int) c1) - firstDigit)*10;
      return result;
    }
    else
    {
      result += (((int) c1) - firstDigit);
    }
    return result;
  }
/*
2.2.2 itemStateChanged

Implementation of the ItemListener belonging to the checkboxes of the
track panel

*/
  public void itemStateChanged(ItemEvent e)
  {
    JCheckBox cb = (JCheckBox)e.getSource();
    int index  = getIndexNo(cb.getText());

    if (e.getStateChange() == ItemEvent.SELECTED)
    {
      changeMute(index, false);
    }
    else
    {
      changeMute(index, true);
    }
  }

/*
5.5.5 load

Loads the sequence of the current MidiFile into the sequencer and
starts playing

*/
  public boolean load()
  {
    if (sequencer.isOpen()) unload();

    try { sequencer.open(); }
    catch (MidiUnavailableException e)
    {
      System.out.println("MidFile 5: Midi unavailable");
      return false;
    }
    try { sequencer.setSequence(thisMidiFile.getSequence());}
    catch (InvalidMidiDataException e)
    {
      System.out.println("MidiFile 6: Invalid Mididfile");
      return false;
    }
/*
A fresh Midi starts at the beginning

*/
    startAt = 0;
/*
The skipping amount is calculated

*/
    skippy = sequencer.getTickLength() / 20;
/*
And finally the length of the Sequence is expressed as MM:SS

*/
    long ticklength = sequencer.getMicrosecondLength();
    ticklength = ticklength / 1000000;
    long minutes = ticklength / 60;
    ticklength = ticklength - minutes*60;
    // ticklength -> now the remaining seconds
    lengthOfCurrentMidi = Long.toString(minutes);
    lengthOfCurrentMidi += ":";
    if (ticklength < 10) { lengthOfCurrentMidi += "0"; }
    lengthOfCurrentMidi += Long.toString(ticklength);
    slider.setMaximum((int) getLengthInTicks());
    setInfoPanel();
    setMetaPanel();
    return true;
  }
/*
2.2.2 unload

Closes the sequencer and releases the captured resources

*/
  public void unload()
  {
    if(sequencer.isOpen())
    {
      sequencer.stop();
      sequencer.close();
    }
    startAt      = 0;
    thisMidiFile = null;
  }
/*
2.2.2 play

Starts the sequencer. If it is already running the speed is set to
~normal~.

*/
  public void play()
  {
    if (!sequencer.isRunning())
    {
      sequencer.setTickPosition(startAt);
      sequencer.start();
    }
    else
    {
      sequencer.setTempoFactor(1.0f);
    }
  }
/*
2.2.2 stop

Stops the sequencer

*/
  public void stop()
  {
    sequencer.stop();
    sequencer.setTickPosition(0);
    startAt = 0;
  }
/*
3.3.3 halt

Stops the sequencer, but the current Position remains - pause function

*/
  public void halt()
  {
    startAt = sequencer.getTickPosition();
    sequencer.stop();
  }
/*
2.2.2 fastforward

Increases the speed by factor 2

*/
   public void fastforward()
  {
    if (sequencer.getTempoFactor() < 8)
    {
      sequencer.setTempoFactor(sequencer.getTempoFactor() * 2.0f);
    }
  }
/*
9.9.9 rewind

Decreases the speedfactor by 2

~Not really a~ rewind ~function but the opposite of fastforward.~

*/
   public void rewind ()
  {
    if (sequencer.getTempoFactor() > 0.25)
    {
      sequencer.setTempoFactor(sequencer.getTempoFactor() * 0.5f);
    }
  }
/*
2.2.2 skipP

Increases the actual position by 1/10 of the whole

*/
  public void skipP()
  {
    if((skippy+sequencer.getTickPosition())<getLengthInTicks())
    {
      sequencer.setTickPosition(sequencer.getTickPosition() + skippy);
    }
  }

/*
4.4.4 skipM

Decreases the actual position by 1/10 of the whole

*/
  public void skipM()
  {
    if (skippy < sequencer.getTickPosition())
    {
      sequencer.setTickPosition(sequencer.getTickPosition() - skippy);
    }
    else
    {
      sequencer.setTickPosition(0);
    }
  }

/*
6.6.6 setPosition

Sets the current position, necceassary for the slider

*/

  public void setPosition(long tick)
  {
    sequencer.setTickPosition(tick);
  }

/*
3.3.3 getPosition

Returns the current Position

*/
  public long getPosition()
  {
    try
    {
      return sequencer.getTickPosition();
    }
    catch (NullPointerException e)
    {
      return 0;
    }
  }
/*
5.4.4 getLengthInTicks

Returns the length of the sequence in ticks

*/
  public long getLengthInTicks()
  {
    return sequencer.getTickLength();
  }

/*
2.2.2 getCurrentTime

Returns the current Position expressed as a ~String~ (MM:SS)

*/
  public String getCurrentTime()
  {
    try
    {
      String time;
      long ticklength = sequencer.getMicrosecondPosition();
      ticklength      = ticklength / 1000000;
      long minutes    = ticklength / 60;
      ticklength      = ticklength - minutes*60;
      // length -> remaining seconds
      time  = Long.toString(minutes);
      time  += ":";
      if (ticklength < 10) {time += "0";}
      time  += Long.toString(ticklength);
      return time;
    }
    catch (NullPointerException e)
    {
      return "0:00";
    }
  }

/*
3.3.3 getTempoFactor

Returns the current tempo factor

*/
  public String getTempoFactor()
  {
    try
    {
      return Float.toString(sequencer.getTempoFactor());
    }
    catch (NullPointerException e)
    {
      return "1.0";
    }
  }

/*
3.3.3 getTempo

Returns the tempo in BPM

*/
  public float getTempo()
  {
    return (sequencer.getTempoInBPM() * sequencer.getTempoFactor());
  }

/*
2.2.2 changeMute

Changes the mute state of target track

*/

  public void changeMute(int track, boolean newMuteState)
  {
    if (track <= thisMidiFile.getNrTracks())
      sequencer.setTrackMute(track-1, newMuteState);
  }

/*
5.5.5 kickThem

Removes all objects with target ID from the playlist. Expecially needed
to remove all midis loaded from a relation, when the relation is removed.

*/
  public void kickThem (String id)
  {
    if (thisMidiFile == null || thisMidiFile.getID().equals(id))
    {
      unload();
    }
    try
    {
      int midiVectorSize = midiVector.size();
      // by removing elements, the vectors size is decreased
      // to avoid NullPointerExceptions, the vector size must be updated
      for (int i = 0; i < midiVectorSize; i++)
      {
        if (((MidiFile) midiVector.get(i)).getID().equals(id))
        {
          midiVector.removeElementAt(i);
          i--;
          midiVectorSize--;
        }
      }
      list.setListData(midiVector);
    }
    catch (Exception e)
    {
      System.out.println("MidiViewer: incorrect Vectorhandling");
    }
  }


/*
3.3 The Relation Viewer

3.3.3 createAndShowRelationGUI

The ~construcor~ of the relation tab. Adapted from the standard
relation viewer.

*/
  public void createAndShowRelationGUI()
  {
    ComboBox     = new JComboBox();
    ScrollPane   = new JScrollPane();
    dummy        = new JPanel();
    CurrentTable = null;
    Tables       = new Vector();
    relPanel.setLayout(new BorderLayout());
    relPanel.add(ComboBox,BorderLayout.NORTH);
    relPanel.add(ScrollPane,BorderLayout.CENTER);
    ComboBox.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent evt)
      {
        showSelectedObject();
      }
    });
  }

/*
4.4.4 showSelectedObject

Shows the table of the selected relation

*/
  private void showSelectedObject()
  {
    int index = ComboBox.getSelectedIndex();

    if( index >= 0 )
    {
      CurrentTable = (JTable)Tables.get(index);
      ScrollPane.setViewportView(CurrentTable);
    }
    else
    ScrollPane.setViewportView(dummy);
  }

/*
2.2.2 createTableFrom

Creates the table of a relation

*/

  private JTable createTableFrom(ListExpr LE, String secondoName)
  {
    int midiPosition = -1;
    boolean result   = true;
    JTable  NTable   = null;
    Vector  relMidis = new Vector();

    if (LE.listLength()!=2)
      return null;
    else
    {
      ListExpr type = LE.first();
      ListExpr value = LE.second();
      ListExpr maintype = type.first();
      // analyse type
      if (type.listLength()!=2 || !maintype.isAtom() ||
        maintype.atomType()!=ListExpr.SYMBOL_ATOM ||
        !(maintype.symbolValue().equals("rel") |
        maintype.symbolValue().equals("mrel"))) return null;

      ListExpr tupletype = type.second();
      ListExpr TupleFirst=tupletype.first();

      if (tupletype.listLength()!=2 || !TupleFirst.isAtom() ||
           TupleFirst.atomType()!=ListExpr.SYMBOL_ATOM ||
           !(TupleFirst.symbolValue().equals("tuple") |
          TupleFirst.symbolValue().equals("mtuple"))) return null;

      ListExpr TupleTypeValue = tupletype.second();
      String[] head=new String[TupleTypeValue.listLength()];

      for(int i=0;!TupleTypeValue.isEmpty()&&result;i++)
      {
        ListExpr TupleSubType = TupleTypeValue.first();

        if (TupleSubType.listLength()!=2) result = false;
        else
        {
          head[i] = TupleSubType.first().writeListExprToString();
          head[i] += "  "+
          TupleSubType.second().writeListExprToString();

          if (TupleSubType.second().writeListExprToString().endsWith("midi"))
          {
            midiPosition = i;
          }
        }
        TupleTypeValue = TupleTypeValue.rest();
      }

      if (result)
      {
        ListExpr TupleValue;
        Vector V = new Vector();
        String[] row;
        int pos;
        ListExpr Elem;

        while (!value.isEmpty())
        {
          TupleValue = value.first();
          row        = new String[head.length];
          pos        = 0;

          while(pos<head.length & !TupleValue.isEmpty())
          {
            Elem = TupleValue.first();

            if (Elem.isAtom() && Elem.atomType()==ListExpr.STRING_ATOM)
            {
              row[pos] = Elem.stringValue();
            }
            else
              if(Elem.isAtom() && Elem.atomType()==ListExpr.TEXT_ATOM)
              {
                 if(Elem.textLength()<40)
                {
                  row[pos] = Elem.textValue();
                }
                else
                {
                  row[pos] = "a long text";
                }
              }
              else
              {
                row[pos] = TupleValue.first().writeListExprToString();
                System.out.println(row[pos]);

                if (pos == midiPosition)
                {
                  Base64Decoder  b64;
                  StringReader sr = new StringReader(
                    TupleValue.first().first().textValue());
                  b64 = new Base64Decoder(sr);
                  MidiFile midi = new MidiFile(b64.getInputStream(),
                    ( "Midi " + V.size() + " of Rel " +
                    secondoName), secondoName);
                  relMidis.addElement(midi);
                  row[pos] = midi.getName();
                }
              }
            pos++;
            TupleValue = TupleValue.rest();
          }
          V.add(row);
          value = value.rest();
        }

        String[][] TableDatas = new String[V.size()][head.length];
        for(int i=0;i<V.size();i++)
        {
          TableDatas[i]=(String[]) V.get(i);
        }
        midiTables.add(relMidis);
        NTable = new JTable(TableDatas,head);
        NTable.getSelectionModel().addListSelectionListener(this);
      }
    }
    if(result)
      return NTable;
    else
      return null;
  }

/*
3.3.3 valueChanged

Implementation of the ListSelectionListener belonging to the relation
tables.

*/
  public void valueChanged(ListSelectionEvent e)
  {
    if (!e.getValueIsAdjusting())
    {
      int last = CurrentTable.getSelectedRow();
      MidiFile midi = (MidiFile)
        ((Vector)midiTables.get(ComboBox.getSelectedIndex())).get(last);
      unload();
      midiVector.addElement(midi);
      list.setListData(midiVector);
      thisMidiFile = midi;
      load();
      long l = getLengthInTicks();
      length = Long.toString(l);
      play();
      slider.setMaximum((int) getLengthInTicks());
      currentTracks = thisMidiFile.getTrackArray();
      setTrackPanel();
      setInfoPanel();
      jtp.setSelectedIndex(0);
    }
  }

/*
5.5.5 containsMidiObject

Checks wheter a relation contains a Midi or not

*/

  public boolean containsMidiObject(SecondoObject o)
  {
    ListExpr LE = o.toListExpr();
    boolean result = true;
    if (LE.listLength()!=2)
      return false;
    else
    {
      ListExpr type     = LE.first();
      ListExpr value    = LE.second();
      ListExpr maintype = type.first();

      if (type.listLength()!=2 || !maintype.isAtom() ||
        maintype.atomType()!=ListExpr.SYMBOL_ATOM ||
        !(maintype.symbolValue().equals("rel") |
        maintype.symbolValue().equals("mrel"))) return false;

      ListExpr tupletype = type.second();
      ListExpr TupleFirst=tupletype.first();

      if (tupletype.listLength()!=2 || !TupleFirst.isAtom() ||
           TupleFirst.atomType()!=ListExpr.SYMBOL_ATOM ||
           !(TupleFirst.symbolValue().equals("tuple") |
          TupleFirst.symbolValue().equals("mtuple"))) return false;

      ListExpr TupleTypeValue = tupletype.second();
      String[] head=new String[TupleTypeValue.listLength()];

      for(int i=0;!TupleTypeValue.isEmpty()&&result;i++)
      {
        ListExpr TupleSubType = TupleTypeValue.first();

        if (TupleSubType.listLength()!=2) result = false;
        else
        {
          head[i] = TupleSubType.first().writeListExprToString();
          head[i] += "  "+
          TupleSubType.second().writeListExprToString();

          if (TupleSubType.second().writeListExprToString().endsWith("midi"))
          {
            return true;
          }
        }
        TupleTypeValue = TupleTypeValue.rest();
      }
    return false;
  }
  }

/*
2.2.2 getIndexOf

Returns the ComboBox index of selected relation

*/
  private int getIndexOf(String S)
  {
    int count =  ComboBox.getItemCount();
    int pos   = -1;

    for( int i = 0; i < count; i++)
    {
      if( ((String)ComboBox.getItemAt(i)).equals(S))
      {
        pos = i;
      }
    }
    return pos;
  }
}
