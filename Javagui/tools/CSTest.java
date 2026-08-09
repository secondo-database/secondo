/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

package tools;

import java.io.ByteArrayOutputStream;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import sj.lang.MessageListener;

/**
  End-to-end test of the Java client/server interface against a live
  SecondoMonitor.

  The Java interface is compiled by the top level make (target java2) and was
  never tested: no junit jar is vendored and Javagui/unittests is in no
  makefile. The C++ client has ClientServer/TestClientServer and
  Tests/csloadtest; the Java client -- the one the GUI and every JDBC user go
  through -- had nothing, so a protocol change could break it silently until
  someone started the GUI.

  Modelled on tools.ExClient, with the one difference that matters: ExClient
  prints and continues, which is why it cannot be used as a gate. Everything
  here asserts, and a failed assertion is visible in the exit status.

    java tools.CSTest &lt;host&gt; &lt;port&gt; &lt;database&gt; [options]

  Options:
    --expect-binary       require the connection to transfer lists in binary
    --expect-text         require the connection to transfer lists as text
    --expect-truncation   run the truncation assertions *instead of* the
                          ordinary ones; only meaningful against a server
                          started with the fault injection flag, which
                          truncates every streamed result -- see
                          checkTruncation() below

  Exit status: 0 all assertions held, 1 the test could not be run at all
  (usage, or no connection), 2 an assertion failed. Every assertion runs even
  after an earlier one failed, so one run reports everything that is broken.
*/
public class CSTest{

  // Largest result the server puts into one chunk. Only used to size the
  // multi-chunk query below so that it is certainly larger than one chunk;
  // the assertion measures the encoded result rather than trusting this.
  private static final int CHUNK_SIZE = 65536;

  // Tuples in the multi-chunk query. At roughly five bytes per tuple in the
  // binary encoding this is ~150 KB, i.e. several chunks with margin.
  private static final int BIG_TUPLES = 30000;

  private static ESInterface si = null;
  private static int failures = 0;

  private static boolean expectBinaryGiven = false;
  private static boolean expectBinary = false;
  private static boolean expectTruncation = false;

  /** Records the outcome of one assertion. Never throws: the remaining
    * assertions are still worth running, and a test that stops at the first
    * failure reports one bug per run. */
  private static void check(boolean ok, String what){
    if(ok){
      System.out.println("ok   : " + what);
    } else {
      System.out.println("FAIL : " + what);
      failures++;
    }
  }

  /** Result of one command: the interface's four out-parameters, kept
    * together so an assertion can look at all of them. */
  private static class Answer{
    ListExpr result = new ListExpr();
    IntByReference errorCode = new IntByReference();
    IntByReference errorPos = new IntByReference();
    StringBuffer errorMessage = new StringBuffer();
  }

  private static Answer send(String command){
    Answer a = new Answer();
    si.secondo(command, a.result, a.errorCode, a.errorPos, a.errorMessage);
    return a;
  }

  /** Sends a command that has to succeed for the test to make sense at all
    * (opening the database). */
  private static boolean sendRequired(String command){
    Answer a = send(command);
    if(a.errorCode.value != 0){
      System.err.println("command failed: " + command);
      System.err.println("  error " + a.errorCode.value + ": "
                         + a.errorMessage.toString());
      return false;
    }
    return true;
  }

  /**
    Asserts that answer is the relation 1..n, i.e. the result of
    "intstream(1,n) transformstream consume".

    Walking the whole list is the point of this helper. A query result is the
    two element list (type value); for a relation the value is a list of
    tuples, each of them a one element list holding the int. Checking every
    one of them is what proves the list arrived intact -- with binary transfer
    the list is reassembled from socket reads that split wherever they like,
    and "no exception was thrown" would not show that a value went missing in
    the middle.
  */
  private static void checkIntRelation(Answer a, int n, String what){
    if(a.errorCode.value != 0){
      check(false, what + ": error " + a.errorCode.value + " ("
                 + a.errorMessage.toString().trim() + ")");
      return;
    }
    if(a.result.listLength() != 2){
      check(false, what + ": result is not a (type value) pair");
      return;
    }
    ListExpr value = a.result.second();
    if(value.listLength() != n){
      check(false, what + ": expected " + n + " tuples, got "
                 + value.listLength());
      return;
    }
    int expected = 1;
    for(ListExpr rest = value; !rest.isEmpty(); rest = rest.rest()){
      ListExpr tuple = rest.first();
      if(tuple.listLength() != 1
         || tuple.first().atomType() != ListExpr.INT_ATOM){
        check(false, what + ": tuple " + expected + " has an unexpected shape "
                   + tuple.writeListExprToString().trim());
        return;
      }
      if(tuple.first().intValue() != expected){
        check(false, what + ": tuple " + expected + " holds "
                   + tuple.first().intValue());
        return;
      }
      expected++;
    }
    check(true, what + ": " + n + " tuples, values 1.." + n);
  }

  /** Size of the list in the binary encoding the server sends it in. Used to
    * show that a result really was larger than one chunk. */
  private static int binarySize(ListExpr list){
    ByteArrayOutputStream buffer = new ByteArrayOutputStream();
    if(!list.writeBinaryTo(buffer)){
      return -1;
    }
    return buffer.size();
  }

  /*
   1. Handshake.

   The connection is already up when this runs (main exits before here if it
   is not), so what is left to assert is what the two ends agreed on. The
   transfer mode is negotiated -- the server announces it in <SecondoIntro>
   and this side follows -- so a caller that knows how the server was
   configured can require the mode, which is what --expect-binary and
   --expect-text are for. Tests/csloadtest asserts the same thing on the C++
   side with the same two options.
  */
  private static void checkHandshake(){
    check(si.isInitialized(), "handshake: connection initialized");
    System.out.println("     : list transfer is "
                       + (si.usesBinaryLists() ? "binary" : "textual"));
    if(expectBinaryGiven){
      check(si.usesBinaryLists() == expectBinary,
            "handshake: list transfer mode is "
            + (expectBinary ? "binary" : "textual") + " as required");
    }
    // The protocol version is checked in both directions -- the client sends
    // it in <Connect>, the server sends it in <SecondoIntro> -- and a mismatch
    // is a hard refusal on either side. So the real drift protection for
    // SecondoInterface.PROTOCOL_VERSION against csp::PROTOCOL_VERSION in
    // include/CSProtocol.h is not this assertion but the fact that connect()
    // above would have failed outright; a grep over the two files could only
    // compare the literals, not the frame readers behind them. What this pins
    // down is that a version really was negotiated and is the one this build
    // speaks, so the handshake cannot silently become a no-op.
    check(si.getServerProtocolVersion()
            == sj.lang.SecondoInterface.PROTOCOL_VERSION,
          "handshake: protocol version "
          + sj.lang.SecondoInterface.PROTOCOL_VERSION + ", got "
          + si.getServerProtocolVersion());
  }

  /*
   2. Round trip of a small result.
  */
  private static void checkSmallResult(){
    checkIntRelation(send("query ten feed consume"), 10, "small result");
  }

  /*
   3. Round trip of a result larger than one chunk.

   Assertion 2 fits in a single read on any sane socket, so it says nothing
   about how the client behaves when the result does not. This one is built to
   span several chunks, and the size assertion is what keeps it that way: a
   future change to the encoding that made the result smaller would otherwise
   quietly turn this into a second copy of assertion 2.
  */
  private static void checkLargeResult(){
    Answer a = send("query intstream(1," + BIG_TUPLES
                    + ") transformstream consume");
    checkIntRelation(a, BIG_TUPLES, "large result");
    if(a.errorCode.value == 0){
      int size = binarySize(a.result);
      check(size > CHUNK_SIZE,
            "large result: " + size + " bytes encoded, more than one "
            + CHUNK_SIZE + " byte chunk");
    }
  }

  /*
   4. An error is reported rather than swallowed.

   "query nosuchobject" fails in the kernel, before any result exists. The
   interesting property is not that it fails but that the failure arrives as
   an error code *and* a message: an empty message is what a client shows the
   user when the server dropped the reason on the floor.
  */
  private static void checkErrorReported(){
    Answer a = send("query nosuchobject");
    check(a.errorCode.value != 0, "error report: non-zero error code");
    check(a.errorMessage.toString().trim().length() > 0,
          "error report: message is not empty");
  }

  /*
   5. The connection survives an error.

   Reporting an error and then losing the connection is not the same as
   reporting an error, and only a second command on the *same* connection can
   tell the two apart.
  */
  private static void checkConnectionSurvivesError(){
    checkIntRelation(send("query ten feed consume"), 10,
                     "after error: connection still usable");
  }

  /*
   6. A message sent while the command runs reaches the listener.

   count2 sends a message through the message center every hundredth tuple, so
   "query ten feed count2" produces exactly one. Messages travel on the same
   socket as the result, ahead of it, and a client that mishandled them would
   either lose the message or lose its place in the stream -- hence both
   assertions: the listener fired, and the result that followed is still
   correct.
  */
  private static void checkMessageInterleaving(){
    final int[] messages = new int[1];
    MessageListener listener = new MessageListener(){
      public void processMessage(ListExpr message){
        messages[0]++;
      }
    };
    si.addMessageListener(listener);
    try{
      Answer a = send("query ten feed count2");
      check(messages[0] > 0, "message: listener was informed ("
                             + messages[0] + " message(s))");
      if(a.errorCode.value != 0){
        check(false, "message: query failed with error " + a.errorCode.value
                   + " (" + a.errorMessage.toString().trim() + ")");
      } else {
        check(a.result.listLength() == 2
              && a.result.second().atomType() == ListExpr.INT_ATOM
              && a.result.second().intValue() == 10,
              "message: the result after the message is still correct");
      }
    } finally {
      si.removeMessageListener(listener);
    }
  }

  /*
   7. An error detected *after* the result started going out.

   This is the case the result framing exists for, and it cannot be provoked
   by any natural query -- a query either fails before its result is written
   or it does not fail. It needs the server to be started with the fault
   injection flag (Test:TruncateStreamedResult), which makes a streamed
   relation stop after a few tuples and report failure, so this block only
   runs with --expect-truncation.

   The third assertion is the one that distinguishes "closed the hole" from
   "reported it slightly better before dying": the server used to drop the
   connection when it noticed a failure it could no longer report.
  */
  private static void checkTruncation(){
    if(!si.usesBinaryLists()){
      // Only a streamed result can be truncated, and the sink that streams
      // declines in textual mode -- there is no incremental textual writer --
      // so the fault has nothing to hook into. Not a failure of anything.
      System.out.println("     : truncation checks skipped (textual transfer)");
      return;
    }
    // The fault injection also sends a message from inside the write, which is
    // the only way to reach the frame's M record -- a message raised while the
    // result is already going out. Nothing natural produces one, so this is the
    // only coverage that record gets.
    final int[] messages = new int[1];
    MessageListener listener = new MessageListener(){
      public void processMessage(ListExpr message){
        messages[0]++;
      }
    };
    si.addMessageListener(listener);
    Answer a;
    try{
      a = send("query ten feed consume");
    } finally {
      si.removeMessageListener(listener);
    }
    // ERR_RESULT_TRUNCATED, mirrored in include/ErrorCodes.h and
    // sj/lang/ServerErrorCodes.java.
    check(a.errorCode.value == 90,
          "truncation: reported as error 90, got " + a.errorCode.value);
    check(a.errorMessage.toString().trim().length() > 0,
          "truncation: message is not empty");
    check(messages[0] > 0,
          "truncation: the message sent mid-result arrived (" + messages[0]
          + " message(s))");
    Answer b = send("query ten feed count");
    check(b.errorCode.value == 0
          && b.result.listLength() == 2
          && b.result.second().atomType() == ListExpr.INT_ATOM
          && b.result.second().intValue() == 10,
          "truncation: the next command on the same connection succeeds");
  }

  private static void usage(){
    System.err.println("usage: java tools.CSTest <host> <port> <database>"
                       + " [--expect-binary|--expect-text]"
                       + " [--expect-truncation]");
  }

  public static void main(String[] args){
    if(args.length < 3){
      usage();
      System.exit(1);
    }
    String host = args[0];
    int port;
    try{
      port = Integer.parseInt(args[1]);
    } catch(NumberFormatException e){
      System.err.println("invalid port number: " + args[1]);
      System.exit(1);
      return; // unreachable, keeps the compiler happy about port
    }
    String database = args[2];
    for(int i = 3; i < args.length; i++){
      if(args[i].equals("--expect-binary")){
        expectBinaryGiven = true;
        expectBinary = true;
      } else if(args[i].equals("--expect-text")){
        expectBinaryGiven = true;
        expectBinary = false;
      } else if(args[i].equals("--expect-truncation")){
        expectTruncation = true;
      } else {
        System.err.println("unknown option: " + args[i]);
        usage();
        System.exit(1);
      }
    }

    // The large result below is a few hundred thousand list nodes. Without
    // this the persistent list implementation falls back to its minimum cache
    // (100 entries) and spills nearly every node to a temporary file, which
    // turns assertion 3 from a second into minutes.
    ListExpr.initialize(500000);
    // Keep the timing chatter of the interface out of the log; the assertions
    // are the output of this program.
    tools.Environment.MEASURE_TIME = false;

    si = new ESInterface();
    si.setHostname(host);
    si.setPort(port);
    si.setUserName("");
    si.setPassWd("");

    if(!si.connect()){
      System.err.println("could not connect to a Secondo server at "
                         + host + ":" + port);
      System.exit(1);
    }

    try{
      if(!sendRequired("open database " + database)){
        // System.exit does not run the finally block below, so say goodbye
        // here rather than leaving the server to notice a dropped socket.
        si.terminate();
        System.exit(1);
      }

      checkHandshake();
      if(expectTruncation){
        // The fault injection flag is per server, not per query: *every*
        // streamed relation result on this connection stops after a few
        // tuples. So the ordinary assertions cannot run here -- they would all
        // fail, and for the wrong reason. A run against a flagged server tests
        // the abort path and nothing else; the run against the unflagged
        // server is the same test's negative control.
        checkTruncation();
      } else {
        checkSmallResult();
        checkLargeResult();
        checkErrorReported();
        checkConnectionSurvivesError();
        checkMessageInterleaving();
      }

      sendRequired("close database");
    } finally {
      si.terminate();
    }

    if(failures > 0){
      System.out.println(failures + " assertion(s) failed");
      System.exit(2);
    }
    System.out.println("successful");
  }
}
