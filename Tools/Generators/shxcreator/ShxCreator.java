

import java.io.*;


public class ShxCreator{
  private File inf;
  private File outf;


  // auxiliary function
  public static int getInt(byte b){ 
     int res = (int) b;
     if(b<0){
         res = res+256;
     }
     return res;   
  }


  /** returns a Integer from big endian codes byte array*/
  public static int getIntBig(byte[] a,int offset){
    if(a.length-offset <4) return -1; 
    int res = 0;
    for(int i=offset;i<offset+4;i++)
       res = res*256+getInt(a[i]);
    return res;
  }


  /** write the integer to the byte array in big endian order,  
      not: works only for positive numbers */
  public static void writeIntBig(int number, byte[] buffer, int offset){
    number = number<0?-1*number:number;
    for(int i=offset+4-1; i>=offset; i--){
        buffer[i] = (byte) (number % 256);
        number = number / 256;
    } 
  }


 
  public ShxCreator(File inf, File outf){
    this.inf = inf;
    this.outf = outf;
  }


  private void copyHeader(FileInputStream in, FileOutputStream out) throws IOException{
    byte[] buffer = new byte[100];
    if(in.available()<100){
      System.err.println("shape file too short");
      System.exit(0);
    }
    in.read(buffer);
    out.write(buffer); 
  }

  private void copyRecords(FileInputStream in, FileOutputStream out) throws IOException{
     int pos = 100; //  position after reading the header
     byte[] recordHeader = new byte[8];
     while(in.available() > 8){ // at least the record header is present
       in.read(recordHeader);
       int contentlength = getIntBig(recordHeader,4);        
       writeIntBig(pos/2 , recordHeader, 0);
       out.write(recordHeader);
       pos = pos+8 + contentlength*2;
       in.skip(contentlength*2);
     }
  }
  

  private void writeFileLength(File f) throws IOException{
    int length = (int) f.length();
    RandomAccessFile out = new RandomAccessFile(f, "rw");
    out.seek(24);
    byte[] buffer = new byte[4];
    writeIntBig(length/2,buffer,0); // use 16 bit words
    out.write(buffer);
    out.close();
  }


  public void create(){
    try{
      FileInputStream in = new FileInputStream(inf);
      FileOutputStream out = new FileOutputStream(outf);
      copyHeader(in,out);
      copyRecords(in,out);
      in.close();
      out.close();
      writeFileLength(outf);
    } catch(IOException e){
       System.err.println("error in reading or writing file");
    }
  }  


  public static void main(String[] args){


     if(args.length!=1){
       System.err.println("usage java ShxCreator <filename>");
       System.exit(1);
     }
     String infname = args[0];

     if(!infname.toLowerCase().endsWith(".shp")){
        System.err.println("Only shp files are allowed");
        System.exit(0);
     }
     String outfname = infname.substring(0,infname.length()-4) + ".shx";


     File f = new File(infname);
     if(!f.exists()){
       System.err.println("File " + f + " not found");
       System.exit(0);
     }

     File outf = new File(outfname);
     if(outf.exists()){
        System.err.println(outfname + " already exists");
        System.exit(1);
     }
     

     ShxCreator s = new ShxCreator(f, outf);
     s.create(); 
  }

  



}
