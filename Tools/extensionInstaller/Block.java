
/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen,
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

/** The class Block describes a modification of an optimizer file. **/

class Block{
   String file=null;     // file to modify
   String section=null;  // section for inserting
   boolean first=false;  // inserting at the begibn or the end of the section
   String content=null;  // content to insert

   /** Rteurns the tag marking the start of the section **/
   public String getSectionStart(){
      return"% Section:Start:"+section;
   }
   /** returns the regular expression coresponding to the start of the section **/
   public String getSectionStartTemplate(){
      return "%\\s*Section:Start:\\s*"+section+"\\s*";
   }

   /** returns the tag describing the end of the section **/
   public String getSectionEnd(){
      return"% Section:End:"+section;
   }
   
   /** returns a regular expression marking the end of the section **/
   public String getSectionEndTemplate(){
      return "%\\s*Section:End:\\s*"+section+"\\s*";
   }

}

