package tools;

import java.io.*;


public class FileTokenizer{
  public FileTokenizer(File file, String separators){
     this.separators=separators;
   
     if(separators==null || separators.length()==0){
       this.separators=" \n\t\r";
     }
     try{
        in = new BufferedReader(new FileReader(file));
     } catch(IOException e){
        in = null;
     }
  }

 public String nextToken(int maxSize){
     if(in==null){
         return null;
     }
     StringBuffer s = null;
     try{
        while(in.ready()){
            int r = in.read();
            if(r<0){
               in.close();
               in = null;
               return null; 
            }
            char c =(char)( r & 0xffff);
            if(separators.indexOf(c)>=0){
               if(s!=null){
                   return s.toString();
               } // otherwise overread the sign
            } else {
               if(s==null){
                  s = new StringBuffer();
               }
               s.append(c);
               if(maxSize>0 && s.length()>=maxSize){
                  return s.toString();
               }
            }
        }
        in.close();
     } catch(IOException e){
         in = null;
         return null;
     }
     in=null;
     return s!=null?s.toString():null;
  }

  public void close(){
     if(in!=null){
        try{
           in.close();
        } catch(IOException e){}
        in = null;
     }

  }


  private String separators;
  private BufferedReader in;

}

