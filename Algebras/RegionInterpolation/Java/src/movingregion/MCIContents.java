package movingregion;
import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.util.*;
import java.io.*;
import java.text.*;

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
    
    public void componentHidden( ComponentEvent e )
    {}
    
    public void componentMoved( ComponentEvent e )
    {}
    
    public void componentResized( ComponentEvent e )
    {
        int heig=e.getComponent().getHeight();
        if(tab.getTabCount()!=2)
        {
            result.setHei(heig);
            firstS.setHei(heig);
            secS.setHei(heig);
            sections.setHei(heig);
            matchviewer.setHei(heig);
        }
        int widt=e.getComponent().getWidth();
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
        String[] matchTypes = { "SimpleMatch", "OverlapMatch", "CentroidMatch","SteinerPointMatch"};
        matchType = new JComboBox();
        for ( String s : matchTypes )
            matchType.addItem( s );
        matchType.addActionListener(this);
        matchParam= new JSlider(0,100,50);
        matchParam.addChangeListener(this);
        resultScroller=new JScrollPane();
        tab=new JTabbedPane(JTabbedPane.BOTTOM);
        heiMod=new SpinnerNumberModel(400,100,1000,10);
        widMod=new SpinnerNumberModel(500,100,1000,10);
        draw=new JPanel();
        drawUtils=new JToolBar();
        wfdisp = false;
        trirep = null;
        setLayout(new BorderLayout());
        knappanel = new JPanel();
        draw.setLayout(new BorderLayout());
        knappanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        export=new JButton("Export (VRML)");
        tekst = new JLabel("VRML Filename:");
        toleransefelt = new JTextField(10);
        vrmlViewerLabel=new JLabel("VRML Application");
        vrmlViewer = new JTextField(10);
        vrmlViewer.setText("dune");
        timeSpinner=new JSpinner(new SpinnerNumberModel(10,1,20,1));
        timeSpinner.addChangeListener(this);
        knappanel.add(tekst);
        knappanel.add(toleransefelt);
        knappanel.add(vrmlViewerLabel);
        knappanel.add(vrmlViewer);
        knappanel.add(matchType);
        knappanel.add(matchParam);
        export.addActionListener(this);
        drawUtils.add(export);
        drawUtils.add(new Label("time of second snapp:"));
        drawUtils.add(timeSpinner);
        tegneomr = new tegnecanvas(this);
        draw.add(tegneomr);
        drawUtils.setRollover(true);
        add(drawUtils,BorderLayout.NORTH);
        tab.addTab("Draw",draw);
        tab.addTab("Draw",draw);
        tab.addTab("Config",knappanel);
        add(tab,BorderLayout.CENTER);
        tegneomr.addComponentListener(this);
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
        firstS=new ConvexHullTreeViewer(tegneomr.getFirstSnapshot());
        secS=new ConvexHullTreeViewer(tegneomr.getSecondSnapshot());
        Match match=null;
        if((this.matchType.getSelectedItem()+"").equals("SimpleMatch"))
        {
            match=new SimpleMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot());;
        }
        if((this.matchType.getSelectedItem()+"").equals("OverlapMatch"))
        {
            match=new OverlappingMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),matchParam.getValue()/100.0);
        }
        if((this.matchType.getSelectedItem()+"").equals("CentroidMatch"))
        {
            match=new CentroidMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),matchParam.getValue()/100.0);
        }
        if((this.matchType.getSelectedItem()+"").equals("SteinerPointMatch"))
        {
            match=new SteinerPointMatch(tegneomr.getFirstSnapshot(),tegneomr.getSecondSnapshot(),matchParam.getValue()/100.0);
        }
        
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
        System.out.println(e);
//        if(e.getSource()==matchType)
//        {
//            if((this.matchType.getSelectedItem()+"").equals("SimpleMatch"))
//            {
//                this.matchParamCenter.setVisible(false);
//                this.matchParamOverlap.setVisible(false);
//            }
//            if((this.matchType.getSelectedItem()+"").equals("OverlapMatch"))
//            {
//                this.matchParamCenter.setVisible(false);
//                this.matchParamOverlap.setVisible(true);
//            }
//            if((this.matchType.getSelectedItem()+"").equals("CentroidMatch"))
//            {
//                this.matchParamCenter.setVisible(true);
//                this.matchParamOverlap.setVisible(false);
//            }
//        }
        if(e.getSource()==matchType)
        {
            newDraw();
            store();
        }
        if (e.getSource() == export)
        {
            String filename;
            filename = toleransefelt.getText();
            String app=vrmlViewer.getText();
            FileOutputStream filestream;
            OutputStreamWriter fs;
            try
            {
                trirep.saveAsVRML(filename+".vrml",((Integer)timeSpinner.getValue()).intValue());
                Runtime.getRuntime().exec(app+" "+filename+".vrml");
            }
            catch(IOException ex)
            {
                System.out.println(ex.getLocalizedMessage());
            }
        }
        this.repaint();
    }
    
    public LineWA[] pointListToLineWA(java.util.Vector pointList)
    {
        LineWA[] result;
        Point p;
        result = new LineWA[pointList.size()];
        for (int a=0;a<result.length;a++)
        {
            p = (Point)pointList.elementAt(a);
            result[a] = new LineWA(p.x, p.y);
        }
        return(result);
    }
}
