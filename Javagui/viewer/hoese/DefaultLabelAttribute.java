
package viewer.hoese;

/**
 This class provides a LabelAttribute for displaying simple
 constant values.
**/

public final class DefaultLabelAttribute implements LabelAttribute{
  /** holds the value for this label **/
  private String text;

  /** Creates a new instance from the given string **/
  public DefaultLabelAttribute(String text){
     this.text = text;
  }

  /** creates a new instance displaying an integer number **/
  public DefaultLabelAttribute(int text){
     this.text = "" + text;
  }

  /** method implementing the LabelAttribute interface **/
  public String getLabel(double time){
     return text;
  } 

}
