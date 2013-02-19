package  viewer.hoese.algebras.raster2;


import java.awt.BorderLayout;
import java.awt.event.*;
import java.awt.Font;
import java.awt.font.*;
import java.awt.geom.*;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import java.awt.*;
import javax.swing.*;
import java.text.AttributedString;
import java.util.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import viewer.hoese.algebras.raster2.*;
import tools.Reporter;


  /**
  *  External window class.
  *  Displays information of a raster object.
  */
public class ExternWindow extends JFrame implements ComponentListener
{
  private JEditorPane textArea = new JEditorPane();
  private JPanel legendArea = new JPanel(new FlowLayout(FlowLayout.LEFT));
  private JButton   closeBtn = new JButton("close");
  private DisplayRaster2 raster = null;
  private JScrollPane scpText;
  private JScrollPane scpLegend;
  private int labelWidth = 1;
  private int labelHeight = 20;
  private int maxNoLegendItems = 200;

  /** creates a new external window **/
  public ExternWindow(){
    super("Raster");
    getContentPane().setLayout(new BorderLayout());
    setSize(500,600);
    getContentPane().add(closeBtn,BorderLayout.SOUTH);
    closeBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
          ExternWindow.this.setVisible(false);
          }
        });
    textArea.setFont(new Font("Monospaced",Font.BOLD,12));
    textArea.setEditable(false); 
    textArea.setContentType("text");
    legendArea.setBackground(Color.WHITE);
    legendArea.addComponentListener(this);
    scpText = new JScrollPane(textArea); 
    scpLegend = new JScrollPane(legendArea);
    scpLegend.setSize(this.getWidth(), 100);
    getContentPane().add(scpText,BorderLayout.NORTH);
    getContentPane().add(scpLegend,BorderLayout.CENTER);    
  }    

  public DisplayRaster2 getRaster(){
    return this.raster;
  }
    
  /**
  * Sets data for text and legend areas.
  */
  public void setRaster(DisplayRaster2 raster){
    this.raster = raster;
    this.labelWidth = raster.getLabelText(raster.getMaxValue()).length()*15;
    
    // set text information
    textArea.setText(raster.getInfo());
    textArea.setCaretPosition(0);
    
    // set legend
    legendArea.removeAll();
    this.fillLegend();
    this.resizeLegend();
  }
  
  /**
  * Generate and add legend items.
  */
  private void fillLegend()
  {
    ColorMap colormap = this.raster.getColorMap();
    
    if (colormap != null)
    {      
      List<Comparable> valueList = colormap.getValuesSorted();
      
      // die Legende soll nicht beliebeig lang werden
      // => maximal maxNoLegendItems Eintraege erzeugen
      int step = 1;
      if (valueList.size() > this.maxNoLegendItems)
      {
        step = valueList.size() / this.maxNoLegendItems;
      }     

      for (int i = 0; i<valueList.size(); i=i+step)
      {
        Comparable val = valueList.get(i);
        
        Color color = colormap.getColorForValue(val);

        String labelText = this.raster.getLabelText(val);
        
        LegendElement legendElem = new LegendElement(labelText, color, this.labelWidth);
        legendArea.add(legendElem);
      }
    }
  
  }
  
  
  /**
  * Fits the legend panel to the number of legend items.
  */
  private void resizeLegend()
  {
    Component[] legendItems = this.legendArea.getComponents();
    int legendElemWidth = this.labelWidth + this.labelHeight + 10 + ((FlowLayout)legendArea.getLayout()).getHgap();
    int legendElemHeight = this.labelHeight + 10 + ((FlowLayout)legendArea.getLayout()).getVgap();
    int elemsInARow = ((Double)this.getSize().getWidth()).intValue() / legendElemWidth;
    int legendHeight = ((legendItems.length / elemsInARow) + 1) * legendElemHeight;
    this.legendArea.setPreferredSize(new Dimension(((Double)this.getSize().getWidth()).intValue(), legendHeight));
    this.legendArea.revalidate();    
  }
  
  /**
  * {@inheritDoc}
  * Needed to resize the legend panel.
  */
  public void componentResized(ComponentEvent e) {
        this.resizeLegend();
        this.repaint();
  }
  
  /**
  * unused
  */
  public void componentHidden(ComponentEvent e) {}
  /**
  * unused
  */
  public void componentMoved(ComponentEvent e) {}
  /**
  * unused
  */
  public void componentShown(ComponentEvent e) {}

 
  
  /**
  * Legend element, consisting of a colored box and a label on a white background.
  */
  private class LegendElement extends JPanel
  {
    private Color color;
    private String labelText;
    private int labelLength;
    private int boxlength = 20;
    private int margin = 5;

    public LegendElement(String pLabelText, Color pColor, int pLabelWidth)
    {
      super();
      setBackground(Color.WHITE);
      this.labelText = pLabelText;
      setToolTipText(pLabelText);
      this.color = pColor;
      this.labelLength = pLabelWidth;
      setPreferredSize(new Dimension(this.labelLength+this.boxlength+2*this.margin, this.boxlength+2*this.margin));
      setLayout(new FlowLayout());
    }
    
    @Override
    public void paintComponent(Graphics g)
    {
      super.paintComponent(g);
      g.setColor(this.color);
      g.fillRect(this.margin, this.margin, this.boxlength, this.boxlength);
      g.setColor(Color.BLACK);
      g.drawRect(this.margin, this.margin, this.boxlength, this.boxlength);
      g.drawString(this.labelText, this.boxlength+2*this.margin, this.boxlength+this.margin);
    }
  }
  
}
