
package viewer.hoese;

/** 
This class provides a standard RenderAttribute.

**/
public class DefaultRenderAttribute implements RenderAttribute{
  private double value;
  
  public DefaultRenderAttribute(double value){
     this.value = value;
  }
  
  

  public double getMinRenderValue(){
     return value;
  }

  public double getMaxRenderValue(){
     return value;
  }
  
  public double getRenderValue(double time){
     return value;
  }


}
