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

1 Implementation of MidiFilter

This class provides a filter for .mid and .midi files and is used by the
JFileChooser of MidiFiles export method.

2 Imports

*/

package viewer.midi;


import java.io.File;
import javax.swing.*;
import javax.swing.filechooser.*;


/*
2 Class ~MidiFilter~

Adapted from ~ImageFilter.java is a 1.4 example used by FileChooserDemo2.java.~

*/
public class MidiFilter extends FileFilter
{
/*
2.2 inherited methods

2.2.1 accept

Accepts all directories and all .mid and .midi files

*/
  public boolean accept(File file)
  {
    if (file.isDirectory())
    {
      return true;
    }
    String extension = getExtension(file);
    if (extension != null)
    {
      if (extension.equals("mid") || extension.equals("midi"))
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    return false;
  }

/*
2.2.2 getDescription

Sets the text shown in the JFileChooser

*/
    public String getDescription()
    {
        return "Just Midis";
    }

/*
2.3 additional method

2.3.1 getExtension

Not inherited, but adapted from the util class of the sun java demo. it returns the suffix of a file name

*/

  public static String getExtension(File file)
	{
    String ext = null;
    String fileName = file.getName();
    int i = fileName.lastIndexOf('.');
    if (i > 0 &&  i < fileName.length() - 1)
    {
      ext = fileName.substring(i+1).toLowerCase();
    }
    return ext;
  }
}
