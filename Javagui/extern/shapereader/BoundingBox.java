package extern.shapereader;


public class BoundingBox{

public BoundingBox(double XMin,double YMin,double XMax,double YMax){
   this.XMin = XMin;
   this.XMax = XMax;
   this.YMin = YMin;
   this.YMax = YMax;
   this.ZMin = this.ZMax = this.MMin = this.MMax = 0.0;
}


private double XMin,XMax,YMin,YMax,ZMin,ZMax,MMin,MMax;
                         


}
