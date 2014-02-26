//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import java.awt.*;
import  java.awt.geom.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.EditorKit;
import javax.swing.text.Document;
import tools.Base64Decoder;
import org.jpedal.*;
import tools.Reporter;
import java.io.*;
import java.util.*;
import gui.SecondoObject;

/**
 * The display class for sets. It extends the general DisplayCollection to
 * be able to use all the funtions of the given subtype
**/

public class Dsplset extends DisplayCollection implements ExternDisplay{

/** The stores defined status */
private boolean defined;

/** The extrenal Frame to display textual subtypes */
private SetFrame Display=null; 

/** The string represantation for the QueryResult */
private String Entry;

/** The Value ListExpr of the given secondo Object */
protected ListExpr tempVal;

/** The Type ListExpr of the given secondo Object */
protected ListExpr tempType;

/** The Constructor of the Dsplset class
 *  It only creates a new VectorFrame 
 * @see <a href="Dsplsetsrc.html#Dsplset">Source</a>
 */
public  Dsplset(){
   if(Display==null){
      Display = new SetFrame();
   }
}

/** A method to return the textual represantation of this Object
 * @return the textual Represantation as a string
 * @see <a href="Dsplsetsrc.html#DspltoString">Source</a>
 */
public String toString(){
   return Entry;
}


/** The init Method of the DsplGeneric Interface
 *  
 * @see <a href="Dsplsetsrc.html#init">Source</a>
 */
public void init (String name, int namewidth, int indent, ListExpr type, ListExpr value, QueryResult qr) 
 {
  String n = extendString(name,namewidth, indent);
  if(type.second().isAtom())
    Entry = n + " : Set of type "+type.second().symbolValue();
  else
    Entry = n + " : Set of type "+type.second().toString();
  qr.addEntry(this);
  tempVal = value;
  tempType = type;
  subtyp=getTypeName(type).symbolValue();
  if(isGraph(subtyp))initSet(value,type.second());
 System.out.println("Groesse des graphVectors: " + graphVector.size());
 }

/** The displayExtern Method of the ExternDisplay Interface
 *  it initializes the SetFrame
 * @see <a href="Dsplsetsrc.html#displayExtern">Source</a>
 */
public void displayExtern(){
    Display.setSource(this); 
    Display.setVisible(true);	
}

/** A method of the Extern Display Interface
 *  Checks wether the VectorFrame is displayed or not
 * @return the boolean result 
 * @see <a href="Dsplsetsrc.html#isDisplayed">Source</a>
 */
public boolean isExternDisplayed(){
   return Display.isVisible();
}

/** A method of the DsplGraph Interface
 *  Checks for the amount of shapes to be drawn
 * @return the number of shapes
 * @see <a href="Dsplsetsrc.html#numberOfShapes">Source</a>
 */
public int numberOfShapes(){
  int sum=0;
  int temp;
  if(isGraph(subtyp)){
    for(int i=0;i<graphVector.size();i++){
	temp=((DsplGraph)graphVector.get(i)).numberOfShapes();
	shapeVector.addElement(new shapeSet(sum,temp));
	sum+=temp;
    }
  }
  System.out.println("Anzahl der Shapes: "+sum);
  return sum;
}

/** A method to initialize the VectorFrame and its components
 * 
 * @see <a href="Dsplsetsrc.html#initVector">Source</a>
 */
private void initSet(ListExpr value, ListExpr type){
 try{
   while(!value.isEmpty()){
   Class t;
    if(type.isAtom()){
     t = Class.forName("viewer.hoese.algebras."+"Dspl"+type.symbolValue());
    }else{
     t = Class.forName("viewer.hoese.algebras."+"Dspl"+type.first().symbolValue());
    }
    DsplGraph o = (DsplGraph)t.newInstance();
   graphVector.addElement(o);
   o.init(type.toString(),0,0,type,value.first(),new QueryResult(new SecondoObject("nix",value.first()),true));
   System.out.println("Der aktuelle List Wert ist "+value.first());
   value=value.rest();
   }
   }catch(ClassNotFoundException e){
   Reporter.showError(" Keine Displayklasse gefunden");
   }catch(InstantiationException e){
   Reporter.showError("Displayklasse konnte nicht initialisiert werden");
  }catch(IllegalAccessException e){
   Reporter.showError("Fehler beim Zugriff auf die Displayklasse");
  }
}

/** A method of the DsplGraph Interface to get a desired shape 
 * @param num the number of the shape
 * @param at
 * @return the desired shape
 * @see <a href="Dsplsetsrc.html#getRenderObject">Source</a>
 */
  public Shape getRenderObject(int num, AffineTransform at){
  Shape value=null;
  if(isGraph(subtyp)){
  int objectNo=findShape(num);
  shapeSet shS=(shapeSet)shapeVector.get(objectNo);
  value=((DsplGraph)graphVector.get(objectNo)).getRenderObject(shS.getPosition(num),at);//richtige shapenummer fehlt
  System.out.println("rendering shape: "+num);
   }
return value;  
}

/** A class which extend JFrame to create an extra Frame to display the set's content
 * @see <a href="Dsplsetsrc.html#SetFrame">Source</a>
 */
private class SetFrame extends JFrame{

/** a reference to the Dsplset Object */
private Dsplset Source;

/** a QueryResult to display the subtype */
private QueryResult qrNEW;

/** a ScrollPane to store the QueryResult */
private JScrollPane textDspl = new JScrollPane();

/** Creates a new Frame displaying the Set .
*/
public SetFrame(){
  getContentPane().setLayout(new BorderLayout());
  setSize(640,480); 

}

/** Sets the Dsplset from which the set data comes. **/
public void setSource(Dsplset S){
   qrNEW= new QueryResult(new SecondoObject("Test",tempVal),true);
   getContentPane().add(textDspl,BorderLayout.CENTER);
   textDspl.setViewportView(qrNEW);
   Source = S;
   initSubtyp();
  
}

/** Initializes the QueryResult **/
private void initSubtyp(){
ListExpr valueCopy = Source.tempVal;
if(valueCopy.isEmpty())qrNEW.addEntry("NO ELEMENTS");
try{
  if(Source.tempType.second().isAtom()){
  Class t = Class.forName("viewer.hoese.algebras."+"Dspl"+Source.tempType.second().symbolValue());
  while(!valueCopy.isEmpty()){
    DsplGeneric testklasse = (DsplGeneric)t.newInstance();
    testklasse.init(Source.tempType.second().toString(),0,0,Source.tempType.second(),valueCopy.first(),qrNEW);
    valueCopy = valueCopy.rest();
  } 
}else{
  Class t = Class.forName("viewer.hoese.algebras."+"Dspl"+Source.tempType.second().first().symbolValue());
  while(!valueCopy.isEmpty()){
    DsplGeneric testklasse = (DsplGeneric)t.newInstance();
    testklasse.init(Source.tempType.second().toString(),0,0,Source.tempType.second(),valueCopy.first(),qrNEW);
    valueCopy = valueCopy.rest();
  } 
}
  }catch(ClassNotFoundException e){
   Reporter.showError(" Keine Displayklasse gefunden");
  }catch(InstantiationException e){
   Reporter.showError("Displayklasse konnte nicht initialisiert werden");
  }catch(IllegalAccessException e){
   Reporter.showError("Fehler beim Zugriff auf die Displayklasse");
  }

}

/** Returns the source of the set data. */
public Dsplset getSource(){
     return Source;
}

} 


}



