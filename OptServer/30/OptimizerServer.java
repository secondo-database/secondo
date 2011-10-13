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
import java.nio.charset.Charset;
import java.util.SortedMap;



public class OptimizerServer extends Thread{

static {
    System.loadLibrary( "jpl" );
    System.loadLibrary( "regSecondo" );
  }


  public static native int registerSecondo();

   


   /** shows a prompt at the console
     */
    private static void showPrompt(){
       cout.print("\n opt-server > ");
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
    //cout.println("registerSecondo successful");
    try{
      JPL.init();
      cout.println("initialisation successful");

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
         cout.println("error in the member call'");
         return false;
      }

      args = new Term[1];
      args[0] = new Atom("auxiliary");
      q = new Query("consult",args);
      if(!q.hasSolution()){
         cout.println("error in loading 'auxiliary.pl'");
         return false;
      }

      args = new Term[1];
      args[0] = new Atom("calloptimizer");
      q = new Query("consult",args);
      if(!q.hasSolution()){
         cout.println("error in loading 'calloptimizer.pl'");
         return false;
       }
       return true;
     } catch(Exception e){
          cout.println("Exception in initialization "+e);
          return false;
     }
    }



    public static boolean halt(){
       try{
          Query q = new Query("halt");
          boolean res = command(q,null);
          return res; 
       } catch(Exception e){
          System.err.println(" error in shutting down the prolog engine");
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
           cout.println("execute query: "+pl_query);
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
               cout.println("no solution found for' "+pl_query.goal() +"/"+pl_query.goal().arity());
            return false;
         } else{
             // check if the used database is changed
             Query dbQuery = new Query("databaseName(X)");
             if(dbQuery.hasMoreSolutions()){
                Hashtable sol = dbQuery.nextSolution();
                Object v = sol.keys().nextElement();
                String name = ""+sol.get(v);
                if(!name.equals(openedDatabase)){
                  if(trace){
                     cout.println("use database "+  name);
                  }
                  openedDatabase=name;
                }

             } else{ // don't close the database

             }
             
             return true;
        }
        } catch(Exception e){
           if(trace)
              cout.println("exception in calling the "+pl_query.goal()+"-predicate"+e);
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
            cout.println("analyse command: "+command);
          }
          Query pl_query = new Query(command);
          if(pl_query==null){
             if(trace)
                cout.println("error in parsing command: "+command);
             return  false;
          }

          boolean res = command(pl_query,Solution);

          if(res & trace & Solution!=null){ // successful
             if(Solution.size()==0)
                cout.println("Yes");
             else
               for(int i=0;i<Solution.size();i++){
                 cout.println("*** Solution "+(i+1)+"  ****");
                 cout.println(Solution.get(i));
               }

          }
          return res;
     }



    /** return the optimized query
      * if no optimization is found, query is returned
      * otherwise the result will be the best query plan
      */
    private synchronized String optimize(String query){
      //cout.println("optimize called with argument \""+query+"\"");
      try{
          if(trace){
               cout.println("\n optimization-input : "+query+"\n");
          }

          Query pl_query = new Query("sqlToPlan('"+query+"', X )");

          String ret ="";
          int number =0;

          while(pl_query.hasMoreSolutions()){
                number++;
                Hashtable solution = pl_query.nextSolution();
                if(solution.size()!=1){
                   if(trace){
                       cout.println("Error: optimization returns more than a single binding");
                   }   
                   return query;
                }
                Enumeration e = solution.keys();
                while(e.hasMoreElements()){
                  ret = "" + solution.get(e.nextElement());
                }
          }
          if(number>1){
             if(trace){
                cout.println("Error: optimization returns more than one solution");
             }
             return query;
          }

          if(number==0){
             if(trace)
                 cout.println("optimization failed - no solution found");
             return query;
          }
          else{ 
             if(trace)
                 cout.println("\n optimization-result : "+ret+"\n");
             
             // free ret from enclosing ''
             ret = ret.trim();
             if(ret.startsWith("'")  && ret.endsWith("'")){
                 if(ret.length()==2){
                   ret = "";
                 } else{
                    ret = ret.substring(1,ret.length()-1);
                 }
             }

             return ret;
          }
         } catch(Exception e){
             if(trace)
                cout.println("\n Exception :"+e);
             showPrompt();
           return  query;
         }

    }

    /** if Name is not the database currenty used,
      * the opened database is closed and the database
      * Name is opened
      */
    private synchronized boolean useDatabase(String Name){
       if(openedDatabase.equals(Name)){
          return true;
       }
       Term[] arg = new Term[1];
       if(!openedDatabase.equals("")){
          if(trace)
             cout.println("close the opened database");
          Query Q = new Query("secondo('close database')");
          command(Q,null);
       }
       
       Query Q_open = new Query("secondo('open database "+Name+"')");
       showPrompt();
       return command(Q_open,null);
   }



   /** a class for communicate with a client */
   private class Server extends Thread{

      /** creates a new Server from given socket */
      public Server(Socket S){
         this.S = S;
         //cout.println("requesting from client");
         try{
            in = new BufferedReader(new InputStreamReader(S.getInputStream()));
            out = new BufferedWriter(new OutputStreamWriter(S.getOutputStream()));
            String First = in.readLine();
            //cout.println("receive :"+First);
            if(First==null){
               if(trace)
                  cout.println("connection broken");
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
                   cout.println("protocol-error , close connection (expect: <who>, received :"+First);
                showPrompt();
                running = false;
             }
         }catch(Exception e){
             if(trace){
                cout.println("Exception occured "+e);
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
               cout.println("Exception in closing connection "+e);
               e.printStackTrace();
            }
        }
        OptimizerServer.Clients--;
        if(trace){
           cout.println("\nbye client");
           cout.println("number of clients is :"+ OptimizerServer.Clients);
        }
        if((OptimizerServer.Clients==0) && OptimizerServer.quitAfterDisconnect){
          // OptimizerServer.halt(); // not allowed from this tread
          System.exit(0);
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
                   cout.println("connection is broken");
                disconnect();
                return;
             }
             while(!input.equals("<end connection>")){

               if(!input.equals("<optimize>") && !input.equals("<execute>") ){ // protocol_error
                   if(trace)
                      cout.println("protocol error( expect: <optimize> or <execute> , found:"+input);
                   disconnect();
                   return;
               }
               //cout.println("receive "+input+" from client");

               execFlag = input.equals("<execute>");
               // read the database name
               input = in.readLine();
               
                //cout.println("receive "+input+" from client");

               if(input==null){
                  if(trace)
                     cout.println("connection is broken");
                  disconnect();
                  return;
               }
               if(!input.equals("<database>")){ // protocol_error
                  if(trace)
                     cout.println("protocol error( expect: <database> , found:"+input);
                  disconnect();
                  return;
               }
               String Database = in.readLine();

               //cout.println("receive "+Database+" from client");

               if(Database==null){
                  cout.println("connection is broken");
                  disconnect();
                  return;
               }else {
                 Database = Database.trim();
               }
               input = in.readLine();

               //cout.println("receive "+input+" from client");

               if(input==null){
                if(trace)
                   cout.println("connection is broken");
                disconnect();
                return;
               }else{
                 input = input.trim();
               }
               if(!input.equals("</database>")){ // protocol error
                   if(trace)
                      cout.println("protocol error( expect: </database> , found:"+input);
                   disconnect();
                   return;
               }
               input = in.readLine();

               //cout.println("receive "+input+" from client");

               if(input==null){
                if(trace)
                   cout.println("connection is broken");
                disconnect();
                return;
                }else{
                  input = input.trim();
                }
               if(!input.equals("<query>")){ // protocol error
                   if(trace)
                        cout.println("protocol error( expect: <query> , found:"+input);
                    disconnect();
                   return;
               }

               StringBuffer res = new StringBuffer();
               // build the query from the next lines
               input = in.readLine();
               
                //cout.println("receive "+input+" from client");

               //cout.println("receive"+input);
               if(input==null){
                if(trace)
                   cout.println("connection is broken");
                disconnect();
                return;
               }
               while(!input.equals("</query>")){
                  res.append(input + "\n");
                  input = in.readLine();
                  //cout.println("receive"+input);
                  if(input==null){
                    if(trace)
                       cout.println("connection is broken");
                    disconnect();
                    return;
                  }
               }

               input = in.readLine();
                 
               //cout.println("receive "+input+" from client");
          
               if(input==null){
                  cout.println("connection is broken");
                  disconnect();
                  return;
               }

               if(! (input.equals("</optimize>") & !execFlag)  & !(input.equals("</execute>") & execFlag)){ //protocol-error
                   if(trace)
                     cout.println("protocol error( expect: </optimize> or </execute>, found:"+input);
                   disconnect();
                   return;
               }

               String Request = res.toString().trim();

               //cout.println("Request is " + Request );

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
               
               //cout.println("receive "+input+" from client");

               if (input==null){
                    cout.println("connection broken");
                  showPrompt();
                  disconnect();
                  return;
               }
             } // while
             cout.println("connection ended normally");
             showPrompt();
           }catch(IOException e){
              cout.println("error in socket-communication");
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
     } catch(java.net.BindException be){
          cout.println("BindException occured");
          cout.println("check if the port "+PortNr+" is already in use");
          return false;
     } catch(Exception e){
       cout.println("unable to create a ServerSocket");
       e.printStackTrace();
       return false;
     }
     return true;
   }

   /**  waits for request from clients
    *   for each new client a new socket communicationis created
    */
   public void run(){
      cout.println("\nwaiting for requests");
      showPrompt();
      while(running){
       try{
           Socket S = SS.accept();
           Clients++;
           if(trace){
               cout.println("\na new client is connected");
               cout.println("number of clients :"+Clients);
               showPrompt();
           }
           (new Server(S)).start();
          } catch(Exception e){
         cout.println("error in communication");
         showPrompt();
       }
     }

   }


  private static void showUsage(){
     cout.println("java -classpath .:<jplclasses> OptimizerServer  PORT [options] ");
     cout.println("<jplclasses> : jar file containing the JPL (prolog) API");
     cout.println("PORT : port for the server");
     cout.println("[Options can be :");
     cout.println("   -autoquit           : exits the server if the last client disconnects");
     cout.println("   -trace_methods      : enables tracing of method calls (for debugging)");
     cout.println("   -trace_instructions : enables tracing of instructions (for debugging)");
     cout.println("   -encoding enc       : switch the output encoding");
  }


   /** creates a new server object
     * process inputs from the user
     * available commands are
     * client : prints out the number of connected clients
     * quit   : shuts down the server
     */
   public static void main(String[] args){
       cout = System.out;
       if(args.length<1){
          showUsage();
          System.exit(1);
       }
       // process options
       Runtime rt = Runtime.getRuntime();
       int pos = 1;
       String console_enc = "utf-8"; // standard
       while(pos<args.length){
           if(args[pos].equals("-trace_methods")){
               rt.traceMethodCalls(true); 
               cout.println("enable method tracing");
               pos++;
           } else if(args[pos].equals("-trace_instructions")){
               rt.traceInstructions(true);
               cout.println("enable instruction tracing");
               pos++;
           } else if(args[pos].equals("-autoquit")){
               quitAfterDisconnect=true;
               cout.println("auto quit enabled");
               pos++;
           } else if(args[pos].equals("-encoding")){
              if(pos+1>=args.length){
                showUsage();
                System.exit(1);
              }
              console_enc = args[pos+1];
              pos++;
              pos++;
           } else {
               cout.println("unknown option " + args[pos]);
               showUsage();
               System.exit(1);
           }
       }

       if(!console_enc.equals("utf-8")){
           SortedMap available = Charset.availableCharsets();
           if(!available.containsKey(console_enc)){
              cout.println("encoding " + console_enc + " unknown");
              cout.println(" Available encodinga are : ");
              cout.println(available.keySet());
              System.exit(1);
           }
           try {
              cout = new PrintStream(System.out, true, console_enc);
           } catch(Exception e){
              cout = System.out;
              cout.println("Problem in changing output encoding, use utg-8");
           }
       } 


       cout.println("\n\n");
       printLicence(cout);
       cout.println("\n");
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
          cout.println("initialization failed");
          System.exit(1);
       }
       if(!OS.createServer()){
         cout.println("creating Server failed");
         System.exit(1);
       }
       OS.running = true;
       OS.start();
       try{
         BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
         cout.print("optserver >");
         String command = "";
         Vector ResVector = new Vector();
         while(!command.equals("quit")){
           command = in.readLine();
           if(command==null){
              command = "";
              continue;
            }
            command = command.trim();
           if(command.equals("clients")){
              cout.println("Number of Clients: "+ Clients);
           }else if(command.equals("quit")){
              if( Clients > 0){
                 cout.print("clients exists ! shutdown anyway (y/n) >");
                 String answer = in.readLine().trim().toLowerCase();
                 if(!answer.startsWith("y"))
                     command="";
              }
           } else if(command.equals("trace-on")){
               trace=true;
               cout.println("tracing is activated");
           } else if(command.equals("trace-off")){
               trace=false;
               cout.println("tracing is deactivated");
           } else if(command.equals("help") | command.equals("?") ){
                cout.println("quit      : quits the server ");
                cout.println("clients   : prints out the number of connected clients");
                cout.println("trace-on  : prints out messages about command, optimized command, open database");
                cout.println("trace-off : disable messages");
           } else if(command.startsWith("exec")){
             String cmdline = command.substring(4,command.length()).trim();
             execute(cmdline,ResVector);
           }else if(!command.equals("")){
              cout.println("unknow command, try help show a list of valid commands");
           }
           if(!command.equals("quit"))
              showPrompt();

         }
         OS.running = false;
       }catch(Exception e){
          OS.running = false;
          cout.println("error in reading commands");
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
       cout.println(" Copyright (C) 2006, University in Hagen, ");
       cout.println(" Faculty of Mathematics and Computer Science,  ");
       cout.println(" Database Systems for New Applications. \n");

       cout.println(" This is free software; see the source for copying conditions.");
       cout.println(" There is NO warranty; not even for MERCHANTABILITY or FITNESS ");
       cout.println(" FOR A PARTICULAR PURPOSE.");
   }


   private boolean optimizer_loaded = false;
   private int PortNr = 1235;
   private static int Clients = 0;
   private static boolean quitAfterDisconnect = false;
   private ServerSocket SS;
   private boolean running;
   private static String openedDatabase ="";
   private static boolean trace = true;
   private static Object SyncObj = new Object();
   private static PrintStream cout;

}




