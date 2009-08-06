/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/


import java.io.*;
import java.util.zip.*;
import java.util.Vector;
import java.util.TreeSet;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.StringTokenizer;


/*
   This class will install a given set of extensions to the
   Secondo Extensible Datanbase System.

*/
public class  ExtensionInstaller{


/** The secondo build dir. **/
private File secondoDir=null;

/** Creates a new instance of the Secondo installer. **/
public ExtensionInstaller(String secondoDirectory){
  if(!secondoDirectory.endsWith(File.separator)){
     secondoDirectory += File.separator;
  }
  File f = new File(secondoDirectory);
  if(f.exists()){
    secondoDir = f;
  } else {
    secondoDir = null;
  }
}

/** Installs all algebra contained in the argument **/
private boolean checkAndInstallAlgebras(Vector<ExtensionInfo> infos){
   // collect all Algebras to check conflicts and dependencies
   Vector<AlgebraInfo> algInfos = new Vector<AlgebraInfo>(infos.size());
   for(int i=0; i< infos.size();i++){
       ExtensionInfo info1 = infos.get(i);
       Vector<AlgebraInfo> ainfos = info1.getAlgebraInfos();
       algInfos.addAll(ainfos);
   }

   if(!AlgebraInfo.check(secondoDir.getAbsolutePath(),algInfos)){
       return false;
   }
   for(int i=0;i<infos.size();i++){
       ExtensionInfo info1 = infos.get(i);
       Vector<AlgebraInfo> ainfos = info1.getAlgebraInfos();
       for(int j=0;j<ainfos.size();j++) {
          ainfos.get(j).install(secondoDir.getAbsolutePath(), info1.getFileName()); 
       }
   }
   return true;
}


/** Installs all viewers contained in the argument **/
private boolean checkAndInstallViewers(Vector<ExtensionInfo> infos){
  // collect all ViewerInfos
  Vector<ViewerInfo> viewerInfos = new Vector<ViewerInfo>(infos.size());
  for(int i=0;i<infos.size();i++){
    ExtensionInfo info = infos.get(i);
    Vector<ViewerInfo> vinfos = info.getViewerInfos();
    for(int j=0;j<vinfos.size();j++){
      ViewerInfo vinfo = vinfos.get(j);
      // check whether all files are present
      boolean ok = false;
      ZipFile f = null;
      try{
        f = new ZipFile(info.getFileName());
        ok = vinfo.filesPresent(f);
      } catch(Exception e){
        e.printStackTrace();
        return false;
      } finally { 
      if(f!=null){
         try{f.close();}catch(Exception e){}
       }
      }       if(!ok){
         return false;
      }
      viewerInfos.add(vinfo);
    }
  }
  if(!ViewerInfo.check(secondoDir.getAbsolutePath(), viewerInfos)){
     return false;
  }

  for(int i=0;i<infos.size();i++){
     ExtensionInfo info = infos.get(i);
     Vector<ViewerInfo> vinfos = info.getViewerInfos();
     for(int j=0;j<vinfos.size();j++){
        if(!vinfos.get(j).install(secondoDir.getAbsolutePath(), info.getFileName())){
           return false;
        }
     }
  }
  return true; 
}

/** Installs extensions of the HoeseViewer **/
private boolean checkAndInstallDisplayClasses(Vector<ExtensionInfo> infos){
   Vector<HoeseInfo> hoeseinfos = new Vector<HoeseInfo>();
   for(int i=0;i<infos.size();i++){
      ExtensionInfo info = infos.get(i);
      Vector<HoeseInfo> hinfos = info.getHoeseInfos();
      for(int j=0;j<hinfos.size();j++){
         HoeseInfo hinfo = hinfos.get(j);
         boolean ok = false;
         ZipFile f = null;
         try{
            f = new ZipFile(info.getFileName());
            ok = hinfo.filesPresent(f);
         } catch (Exception e){
            e.printStackTrace();
            return false;
         } finally{
            if(f!=null) {try{f.close();}catch(Exception e){}}
         }
         if(!ok){
           return false;
         }
         hoeseinfos.add(hinfo);
      }
   }
   if(!HoeseInfo.check(secondoDir.getAbsolutePath(), hoeseinfos)){
     return false;
   }
   for( int i=0;i<infos.size();i++){
      ExtensionInfo info = infos.get(i);
      String zip = info.getFileName();
      Vector<HoeseInfo> hinfos = info.getHoeseInfos();
      for(int j=0;j<hinfos.size();j++){
         if(!hinfos.get(j).install(secondoDir.getAbsolutePath(),zip)){
            return false;
         }
      }
   }
   return true;
}

/** Installs extensions of the Optimizer **/
private boolean checkAndInstallOptimizerExtensions(Vector<ExtensionInfo> infos){
   Vector<OptimizerInfo> optinfos = new Vector<OptimizerInfo>();
   for(int i=0;i<infos.size();i++){
      ExtensionInfo info = infos.get(i);
      Vector<OptimizerInfo> oinfos = info.getOptimizerInfos();
      for(int j=0;j<oinfos.size();j++){
        OptimizerInfo oinfo = oinfos.get(j);
        boolean ok = false;
        ZipFile f = null;
        try{
           f = new ZipFile(info.getFileName());
           ok = oinfo.filesPresent(f);
        } catch (Exception e){
           e.printStackTrace();
           return false;
        } finally{
           if(f!=null) {try{f.close();}catch(Exception e){}}
        }
        if(!ok){
          return false;
        }
        optinfos.add(oinfo);
      }
   }
   if(!OptimizerInfo.check(secondoDir.getAbsolutePath(), optinfos)){
     return false;
   }
   for( int i=0;i<infos.size();i++){
      ExtensionInfo info = infos.get(i);
      String zip = info.getFileName();
      Vector<OptimizerInfo> oinfos = info.getOptimizerInfos();
      for(int j=0;j<oinfos.size();j++){
        OptimizerInfo oinfo = oinfos.get(j);
        try{
          if(!oinfo.install(secondoDir.getAbsolutePath(),new ZipFile(info.getFileName()))){
            return false;
          }
        } catch(Exception e){
          e.printStackTrace();
          return false;
        }
     }
   }
   return true;
}

private Vector<ExtensionInfo> readInfos(String[] extensionFiles){
  try{
     // step one: extract the ExtensionInformations
     Vector<ExtensionInfo> infos = new Vector<ExtensionInfo>(extensionFiles.length);
     for(int i=0; i< extensionFiles.length; i++){
        ZipFile zipFile = new ZipFile(extensionFiles[i]);
        Enumeration entries = zipFile.entries();
        boolean found = false;
        while(entries.hasMoreElements() && ! found){
           ZipEntry entry = (ZipEntry) entries.nextElement();
           if(!entry.isDirectory() && entry.getName().equals("SecondoExtension.xml")){
              ExtensionInfo info = new ExtensionInfo(extensionFiles[i],
                                                     zipFile.getInputStream(entry));
              if(!info.isValid()){
                 throw new Exception("Invalid xml file");
              }else{
                infos.add(info);
              }
              found = true;
           }
        }
        if(!found){
          throw new Exception("xml file not found");
        }
        zipFile.close();
     }
     return infos;
  } catch(Exception e){
    e.printStackTrace();
    return null;
  }


}

/** Installs a set of modules. **/
public boolean installExtensions(String[] extensionFiles){
   Vector<ExtensionInfo> infos = readInfos(extensionFiles);
   if(infos==null){
     return false;
   }
   checkAndInstallAlgebras(infos);
   checkAndInstallViewers(infos);
   checkAndInstallDisplayClasses(infos);
   checkAndInstallOptimizerExtensions(infos);
   return true; 
}

private boolean unInstallAlgebras(Vector<ExtensionInfo> infos){
   boolean ok = true;
   for(int i=0;i<infos.size();i++){
      ExtensionInfo extinfo = infos.get(i);
      Vector<AlgebraInfo> alginfos = extinfo.getAlgebraInfos();
      for(int j=0;j<alginfos.size();j++){
         if(!alginfos.get(j).unInstall(secondoDir.getAbsolutePath(), extinfo.getFileName())){
           System.err.println("Problem in unINstalling algebra"+alginfos.get(j).getAlgebraName());
           ok = false;
         }
      }
   }
   return ok;
}

/** unInstalls extsnions **/
public boolean unInstallExtensions(String[] extensionFiles){
  Vector<ExtensionInfo> infos = readInfos(extensionFiles);
  if(infos==null){
    return false;
  }
  unInstallAlgebras(infos);
  System.err.println("uninstalling for Viewer, DisplayClasses, and optimizer not implemented yet");
 // unInstallViewers(infos);
 // unInstallDisplayClasses(infos);
 // unOptimizerExtensions(infos);
  return true; 

}




}

