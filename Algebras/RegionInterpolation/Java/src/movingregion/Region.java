

package movingregion;
import java.util.*;
import java.awt.*;
import java.io.*;
public class Region implements RegionTreeNode, Cloneable, Serializable
{
    Vector Faces;
    Vector OriginalVertices;
    double OffsetX;
    double OffsetY;
    double scaleX;
    double scaleY;
    static final long serialVersionUID =-7777879906774960304l;
    public Region()
    {
        Faces=new Vector();
    }
    public void addFace(Face newFace)
    {
        Faces.add(newFace);
    }
    public int getNrOfFaces()
    {
        return(Faces.size());
    }
    
    public Face getFace(int index)
    {
        return((Face)Faces.elementAt(index));
    }
    
    public Face[] getFaces()
    {
        Face[] res=new Face[getNrOfFaces()];
        for(int i=0;i< getNrOfFaces();i++)
        {
            res[i]=this.getFace(i);
        }
        return(res);
    }
    
    public Face[] splitOnLine(LineWA[] splitLine)
    {
        Vector resv=new Vector();
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            Face trythis=this.getFace(i);
            Face tmp=trythis.splitOnLine(splitLine);
            if(tmp!=null)
            {
                this.addFace(tmp);
                resv.add(tmp);
                resv.add(trythis);
            }
        }
        Face[] res=new Face[resv.size()];
        for(int i=0;i<resv.size();i++)
        {
            res[i]=(Face)resv.elementAt(i);
        }
        return(res);        
    }
    
    public void paintRegion(Graphics g, boolean isActive)
    {
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            this.getFace(i).paintFace(g,isActive);
        }
    }
    
    public String toString()
    {
        String res="";
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            res=res+"Facenr. "+i+'\n';
            res=res+this.getFace(i);
        }
        return(res);
    }
    
    
    public void removeFace(int index)
    {
        this.Faces.remove(index);
    }
    
//    public Region(String Filename, int x,int y)
//    {
//        OriginalVertices=new Vector();
//        int State=0;
//        String Number="";
//        Reader f = null;
//        Vector tmp=new Vector();
//        try
//        {
//            f = new FileReader( Filename );
//            
//            for ( int c; ( c = f.read() ) != -1; )
//            {
//                char ch=(char) c;
//                if(ch=='\n'||ch=='\r'||ch=='\b'||ch=='\f')
//                    continue;
//                if (ch=='t')
//                    ch=' ';
//                switch (State)
//                {
//                    case 0 :
//                    {
//                        switch (ch)
//                        {
//                            case '(':case 'r':case 'e':case 'g':case 'i':case 'o':case ' ':
//                            {
//                                break;
//                            }
//                            case 'n':
//                            {
//                                State=1;
//                                break;
//                            }
//                            default:
//                            {
//                                System.out.println("Falsches Zeichen:"+ch);
//                            }
//                        }
//                        break;
//                    }
//                    
//                    case 1 :
//                    {
//                        switch (ch)
//                        {
//                            case ' ':
//                            {
//                                break;
//                            }
//                            case '(':
//                            {
//                                State=2;
//                                break;
//                            }
//                            default:
//                            {
//                                System.out.println("Falsches Zeichen:"+ch);
//                            }
//                        }
//                        break;
//                    }
//                    case 2 :
//                    {
//                        switch (ch)
//                        {
//                            case ' ':
//                            {
//                                break;
//                            }
//                            case '(':
//                            {
//                                State=3;
//                                break;
//                            }
//                            default:
//                            {
//                                System.out.println("Falsches Zeichen:"+ch);
//                            }
//                        }
//                        break;
//                    }
//                    
//                    case 3 :
//                    {
//                        switch (ch)
//                        {
//                            case ' ':
//                            {
//                                break;
//                            }
//                            case '(':
//                            {
//                                State=4;
//                                break;
//                            }
//                            default:
//                            {
//                                System.out.println("Falsches Zeichen:"+ch);
//                            }
//                        }
//                        break;
//                    }
//                    case 4 :
//                    {
//                        switch (ch)
//                        {
//                            case ' ':
//                            {
//                                break;
//                            }
//                            case '(':
//                            {
//                                State=5;
//                                break;
//                            }
//                            default:
//                            {
//                                System.out.println("Falsches Zeichen:"+ch);
//                            }
//                        }
//                        break;
//                    }
//                    case 5:
//                    {
//                        switch (ch)
//                        {
//                            case ')':
//                            {
//                                State=6;
//                                break;
//                            }
//                            default:
//                            {
//                                double[] test=readTwoDoubles(f);
//                                System.out.println("5 "+test[0]);
//                                System.out.println("5 "+test[1]);
//                                tmp.add(test);
//                                break;
//                            }
//                        }
//                        break;
//                    }
//                    case 6:
//                    {
//                        switch(ch)
//                        {
//                            case ')':
//                            {
//                                OriginalVertices.add(tmp);
//                                State=7;
//                                break;
//                            }
//                            default:
//                            {
//                                System.out.println("Falsches Zeichen:"+ch);
//                            }
//                        }
//                        break;
//                    }
//                    default:
//                    {
//                        System.out.println("not yet");
//                    }
//                }
//                System.out.print( ch );
//            }
//            f.close();
//            double minx=((double[])((Vector)this.OriginalVertices.elementAt(0)).elementAt(0))[0];
//            double miny=((double[])((Vector)this.OriginalVertices.elementAt(0)).elementAt(0))[1];
//            double maxx=((double[])((Vector)this.OriginalVertices.elementAt(0)).elementAt(0))[0];
//            double maxy=((double[])((Vector)this.OriginalVertices.elementAt(0)).elementAt(0))[1];
//            for (int i=0;i<this.OriginalVertices.size();i++)
//            {
//                Vector tmpv=(Vector)this.OriginalVertices.elementAt(i);
//                for(int j=0;j<tmpv.size();j++)
//                {
//                    double[] tmpa=(double[])tmpv.elementAt(j);
//                    minx=Math.min(minx,tmpa[0]);
//                    miny=Math.min(miny,tmpa[1]);
//                    maxx=Math.max(minx,tmpa[0]);
//                    maxy=Math.max(miny,tmpa[1]);
//                }
//            }
//            OffsetX=minx;
//            OffsetY=miny;
//            scaleX=x/(maxx-minx);
//            scaleY=y/(maxy-miny);
//            System.out.println(minx+" "+miny+" "+maxx+" "+maxy+" "+scaleX+" "+scaleY);
//            restoreFromOriginal();
//            
//        }
//        catch ( IOException e )
//        {
//            System.out.println( "Error reading file!" );
//        }
//    }
    
//    private void restoreFromOriginal()
//    {
//        Faces=new Vector();
//        for (int i=0;i<this.OriginalVertices.size();i++)
//        {
//            Vector tmpcycle=(Vector)this.OriginalVertices.elementAt(0);
//            LineWA[] tmpline=new LineWA[tmpcycle.size()];
//            
//            for(int j=0;j<tmpcycle.size();j++)
//            {
//                double[] tmpdouble=(double[])tmpcycle.elementAt(j);
//                int test=(int)((tmpdouble[0]-OffsetX)*scaleX);
//                tmpline[j]=new LineWA((int)((tmpdouble[0]-OffsetX)*scaleX),(int)((tmpdouble[1]-OffsetY)*scaleY));
//            }
//            Face tmpface=new Face(tmpline);
//            Faces.add(tmpface);
//        }
//    }
    public void writeRegionToVRML( OutputStreamWriter fs,
            double time, String color,String shininess, String transparency) throws IOException
    {
        for(int i=0;i<getNrOfFaces();i++)
        {
            getFace(i).writeFaceToVRML(fs,time,color,shininess,transparency);
        }
    }
    
    private double[] readTwoDoubles(Reader f)throws IOException
    {
        double[] res=new double[2];
        char ch;
        String[] number=new String[2];
        number[0]="";
        number[1]="";
        int active=0;
        while ((ch=(char)f.read())!=')')
        {
            if(ch=='\n'||ch=='\r'||ch=='\b'||ch=='\f')
                continue;
            if (ch=='t')
                ch=' ';
            if(ch==' '&&number[active].equals(""))
                continue;
            if(ch==' '&&!(number[active].equals("")))
                active++;
            if(ch=='0'||ch=='1'||ch=='2'||ch=='3'||ch=='4'||ch=='5'||ch=='6'||ch=='7'||ch=='8'||ch=='9'||ch=='.'||ch=='-')
                number[active]=number[active]+ch;
        }
        
        res[0]=Double.parseDouble(number[0]);
        res[1]=Double.parseDouble(number[1]);
        return res;
    }
    
    protected Object clone()
    {
        Region res=new Region();
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            Face tmp=(Face)this.getFace(i).clone();
            tmp.setParent(res);
            res.addFace(tmp);
        }
        return(res);
    }
}
