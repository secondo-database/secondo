

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import java.io.*;


/**
 * A displayclass for the string-type, alphanumeric only
 */
public class Dsplbinfile extends DsplGeneric implements DsplSimple{

  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is
   * neccessary for the displaying this type in the queryresultlist.
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
     int size = 0;
     byte[] dummy = new byte[128];
     int r;
     String V="";
     try{
        BufferedInputStream in = new BufferedInputStream(value.decodeText());
        while( (r=in.read(dummy))>=0)
            size += r;
        V = type.symbolValue() + ":" + size+" bytes";
	in.close();
    } catch(Exception E){
      E.printStackTrace();
      V = "error";
    }
     qr.addEntry(V);
     return;
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     int size = 0;
     byte[] dummy = new byte[128];
     int r;
     String V="";
     try{
        BufferedInputStream in = new BufferedInputStream(value.decodeText());
        while( (r=in.read(dummy))>=0)
            size += r;
	in.close();
	V = ""+size+" bytes";
     }catch(Exception e){
        e.printStackTrace();
        V = "error";
     }

     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     qr.addEntry(T + " : " + V);
     return;

  }

  private String extendString(String S ,int MinWidth){
   String R = new String(S);
   int NoSpaces=MinWidth-R.length();
   for(int i=0;i<NoSpaces;i++)
      R = ' '+R;
   return R;
  }


}



