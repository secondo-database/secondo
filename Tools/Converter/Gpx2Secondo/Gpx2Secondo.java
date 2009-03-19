import java.io.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.util.Vector;

class TrackPoint{
  public TrackPoint(String time, double lat, double lon, double ele){
     this.time = time;
     this.lat = lat;
     this.lon = lon;
     this.ele = ele;
  }

 String getTime(){ return time; }
 double getLon(){ return lon; }
 double getLat(){ return lat; }

 private String time;
 private double lon;
 private double lat;
 private double ele;

}

public class Gpx2Secondo {

static String getTime(Node n){
   if(n.hasChildNodes()){
     String T = n.getFirstChild().getNodeValue(); 
     T = T.replaceAll("T","-");
     T = T.replaceAll("Z","");
     return T;
   } else {
      return null;
   }
}

static Double getEle(Node n){
   return null; // ignore information
}


static TrackPoint getTP(Node n){
  NamedNodeMap m = n.getAttributes();
  Double lon = null;
  Double lat = null;
  for(int i=0;i<m.getLength();i++){
     Node a = m.item(i);
     if(a.getNodeName().equals("lat")){
        lat=new Double(Double.parseDouble(a.getNodeValue()));
     }
     if(a.getNodeName().equals("lon")){
        lon=new Double(Double.parseDouble(a.getNodeValue()));
     }
  }
  if(lat==null || lon==null){
    System.err.println("missing position");
    return null;
  }
  NodeList nl = n.getChildNodes();
  String time = null;
  Double ele = null;
  for(int i=0;i<nl.getLength();i++){
     Node c = nl.item(i);
     if(c.getNodeName().equals("time")){
       time = getTime(c);
     }
     if(c.getNodeName().equals("ele")){
        ele = getEle(c);
     }
  } 
  if(time==null){
    System.err.println("missing time");
    return null;
  }
  if(ele==null){
     ele = new Double(0.0);
  }
  return new TrackPoint(time,lat.doubleValue(), lon.doubleValue(), ele.doubleValue());
}


static void WriteTrack(Vector<TrackPoint> track){

  if(track.size()<2){
    System.err.println("too less points in the track");
    return;
  }
  for(int i=1;i<track.size();i++){
    TrackPoint tp1 = track.get(i-1);
    TrackPoint tp2 = track.get(i);
    if(!tp1.getTime().equals(tp2.getTime())){
      System.out.print("( (");   // open unit , open interval
      System.out.print("\""+tp1.getTime() + "\" ");
      System.out.print("\"" + tp2.getTime() + "\" TRUE FALSE )"); // close interval
      System.out.print("( " + tp1.getLon() + " " + tp1.getLat() + 
                           " " + tp2.getLon() + " " + tp2.getLat() +" )" );
  
      System.out.println(") "); // close unit
    }
  } 

}

static void processSegment(Node n){
  NodeList nl = n.getChildNodes();
  Vector<TrackPoint> track = new Vector<TrackPoint>(nl.getLength());
  for(int i=0;i<nl.getLength();i++){
     Node c = nl.item(i);
     if(c.getNodeName().equals("trkpt")){
        TrackPoint tp = getTP(c);
        if(tp!=null){
          track.add(tp);
        }
     }
  } 
  WriteTrack(track);
}
	

static void processTrack(Node n, String fileName){
  System.out.println("("); // open tuple
  NodeList nl = n.getChildNodes();
  // first search for name
  String name = "";
  for(int i=0;i<nl.getLength();i++){
    if(nl.item(i).getNodeName().equals("name")){
      name = nl.item(i).getFirstChild().getNodeValue(); 
    }
  }
  System.out.println("'"+fileName+"' ");
  System.out.println("'"+ name + "'");
  System.out.println("("); // open mpoint
  for(int i=0;i<nl.getLength();i++){
      if(nl.item(i).getNodeName().equals("trkseg")){
        processSegment(nl.item(i));
      }
    }
  System.out.println(")"); // close mpoint

  System.out.println(")"); // close tuple 
}


static void processDoc( Document docu, String fileName)
{
  Element root = docu.getDocumentElement();
  if(root==null){
     System.err.println("Error in document");
     return;
  }
  if(!root.getTagName().equals("gpx")){
     System.err.println("not an gpx file");
     return;
  }
  NodeList nl = root.getChildNodes();
  for(int i=0; i< nl.getLength();i++){
    Node n = nl.item(i);
    if(n.getNodeName().equals("trk")){
      System.err.println("track found");
      processTrack(n, fileName); 
    } 
  }


}

static void processFile(String fileName){
	try {
     File f = new File(fileName);
   	 Document d = DocumentBuilderFactory.newInstance().
                  newDocumentBuilder().parse(new File(fileName ));
   	processDoc(d, fileName);
 	}	catch( Exception e ) {
    e.printStackTrace();
 	} 

}


public static void main( String [] args )
{
  System.out.println("(OBJECT traces () (rel(tuple((FileName text) (Name text)(Trip mpoint)))) \n (");
	if( args.length >0 ){
     for(int i=0; i< args.length;i++){
       	processFile(args[i]);
     }
  } else {
    BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
    try{
       while(in.ready()){
          String fn = in.readLine();
         processFile(fn); 
       }  
    } catch(Exception e){
       e.printStackTrace();
    }
  }
  System.out.println("))");
}


}
