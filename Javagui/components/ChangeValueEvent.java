package components;

public class ChangeValueEvent extends java.awt.AWTEvent{

public ChangeValueEvent(Object source,long value){
   super(source,RESERVED_ID_MAX+1);
   this.value = value;
}


public long getValue(){
   return value;
}

long value;
}
