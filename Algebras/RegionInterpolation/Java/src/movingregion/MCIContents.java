package movingregion;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import java.util.*;
import java.io.*;

public class MCIContents extends JApplet implements ActionListener,ChangeListener,ComponentListener
{
    JTabbedPane tab;
    JPanel knappanel;
    JToolBar drawUtils;
    JPanel draw;
    JSpinner timeSpinner;
    JComboBox matchType;
    JButton export;
    tegnecanvas tegneomr;
    Vector firstsnap, secondsnap;
    mLineRep trirep;
    TriRepOutPutCanvas result;
    JLabel tekst,vrmlViewerLabel;
    JTextField toleransefelt;
    JTextField vrmlViewer;
    ConvexHullTreeViewer firstS;
    ConvexHullTreeViewer secS;
    SpinnerNumberModel heiMod,widMod;
    boolean wfdisp;
    JScrollPane resultScroller;
    SectionViewer sections;
    MatchViewer matchviewer;
    JSlider matchParam;
    JSlider AreaWeight;
    JSlider OverlapWeight;
    JSlider HausdorffWeight;
    JSlider LinearWeight;
    JPanel WeightPanel;
    JPanel AreaWeightPanel;
    JPanel HausdorffWeightPanel;
    JPanel OverlapWeightPanel;
    JPanel LinearWeightPanel;
    JTextField AreaRatingRes=new JTextField("0.00");
    JTextField OverlapRatingRes=new JTextField("0.00");
    JTextField HausdorffRatingRes=new JTextField("0.00");
    JTextField LinearRatingRes=new JTextField("0.00");
    
    public void componentHidden( ComponentEvent e )
    {}
    
    public void componentMoved( ComponentEvent e )
    {}
    
    public void componentResized( ComponentEvent e )
    {
        int heig=this.knappanel.getHeight();        
        //int heig=e.getComponent().getHeight();        
        if(tab.getTabCount()!=2)
        {
            result.setHei(heig);
            firstS.setHei(heig);
            secS.setHei(heig);
            sections.setHei(heig);
            matchviewer.setHei(heig);
        }
        //int widt=e.getComponent().getWidth();
        int widt=this.knappanel.getWidth();
        if(tab.getTabCount()!=2)
        {
            result.setWid(widt);
            firstS.setWid(widt);
            secS.setWid(widt);
            sections.setWid(widt);
            matchviewer.setWid(widt);
        }
    }
    
    public void componentShown( ComponentEvent e )
    {}
    
    public void init()
    {
        String[] matchTypes = {"OptimalMatch", "OverlapMatch", "CentroidMatch","SteinerPointMatch"};
        matchType = new JComboBox();
        for ( int i=0;i< matchTypes.length;i++ )
        {
            matchType.addItem( matchTypes[i]);
        }
        matchType.addActionListener(this);
        JPanel MTPanel=new JPanel();
        MTPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        MTPanel.add(matchType);
        matchParam= new JSlider(0,100,50);
        matchParam.setMinimumSize(new Dimension(30,1));
        matchParam.addChangeListener(this);
        JPanel MPPanel=new JPanel();
        MPPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        MPPanel.add(matchParam);
        resultScroller=new JScrollPane();
        tab=new JTabbedPane(JTabbedPane.BOTTOM);
        heiMod=new SpinnerNumberModel(400,100,1000,10);
        widMod=new SpinnerNumberModel(500,100,1000,10);
        draw=new JPanel();
        drawUtils=new JToolBar();
        wfdisp = false;
        trirep = null;
        getContentPane().setLayout(new BorderLayout());
        knappanel = new JPanel();
        draw.setLayout(new BorderLayout());
        knappanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        export=new JButton("Export (VRML)");                
        export.setBorder(BorderFactory.createBevelBorder(BevelBorder.RAISED));
        export.setMargin(new Insets(3,3,3,3));
        tekst = new JLabel("VRML Filename:");
        toleransefelt = new JTextField(10);
        JPanel VRMLPanel =new JPanel();
        VRMLPanel.setBorder(BorderFactory.createBevelBorder(BevelBorder.LOWERED));
        JPanel matchPanel=new JPanel();
        matchPanel.setBorder(BorderFactory.createBevelBorder(BevelBorder.LOWERED));
        matchPanel.setLayout(new GridLayout(3,1));
        vrmlViewerLabel=new JLabel("VRML Application");
        vrmlViewer = new JTextField(10);
        vrmlViewer.setText("dune");
        timeSpinner=new JSpinner(new SpinnerNumberModel(10,1,20,1));
        timeSpinner.addChangeListener(this);
        VRMLPanel.add(tekst);
        VRMLPanel.add(toleransefelt);
        VRMLPanel.add(vrmlViewerLabel);
        VRMLPanel.add(vrmlViewer);
        knappanel.add(VRMLPanel);
        matchPanel.add(MTPanel);
        matchParam.setVisible(false);
        matchPanel.add(MPPanel);        
        WeightPanel=new JPanel();
        AreaWeightPanel=new JPanel();
        OverlapWeightPanel=new JPanel();
        HausdorffWeightPanel=new JPanel();
        LinearWeightPanel=new JPanel();
        AreaWeight= new JSlider(0,100,50);
        AreaWeight.addChangeListener(this);
        OverlapWeight= new JSlider(0,100,50);
        OverlapWeight.addChangeListener(this);
        HausdorffWeight= new JSlider(0,100,50);
        HausdorffWeight.addChangeListener(this);
        LinearWeight= new JSlider(0,100,50);
        LinearWeight.addChangeListener(this);
        AreaWeightPanel.setLayout(new GridLayout(3,0));
        AreaWeightPanel.add(new JLabel("AreaRating"));
        AreaWeightPanel.add(AreaWeight);
        AreaWeightPanel.add(AreaRatingRes);
        
        OverlapWeightPanel.setLayout(new GridLayout(3,0));
        OverlapWeightPanel.add(new JLabel("OverlapRating"));
        OverlapWeightPanel.add(OverlapWeight);
        OverlapWeightPanel.add(OverlapRatingRes);
        
        HausdorffWeightPanel.setLayout(new GridLayout(3,0));
        HausdorffWeightPanel.add(new JLabel("HausdorffRating"));
        HausdorffWeightPanel.add(HausdorffWeight);
        HausdorffWeightPanel.add(HausdorffRatingRes);
        
        LinearWeightPanel.setLayout(new GridLayout(3,0));
        LinearWeightPanel.add(new JLabel("LinearRating"));
        LinearWeight.setMinimumSize(new Dimension(30,1));
        LinearWeightPanel.add(LinearWeight);
        LinearWeightPanel.add(LinearRatingRes);
        
        WeightPanel.setLayout(new GridLayout(0,4));
        WeightPanel.add(AreaWeightPanel);
        WeightPanel.add(OverlapWeightPanel);
        WeightPanel.add(HausdorffWeightPanel);
        WeightPanel.add(LinearWeightPanel);       
        matchPanel.add(WeightPanel);        
        knappanel.add(matchPanel);
        export.addActionListener(this);
        drawUtils.add(export);
        drawUtils.addSeparator();
        JPanel test=new JPanel();
        test.setLayout(new FlowLayout(FlowLayout.RIGHT));
        test.add(new Label("time of second snap:"));
        test.add(timeSpinner);
        drawUtils.add(test);
        tegneomr = new tegnecanvas(this);
        draw.add(tegneomr);
        drawUtils.setRollover(true);
        getContentPane().add(drawUtils,BorderLayout.NORTH);
        tab.addTab("Draw",draw);
        tab.addTab("Draw",draw);
        tab.addTab("Config",knappanel);
        getContentPane().add(tab,BorderLayout.CENTER);
        tegneomr.addComponentListener(this);
        //  tegneomr.setSnapshoot(new Region("/home/java/testCenter",300,200));
    }
    
    public void start()
    {
    }
    
    public void stateChanged(ChangeEvent e)
    {
        if(e.getSource()==matchParam)
        {
            newDraw();
            store();
        }
        
        if(e.getSource()==AreaWeight||e.getSource()==OverlapWeight||e.getSource()==HausdorffWeight||e.getSource()==LinearWeight)
        {
            newDraw();
            store();
        }
        if(e.getSource()==timeSpinner&&result!=null)
        {
            result.setTimeShift(((Integer)timeSpinner.getValue()).intValue());
            resultScroller.setViewportView( result );
        }
    }
    
    public void newDraw()
    {
        if(tab.getTabCount()!=2)
        {
            tab.remove(2);
            tab.remove(2);
            tab.remove(2);
            tab.remove(2);
            tab.remove(2);
        }
        if (wfdisp)
        {
            remove(result);
            wfdisp = false;
        }
    }
    
    public void store()
    {        
        Match match=null;
        if((this.matchType.getSelectedItem()+"").equals("SimpleMatch"))
        {
            match=new SimpleMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot());
        }
        if((this.matchType.getSelectedItem()+"").equals("OverlapMatch"))
        {
            match=new OverlappingMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),matchParam.getValue()/100.0,true);
        }
        if((this.matchType.getSelectedItem()+"").equals("CentroidMatch"))
        {
            match=new CentroidMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),matchParam.getValue()/100.0,true);
        }
        if((this.matchType.getSelectedItem()+"").equals("SteinerPointMatch"))
        {
            match=new SteinerPointMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),matchParam.getValue()/100.0,true);
        }
        if((this.matchType.getSelectedItem()+"").equals("OptimalMatch"))
        {
            match=new OptimalMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),AreaWeight.getValue()/100.0,OverlapWeight.getValue()/100.0,HausdorffWeight.getValue()/100.0,LinearWeight.getValue()/100.0);
            
        }
        this.AreaRatingRes.setText(match.getAreaRating()+"");
        this.OverlapRatingRes.setText(match.getOverlapRating()+"");
        this.HausdorffRatingRes.setText(match.getHausdorffRating()+"");
        this.LinearRatingRes.setText(match.getLinarRating()+"");
        firstS=new ConvexHullTreeViewer(match.getSource());
        secS=new ConvexHullTreeViewer(match.getTarget());
        trirep=new mLineRep(match);
        result = new TriRepOutPutCanvas(trirep,((Integer)timeSpinner.getValue()).intValue());
        matchviewer=new MatchViewer(match);
        tab.addTab("Match",matchviewer);
        resultScroller.setViewportView( result );
        sections=new SectionViewer(trirep);
        tab.addTab("Result",resultScroller);
        tab.addTab("First",firstS);
        tab.addTab("Second",secS);
        tab.addTab("Sections",sections);
        wfdisp = true;        
        ComponentEvent e=new ComponentEvent(tegneomr,ComponentEvent.COMPONENT_RESIZED);
        this.componentResized(e);
    }
    
    public void actionPerformed(ActionEvent e)
    {
        if(TriRepUtil.debugging)
            System.out.println(e);
        if(e.getSource()==matchType)
        {
            if(matchType.getSelectedItem().equals("OptimalMatch"))
            {
                matchParam.setVisible(false);
                this.AreaWeight.setVisible(true);
                this.HausdorffWeight.setVisible(true);
                this.LinearWeight.setVisible(true);
                this.OverlapWeight.setVisible(true);
            }
            else
            {     
                this.AreaWeight.setVisible(false);
                this.HausdorffWeight.setVisible(false);
                this.LinearWeight.setVisible(false);
                this.OverlapWeight.setVisible(false);
                matchParam.setVisible(true);     
            }
            if(tegneomr.isready())
            {
                newDraw();
                store();
            }            
        }
        if (e.getSource() == export)
        {
            String filename;
            filename = toleransefelt.getText();
            String app=vrmlViewer.getText();            
            try
            {
                if(trirep!=null)
                {
                    trirep.saveAsVRML(filename+".vrml",((Integer)timeSpinner.getValue()).intValue());
                    Runtime.getRuntime().exec(app+" "+filename+".vrml");
                }
            }
            catch(IOException ex)
            {
                if(TriRepUtil.debuggingWarnings)
                    System.out.println(ex.getLocalizedMessage());
            }
        }
        this.repaint();
    }
    
    public LineWA[] pointListToLineWA(java.util.Vector pointList)
    {
        LineWA[] res;
        Point p;
        res = new LineWA[pointList.size()];
        for (int a=0;a<res.length;a++)
        {
            p = (Point)pointList.elementAt(a);
            res[a] = new LineWA(p.x, p.y);
        }
        return(res);
    }
}
