package tools;

import extern.*;
import sj.lang.ListExpr;

public class xxx2secondo{

private static ImportManager IM;


private static void convertFile(String FileName){

  ListExpr LE;
  System.out.println("convert File: "+FileName);
  LE = IM.importFile(FileName);
  if(LE==null){
     System.err.println("error in converting File: "+FileName);
     return;
  }
  String ObjectName;
  int ExtPos = FileName.lastIndexOf(".");
  if(ExtPos>0)
      ObjectName = FileName.substring(0,ExtPos).trim();
  else
      ObjectName = FileName;
  int PathPos;
  PathPos = Math.max(ObjectName.lastIndexOf("/"),ObjectName.lastIndexOf("\\"));
  if(PathPos>=0)
     ObjectName = ObjectName.substring(PathPos+1);
  // replace not allowed characters
  ObjectName = ObjectName.replace(' ','_');
  ObjectName = ObjectName.replace('-','_');
  ObjectName = ObjectName.replace('+','_');
  ObjectName = ObjectName.replace('.','_');
  ListExpr outList;
  if(LE.listLength()!=2)
      outList=LE;
  else
      outList = ListExpr.sixElemList(ListExpr.symbolAtom("OBJECT"),
                                     ListExpr.symbolAtom(ObjectName),
	 			     ListExpr.theEmptyList(),
				     LE.first(),
				     LE.second(),
				     ListExpr.theEmptyList());

  String outFileName = FileName+".obj";
  if(outList.writeToFile(outFileName)!=0)
     System.err.println("error in writing file "+outFileName);

}


public static void main(String[] args){
  IM = new ImportManager();
  for(int i=0;i<args.length;i++)
      convertFile(args[i]);
}

}
