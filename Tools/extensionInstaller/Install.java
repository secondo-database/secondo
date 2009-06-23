


public class Install{


public static void main(String[] args){
   String secDir = System.getProperty("SECONDO_BUILD_DIR");
   if(secDir==null || secDir.length()==0){
       System.out.println("SECONDO_BUILD_DIR is not defined!");
       System.exit(-1);;
   }
   System.out.println("SECONDO_BUILD_DIR = " + secDir);
   if(args.length<1){
      System.out.println("Missing argument");
      System.exit(-1);
   }
   ExtensionInstaller si = new ExtensionInstaller(secDir);
   si.installExtensions(args);
}

}
