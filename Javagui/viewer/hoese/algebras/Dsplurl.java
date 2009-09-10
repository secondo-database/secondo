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


/**
 * A displayclass for the string-type, alphanumeric only
 */
public class Dsplurl extends DsplGeneric implements LabelAttribute{

   /** a class representing an Url **/
   public static class Url implements Comparable{
     private String protocol;
     private String host;
     private String file;
     private boolean defined;

     /** creates a new undefined url **/
     public Url(){
         protocol ="";
         host="";
         file=""; 
         defined = false;
     }

     /** creates an url from the given values **/
     public Url(String protocol, String host, String file){
        this.protocol = protocol;
        this.host = host;
        this.file = file;
        defined = true;
     }

     /** reads the content of this url from the given list.
       * if the list does not represent a valid list expression,
       * this instance is not changed and the result is false.
       **/
     public boolean readFrom(ListExpr value){
        if(value.listLength()==2 && value.first().atomType()==ListExpr.SYMBOL_ATOM &&
           value.first().symbolValue().equals("url")){
           value = value.second();
        }


        if(DsplGeneric.isUndefined(value)){
           defined = false;
           return true;
        }
        if(value.listLength()!=3 || 
          value.first().atomType()!=ListExpr.STRING_ATOM ||
          value.second().atomType()!=ListExpr.TEXT_ATOM || 
          value.third().atomType()!=ListExpr.TEXT_ATOM){
          System.out.println("invalid url structure detected " );
          System.out.println("list is " + value );
          return false; 
        } else {
           protocol =  value.first().stringValue();
           host = value.second().textValue();
           file = value.third().textValue();
           defined = true;
           return true;
        }
     }

     /** reads this url from text. If text does not represent a
       * valid full qualified url, this url is not changed.
       **/
     public boolean readFrom(String text){
        try{
          java.net.URL tmp = new java.net.URL(text);
          protocol = tmp.getProtocol();
          host = tmp.getHost();
          file = tmp.getPath();
          defined = true;
          return true;
        } catch(Exception e){
            tools.Reporter.debug(e);
            return false;
        }

     }

     public boolean equals(Object o){
        return this.toString().equals(o.toString());
     }

     public int compareTo(Object o){
         return this.toString().compareTo(o.toString());
     }

     public boolean readFrom(String text, Url base){
         try{
            java.net.URL tmp = base.getURL();
            java.net.URL tmp2 = new java.net.URL(tmp,text);
            protocol = tmp2.getProtocol();
            host = tmp2.getHost();
            file = tmp2.getPath();
            defined = true;
            return true;
         }catch(Exception e){
            return false;
         }   

     }


     private java.net.URL getURL(){
       try {
         return new java.net.URL(""+this);
       } catch(java.net.MalformedURLException e){
           return null;
       }
     }

     public String toString(){
       if(!defined){
           return "undefined";
       }
       return protocol+"://"+host+file;
     }

     


   }

  
   Url url;
   String label;


  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value,QueryResult qr)
  {
     url = new Url();
     String V;
     String T = name;
     T=extendString(T,nameWidth, indent);
     if(url.readFrom(value)){
         V=url.toString();
         label=V;
     } else {
         V = "Error";
         label ="";
     }
     qr.addEntry(T + " : " + V);
     

     return;

  }

  public String getLabel(double time){
     return label;
  }


}



