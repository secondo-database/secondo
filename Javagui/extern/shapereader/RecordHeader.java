package extern.shapereader;
import extern.numericreader.*;

public class RecordHeader{
  
  public int getRecordNumber(){
     return RecordNumber;
  }

  public int getContentLength(){
     return ContentLength;
  }

  public boolean readFrom(char[] Buffer){
     if(Buffer.length!=8)
        return false;
     RecordNumber = NumericReader.getIntBig(Buffer,0);
     ContentLength = NumericReader.getIntBig(Buffer,4);
     return true;
  }
   
  private int RecordNumber=0;
  private int ContentLength=0;
}
