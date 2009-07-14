
class Block{
   String file=null;
   String section=null;
   boolean first=false;
   String content=null;

   public String getSectionStart(){
      return"% Section:Start:"+section;
   }
   public String getSectionStartTemplate(){
      return "%\\s*Section:Start:\\s*"+section+"\\s*";
   }
   public String getSectionEnd(){
      return"% Section:End:"+section;
   }
   public String getSectionEndTemplate(){
      return "%\\s*Section:End:\\s*"+section+"\\s*";
   }

}

