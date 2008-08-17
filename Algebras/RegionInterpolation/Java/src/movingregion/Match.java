

package movingregion;

import java.util.*;

public abstract class Match
{
    Region source;
    Region target;
    HashMap maps;
    String name;
    String description;
    double Hausdorffrating=0.0;
    double Ovelaprating=0.0;
    double Arearating=0.0;
    private int NrOfRatings=0;
    double linearRating=0.0;
    
    public abstract void matchCHTNs(ConvexHullTreeNode[] chtn1,ConvexHullTreeNode[] chtn2);
    public abstract void matchFaces(Face[] chtn1,Face[] chtn2);
    public abstract Face getBestMatch(Face source,Face[] targets);
    public abstract ConvexHullTreeNode getBestMatch(ConvexHullTreeNode source,ConvexHullTreeNode[] targets);
    
    public Match(Region source, Region target,String name,String description)
    {
        this.description=description;
        this.name=name;
        source.setSourceOrTarget(0);
        this.source=source;
        target.setSourceOrTarget(1);
        this.target=target;
        maps=new HashMap();
    }
    
    private void removeSingleMatch(RegionTreeNode toDelete)
    {
        maps.remove(toDelete);
        Iterator it=maps.keySet().iterator();
        while(it.hasNext())
        {
            SingleMatch tmp=(SingleMatch)it.next();
            for(int i=0;i<tmp.getNrTargets();i++)
            {
                if(toDelete!=null&&toDelete.equals(tmp.getTargetAt(i)))
                    tmp.removeTarget(i);
            }
        }
    }
    
    public void removeMatches(RegionTreeNode toDelete)
    {
        this.removeSingleMatch(toDelete);
        if(toDelete instanceof ConvexHullTreeNode)
        {
            ConvexHullTreeNode[] children=((ConvexHullTreeNode)toDelete).getChildren();
            for(int i=0;i<children.length;i++)
            {
                removeMatches(children[i]);
            }
        }
        
        if(toDelete instanceof Face)
        {
            Face deleteFace=((Face)toDelete);
            removeMatches(deleteFace.getCycle());
            for(int i=0;i<deleteFace.getNrOfHoles();i++)
            {
                removeMatches(deleteFace.getHole(i));
            }
        }
        
        if(toDelete instanceof Region)
        {
            Region deleteRegion=((Region)toDelete);
            for(int i=0;i<deleteRegion.getNrOfFaces();i++)
            {
                removeMatches(deleteRegion.getFace(i));
            }
        }
    }
    
    public void setMatch(Match newOne)
    {
        this.source=newOne.source;
        this.target=newOne.target;
        this.maps=newOne.maps;
        this.name=newOne.name;
        this.description=newOne.description;
        this.Hausdorffrating=newOne.Hausdorffrating;
        this.Ovelaprating=newOne.Ovelaprating;
        this.Arearating=newOne.Arearating;
        this.linearRating=newOne.linearRating;
        
    }
    
    public void addName(String newName)
    {
        this.name=name+"§"+newName;
    }
    
    public String getName()
    {
        return(name);
    }
    
    public double getAreaRating()
    {
        return(this.Arearating);
    }
    
    public double getOverlapRating()
    {
        return(this.Ovelaprating);
    }
    
    public double getHausdorffRating()
    {
        return(this.Hausdorffrating);
    }
    
    public double getLinarRating()
    {
        return(this.linearRating);
    }
    
    public String getDescription()
    {
        return(description);
    }
    
    public Region getSource()
    {
        return(source);
    }
    
    public Region getTarget()
    {
        return(target);
    }
    
    public void addMatch(RegionTreeNode source,RegionTreeNode target)
    {
        SingleMatch tmp=(SingleMatch)this.maps.get(source);
        if(tmp==null)
        {
            tmp=new SingleMatch(source,target);
            maps.put(tmp,tmp);
        }
        else
        {
            tmp.addTarget(target);
        }
    }
    
    public RegionTreeNode[] getMatches(RegionTreeNode source)
    {
        
        SingleMatch tmp=(SingleMatch)this.maps.get(source);
        if(tmp==null)
            return null;
        tmp.removeNulls();
        RegionTreeNode[] res=new RegionTreeNode[tmp.getNrTargets()];
        for(int i=0;i<res.length;i++)
        {
            res[i]=tmp.getTargetAt(i);
        }
        return(res);
    }
    
    public ConvexHullTreeNode[] getTargetChildren(RegionTreeNode source)
    {
        RegionTreeNode[] tmp=getMatches(source);
        HashSet tmpSet=new HashSet();
        for(int i=0;i<tmp.length;i++)
        {
            if(tmp[i] instanceof Face)
            {
                Face tmpf=(Face)tmp[i];
                ConvexHullTreeNode[] tmpc=tmpf.getCycle().getChildren();
                for(int j=0;j< tmpc.length;j++)
                {
                    tmpSet.add(tmpc[j]);
                }
                
                for(int j=0;j< tmpf.getNrOfHoles();j++)
                {
                    tmpSet.add(tmpf.getHole(j));
                }
            }
            if(tmp[i] instanceof ConvexHullTreeNode)
            {
                ConvexHullTreeNode tmpf=(ConvexHullTreeNode)tmp[i];
                ConvexHullTreeNode[] tmpc=tmpf.getChildren();
                for(int j=0;j< tmpc.length;j++)
                {
                    tmpSet.add(tmpc[j]);
                }
            }
        }
        Object[] tmpArray=tmpSet.toArray();
        ConvexHullTreeNode[] res=new ConvexHullTreeNode[tmpArray.length];
        for (int i=0;i<tmpArray.length;i++)
        {
            res[i]=(ConvexHullTreeNode)tmpArray[i];
        }
        return(res);
    }
    
    public void fertig()
    {
        //generateRatings();
        Iterator keyIter=maps.keySet().iterator();
        int iterationen=0;
        while(keyIter.hasNext())
        {
            if(iterationen>maps.size()*3)
            {
                maps.clear();
                if(TriRepUtil.debuggingWarnings)
                    System.out.println("Match "+this.getName()+" ließ sich nicht finalisieren und wird gelöscht");
                this.Arearating=0;
                this.Hausdorffrating=0;
                this.Ovelaprating=0;
                this.linearRating=0;
                break;
            }
            iterationen++;
            SingleMatch tmp=(SingleMatch)maps.get(keyIter.next());
            if(TriRepUtil.debugging)
            {
                System.out.println("EING"+ iterationen);
                System.out.println(tmp);
            }
            if(tmp!=null&&tmp.getNrTargets()==1&&(tmp.getTargetAt(0)instanceof ConvexHullTreeNode))
            {
                ConvexHullTreeNode stmp=(ConvexHullTreeNode)tmp.getSource();
                ConvexHullTreeNode ttmp=(ConvexHullTreeNode)tmp.getTargetAt(0);
                if((stmp.getParentNode()instanceof ConvexHullTreeNode )&&(ttmp.getParentNode()instanceof ConvexHullTreeNode ))
                {
                    ConvexHullTreeNode parentS=((ConvexHullTreeNode)stmp.getParentNode());
                    ConvexHullTreeNode parentT=((ConvexHullTreeNode)ttmp.getParentNode());
                    LineWA[] linesS=parentS.getLineForChild(stmp);
                    LineWA[] linesT=parentT.getLineForChild(ttmp);
                    if(linesS!=null&&linesT!=null)
                    {
                        LineWA[] linesNeiS=new LineWA[4];
                        LineWA[] linesNeiT=new LineWA[4];
                        linesNeiS[1]=linesS[0];
                        linesNeiS[2]=linesS[1];
                        linesNeiT[1]=linesT[0];
                        linesNeiT[2]=linesT[1];
                        LineWA[] parentSLL=parentS.getOrderedOutLine();
                        LineWA[] parentTLL=parentT.getOrderedOutLine();
                        for(int i=0;i<parentSLL.length;i++)
                        {
                            if(parentSLL[i].equals(linesS[0])&&parentSLL[(i+1)%parentSLL.length].equals(linesS[1]))
                            {
                                linesNeiS[0]=parentSLL[(i-1+parentSLL.length)%parentSLL.length];
                                linesNeiS[3]=parentSLL[(i+2)%parentSLL.length];
                            }
                        }
                        for(int i=0;i<parentTLL.length;i++)
                        {
                            if(parentTLL[i].equals(linesT[0])&&parentTLL[(i+1)%parentTLL.length].equals(linesT[1]))
                            {
                                linesNeiT[0]=parentTLL[(i-1+parentTLL.length)%parentTLL.length];
                                linesNeiT[3]=parentTLL[(i+2)%parentTLL.length];
                            }
                        }
                        TriRepUtil.computeLineAngles(linesNeiS);
                        TriRepUtil.computeLineAngles(linesNeiT);
                        double angleT=linesNeiT[1].angle;
                        double angleS=linesNeiS[1].angle;
                        if(linesNeiS[0].angle>linesNeiS[1].angle)
                        {
                            linesNeiS[1].angle=linesNeiS[1].angle+2*Math.PI;
                        }
                        if(linesNeiS[1].angle>linesNeiS[2].angle)
                        {
                            linesNeiS[2].angle=linesNeiS[2].angle+2*Math.PI;
                        }
                        if(linesNeiT[0].angle>linesNeiT[1].angle)
                        {
                            linesNeiT[1].angle=linesNeiT[1].angle+2*Math.PI;
                        }
                        if(linesNeiT[1].angle>linesNeiT[2].angle)
                        {
                            linesNeiT[2].angle=linesNeiT[2].angle+2*Math.PI;
                        }
                        if((angleS<linesNeiT[0].angle||angleS>linesNeiT[2].angle)&&(angleS+2*Math.PI<linesNeiT[0].angle||angleS+2*Math.PI>linesNeiT[2].angle))
                        {
//                            ConvexHullTreeNode source=(ConvexHullTreeNode)tmp.getSource();
//                            ConvexHullTreeNode sourceCycle=(ConvexHullTreeNode)source.getParentNode();
//                            Face parent=(Face)sourceCycle.getParentNode();
//                            parent.concavity2Hole(source);
                            tmp.removeTargets();
                        }
                        if((angleT<linesNeiS[0].angle||angleT>linesNeiS[2].angle)&&(angleT+2*Math.PI<linesNeiS[0].angle||angleT+2*Math.PI>linesNeiS[2].angle))
                        {
//                            ConvexHullTreeNode source=(ConvexHullTreeNode)tmp.getSource();
//                            ConvexHullTreeNode sourceCycle=(ConvexHullTreeNode)source.getParentNode();
//                            Face parent=(Face)sourceCycle.getParentNode();
//                            parent.concavity2Hole(source);
                            tmp.removeTargets();
                        }
                    }
                }
            }//1zu1
            
            if(tmp!=null&&tmp.getNrTargets()>1)
            {
                
                if(tmp.getSource() instanceof Face)
                {
                    if(TriRepUtil.debugging)
                    {
                        System.out.println("SplitFF");
                        System.out.println(this.source);
                        System.out.println("Target");
                        System.out.println(this.target);
                    }
                    Face sou=(Face)tmp.getSource();
                    Face t1=(Face)tmp.getTargetAt(0);
                    Face t2=(Face)tmp.getTargetAt(1);
                    this.removeMatches(sou);
                    this.removeMatches(t1);
                    this.removeMatches(t2);
                    Region parentsou= sou.getParent();
                    Face[] newSources=parentsou.splitOnLine(sou.getCycle().getSplitLine(t1.getCycle(),t2.getCycle()));
                    if(newSources.length==2)
                    {
                        Face[] oldtargets=new Face[2];
                        oldtargets[0]=t1;
                        oldtargets[1]=t2;
                        Face[] new1=new Face[1];
                        Face[] new2=new Face[1];
                        Face[] old1=new Face[1];
                        Face[] old2=new Face[1];
                        new1[0]=newSources[0];
                        new2[0]=newSources[1];
                        old1[0]=this.getBestMatch(newSources[0],oldtargets);
                        old2[0]=this.getBestMatch(newSources[1],oldtargets);
                        this.matchFaces(new1,old1);
                        this.matchFaces(new2,old2);
                        for(int i=2;i<tmp.getNrTargets();i++)
                        {
                            Face[] tmpTar=new Face[1];
                            tmpTar[0]=(Face)tmp.getTargetAt(i);
                            Face[] tmpSour=new Face[1];
                            tmpSour[0]=this.getBestMatch(tmpTar[0],newSources);
                            this.matchFaces(tmpSour,tmpTar);
                        }
                    }
                    else
                   //Split failed match each face with best matched
                   {
                      Face[] tmpSour=new Face[1];
                      tmpSour[0]=sou;
                      Face[] tmpTar=new Face[tmp.getNrTargets()];
                      Face[] tmpTar2=new Face[1];
                      for(int i = 0; i < tmp.getNrTargets(); i++)
                      {
                         tmpTar[i]= (Face)tmp.getTargetAt(i);
                      }
                      tmpTar2[0]=getBestMatch(sou, tmpTar);
                      matchFaces(tmpSour, tmpTar2);             
                    }
                    keyIter=maps.keySet().iterator();
                    continue;
                }
                
                if(tmp.getSource() instanceof ConvexHullTreeNode && 
                        ((ConvexHullTreeNode)tmp.getSource()).isHole() &&
                        ((ConvexHullTreeNode)tmp.getSource()).getParentNode() instanceof Face)
                {
                    if(TriRepUtil.debugging)
                    {
                        System.out.println(tmp);
                        System.out.println("SplitHH");
                        System.out.println(this.source);
                        System.out.println("Target");
                        System.out.println(this.target);
                    }
                    ConvexHullTreeNode sou=(ConvexHullTreeNode)tmp.getSource();
                    ConvexHullTreeNode t1=(ConvexHullTreeNode)tmp.getTargetAt(0);
                    ConvexHullTreeNode t2=(ConvexHullTreeNode)tmp.getTargetAt(1);
                    this.removeMatches(sou);
                    this.removeMatches(t1);
                    this.removeMatches(t2);
                    RegionTreeNode parentsou= sou.getParentNode();
                    if(TriRepUtil.debugging)
                        System.out.println(tmp);
                    LineWA[][] newSources=sou.getSplitNodes(sou.getSplitLine(t1,t2));
                    ConvexHullTreeNode[] newSourcesCHTN=new ConvexHullTreeNode[2];
                    if(newSources!=null && newSources.length==2&&newSources[0]!=null && newSources[1]!=null)
                    {
                        ConvexHullTreeNode[] oldtargets=new ConvexHullTreeNode[2];                        
                        oldtargets[0]=t1;
                        oldtargets[1]=t2;
                        ((Face)parentsou).removeHole(sou);
                        newSourcesCHTN[0]=new ConvexHullTreeNode(newSources[0],true,parentsou,this.source.sourceOrTarget);
                        newSourcesCHTN[1]=new ConvexHullTreeNode(newSources[1],true,parentsou,this.source.sourceOrTarget);
                        ((Face)parentsou).addHole(newSourcesCHTN[0]);
                        ((Face)parentsou).addHole(newSourcesCHTN[1]);
                        ConvexHullTreeNode[] new1=new ConvexHullTreeNode[1];
                        ConvexHullTreeNode[] new2=new ConvexHullTreeNode[1];
                        ConvexHullTreeNode[] old1=new ConvexHullTreeNode[1];
                        ConvexHullTreeNode[] old2=new ConvexHullTreeNode[1];
                        new1[0]=newSourcesCHTN[0];
                        new2[0]=newSourcesCHTN[1];
                        old1[0]=this.getBestMatch(newSourcesCHTN[0],oldtargets);
                        old2[0]=this.getBestMatch(newSourcesCHTN[1],oldtargets);
                        this.matchCHTNs(new1,old1);
                        this.matchCHTNs(new2,old2);
                        for(int i=2;i<tmp.getNrTargets();i++)
                        {
                            ConvexHullTreeNode[] tmpTar=new ConvexHullTreeNode[1];
                            tmpTar[0]=(ConvexHullTreeNode)tmp.getTargetAt(i);
                            ConvexHullTreeNode[] tmpSour=new ConvexHullTreeNode[1];
                            tmpSour[0]=this.getBestMatch(tmpTar[0],newSourcesCHTN);
                            this.matchCHTNs(tmpSour,tmpTar);
                        }
                    } 
                    else 
                    {
                        ConvexHullTreeNode[] tmpSour = new ConvexHullTreeNode[1];
                        tmpSour[0] = sou;
                        ConvexHullTreeNode[] tmpTar = new ConvexHullTreeNode[tmp.getNrTargets()];
                        ConvexHullTreeNode[] tmpTar2 = new ConvexHullTreeNode[1];
                        for (int i = 0; i < tmp.getNrTargets(); i++) 
                        {
                            tmpTar[i] = (ConvexHullTreeNode) tmp.getTargetAt(i);
                        }
                        tmpTar2[0] = getBestMatch(sou, tmpTar);
                        this.matchCHTNs(tmpSour, tmpTar2);
                    }
                    keyIter=maps.keySet().iterator();
                    continue;
                }

                if (tmp.getSource() instanceof ConvexHullTreeNode) 
                {
                    if(TriRepUtil.debugging)
                        System.out.println("1 zu n ungelöst");
                    ConvexHullTreeNode[] tmpSour = new ConvexHullTreeNode[1];
                    tmpSour[0] = (ConvexHullTreeNode) tmp.getSource();
                    removeMatches((ConvexHullTreeNode) tmp.getSource());
                    ConvexHullTreeNode[] tmpTar = new ConvexHullTreeNode[tmp.getNrTargets()];
                    ConvexHullTreeNode[] tmpTar2 = new ConvexHullTreeNode[1];
                    for (int i = 0; i < tmp.getNrTargets(); i++) 
                    {
                        tmpTar[i] = (ConvexHullTreeNode) tmp.getTargetAt(i);
                    }
                    tmpTar2[0] = getBestMatch((ConvexHullTreeNode) tmp.getSource(), tmpTar);
                    
                    matchCHTNs(tmpSour, tmpTar2);
                    keyIter = maps.keySet().iterator();
                    if(TriRepUtil.debugging)
                        System.out.println("Weiter");
                    continue;
                }
//                if(tmp.getSource() instanceof ConvexHullTreeNode&&((ConvexHullTreeNode)tmp.getSource()).getParentNode()instanceof ConvexHullTreeNode)
//                {
////                    vector<ConvexHullTreeNode*> tmpSour;
////             tmpSour.push_back( (ConvexHullTreeNode*) tmp->getSource());             
////             removeMatches((ConvexHullTreeNode*) tmp->getSource());                                               
////             vector<ConvexHullTreeNode*> tmpTar;
////             vector<ConvexHullTreeNode*> tmpTar2;
////             for(int i = 0; i < tmp->getNrTargets(); i++)
////             {
////               tmpTar.push_back( (ConvexHullTreeNode*) tmp->getTargetAt(i));
////             }
////             tmpTar2.push_back(getBestMatch( (ConvexHullTreeNode*) tmp->getSource(), &tmpTar));
////             matchCHTNs(&tmpSour, &tmpTar2);            
////               iter = maps.begin();                                             
////               continue;         
////               
//                    ConvexHullTreeNode sou=(ConvexHullTreeNode)tmp.getSource();
//                    ConvexHullTreeNode t1=(ConvexHullTreeNode)tmp.getTargetAt(0);
//                    ConvexHullTreeNode t2=(ConvexHullTreeNode)tmp.getTargetAt(1);
//                    this.removeMatches(sou);
//                    this.removeMatches(t1);
//                    this.removeMatches(t2);
//                    RegionTreeNode parentsou= sou.getParentNode();
//                    LineWA[][] newSources=sou.getSplitNodes(sou.getSplitLine(t1,t2));
//                    ConvexHullTreeNode[] newSourcesCHTN=new ConvexHullTreeNode[2];
//                    keyIter=maps.keySet().iterator();
////                    if(newSources!=null&&newSources.length==2&&newSources[0]!=null && newSources[1]!=null)
////                    {
////                        ((ConvexHullTreeNode)parentsou).removeChild(sou);
////                        newSourcesCHTN[0]=((ConvexHullTreeNode)parentsou).addChild(newSources[0]);
////                        newSourcesCHTN[1]=((ConvexHullTreeNode)parentsou).addChild(newSources[1]);
////                        
////                        
////                        
//////                    if(parentsou instanceof ConvexHullTreeNode&& newSources.length==2)
////                    {
////                        ((ConvexHullTreeNode)parentsou).removeChild(sou);
////                        ((ConvexHullTreeNode)parentsou).addHole(new ConvexHullTreeNode(newSources[0],true,parentsou));
////                        ((ConvexHullTreeNode)parentsou).addHole(new ConvexHullTreeNode(newSources[1],true,parentsou));
////                    }
////                    if(newSources.length==2)
////                    {
////                        ConvexHullTreeNode[] oldtargets=new ConvexHullTreeNode[2];
////                        
////                        oldtargets[0]=t1;
////                        oldtargets[1]=t2;
////                        
////                        ConvexHullTreeNode[] new1=new ConvexHullTreeNode[1];
////                        ConvexHullTreeNode[] new2=new ConvexHullTreeNode[1];
////                        ConvexHullTreeNode[] old1=new ConvexHullTreeNode[1];
////                        ConvexHullTreeNode[] old2=new ConvexHullTreeNode[1];
////                        new1[0]=newSourcesCHTN[0];
////                        new2[0]=newSourcesCHTN[1];
////                        old1[0]=this.getBestMatch(newSourcesCHTN[0],oldtargets);
////                        old2[0]=this.getBestMatch(newSourcesCHTN[1],oldtargets);
////                        this.matchCHTNs(new1,old1);
////                        this.matchCHTNs(new2,old2);
////                        for(int i=2;i<tmp.getNrTargets();i++)
////                        {
////                            ConvexHullTreeNode[] tmpTar=new ConvexHullTreeNode[1];
////                            tmpTar[0]=(ConvexHullTreeNode)tmp.getTargetAt(i);
////                            ConvexHullTreeNode[] tmpSour=new ConvexHullTreeNode[1];
////                            tmpSour[0]=this.getBestMatch(tmpTar[0],newSourcesCHTN);
////                            this.matchCHTNs(tmpSour,tmpTar);
////                        }
////                    }
////                    else
////                    {
//////                        this.Arearating=0;
//////                        this.Hausdorffrating=0;
//////                        this.Ovelaprating=0;
//////                        this.linearRating=0;
//////                    }
////                    keyIter=maps.keySet().iterator();
////                    continue;
////                }
//                if(tmp.getNrTargets()>1)
//                {
//                    while(tmp.getNrTargets()>1)
//                    {
//                        tmp.removeTarget(1);
//                    }
//                }
////                if(tmp.getTargetAt(0)instanceof ConvexHullTreeNode)
////                {
////                    Vector Children=new Vector();
////                    ConvexHullTreeNode Parent=null;
////                    if(((ConvexHullTreeNode)tmp.getTargetAt(0)).getParentNode()instanceof ConvexHullTreeNode)
////                    {
////                        Parent=(ConvexHullTreeNode)((ConvexHullTreeNode)tmp.getTargetAt(0)).getParentNode();
////                    }
////                    
////                    
////                    boolean sameParent=true;
////                    for(int i=0;i<tmp.getNrTargets();i++)
////                    {
////                        if(!(tmp.getTargetAt(i)instanceof ConvexHullTreeNode))
////                        {
////                            sameParent=false;
////                        }
////                        else
////                        {
////                            if(Parent!=null&&Parent.equals(((ConvexHullTreeNode)tmp.getTargetAt(i)).getParentNode()))
////                            {
////                                Children.add((ConvexHullTreeNode)tmp.getTargetAt(i));
////                            }
////                            else
////                            {
////                                sameParent=false;
////                            }
////                        }
////                    }
////                    if(sameParent)
////                    {
////                        ConvexHullTreeNode[] ChildList=new ConvexHullTreeNode[Children.size()];
////                        for(int i=0;i<Children.size();i++)
////                        {
////                            ChildList[i]=(ConvexHullTreeNode)Children.get(i);
////                        }
////                        Parent.joinChildren(ChildList);
////                        tmp.removeTargets();
////                        tmp.addTarget(ChildList[0].getParentNode());
////                    }
//             }
            }//1Zu n
            
        }//while
    }//func
    
    protected void generateRatings()
    {
        LineWA[][]lines=new LineWA[source.getNrOfFaces()+target.getNrOfFaces()][];
        
        for(int i=0;i< source.getNrOfFaces();i++)
        {
            lines[i]=source.getFace(i).getCycle().getLines();
            RegionTreeNode[] tmp=this.getMatches(source.getFace(i));
            if(tmp==null||tmp.length==0||tmp[0]==null)
            {
                Hausdorffrating+=TriRepUtil.getDiameter(source.getFace(i).getCycle().getLines());
                Ovelaprating+=0.0;
                Arearating+=0.0;
                linearRating+=.5;
                NrOfRatings++;
            }
            else
            {
                if(this.getMatches(tmp[0])!=null && this.getMatches(tmp[0]).length<=1)
                {
                    rateFace(source.getFace(i),tmp);
                }
            }
        }
        for(int i=0;i<target.getNrOfFaces();i++)
        {
            lines[i+source.getNrOfFaces()]=target.getFace(i).getCycle().getLines();
            RegionTreeNode[] tmp=this.getMatches(target.getFace(i));
            if(tmp==null||tmp.length==0||tmp[0]==null)
            {
                Hausdorffrating+=TriRepUtil.getDiameter(target.getFace(i).getCycle().getLines());
                Ovelaprating+=0.0;
                Arearating+=0.0;
                linearRating+=.5;
                NrOfRatings++;
            }
            else
            {
                if(this.getMatches(tmp[0])!=null && this.getMatches(tmp[0]).length<=1)
                {
                    rateFace(target.getFace(i),tmp);
                }
            }
        }
        
        this.Arearating=this.Arearating/this.NrOfRatings;
        this.Ovelaprating=this.Ovelaprating/this.NrOfRatings;
        this.Hausdorffrating=1-this.Hausdorffrating/this.NrOfRatings/TriRepUtil.getMaxDistance2(lines);
        this.linearRating=this.linearRating/this.NrOfRatings;
        this.description=description+" Ratings:"+Arearating+" "+Ovelaprating+" "+Hausdorffrating+" "+linearRating;
    }
    
    private void rateFace(Face source, RegionTreeNode[] targets)
    {
        double sourceArea=Math.abs(TriRepUtil.getArea(source.getCycle().getLines()));
        double targetArea=0.0;
        double sumOverlaps=0.0;
        double HausdorffTmp=0.0;
        for(int i=0;i<targets.length;i++)
        {
            if(TriRepUtil.getHausdorfDistance(source.getCycle().getLines(),((Face)targets[i]).getCycle().getLines())>HausdorffTmp)
            {
                HausdorffTmp=TriRepUtil.getHausdorfDistance(source.getCycle().getLines(),((Face)targets[i]).getCycle().getLines());
            }
            targetArea+=Math.abs(TriRepUtil.getArea(((Face)targets[i]).getCycle().getLines()));
            sumOverlaps+=Math.abs(TriRepUtil.getSingleOverlap(source.getCycle().getLines(),((Face)targets[i]).getCycle().getLines()));
        }
        this.NrOfRatings++;
        this.Arearating+=Math.min(sourceArea,targetArea)/Math.max(sourceArea,targetArea);
        this.Ovelaprating+=sumOverlaps/sourceArea;
        this.Hausdorffrating+=HausdorffTmp/targets.length;
        this.linearRating+=1.0/targets.length;
        ConvexHullTreeNode[] children=source.getCycle().getChildren();
        for(int i=0;i<children.length;i++)
        {
            RegionTreeNode[] tmp=this.getMatches(children[i]);
            if(tmp==null||tmp.length==0||tmp[0]==null)
            {
                Hausdorffrating+=TriRepUtil.getDiameter(source.getCycle().getLines());
                Ovelaprating+=0.0;
                Arearating+=0.0;
                linearRating+=0.5;
                NrOfRatings++;
            }
            else
            {
                if(this.getMatches(tmp[0]).length<=1)
                {
                    rateCHTN(children[i],tmp);
                }
            }
        }
        
        for(int i=0;i<source.getNrOfHoles();i++)
        {
            RegionTreeNode[] tmp=this.getMatches(source.getHole(i));
            if(tmp==null||tmp.length==0||tmp[0]==null)
            {
                Hausdorffrating+=TriRepUtil.getDiameter(source.getHole(i).getLines());
                Ovelaprating+=0.0;
                Arearating+=0.0;
                linearRating+=0.5;
                NrOfRatings++;
            }
            else
            {
                if(this.getMatches(tmp[0]).length<=1)
                {
                    rateCHTN(source.getHole(i),tmp);
                }
            }
        }
    }
    
    private void rateCHTN(ConvexHullTreeNode source, RegionTreeNode[] targets)
    {
        double sourceArea=Math.abs(TriRepUtil.getArea(source.getLines()));
        double targetArea=0.0;
        double sumOverlaps=0.0;
        double HausdorffTmp=0.0;
        for(int i=0;i<targets.length;i++)
        {
            if(TriRepUtil.getHausdorfDistance(source.getLines(),((ConvexHullTreeNode)targets[i]).getLines())>HausdorffTmp)
            {
                HausdorffTmp=TriRepUtil.getHausdorfDistance(source.getLines(),((ConvexHullTreeNode)targets[i]).getLines());
            }
            targetArea+=Math.abs(TriRepUtil.getArea(((ConvexHullTreeNode)targets[i]).getLines()));
            sumOverlaps+=Math.abs(TriRepUtil.getSingleOverlap(source.getLines(),((ConvexHullTreeNode)targets[i]).getLines()));
        }
        this.NrOfRatings++;
        this.Arearating+=Math.min(sourceArea,targetArea)/Math.max(sourceArea,targetArea);
        this.Ovelaprating+=sumOverlaps/(sourceArea+targetArea);
        this.Hausdorffrating+=HausdorffTmp/targets.length;
        linearRating+=1.0/targets.length;
        ConvexHullTreeNode[] children=source.getChildren();
        for(int i=0;i<children.length;i++)
        {
            RegionTreeNode[] tmp=this.getMatches(children[i]);
            if(tmp==null||tmp.length==0||tmp[0]==null)
            {
                Hausdorffrating+=TriRepUtil.getDiameter(source.getLines());
                Ovelaprating+=0.0;
                Arearating+=0.0;
                linearRating+=0.5;
                NrOfRatings++;
            }
            else
            {
                rateCHTN(children[i],tmp);
            }
        }
    }
    
    public double getRating(double AreaWeight,double OverlapWeigth,double HausdorffWeight,double LinearWeigth)
    {
        return(this.Arearating*AreaWeight+this.Ovelaprating*OverlapWeigth+this.linearRating*LinearWeigth+this.Hausdorffrating*HausdorffWeight);
    }
    
    public String toString()
    {
        String res="";
        Object[] tmp=maps.values().toArray();
        for(int i=0;i<tmp.length;i++)
        {
            res=res+'\n'+((SingleMatch)tmp[i]);
        }
        return(res);
    }
    
}
