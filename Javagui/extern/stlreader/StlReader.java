
package extern.stlreader;

import extern.SecondoImporter;
import sj.lang.ListExpr;
import java.io.*;
import tools.FileTokenizer;
import tools.RefBool;


public class StlReader implements SecondoImporter{


  public ListExpr getList(String fileName){
     File f = new File(fileName);
     errorMsg = "NO_ERROR";
     if(!f.exists()){
        errorMsg = "File " + fileName + " does not exists";
        return null;
     }
     if(!f.isFile()){
        errorMsg = "File " + fileName + " is not a regular file";
        return null;
     } try{
        return getTriangles(f);
     } catch(IOException e){
        errorMsg = "Problem in reading file " + fileName;
        return null;
     }
  }


  public String getErrorString(){
     return errorMsg;
  }

  private String errorMsg = "NO_ERROR";


  /** checks for binary of ascii stl-file.
   **/
  private boolean isBinary(File f) throws IOException{
    FileTokenizer ft = new FileTokenizer(f,null);
    String s = ft.nextToken(84);
    ft.close();
    if("solid".equals(s)){
        return false;
    } else {
       return true;
    }
 }


  /** Reads a single triangle from a binary stl file
    */
 ListExpr  getTriangleList(DataInputStream in) throws IOException{
    float i = Float.intBitsToFloat(little2big(in.readInt()));
    float j = Float.intBitsToFloat(little2big(in.readInt()));
    float k = Float.intBitsToFloat(little2big(in.readInt()));
    float x1 = Float.intBitsToFloat(little2big(in.readInt()));
    float y1 = Float.intBitsToFloat(little2big(in.readInt()));
    float z1 = Float.intBitsToFloat(little2big(in.readInt()));
    float x2 = Float.intBitsToFloat(little2big(in.readInt()));
    float y2 = Float.intBitsToFloat(little2big(in.readInt()));
    float z2 = Float.intBitsToFloat(little2big(in.readInt()));
    float x3 = Float.intBitsToFloat(little2big(in.readInt()));
    float y3 = Float.intBitsToFloat(little2big(in.readInt()));
    float z3 = Float.intBitsToFloat(little2big(in.readInt()));
    short attrByteCount = little2big(in.readShort());

    ListExpr p1 = ListExpr.threeElemList( ListExpr.realAtom(x1),
                                          ListExpr.realAtom(y1),
                                          ListExpr.realAtom(z1));
    ListExpr p2 = ListExpr.threeElemList( ListExpr.realAtom(x2),
                                          ListExpr.realAtom(y2),
                                          ListExpr.realAtom(z2));
    ListExpr p3 = ListExpr.threeElemList( ListExpr.realAtom(x3),
                                          ListExpr.realAtom(y3),
                                          ListExpr.realAtom(z3));
    return ListExpr.threeElemList(p1,p2,p3);
 }

   static int little2big(int i) {
      return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
   }

   static short little2big(short i) {
      return (short)(((i&(short)0xff00)<<8)|((i&(short)0xff0000)>>8));
   }

   private static void readHeader(DataInputStream in) throws IOException{
     byte[] b = new byte[80];
     int bytes  = in.read(b);
   }



 private ListExpr getTrianglesBinary(File file) throws IOException{
    DataInputStream in = new DataInputStream(new BufferedInputStream(new FileInputStream(file)));
    readHeader(in);
    int facets1 = in.readInt();
    facets1 = little2big(facets1);
    long facets;
    if(facets1>=0){
       facets = facets1;
    } else {
       facets = facets1 & 0xFFFFFFFFL;
    }
    ListExpr list = null;
    ListExpr last = null;
    for(long i=0;i<facets;i++){
       ListExpr t = getTriangleList(in);
       if(t==null){
           in.close();
           return null;
       }
       if(list==null){
          list = ListExpr.oneElemList(t);
          last = list;
       } else {
          last = ListExpr.append(last,t);
       }
    }
    in.close();
    if(list==null){
       errorMsg = "found no triangles in binary stl file";
       return null;
    }
    return list;
 }



 private ListExpr readPoint(FileTokenizer ft){
   if(!"vertex".equals(ft.nextToken(0))) return null;
   try{
     double x = Double.parseDouble(ft.nextToken(0));
     double y = Double.parseDouble(ft.nextToken(0));
     double z = Double.parseDouble(ft.nextToken(0));
     return ListExpr.threeElemList( ListExpr.realAtom(x),
                                    ListExpr.realAtom(y),
                                    ListExpr.realAtom(z));                 
   } catch(NumberFormatException e){
     errorMsg = "One coordinate does not represent a real";
     return null;
  }
}

 private ListExpr  getTriangleListASCII(FileTokenizer ft, RefBool end){
    String t = ft.nextToken(0);
    if("endsolid".equals(t)){
       end.value = true;
       return null;
    }
    if(!"facet".equals(t)){
        errorMsg = "facet missing";
        return null;
    }
    t = ft.nextToken(0);
    if(!"normal".equals(t)){
        errorMsg = "normal missing";
        return null;
    }
    // jump over normal vector
    for(int i=0;i<3;i++){
        t = ft.nextToken(0);
        try{
							Double.parseDouble(t);
        } catch(NumberFormatException e){
           errorMsg = "found non-real value in normal vector";
           return null;
        }
    }
    if(!"outer".equals(ft.nextToken(0))){ errorMsg = "outer missing";return null; }
    if(!"loop".equals(ft.nextToken(0))){ errorMsg = "loop missing";return null; }
    ListExpr p1 = readPoint(ft);
    if(p1==null) return null; 
    ListExpr p2 = readPoint(ft);
    if(p2==null) return null; 
    ListExpr p3 = readPoint(ft);
    if(p3==null) return null; 
    if(!"endloop".equals(ft.nextToken(0))) {errorMsg = "missing endloop"; return null;}
    if(!"endfacet".equals(ft.nextToken(0))) { errorMsg = "missing endfacet"; return null;}
    return ListExpr.threeElemList(p1,p2,p3);
 }

 private ListExpr getTrianglesASCII(File file) throws IOException{
    FileTokenizer ft = new FileTokenizer(file,null);
    String t = ft.nextToken(0);
    if(!"solid".equals(t)) {errorMsg = "missing solid"; return null;}
    t = ft.nextToken(0);
    if(t==null) {errorMsg = "file incomplete";return null;}


    boolean ok = true;
    RefBool end = new RefBool();
    end.value = false;
    ListExpr list = null;
    ListExpr last = null;
    while(!end.value){
      ListExpr tr= getTriangleListASCII(ft,end);
      if(!end.value){
          if(tr==null){
             ft.close();
             return null;
          }
          if(list==null){
            list= ListExpr.oneElemList(tr);
            last = list;
          } else {
            last = ListExpr.append(last,tr);
          }
      }
    } 
    ft.close();
    if(list==null){
      errorMsg= "no triangles found";
      return null;
    }
    return list;
 }


 private ListExpr  getTriangles(File file) throws IOException{
    ListExpr res = null;
    if(isBinary(file)){
       res =  getTrianglesBinary(file);
    }  else {
       res = getTrianglesASCII(file);
    }
    if(res==null) return null;
    return ListExpr.twoElemList(ListExpr.symbolAtom("volume3d"), res);

 }


}
