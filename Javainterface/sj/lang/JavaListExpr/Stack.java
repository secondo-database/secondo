/*
  This class implements a simple stack with the familar operations.
*/

package sj.lang.JavaListExpr;

public class Stack{

public Stack(){
   TopElem = null;
}

public void push(Object o){
   TopElem = new List(o,TopElem);
}

public Object top(){
   return TopElem.content;
}

public Object pop(){
   Object T = TopElem.content;
   TopElem = TopElem.next;
   return T;
}

public boolean isEmpty(){
   return TopElem == null;
}

private List TopElem;


private class List{


List(Object o,List rest){
  content = o;
  next = rest;
}


private Object content;
private List next;


}


}
