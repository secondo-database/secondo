package appGuiUtil;


import java.awt.Component;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JCheckBox;
import javax.swing.JList;
import javax.swing.ListCellRenderer;
import javax.swing.ListModel;
import javax.swing.UIManager;

public class JCheckBoxList
  extends JList
{
  private static final long serialVersionUID = 1L;
  
  public JCheckBoxList()
  {
    setCellRenderer(new CellRenderer());
    addMouseListener(new MouseAdapter()
    {
      public void mousePressed(MouseEvent e)
      {
        int index = JCheckBoxList.this.locationToIndex(e.getPoint());
        if (index != -1)
        {
          JCheckBox checkbox = (JCheckBox)JCheckBoxList.this.getModel().getElementAt(
            index);
          checkbox.setSelected(!checkbox.isSelected());
          JCheckBoxList.this.repaint();
        }
      }
    });
    setSelectionMode(0);
  }
  
  protected class CellRenderer
    implements ListCellRenderer
  {
    protected CellRenderer() {}
    
    public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus)
    {
      JCheckBox checkbox = (JCheckBox)value;
      if (!isSelected) {
        checkbox.setBackground(UIManager.getColor("List.background"));
      }
      return checkbox;
    }
  }
  
  public void selectAll()
  {
    int size = getModel().getSize();
    for (int i = 0; i < size; i++)
    {
      JCheckBox checkbox = (JCheckboxWithObject)getModel()
        .getElementAt(i);
      checkbox.setSelected(true);
    }
    repaint();
  }
  
  public void deselectAll()
  {
    int size = getModel().getSize();
    for (int i = 0; i < size; i++)
    {
      JCheckBox checkbox = (JCheckboxWithObject)getModel()
        .getElementAt(i);
      checkbox.setSelected(false);
    }
    repaint();
  }
}
