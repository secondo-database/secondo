package appGuiUtil;


import javax.swing.JCheckBox;
import secondoPostgisUtil.IGlobalParameters;

public class JCheckboxWithObject
  extends JCheckBox
  implements IGlobalParameters
{
  private static final long serialVersionUID = 1L;
  private Object object;
  
  public JCheckboxWithObject(Object object)
  {
    this.object = object;
    setText(object.toString());
  }
  
  public Object getObject()
  {
    return this.object;
  }
  
  public void setObject(Object object)
  {
    this.object = object;
    setText(object.toString());
  }
}
