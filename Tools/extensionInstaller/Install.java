


public class Install{


public static void main(String[] args){
   String secDir = System.getenv("SECONDO_BUILD_DIR");
   if(secDir==null){
       System.out.println("SECONDO_BUILD_DIR is not defined!");
       System.exit(-1);;
   }
   if(args.length<1){
      System.out.println("Missing argument");
      System.exit(-1);
   }
   ExtensionInstaller si = new ExtensionInstaller(secDir);
   si.installExtensions(args);
}

}
