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


    private boolean command(String cmd){
      try{
	  Term[] arg = new Term[1];
          arg[0] = new Atom(cmd);
	  Query pl_query = new Query(new Atom("secondo"),arg);
	  String ret ="";
	  int number =0;
	  while(pl_query.hasMoreSolutions(Prolog.Q_NODEBUG)){
	       number++;
	       pl_query.nextSolution();
	  }
	  if(number==0){
	     if(trace){
	        System.out.println("no solution for '"+cmd+"' found");
	     }
	     return false;
	  }
	  else
	     return true;

	 }catch(Exception e){
	    if(trace)
	       System.out.println("exception in calling the secondo-predicate"+e);
	    return false;
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
	        java.util.Hashtable solution = pl_query.nextSolution();
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

	 try{
             String input = in.readLine();
	     if(input==null){
	        if(trace)
		   System.out.println("connection is broken");
		disconnect();
		return;
	     }
             while(!input.equals("<end connection>")){
               if(!input.equals("<optimize>")){ // protocol_error
	           if(trace)
		      System.out.println("protocol error( expect: <optimize> , found:"+input);
                   disconnect();
		   return;
	       }
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
	       if(!openedDatabase.equals(Database)){
	          if(!openedDatabase.equals("")){
		    if(trace)
		        System.out.println("close database "+openedDatabase);
		    if(!command("close database "+openedDatabase)){
                       System.err.println("error in closing database \""+openedDatabase +"\"");
		    }
		  }
		  if(!Database.equals("")){
		    if(trace)
		       System.out.println("open database "+Database);
		    if(!command("open database "+Database)){
		       System.err.println("error in opening database \""+Database+"\"");
		    }else{
		       openedDatabase=Database;
		    }
		  }
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
	       if(!input.equals("</optimize>")){ // protocol error
	           if(trace)
		     System.out.println("protocol error( expect: </optimize> , found:"+input);
                   disconnect();
		   return;
	       }

	       String NoOptimized = res.toString().trim();
	       //System.out.println("query tro optimize :"+NoOptimized);
	       // NoOptimized contains the whole query without any newlines
	       String opt = OptimizerServer.this.optimize(NoOptimized)+"\n";
	       showPrompt();
	       //System.out.println("optimized="+opt);
	       out.write("<answer>\n",0,9);
	       out.write(opt,0,opt.length());
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
      System.out.println("waiting for requests");
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
         while(!command.equals("quit")){
           command = in.readLine().trim().toLowerCase();
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
	   } else{
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



   private boolean optimizer_loaded = false;
   private int PortNr = 1235;
   private static int Clients = 0;
   private ServerSocket SS;
   private boolean running;
   private String openedDatabase ="";
   private static boolean trace = true;

}
