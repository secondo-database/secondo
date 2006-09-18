//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
import jpl.Util;


public class OptimizerServer extends Thread{

static {
    System.loadLibrary( "jpl" );
  }


  public static native int registerSecondo();

   


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
    System.out.println("registerSecondo successful");
    try{
      System.out.println("call JPL.init()");
      JPL.init();
      System.out.println("initialisation successful");

      // VTA - 18.09.2006 
      // I added this piece of code in order to run with newer versions
      // of prolog. Without this code, the libraries (e.g. lists.pl) are
      // not automatically loaded. It seems that something in our code
      // (auxiliary.pl and calloptimizer.pl) prevents them to be
      // automatically loaded. In order to solve this problem I added
      // a call to 'member(x, [x]).' so that the libraries are loaded
      // before running our scripts.
      Term[] args = new Term[2];
      args[0] = new Atom("x");
      args[1] = jpl.Util.termArrayToList( new Term[] { new Atom("x") } );
      Query q = new Query("member",args);
      if(!q.hasSolution()){
         System.out.println("error in the member call'");
         return false;
      }

      args = new Term[1];
      args[0] = new Atom("auxiliary");
      q = new Query("consult",args);
      if(!q.hasSolution()){
         System.out.println("error in loading 'auxiliary.pl'");
         return false;
      }

      args = new Term[1];
      args[0] = new Atom("calloptimizer");
      q = new Query("consult",args);
      if(!q.hasSolution()){
         System.out.println("error in loading 'calloptimizer.pl'");
         return false;
       }
       return true;
     } catch(Exception e){
          System.out.println("Exception in initialization "+e);
          return false;
     }
    }


    /** invokes a prolog predicate
      * all terms in variables must be contained in arguments
      * variables and results can be null if no result is desired
      */
    synchronized private static boolean command(Query pl_query, Vector results){
      if(pl_query==null)
         return false;
      try{
         if(trace){
           System.out.println("execute query: "+pl_query);
         }
         String ret ="";
         int number =0; // the number of solutions
         if(results!=null)
            results.clear();
         while(pl_query.hasMoreSolutions()){
            number++;
            Hashtable solution = pl_query.nextSolution();
            if(results!=null){
               ret = "";
               if(solution.size()<=0){
                   results.add("yes");
               } else{
                  Enumeration vars = solution.keys();
                  int varnum =0;
                  while(vars.hasMoreElements()){
                      Object v =  vars.nextElement();
                      if (varnum>0) ret += " $$$ ";
                      ret += ( v + " = " + solution.get(v));
                  }
                  results.add(ret);
               }
            }
         }
         if(number == 0){
            if(trace)
               System.out.println("no solution found for' "+pl_query.goal() +"/"+pl_query.goal().arity());
            return false;
         } else{
             // check if a new database is opened
             if(pl_query.goal().toString().equals("secondo") & pl_query.goal().arity()>0){
                String first = pl_query.goal().arg(1).toString().trim();
                if(first.startsWith("open ") && first.indexOf(" database ")>0){
                   int lastindex = first.lastIndexOf(" ");
                   openedDatabase = first.substring(lastindex).trim();
                   if(trace){
                      System.out.println("open database "+ openedDatabase);
                      showPrompt();
                   }
                }
             }
             return true;
        }
        } catch(Exception e){
           if(trace)
              System.out.println("exception in calling the "+pl_query.goal()+"-predicate"+e);
           return false;
        }
    }


    /** analyzed the given string and extract the command and the argumentlist
      * <br> then the given command is executed
      * all Solutions are stored into the Vector Solution
      */
    private synchronized static boolean execute(String command,Vector Solution){
          if(Solution!=null)
              Solution.clear();
              
          // clients should not have the possibility
          // to shut down the Optimizer-Server
          command = command.trim();
          if(command.startsWith("halt ") || command.equals("halt"))
             return false;

          Vector TMPVars = new Vector();
          if(trace){
            System.out.println("analyse command: "+command);
          }
          Query pl_query = new Query(command);
          if(pl_query==null){
             if(trace)
                System.out.println("error in parsing command: "+command);
             return  false;
          }

          boolean res = command(pl_query,Solution);

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



    /** return the optimized query
      * if no optimization is found, query is returned
      * otherwise the result will be the best query plan
      */
    private synchronized String optimize(String query){
      try{
          if(trace)
               System.out.println("\n optimization-input : "+query+"\n");
          Term[] args = new Term[2];
          Variable X = new Variable("X");
          args[0] = new Atom(query);
          args[1] = X;
          Query pl_query = new Query("sqlToPlan",args);

          String ret ="";
          int number =0;
          while(pl_query.hasMoreSolutions()){
                number++;
                Hashtable solution = pl_query.nextSolution();
                // ret = ret+" "+solution.get(X);
                ret = ""+solution.get(X);
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
       Term[] arg = new Term[1];
       if(!openedDatabase.equals("")){
          if(trace)
             System.out.println("close the opened database");
          arg[0] = new Atom("close database");
          Query Q = new Query("secondo",arg);
          command(Q,null);
       }
       
       arg[0] = new Atom("open database "+Name);
       Query Q_open = new Query("secondo",arg);
       showPrompt();
       return command(Q_open,null);
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
       e.printStackTrace();
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
       if(args.length<1){
        System.err.println("usage:  java OptimizerServer -classpath .:<jplclasses> OptimizerServer  PortNumber");
              System.exit(1);
       }
       // process options
       Runtime rt = Runtime.getRuntime();
       for(int i=0;i<args.length-1;i++){
           if(args[i].equals("-trace_methods")){
               rt.traceMethodCalls(true); 
               System.out.println("enable method tracing");
           }
           if(args[i].equals("-trace_instructions")){
               rt.traceInstructions(true);
               System.out.println("enable instruction tracing");
           }
       }
       System.out.println("\n\n");
       printLicence(System.out);
       System.out.println("\n");
       String arg = args[args.length-1];
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
               trace=false;
               System.out.println("tracing is deactivated");
           } else if(command.equals("help") | command.equals("?") ){
                System.out.println("quit      : quits the server ");
                System.out.println("clients   : prints out the number of connected clients");
                System.out.println("trace-on  : prints out messages about command, optimized command, open database");
                System.out.println("trace-off : disable messages");
           } else if(command.startsWith("exec")){
             String cmdline = command.substring(4,command.length()).trim();
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
          if(trace)
             e.printStackTrace();
       }
       try{
          Query q = new Query("halt");
          command(q,null); 
       } catch(Exception e){
          System.err.println(" error in shutting down the prolog engine");
       }

   }


   /** Prints out the licence information of this software **/
   public static void printLicence(PrintStream out){
       out.println(" Copyright (C) 2004, University in Hagen, ");
       out.println(" Department of Computer Science,  ");
       out.println(" Database Systems for New Applications. \n");

       out.println(" This is free software; see the source for copying conditions.");
       out.println(" There is NO warranty; not even for MERCHANTABILITY or FITNESS ");
       out.println(" FOR A PARTICULAR PURPOSE.");
   }


   private boolean optimizer_loaded = false;
   private int PortNr = 1235;
   private static int Clients = 0;
   private ServerSocket SS;
   private boolean running;
   private static String openedDatabase ="";
   private static boolean trace = true;
   private static Object SyncObj = new Object();

}




