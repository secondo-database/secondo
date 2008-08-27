/*
 * mLineRep.java
 *
 * Created on 29. August 2007, 23:46
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;

import java.util.*;
import java.io.*;
/**
 *
 * @author java
 */
public class mLineRep
{
    Match myMatch;
    Vector triangles;
    int secTime=1;
    
    
    /** Creates a new instance of mLineRep */
    public mLineRep(Match myMatch)
    {
        this.secTime=1;
        this.myMatch=myMatch;
        triangles=new Vector();
        for(int i=0;i<myMatch.getSource().getNrOfFaces();i++)
        {
            Face tmpFace=myMatch.getSource().getFace(i);
            RegionTreeNode[] matched=(this.myMatch.getMatches(tmpFace));
            if(matched==null||matched.length==0||matched[0]==null)
            {
                addTrianglesFromFaceNull(tmpFace,0,i);
            }
            else
            {
                if(matched.length==1)
                {
                    addTrianglesFromFaceFace(tmpFace,(Face)matched[0],0,this.secTime,i);
                }
                else
                {
                    if(TriRepUtil.debuggingWarnings)
                        System.out.println("Problem, mehrere Matches");
                }
            }
        }
        
        for(int i=0;i<myMatch.getTarget().getNrOfFaces();i++)
        {
            Face tmpFace=myMatch.getTarget().getFace(i);
            RegionTreeNode[] matched=(this.myMatch.getMatches(tmpFace));
            if(matched==null||matched.length==0||matched[0]==null)
            {
                addTrianglesFromFaceNull(tmpFace,this.secTime,myMatch.getSource().getNrOfFaces()+i);
            }
        }
    }
    
    public triangle getTriangle(int index)
    {
        return((triangle)triangles.elementAt(index));
    }
    
    private int findIndex(PointWNL p1,PointWNL p2,int faceNr ,int cycleNr)
    {
        int res=-1;
        for (int i=0;i<triangles.size();i++)
        {
            if(this.getTriangle(i).p1.equals(p1) &&
                    this.getTriangle(i).p2.equals(p2) &&
                    this.getTriangle(i).faceNr==faceNr &&
                    this.getTriangle(i).cycleNr==cycleNr)
                res=i;
            if(this.getTriangle(i).p1.equals(p2) &&
                    this.getTriangle(i).p2.equals(p1) &&
                    this.getTriangle(i).faceNr==faceNr &&
                    this.getTriangle(i).cycleNr==cycleNr)                    
                res=i;
        }
        return(res);
    }
    private void removeTriangle(PointWNL p1,PointWNL p2,int faceNr,int cycleNr)
    {
        int find=this.findIndex(p1,p2,faceNr,cycleNr);
        while(find!=-1)
        {
            triangles.remove(find);
            find=this.findIndex(p1,p2,faceNr,cycleNr);
        }
    }

    private void removeTrapezoid(PointWNL p1, PointWNL p2, PointWNL p3,
            PointWNL p4, int faceNr, int cycleNr) 
    {
        int find = findIndex(p1, p2, faceNr, cycleNr);
        while (find != -1) 
        {
            triangles.remove(find);
            find = findIndex(p1, p2, faceNr, cycleNr);
        }
        find = findIndex(p3, p4, faceNr, cycleNr);
        while (find != -1) 
        {
            triangles.remove(find);
            find = findIndex(p3, p4, faceNr, cycleNr);
        }
    }
    
    private PointWNL getCorrespondingPoint(PointWNL p1,PointWNL p2,int faceNr,int cycleNr)
    {
        PointWNL res=null;
        int find=this.findIndex(p1,p2,faceNr,cycleNr);
        if(find!=-1)
        {
            res=this.getTriangle(find).p3;
        }
        return(res);
    }
    
    private void addTriangle(PointWNL p1,PointWNL p2, PointWNL p3,int faceNr, int cycleNr)
    {
        if(p1.t!=p2.t)
            if(TriRepUtil.debuggingWarnings)
                System.out.println("Fehler erste beide Punkte müssen dasselbe t haben.");
        if(p1.t==p3.t)
            if(TriRepUtil.debuggingWarnings)
                System.out.println("Fehler dritter Punkt muss ein anderes t haben.");
        PointWNL corr=this.getCorrespondingPoint(p1,p2,faceNr,cycleNr);
        if(corr==null)
        {
            triangles.add(new triangle(p1,p2,p3,faceNr,cycleNr));
        }
        else
        {
            if(corr.equals(p3))
            {
                removeTriangle(p1,p2,faceNr,cycleNr);
            }
            else
            {
                if(TriRepUtil.debuggingWarnings)
                    System.out.println("Dritte Punkte stimmen nicht");
                triangles.add(new triangle(p1,p2,p3,faceNr,cycleNr));
            }
        }
    }
    
    
    public int findMatchingIndex(LineWA[] s,int j ,double angle, boolean ka)
    {
        if(TriRepUtil.debugging)
            System.out.println(Math.toDegrees(angle)+" "+j);        
        if(Math.abs(s[0].angle-angle)<0.0001)
        {
            if(ka)
            {
                return(0);                
            }
            else
            {
                return(1);
            }
        }            
        if(angle<s[0].angle)
        {
            if(TriRepUtil.debugging)
                System.out.println(s[0].angle+" > "+angle+" Abbruch");
            return(0);
        }
        if(Math.abs(s[s.length-1].angle-angle)<0.0001)
        {
            if(ka)
            {
                return(0);                
            }
            else
            {
                return(s.length-1);
            }
        }        
        if(angle>s[s.length-1].angle)
        {
            if(TriRepUtil.debugging)
                System.out.println(s[0].angle+" < "+angle+" Abbruch");
            return(0);
        }
           
        while(  !(Math.abs(s[j].angle-angle)<0.0001 ||
                s[j].angle>angle) ||
                !(Math.abs(s[(j-1+s.length)%s.length].angle-angle)<0.0001 ||
                s[(j-1+s.length)%s.length].angle<angle))
        {
            if(j!=0 && angle<s[j-1].angle)
            {
                if(TriRepUtil.debugging)
                    System.out.println("Down");
                j--;
            }
            else
            {
                if (TriRepUtil.debugging) {
                    System.out.println("Up");
                }
                j++;

            }
        }
        if(TriRepUtil.debugging)
            System.out.println(Math.toDegrees(s[j].angle)+" "+j);
        if(Math.abs(s[j].angle-angle)<0.0001&&ka)
        {
            if(TriRepUtil.debugging)
                System.out.println("GLeicheWinkel "+j+ ka);
            j=(j + 1) % s.length;            
        }
        return(j);
    }
    
    public void rotaring_pane(ConvexHullTreeNode chtn1,ConvexHullTreeNode chtn2, int time1, int time2,int faceNr,int cycleNr)
    {
        LineWA[] s1=chtn1.getOrderedOutLine();
        LineWA[] s2=chtn2.getOrderedOutLine();
        if(TriRepUtil.debugging)
        {
            System.out.println("RP bekam:");
            TriRepUtil.printLineList(s1);
            System.out.println("und");
            TriRepUtil.printLineList(s2);
        }
        TriRepUtil.computeLineAngles(s1);
        TriRepUtil.computeLineAngles(s2);
        Arrays.sort(s1);
        Arrays.sort(s2);
        if(TriRepUtil.debugging)
        {
            System.out.println("nach dem Sortieren:");
            TriRepUtil.printLineList(s1);
            System.out.println("und");
            TriRepUtil.printLineList(s2);
        }
        int jj=0;
        for(int i=0;i< s1.length;i++)
        {
            if(TriRepUtil.debugging)
            {
                System.out.println("zu       "+new PointWNL(s1[i].x,s1[i].y,time1));
                System.out.println("Line von "+new PointWNL(s1[(i+1+s1.length)%s1.length].x,s1[(i+1+s1.length)%s1.length].y,time1));
            }
            jj=findMatchingIndex(s2,jj,s1[i].angle,true);
            if(TriRepUtil.debugging)
                System.out.println("Match    "+new PointWNL(s2[jj].x,s2[jj].y,time2));
            this.addTriangle(new PointWNL(s1[i].x,s1[i].y,time1),
                    new PointWNL(s1[(i+1+s1.length)%s1.length].x,
                    s1[(i+1+s1.length)%s1.length].y,time1),
                    new PointWNL(s2[jj].x,s2[jj].y,time2),
                    faceNr,cycleNr);           
        }
        jj=0;//s1.length-1;
        if(TriRepUtil.debugging)
            System.out.println("Und Weiter");
        for(int i=0;i<s2.length;i++)
        {
            if(TriRepUtil.debugging)
            {
                System.out.println("zu       "+new PointWNL(s2[i].x,s2[i].y,time2));
                System.out.println("Line von "+new PointWNL(s2[(i+1+s2.length)%s2.length].x,s2[(i+1+s2.length)%s2.length].y,time2));
            }
            jj=findMatchingIndex(s1,jj,s2[i].angle,false);
            if(TriRepUtil.debugging)
                System.out.println("Match    "+new PointWNL(s1[jj].x,s1[jj].y,time1));
            this.addTriangle(new PointWNL(s2[i].x,s2[i].y,time2),
                    new PointWNL(s2[(i+1+s2.length)%s2.length].x,
                    s2[(i+1+s2.length)%s2.length].y,time2),
                    new PointWNL(s1[jj].x,s1[jj].y,time1),
                    faceNr,cycleNr);
        }
    }
    
    public void addTrianglesFromCHTPoint(ConvexHullTreeNode chtn,int time,int x, int y,int t,int faceNr,int cycleNr)
    {
        addTrianglesFromCHTPoint(chtn,time,new PointWNL(x,y,t),faceNr,cycleNr);
    }
    
    private void addTrianglesFromCHTPoint(ConvexHullTreeNode chtn,int time,PointWNL p3,int faceNr,int cycleNr)
    {
        if(TriRepUtil.debugging)
            System.out.println("CHTPOint "+p3);
        LineWA[] tmp=chtn.getOrderedOutLine();
        PointWNL p1;
        PointWNL p2;
        for(int i=0;i< tmp.length;i++)
        {
            p1=new PointWNL(tmp[i].x,tmp[i].y,time);
            p2=new PointWNL(tmp[(i+1)%tmp.length].x,tmp[(i+1)%tmp.length].y,time);
            this.addTriangle(p1,p2,p3,faceNr,cycleNr);
        }
        ConvexHullTreeNode[] children=chtn.getChildren();
        for(int i=0;i<children.length;i++)
        {
            this.addTrianglesFromCHTPoint(children[i],time,p3,faceNr,cycleNr);
        }
    }
    
    public void addTrianglesFromCHTNull(ConvexHullTreeNode cht,int time,int faceNr, int cycleNr)
    {
        if(TriRepUtil.debugging)
            System.out.println("CHTNull "+cht+" "+time);
        LineWA[] edgesLine=cht.getOrderedOutLine();
        PointWNL[] edges=new PointWNL[edgesLine.length];
        for(int i=0;i< edgesLine.length;i++)
        {
            edges[i]=new PointWNL(edgesLine[i].x,edgesLine[i].y,time);
        }
        for(int i=0;i<edges.length;i++)
        {
            PointWNL p3=this.getCorrespondingPoint(edges[i],edges[(i+1)%edges.length],faceNr,cycleNr);
            if(p3!=null)
            {
                this.addTrianglesFromCHTPoint(cht,time,p3,faceNr,cycleNr);
                break;
            }
        }
    }
    
    public void addTrianglesFromCHTCHT(ConvexHullTreeNode cht1,ConvexHullTreeNode cht2,int time1,int time2,int faceNr,int cycleNr)
    {
        if(TriRepUtil.debugging)
            System.out.println("CHTCHT");
        this.rotaring_pane(cht1,cht2,time1,time2,faceNr,cycleNr);
        
        if(cht1.getParentNode() instanceof ConvexHullTreeNode)
        {
            LineWA[] line1 = ((ConvexHullTreeNode) cht1.getParentNode()).getLineForChild(cht1);                               
            LineWA[] line2 = ((ConvexHullTreeNode) cht2.getParentNode()).getLineForChild(cht2);                               
            PointWNL p1 = new PointWNL(line1[0].x,line1[0].y,time1);
            PointWNL p2 = new PointWNL(line1[1].x,line1[1].y,time1);
            PointWNL p3 = new PointWNL(line2[0].x,line2[0].y,time2);
            PointWNL p4 = new PointWNL(line2[1].x,line2[1].y,time2);
            removeTrapezoid(p1, p2, p3, p4, faceNr, cycleNr);
        }
        
        for(int i=0;i<cht1.getChildren().length;i++)
        {
            RegionTreeNode[] matches =this.myMatch.getMatches(cht1.getChildren()[i]);
            if(matches==null||matches.length==0||matches[0]==null)
            {
                addTrianglesFromCHTNull(cht1.getChildren()[i],time1,faceNr,cycleNr);
            }
            else
            {
                if(matches.length==1)
                {
                    addTrianglesFromCHTCHT(cht1.getChildren()[i],(ConvexHullTreeNode)matches[0],time1,time2,faceNr,cycleNr);
                }
                else
                {
                    if(TriRepUtil.debuggingWarnings)
                        System.out.println("Problem mehrere Matches CHT");
                }
            }
        }
        
        for(int i=0;i<cht2.getChildren().length;i++)
        {
            RegionTreeNode[] matches =this.myMatch.getMatches(cht2.getChildren()[i]);
            if(matches==null||matches.length==0||matches[0]==null)
            {
                addTrianglesFromCHTNull(cht2.getChildren()[i],time2,faceNr,cycleNr);
            }
        }
    }
    
    public int getSecTime(int currTime)
    {
        if(currTime==0)
            return(this.secTime);
        else
            return(0);
    }
      
    public void addTrianglesFromFaceFace(Face face1,Face face2, int time1, int time2, int faceNr)
    {
        if(TriRepUtil.debugging)
            System.out.println("FaceFace");
        this.addTrianglesFromCHTCHT(face1.getCycle(),face2.getCycle(),time1,time2,faceNr,0);
        for (int i=0;i<face1.getNrOfHoles();i++)
        {
            RegionTreeNode[] matchedHole=(this.myMatch.getMatches(face1.getHole(i)));
            if(matchedHole==null||matchedHole.length==0||matchedHole[0]==null)
            {
                PointWNL p3=new PointWNL(face1.getHole(i).getCenter().x,face1.getHole(i).getCenter().y,time2);
                this.addTrianglesFromCHTPoint(face1.getHole(i),time1,p3,faceNr, i+1);
            }
            else
            {
                if(matchedHole.length==1)
                {
                    this.addTrianglesFromCHTCHT(face1.getHole(i),((ConvexHullTreeNode)matchedHole[0]),time1,time2,faceNr,i+1);
                }
            }
        }
        for (int i=0;i<face2.getNrOfHoles();i++)
        {
            RegionTreeNode[] matchedHole=(this.myMatch.getMatches(face2.getHole(i)));
            if(matchedHole==null||matchedHole.length==0||matchedHole[0]==null)
            {
                PointWNL p3=new PointWNL(face2.getHole(i).getCenter().x,face2.getHole(i).getCenter().y,time1);
                this.addTrianglesFromCHTPoint(face2.getHole(i),time2,p3,faceNr,i+face1.getNrOfHoles()+1);
            }
        }
    }
    public void addTrianglesFromFaceNull(Face face,int time,int faceNr)
    {
        if(TriRepUtil.debugging)
            System.out.println("FaceNull");
        PointWNL p3=new PointWNL(face.getCycle().getCenter().x,face.getCycle().getCenter().y,getSecTime(time));
        this.addTrianglesFromCHTPoint(face.getCycle(),time,p3,faceNr,0);
        for (int i=0;i<face.getNrOfHoles();i++)
        {
            this.addTrianglesFromCHTPoint(face.getHole(i),time,p3,faceNr,i+1);
        }
    }
    
    public LineWA[][] getSection(double time)
    {
        Vector res=new Vector();
        for(int i=0;i<this.triangles.size();i++)
        {
            LineWA[] tmp=new LineWA[2];
            triangle tmptri=this.getTriangle(i);
            if(tmptri.p1.t==0)
            {
                tmp[0]= new LineWA(((int)Math.round(tmptri.p1.x+(tmptri.p3.x-tmptri.p1.x)*time)),(int)Math.round(tmptri.p1.y+(tmptri.p3.y-tmptri.p1.y)*time));
                tmp[1]= new LineWA((int)Math.round(tmptri.p2.x+(tmptri.p3.x-tmptri.p2.x)*time),(int)Math.round(tmptri.p2.y+(tmptri.p3.y-tmptri.p2.y)*time));
            }
            else
            {
                tmp[0]= new LineWA(((int)Math.round(tmptri.p3.x+(tmptri.p1.x-tmptri.p3.x)*time)),(int)Math.round(tmptri.p3.y+(tmptri.p1.y-tmptri.p3.y)*time));
                tmp[1]= new LineWA((int)Math.round(tmptri.p3.x+(tmptri.p2.x-tmptri.p3.x)*time),(int)Math.round(tmptri.p3.y+(tmptri.p2.y-tmptri.p3.y)*time));
            }
            res.add(tmp);
        }
        LineWA[][] res2=new LineWA[res.size()][2];
        for(int i=0;i<res.size();i++)
        {
            res2[i]=(LineWA[])res.elementAt(i);
        }
        
        return(res2);
    }
    
    public void saveAsVRML(String filename,int time) throws FileNotFoundException, IOException
    {
        FileOutputStream filestream;
        OutputStreamWriter fs;
        int cp, counter;
        Vector convertlist;
        PointWNL point;
        filestream = new FileOutputStream(filename);
        fs = new OutputStreamWriter(filestream);
        fs.write("#VRML V2.0 utf8\n\n");
        myMatch.getSource().writeRegionToVRML(fs,0,"1 0 0",".5",".4");
        myMatch.getTarget().writeRegionToVRML(fs,time,"0 .5 1",".5",".4");
        for(int i=0;i<this.triangles.size();i++)
        {
            this.writeTriangleToVRML(fs,time,"1 1 0",".5",".1",i);
        }
        fs.write("PointLight {\n");
        fs.write("   attenuation 3.999999e-2 0 0\n");
        fs.write("   location -100 -100 1\n");
        fs.write("}\n");
        fs.write("Background {\n");
        fs.write("   skyColor [\n");
        fs.write("   0.0 0.1 0.8,\n");
        fs.write("   0.0 0.5 1.0,\n");
        fs.write("   1.0 1.0 1.0]\n");
        fs.write("   skyAngle [0, 1.571]\n");
        fs.write("}\nViewpoint   {  orientation 0.25 -0.4 -0.8 2  position 0 0  1   }");
        fs.flush();
        filestream.close();
    }
    
    
    public void writeTriangleToVRML( OutputStreamWriter fs,
            double time,String color,String shininess, String transparency, int index) throws IOException
    {
        
        fs.write("Shape {\n");
        fs.write("   appearance Appearance {\n");
        fs.write("      material Material { diffuseColor "+color+" shininess "+shininess+" transparency "+transparency+"}\n");
        fs.write("   }");
        fs.write("   geometry IndexedFaceSet {\n");
        fs.write("      coord Coordinate {\n");
        fs.write("         point [\n");
        fs.write("                ");
        fs.write(Float.toString((((float)this.getTriangle(index).p1.x)/20)-5));
        fs.write(" ");
        fs.write(Float.toString((((float)this.getTriangle(index).p1.y)/-20)-6));
        fs.write(" ");
        fs.write(Double.toString(((float)(this.getTriangle(index).p1.t)*time)));
        fs.write("\n");
        
        fs.write(Float.toString((((float)this.getTriangle(index).p2.x)/20)-5));
        fs.write(" ");
        fs.write(Float.toString((((float)this.getTriangle(index).p2.y)/-20)-6));
        fs.write(" ");
        fs.write(Double.toString(((float)(this.getTriangle(index).p2.t)*time)));
        fs.write("\n");
        
        fs.write(Float.toString((((float)this.getTriangle(index).p3.x)/20)-5));
        fs.write(" ");
        fs.write(Float.toString((((float)this.getTriangle(index).p3.y)/-20)-6));
        fs.write(" ");
        fs.write(Double.toString(((float)(this.getTriangle(index).p3.t)*time)));
        fs.write("\n");
        fs.write("               ]\n");
        fs.write("         }\n");
        fs.write("      coordIndex [ 0 1 2]\n");
        fs.write("      solid FALSE\n");
        fs.write("   }\n");
        fs.write("}\n");
    }
}