import java.io.*;
import java.net.*;
import java.util.*;

import jpl.JPL;
import jpl.Atom;
import jpl.Query;
import jpl.Term;
import jpl.Variable;
import jpl.fli.*;


public class OptimizerServer extends Thread{

   private  int registerSecondo(){
      return jpl.fli.Prolog.registerSecondo();
   }

   /** shows a prompt at the console
     */
    private static void showPrompt(){
       System.out.print("\n opt-server > ");
    }


    /** init  prolog
      * register the secondo predicate
      * loads the optimizer prolog code
      */
    private boolean initialize(){
      // later here is to invoke the init function which
      // registers the Secondo(command,Result) predicate to prolog
    if(registerSecondo()!=0){
       System.err.println("error in registering the secondo predicate ");
       return false;
    }
    try{
      JPL.init();
      Term[] args = new Term[1];
      args[0] = new Atom("auxiliary");
      Query q = new Query("consult",args);
      if(!q.query()){
         System.out.println("error in loading 'auxiliary.pl'");
         return false;
      }
      args[0] = new Atom("calloptimizer");
      q = new Query("consult",args);
      if(!q.query()){
         System.out.println("error in loading 'calloptimizer.pl'");
         return false;
       }
       return true;
     } catch(Exception e){
          System.out.println("Exception in initialization "+e);
          return false;
     }
    }


    /** invokes a prolog predicate */
    synchronized private static boolean command(String cmd, PL_Parameter[] args,Vector results){
      try{

          if(trace){
	     System.out.print("invoke command("+cmd+" , ");
	     for(int i=0;i<args.length;i++)
	        System.out.print(args[i]+" , ");
	     System.out.println("(...)");
	  }
	 Term[] pl_args= new Term[args.length];
	 Vector tmpVars = new Vector(args.length); // use a variable only one time
         PL_Parameter VP;
	 int index;
         for(int i=0;i<args.length;i++){
	     if(!args[i].isVariable()){ // an atom dont't need a special treatment
                pl_args[i] = args[i].getTerm();
	     }else{
                VP = args[i];
		if((index=tmpVars.indexOf(VP))>=0){ // variable already used
                   pl_args[i] = ((PL_Parameter)tmpVars.get(i)).getTerm();
		}else{ // a new variable
                    tmpVars.add(VP);
		    pl_args[i]=VP.getTerm();
		}
	     }
	 }


	 Query pl_query = new Query(new Atom(cmd),pl_args);

	 String ret ="";
         int number =0; // the number of solutions
	 if(results!=null)
	    results.clear();
         while(pl_query.hasMoreSolutions(Prolog.Q_NODEBUG)){
            number++;
	    Hashtable solution = pl_query.nextSolution();
	    if(results!=null){
	       ret = "";
               if(tmpVars.size()==0)
	           results.add("yes");
	       else{
	          for(int i=0;i<tmpVars.size();i++){
	              PL_Parameter P = (PL_Parameter)tmpVars.get(i);
		      if (i>0) ret += " $$$ ";
	              ret += ( P.getName() + " = " + solution.get(P.getVariable()));
	          }
	          results.add(ret);
	       }
	    }
         }
         if(number == 0){
            if(trace)
	       System.out.println("no solution found for' "+cmd +"<"+args.length+">");
	    return false;
         } else
             return true;
	} catch(Exception e){
	   if(trace)
              System.out.println("exception in calling the "+cmd+"-predicate"+e);
	   return false;
	}
    }


    /** analyzed the given string and extract the command and the argumentlist
      * <br> then the given command is executed
      * all Solutions are stored into the Vector Solution
      */
    private static boolean execute(String command,Vector Solution){

       // first split command and arguments
       int Pos = 0;
       int length = command.length();
       char c = ' ';
       String CurString="";
       java.util.Vector V = new java.util.Vector();
       while(Pos<length){
          while( Pos<length && (c = command.charAt(Pos))==' ') // ignore spaces
	      Pos++;
	  if(c=='\"'){
	     CurString+=c;;
	     Pos++; // found a string
             while(Pos<length && (c=command.charAt(Pos))!='\"') {
	        CurString += c;
		Pos++;
	     }
	     Pos++;
	     CurString +=c;
	     V.add(CurString);
	     CurString ="";
	  }else if(c=='\''){
             Pos++; // found a string
             while(Pos<length && (c=command.charAt(Pos))!='\'') {
	        CurString += c;
		Pos++;
	     }
	     Pos++;
	     V.add(CurString);
	     CurString ="";
	  }
	  else{
             while(Pos<length && (c=command.charAt(Pos))!=' ' && c!='"'){
	        CurString +=c;
		Pos++;
	     }
	     V.add(CurString);
	     CurString="";
	  }
       }


       if(V.size()<1){
          if(trace)
	     System.out.println("execute is invoked with a empty command");
	  return false;
       }else{
          String cmd = (String) V.get(0);
	  PL_Parameter[] args = new PL_Parameter[V.size()-1];
	  for(int i=0;i<args.length;i++){
	      args[i]= new PL_Parameter((String)V.get(i+1));

	   }

          boolean res = command(cmd,args,Solution);
	  
	  if(res & trace & Solution!=null){ // successful
             if(Solution.size()==0)
	        System.out.println("Yes");
	     else
	       for(int i=0;i<Solution.size();i++){
                 System.out.println("*** Solution "+(i+1)+"  ****");
                 System.out.println(Solution.get(i));
	       }

	  }
	  return res;
       }

    }



    /** return the optimized query
      * if no optimization is found, query is returned
      * otherwise the result will be the best query plan
      */
    private synchronized String optimize(String query){
      try{
	  if(trace)
	       System.out.println("\n optimization-input : "+query+"\n");
	  Term[] args = new Term[2];
          Variable X = new Variable();
	  args[0] = new Atom(query);
	  args[1] = X;
	  Query pl_query = new Query(new Atom("sqlToPlan"),args);

	  String ret ="";
	  int number =0;
	  while(pl_query.hasMoreSolutions(Prolog.Q_NODEBUG)){
	        number++;
	        Hashtable solution = pl_query.nextSolution();
                ret = ret+" "+solution.get(X);
	  }
	  if(number==0){
	     if(trace)
	         System.out.println("optimization failed - no solution found");
	     return query;
	  }
	  else{
	     if(trace)
	         System.out.println("\n optimization-result : "+ret+"\n");
	     return ret;
	  }
	 } catch(Exception e){
	     if(trace)
	        System.out.println("\n Exception :"+e);
	     showPrompt();
	   return  query;
	 }

    }

    /** if Name is not the database currenty used,
      * the opened database is closed and the database
      * Name is opened
      */
    private synchronized boolean useDatabase(String Name){
       if(openedDatabase.equals(Name))
          return true;
       PL_Parameter[] arg = new PL_Parameter[1];
       if(!openedDatabase.equals("")){
          arg[0] = new PL_Parameter("close database");
          command("secondo",arg,null);
       }
       arg[0] = new PL_Parameter("open database "+Name);
       return command("secondo",arg,null);
   }










   /** a class for communicate with a client */
   private class Server extends Thread{

      /** creates a new Server from given socket */
      public Server(Socket S){
         this.S = S;
         //System.out.println("requesting from client");
         try{
	    in = new BufferedReader(new InputStreamReader(S.getInputStream()));
            out = new BufferedWriter(new OutputStreamWriter(S.getOutputStream()));
	    String First = in.readLine();
	    //System.out.println("receive :"+First);
	    if(First==null){
	       if(trace)
	          System.out.println("connection broken");
	        showPrompt();
		running=false;
		return;
	    }
            if(First.equals("<who>")){
               out.write("<optimizer>\n",0,12);
	       out.flush();
	       running = true;
             }else{
	        if(trace)
                   System.out.println("protocol-error , close connection (expect: <who>, received :"+First);
		showPrompt();
		running = false;
             }
         }catch(Exception e){
             if(trace){
	        System.out.println("Exception occured "+e);
	        e.printStackTrace();
	     }
	     showPrompt();
	     running = false;
         }
      }

      /** disconnect this server from a client */
      private void disconnect(){
        // close Connection if possible
	try{
	   if(S!=null)
	      S.close();
	}catch(Exception e){
	    if(trace){
	       System.out.println("Exception in closing connection "+e);
	       e.printStackTrace();
	    }
	}
        OptimizerServer.Clients--;
	if(trace){
           System.out.println("\nbye client");
	   System.out.println("number of clients is :"+ OptimizerServer.Clients);
	}
	showPrompt();
      }




       /**  processes requests from clients until the client
        *  finish the connection or an error occurs
	*/
      public void run(){
         if(!running){
           disconnect();
	   return;
	 }

	 boolean execFlag=false; // flags for control execute or optimize request
	 try{
             String input = in.readLine();
	     if(input==null){
	        if(trace)
		   System.out.println("connection is broken");
		disconnect();
		return;
	     }
             while(!input.equals("<end connection>")){

	       if(!input.equals("<optimize>") && !input.equals("<execute>") ){ // protocol_error
	           if(trace)
		      System.out.println("protocol error( expect: <optimize> or <execute> , found:"+input);
                   disconnect();
		   return;
	       }
	       execFlag = input.equals("<execute>");
	       // read the database name
	       input = in.readLine();
               if(input==null){
	          if(trace)
		     System.out.println("connection is broken");
		  disconnect();
		  return;
	       }
	       if(!input.equals("<database>")){ // protocol_error
	          if(trace)
		     System.out.println("protocol error( expect: <database> , found:"+input);
                  disconnect();
		  return;
	       }
               String Database = in.readLine();
	       if(Database==null){
	          System.out.println("connection is broken");
		  disconnect();
		  return;
	       }else {
	         Database = Database.trim();
	       }
	       input = in.readLine();
	       if(input==null){
	        if(trace)
		   System.out.println("connection is broken");
		disconnect();
		return;
	       }else{
	         input = input.trim();
	       }
	       if(!input.equals("</database>")){ // protocol error
	           if(trace)
		      System.out.println("protocol error( expect: </database> , found:"+input);
	           disconnect();
		   return;
	       }
               input = in.readLine();
	       if(input==null){
	        if(trace)
		   System.out.println("connection is broken");
		disconnect();
		return;
	        }else{
		  input = input.trim();
		}
	       if(!input.equals("<query>")){ // protocol error
	           if(trace)
 	 	      System.out.println("protocol error( expect: <query> , found:"+input);
 	           disconnect();
		   return;
	       }

	       StringBuffer res = new StringBuffer();
	       // build the query from the next lines
	       input = in.readLine();
	       //System.out.println("receive"+input);
	       if(input==null){
	        if(trace)
		   System.out.println("connection is broken");
		disconnect();
		return;
	       }
	       while(!input.equals("</query>")){
                  res.append(input + " ");
	          input = in.readLine();
		  //System.out.println("receive"+input);
		  if(input==null){
	            if(trace)
		       System.out.println("connection is broken");
		    disconnect();
		    return;
	          }
	       }

	       input = in.readLine();
	       if(input==null){
	          System.out.println("connection is broken");
		  disconnect();
		  return;
	       }

	       if(! (input.equals("</optimize>") & !execFlag)  & !(input.equals("</execute>") & execFlag)){ //protocol-error
	           if(trace)
		     System.out.println("protocol error( expect: </optimize> or </execute>, found:"+input);
                   disconnect();
		   return;
	       }

	       String Request = res.toString().trim();
	       Vector V = new Vector();
	       synchronized(SyncObj){
	          useDatabase(Database);
		  if(!execFlag){
   	              String opt = OptimizerServer.this.optimize(Request);
		      if(!opt.equals(Request))
		         V.add(opt);
	          }else{
		     execute(Request,V);
		  }
               }

	       showPrompt();

	       out.write("<answer>\n",0,9);
               String answer;
	       for(int i=0;i<V.size();i++){
	          answer = (String) V.get(i);
		  out.write(answer+"\n",0,answer.length()+1);
	       }
	       out.write("</answer>\n",0,10);
               out.flush();
               input = in.readLine().trim();
	       if (input==null){
  	          System.out.println("connection broken");
		  showPrompt();
	          disconnect();
	          return;
	       }
             } // while
	     System.out.println("connection ended normaly");
	     showPrompt();
	   }catch(IOException e){
	      System.out.println("error in socket-communication");
	      disconnect();
	      showPrompt();
	      return;
	   }
             disconnect();
         }

      private BufferedReader in;
      private BufferedWriter out;
      private boolean running;
      private Socket S;

    }



   /** creates the server process */
   private boolean createServer(){
     SS=null;
     try{
        SS = new ServerSocket(PortNr);
     } catch(Exception e){
       System.out.println("unable to create a ServerSocket");
       return false;
     }
     return true;
   }

   /**  waits for request from clients
    *   for each new client a new socket communicationis created
    */
   public void run(){
      System.out.println("\nwaiting for requests");
      showPrompt();
      while(running){
       try{
           Socket S = SS.accept();
	   Clients++;
	   if(trace){
	       System.out.println("\na new client is connected");
	       System.out.println("number of clients :"+Clients);
	       showPrompt();
	   }
	   (new Server(S)).start();
	  } catch(Exception e){
         System.out.println("error in communication");
	 showPrompt();
       }
     }

   }



   /** creates a new server object
     * process inputs from the user
     * available commands are
     * client : prints out the number of connected clients
     * quit   : shuts down the server
     */
   public static void main(String[] args){
       if(args.length!=1){
        System.err.println("usage:  java OptimizerServer -classpath .:<jplclasses> OptimizerServer  PortNumber");
	System.exit(1);
       }
       String arg = args[0];
       OptimizerServer OS = new OptimizerServer();
       try{
          int P = Integer.parseInt(arg);
	  if(P<=0){
	    System.err.println("the Portnumber must be greater then zero");
            System.exit(1);
	  }
	  OS.PortNr=P;
       }catch(Exception e){
          System.err.println("the Portnumber must be an integer");
	  System.exit(1);
       }

       try{
         Class.forName("jpl.fli.Prolog"); // ensure to load the jpl library
	} catch(Exception e){
	   System.err.println("loading prolog class failed");
	   System.exit(1);
	}

       if(! OS.initialize()){
          System.out.println("initialization failed");
	  System.exit(1);
       }
       if(!OS.createServer()){
         System.out.println("creating Server failed");
	 System.exit(1);
       }
       OS.running = true;
       OS.start();
       try{
         BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
	 System.out.print("optserver >");
         String command = "";
	 Vector ResVector = new Vector();
         while(!command.equals("quit")){
           command = in.readLine().trim();
	   if(command.equals("clients")){
	      System.out.println("Number of Clients: "+ Clients);
	   }else if(command.equals("quit")){
	      if( Clients > 0){
	         System.out.print("clients exists ! shutdown anyway (y/n) >");
	         String answer = in.readLine().trim().toLowerCase();
	         if(!answer.startsWith("y"))
	             command="";
	      }
	   } else if(command.equals("trace-on")){
	       trace=true;
	       System.out.println("tracing is activated");
	   } else if(command.equals("trace-off")){
	       trace=true;
	       System.out.println("tracing is deactivated");
	   } else if(command.equals("help") | command.equals("?") ){
	        System.out.println("quit      : quits the server ");
		System.out.println("clients   : prints out the number of connected clients");
		System.out.println("trace-on  : prints out messages about command, optimized command, open database");
		System.out.println("trace-off : disable messages");
	   } else if(command.startsWith("exec")){
             String cmdline = command.substring(5,command.length());
	     execute(cmdline,ResVector);
	   }else{
	      System.out.println("unknow command, try help show a list of valid commands");
	   }
           if(!command.equals("quit"))
	      showPrompt();

	 }
	 OS.running = false;
       }catch(Exception e){
          OS.running = false;
	  System.out.println("error in reading commands");
       }
       try{
          JPL.halt();
       } catch(Exception e){
          System.err.println(" error in shutting down the prolog engine");
       }

   }


   private static class PL_Parameter{
      /* creates a new PL_Parameter */
      public PL_Parameter(String Name){
        this.name=Name.trim();
        if(isUpperCase(name.charAt(0)) && !(name.indexOf(' ')>=0)){
	  isVar=true;
	  atomValue=null;
	  varValue = new Variable();
	} else{
	   isVar=false;
	   varValue=null;
	   atomValue= new Atom(name);
	}
      }

      /** returns true if this parameter is a variable */
      public boolean isVariable(){
         return isVar;
      }

      /** returns the atom.Value of this parameter
        * if it's a variable null is returned
	*/
      public Atom getAtom(){
         return atomValue;
      }

      /** returns the variable of this parameter
        * if it's a atom null is returned
	*/
      public Variable getVariable(){
         return varValue;
      }


      /** returns the name of this parameter*/
      public String getName(){
         return name;
      }

      /** returns true if the parameters are equal
        */
      public boolean equals(Object o){
         if(o==null | !(o instanceof PL_Parameter))
	    return false;
	 return name.equals( ((PL_Parameter) o).name);
      }

      /** returns the variable-value if it's a variable,
        * otherwise the atom-value is returned
	*/
      public Term getTerm(){
        if(isVar)
	  return  varValue;
	else
	   return atomValue;
      }

      /** returns true if c in A-Z */
      private boolean isUpperCase(char c){
        return c>='A' && c <='Z';
      }
      
      public String toString(){
         return name;
      }

      private String name;
      private boolean isVar;
      private Variable varValue;
      private Atom atomValue;

   }


   private boolean optimizer_loaded = false;
   private int PortNr = 1235;
   private static int Clients = 0;
   private ServerSocket SS;
   private boolean running;
   private String openedDatabase ="";
   private static boolean trace = true;
   private Object SyncObj = new Object();

}
