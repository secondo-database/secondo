import java.io.*;
import java.net.*;
import java.util.*;

import jpl.JPL;
import jpl.Atom;
import jpl.Query;
import jpl.Term;
import jpl.Variable;
import jpl.fli.*;
import jpl.Compound;
import jpl.List;



class PrologParser{

       private class Scanner{
         public Scanner(String i){
	    input = i;
	    nextStart=0;
	    length=input.length();
	 }

         int getNext(){
             int state = 0;
	     int startPos = nextStart;
	     int pos = nextStart;
	     char c=' ';
	     while(pos<length){
                c = input.charAt(pos);
	        switch(state){
		   case(0) :  if(isWhiteSpace(c)){
		                  pos++;
			       }
			       else if(c=='\"'){
                                  startPos=pos;
				  pos++;
				  state=1;
			       }else if( c=='\''){
			          startPos=pos;
				  pos++;
				  state=2;
			       }else if(c=='('){
                                   nextStart=pos+1;
				   value="(";
				   return OPENBRACKET1;
			       }else if(c=='['){
                                   nextStart=pos+1;
				   value="[";
				   return OPENBRACKET2;
			       }else if(c==')'){
                                   nextStart=pos+1;
				   value=")";
				   return CLOSEBRACKET1;
			       }else if(c==']'){
                                   nextStart=pos+1;
				   value="]";
				   return CLOSEBRACKET2;
			       }else if (isUpperCase(c)){
			          startPos=pos;
				  state=3;
				  pos++;
			       }else{
			          startPos=pos;
				  pos++;
                                  state=4;
			       }
			       break;

                   case(1) :  if(c=='\"'){  // string found
                                 nextStart=pos+1;
				 value = input.substring(startPos,pos+1);
				 return ATOM;
		              }else{
			         pos++;
			      }
			      break;
		   case(2) :  if(c=='\''){  // founc a complex atom
		                 nextStart=pos+1;
				 value = input.substring(startPos+1,pos);
				 return ATOM;
    		              } else{
			        pos++;
			      }
			      break;
	           case(3)  : if(isSeparator(c)){
		                  nextStart=pos;
				  value = input.substring(startPos,pos);
				  return VARIABLE;
		              }else{
			         pos++;
			      }
			      break;
                   case(4)  : if(isSeparator(c)){
		                  nextStart=pos;
				  value = input.substring(startPos,pos);
				  return ATOM;
		              }else{
			         pos++;
			      }
			      break;

		}
	     }
	     // end of input
	     nextStart = length;
	     switch(state){
	        case(0) : value ="";
		          return EOI;
	        case(1) : value ="unclosed string literal";
		          return ERROR;
		case(2) : value ="unclosed atom";
		          return ERROR;
	        case(3) : value = input.substring(startPos);
		          return VARIABLE;
	        case(4) : value = input.substring(startPos);
		          return ATOM;
	     }
             value = "unexpected end";
	     return ERROR;

	 }

         /** returns the next token
	   * in the next getNext command the same token is returned
	   * the value is updated
	   */
	 public int preview(){
	    int oldStart = nextStart;
	    int Token = getNext();
	    nextStart = oldStart;
	    return Token;
	 }


	 /** returns true if c is upper case */
	 private boolean isUpperCase(char c){
             return c>='A' && c <='Z';
	 }

	 /** returns true if c is a separator without meaning*/
	 private boolean isWhiteSpace(char c){
	    return c==' ' | c=='\n' | c=='\t'  |   c==',' | c==';' | c==':' | c=='|';
	 }

	 /** returns true if c is a character which indicates the end of a atom or a variable */
	 private boolean isSeparator(char c){
	   return isWhiteSpace(c) | c=='(' | c==')' | c=='[' | c==']' | c=='\"'  | c=='\'';
         }

	 /** return the value of the last getNext or preview command */
	 public  String getValue(){
	     return value;
	 }

	 private String value;
	 private int nextStart;
	 private String input;
	 private int length;

       private int EOI = 0; // end of input
       private int ATOM = 1;
       private int VARIABLE = 2;
       private int OPENBRACKET1 = 3;  // (
       private int OPENBRACKET2 = 4;  // [
       private int CLOSEBRACKET1 = 5; // )
       private int CLOSEBRACKET2 = 6; // ]
       private int ERROR = -1;

     } // class scanner



    /** the method constructs a prolog query from given input
      * if the string is not a valid query, null is returned
      * all
      */
    public Query parse(String Input,Vector Variables){
       if(Variables==null)
          Variables = new Vector();
       Variables.clear();
       pos = 0;
       ErrorMessage="";
       if(Input==null){
          ErrorMessage="null is not allowed as input ";
          return null;
       }
       Scan = new Scanner(Input);
       Token = Scan.getNext();
       if(Token != Scan.ATOM){
          ErrorMessage="prolog command must begin with an  atom";
	  return null;
       }
       Atom Command = new Atom(Scan.getValue());
       int P = Scan.preview();
       Vector V;
       if (P==Scan.OPENBRACKET1){
          P = Scan.getNext(); // read over the bracket
          V = getArgumentList(Variables,Scan.CLOSEBRACKET1);
       }
       /*else if(P==Scan.OPENBRACKET2){
          P = Scan.getNext();
          V = getArgumentList(Variables,Scan.CLOSEBRACKET2);
       }*/
       else
          V = getArgumentList(Variables,Scan.EOI);


       P=Scan.preview();
       if(P!=Scan.EOI){
          ErrorMessage="input after end of command found";
	  return null;
       }
       if(V==null)  // an error is occured
         return null;

       Term[] args= new Term[V.size()];
       for(int i=0;i<V.size();i++)
          args[i] = (Term) V.get(i);
       return new Query(Command,args);
    }


    private Vector getArgumentList(Vector Variables,int EndToken){
        int T = Scan.getNext();
	Vector result = new Vector();
        while(T!=EndToken){ // add the next argument into the Vector

	   if(T==Scan.VARIABLE){
	     NamedVariable Var = new NamedVariable(Scan.getValue());
	     int index = Variables.indexOf(Var);
	     if(index<0){      // a new Variable
	        result.add(Var.getTerm());
		Variables.add(Var);
	     }else{
	        result.add(Variables.get(index));
	     }
	   }else if(T==Scan.ATOM){
	       Atom A = new Atom(Scan.getValue());
               int P = Scan.preview();
	       if(P==Scan.OPENBRACKET1){
                  int dummy = Scan.getNext(); // read the open bracket
		  Vector sublist = getArgumentList(Variables,Scan.CLOSEBRACKET1);
		  if(sublist==null)  // error in parsing sublist
		     return null;
		  Term[] args = new Term[sublist.size()];
		  for(int i=0;i<sublist.size();i++)
		      args[i] = (Term) sublist.get(i);
		  result.add(new Compound(A,args));
	       } else{
	          result.add(A);
	       }

	   }else if(T==Scan.OPENBRACKET2){ // indicating a prolog list
              Vector sublist = getArgumentList(Variables,Scan.CLOSEBRACKET2);
      	      if(sublist==null)  // error in parsing sublist
		 return null;
     	      if(sublist.size()==0)
	         result.add(List.NIL); // found a empty list
	      else{
              List L = new List((Term)sublist.get(sublist.size()-1),List.NIL); // build a list
	      for(int i=sublist.size()-2;i>=0;i--){
		   L = new List( (Term) sublist.get(i),L);
              }
                 result.add(L);
	      }
	    }  else {
	       if(T==Scan.ERROR)
	         ErrorMessage=Scan.getValue();
	       else
	         ErrorMessage="error in building argument list"+T;
	       return null;
         }
         T = Scan.getNext();

	}
	return result;
    }


    // for tests only
    public static void main(String[] args){
      System.out.println("only a test for the prolog parser");
      System.out.println("type in your prolog command and the resulting query is printed out");

      try{
         BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
	 PrologParser P = new PrologParser();
	 String line;
	 Query Q;
	 while(!(line=in.readLine()).equals("exit")){
	    Q = P.parse(line,null);
	    if(Q==null){
	      System.out.println("error in parsing command: "+P.ErrorMessage);
	    }else
	       System.out.println("=> "+Q);
	 }

      }
      catch(Exception e){
        e.printStackTrace();
      }

    }


    private int Token;
    private int pos; // position in input-stream
    private Vector arguments;
    private Scanner Scan;
    public String ErrorMessage="";



}


