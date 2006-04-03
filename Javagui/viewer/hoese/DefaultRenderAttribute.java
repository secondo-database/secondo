
package viewer.hoese;

/** 
This class provides a standard RenderAttribute.

**/
public final class DefaultRenderAttribute implements RenderAttribute{
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

  public boolean isDefined(double time){
    return true;
  }

  public boolean canBeDefined(){
    return true;
  }
  
  public double getRenderValue(double time){
     return value;
  }


}
