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

public class OptTest {

    static {
        System.loadLibrary("jpl");
        System.loadLibrary("regOptTest");
    }

    public static native int registerSecondo();

    private String openedDatabase = "";
    private String OS_pl_encoding = null;

    static final int UNDEFINED = 0;
    static final int SETUP = 1;
    static final int TESTCASE = 2;
    static final int TEARDOWN = 3;

    // multiline commands
    static final int APPENDTOCOMMAND = 4;

    // special states for testcases
    static final int AWAITINGYIELD = 1;
    static final int AWAITINGCOMMAND = 2;

    static final int NEXTCOMMAND = 5;

    // flags for result comparison
    static final int NOCOMPARE = 2;
    static final int STRINGCOMPARE = 3;
    static final int SQLRESULTCOMPARE = 4;
    static final int OPTCOMPARE = 5;

    String resultfilename = "result.log";
    String auxiliarydest = "../../Optimizer/auxiliary";
    String calloptimizerdest = "../../Optimizer/calloptimizer";

    // globale Variables to have mutliple returnvalues
    String Solution = "";
    Hashtable[] solutions;
    String summary = "\n\nThe result of the testfile : ";

    /* 
    ~parseTestfile~ takes a filename String which should be a testfile. It is opened
    and processes it line
    by line.
     
    */

    private boolean parseTestfile(String testfilename) {
        summary += testfilename;
        summary += "\nThe following checks failed : \n";
        String testresults = "";
        String yield = "";
        String testfile = readTestfile(testfilename);
        if (testfile.isEmpty()) {
            System.out
                    .println("Error : Testfile " + testfilename + " is empty");
            return false;
        }

        // split does remove emptylines
        String linebuffer[] = testfile.split("[\\r\\n]+");
        int numberoflines = linebuffer.length;

        System.out.println("Parsing " + numberoflines + " lines");
        int state = UNDEFINED;
        int testcasestate = UNDEFINED;
        int setupstate = UNDEFINED;
        String currentline = "";
        String optcommand = "";
        String testcasename = "";
        int yieldflag = 0;

        for (int lineno = 0; lineno < numberoflines; lineno++) {
            currentline = linebuffer[lineno];
            // filter out comments
            if (currentline.startsWith("# ") || currentline.equals("#")
                    || currentline.trim().isEmpty()) {
                // System.out.println("ignoring Comment");
            } else {
                switch (state) {
                case UNDEFINED:
                    if (currentline.startsWith("#setup")) {
                        state = SETUP;
                        if (currentline.trim().length() > 6) {
                            String testfilecomment = currentline.substring(6);
                            if (testfilecomment.length() > 0) {
                                System.out.println("Testfile name : "
                                        + testfilename);
                                System.out.println("Testfile description : "
                                        + testfilecomment.trim().substring(
                                                testfilecomment.indexOf(" ")));
                            }
                        }
                    } else {
                        System.out
                                .println("Error: Expected #setup directive here");
                        return false;
                    }
                    break;
                case SETUP:
                    if (currentline.startsWith("#testcase")) {
                        //System.out.println("INFO : changing state to TESTCASE");
                        state = TESTCASE;
                        testcasestate = AWAITINGYIELD;
                        System.out.println("Testcasename : "
                                + currentline.trim().substring(
                                        currentline.indexOf(" ")));
                        testcasename = currentline.substring(currentline
                                .indexOf(" "));
                    } else {
                        if (currentline.startsWith("#setup")) {
                            System.out
                                    .println("Error : Already in SETUP state in line : "
                                            + (lineno + 1));
                            return false;
                        }
                        if (currentline.startsWith("#teardown")) {
                            System.out
                                    .println("Error : Testfile changing from SETUP state to TEARDOWN is useless in line :"
                                            + (lineno + 1) + ".");
                            return false;
                        }
                        switch (setupstate) {
                        case UNDEFINED:
                            if (currentline.endsWith(";")) {
                                optcommand += currentline.substring(0,
                                        currentline.length() - 1);
                                if (!execute(optcommand)) {
                                    System.out
                                            .println("Error: setup failed while executing :\n"
                                                    + optcommand);
                                    return false;
                                }
                                // reset optcommand
                                optcommand = "";
                            } else {
                                optcommand += currentline + " ";
                                //System.out.println("INFO : changing state to APPENDTOCOMMAND");
                                setupstate = APPENDTOCOMMAND;
                            }
                            break;
                        case APPENDTOCOMMAND:
                            if (currentline.endsWith(";")) {
                                optcommand += currentline;
                                if (!execute(optcommand)) {
                                    System.out
                                            .println("Error: setup failed while executing :\n"
                                                    + optcommand);
                                    return false;
                                }
                                optcommand = "";
                                break;
                            } else {
                                optcommand += currentline + " ";
                            }
                        }
                    }
                    break;
                case TESTCASE:
                    /*
                     * example testcase: 
                     * #testcase <testcasename>
                     * #yields <compareflag> <answer>|@filename
                     * <optimizer command>
                     */

                    // catch statechange errors first
                    if (currentline.startsWith("#setup")) {
                        System.out
                                .println("Error : Testfile changing from TESTCASE state to SETUP which is not allowed in line : "
                                        + (lineno + 1) + ".");
                        return false;
                    }
                    // again statebased		    
                    switch (testcasestate) {
                    case UNDEFINED:
                        System.out
                                .println("Error : testcasestate cannot be undefined in line "
                                        + (lineno + 1) + ".");
                        return false;
                    case AWAITINGYIELD:
                        if (currentline.startsWith("#yields")) {
                            if (currentline.trim().length() > 7) {
                                // gets yieldflag
                                yield = currentline.substring(7).trim();
                                if (yield.indexOf(" ") != -1) {
                                    yieldflag = Integer.parseInt(yield
                                            .substring(0, yield.indexOf(" ")));
                                    yield = yield.substring(yield.indexOf(" "),
                                            yield.length()).trim();
                                    // if there is yield make checks else pass
                                    if (yield.length() > 0) {
                                        if (yield.startsWith("@")) {
                                            String yieldfilename = yield
                                                    .substring(1);
                                            System.out.println("Reading file "
                                                    + yieldfilename);
                                            yield = readTestfile(yieldfilename);
                                        }
                                    }
                                } else {
                                    yieldflag = STRINGCOMPARE;
                                }

                            } else {
                                yieldflag = NOCOMPARE;
                                yield = "";
                            }
                            //System.out.println("INFO : changing teststate to AWAITINGCOMMAND");
                            testcasestate = AWAITINGCOMMAND;
                        } else {
                            System.out
                                    .println("Error : Missing #yields directive in line : "
                                            + (lineno + 1) + ".");
                            return false;
                        }
                        break;
                    case AWAITINGCOMMAND:
                        if (currentline.startsWith("#")) {
                            System.out
                                    .println("Error : Directives not allowed, awaiting optimizer command: "
                                            + yield
                                            + " in line :"
                                            + (lineno + 1) + ".");
                        }
                        if (currentline.endsWith(";")) {
                            optcommand += currentline.substring(0,
                                    currentline.length() - 1);
                            if (!executeTest(optcommand, yield, yieldflag)) {
                                System.out
                                        .println("Error: Testcasecommand failed while executing :\n"
                                                + optcommand);
                                return false;
                            }
                            optcommand = "";
                            //System.out.println("INFO : changing teststate to NEXTCOMMAND");
                            testcasestate = NEXTCOMMAND;
                        } else {
                            optcommand = currentline;
                            //System.out.println("INFO : changing teststate to APPENDTOCOMMAND");
                            testcasestate = APPENDTOCOMMAND;
                        }
                        break;
                    case APPENDTOCOMMAND:
                        if (currentline.startsWith("#")) {
                            System.out
                                    .println("Error : Directives or comments are not allowed, was awaiting something to append to "
                                            + optcommand
                                            + " in line :"
                                            + (lineno + 1) + ".");
                        }
                        if (currentline.endsWith(";")) {
                            optcommand += currentline.substring(0,
                                    currentline.length() - 1);
                            if (!executeTest(optcommand, yield, yieldflag)) {
                                System.out
                                        .println("Error: Testcasecommand failed while executing :\n"
                                                + optcommand);
                                return false;
                            }
                            optcommand = "";
                            //System.out.println("INFO : changing teststate to NEXTCOMMAND");
                            testcasestate = NEXTCOMMAND;
                        } else {
                            optcommand += currentline + " ";
                            //System.out.println("INFO : changing teststate to APPENDTOCOMMAND");
                            testcasestate = APPENDTOCOMMAND;
                        }

                        break;
                    case NEXTCOMMAND:
                        optcommand = "";
                        if (currentline.startsWith("#testcase")) {
                            testcasename = currentline.substring(currentline
                                    .indexOf(" "));
                            //System.out.println("Info : Changing state to TESTCASE.");
                            testcasestate = AWAITINGYIELD;
                            System.out.println("Testcasename : "
                                    + currentline.trim().substring(
                                            currentline.indexOf(" ")));
                        } else {
                            if (currentline.startsWith("#setup")) {
                                System.out
                                        .println("Error : Already in SETUP state in line : "
                                                + (lineno + 1));
                                return false;
                            }
                            if (currentline.startsWith("#teardown")) {
                                //System.out.println("INFO : changing state to TEARDOWN");
                                state = TEARDOWN;
                            }
                        }
                        break;
                    }
                    break;
                case TEARDOWN:
                    if (currentline.startsWith("#setup")) {
                        System.out
                                .println("Error : Testfile changing from TEARDOWN state to SETUP which is not allowed in line : "
                                        + lineno + ".");
                        return false;
                    }
                    if (currentline.startsWith("#testcase")) {
                        System.out
                                .println("Error : Testfile changing from TEARDOWN state to TESTCASE which is not allowed in line : "
                                        + lineno + ".");
                        return false;
                    }
                    if (currentline.endsWith(";")) {
                        optcommand += currentline.substring(0,
                                currentline.length() - 1);
                        if (!execute(optcommand)) {
                            System.out
                                    .println("Error: setup failed while executing :\n"
                                            + optcommand);
                            return false;
                        }
                        optcommand = "";
                    } else {
                        optcommand += currentline + " ";
                    }
                    break;
                }
            }
        }
        System.out.println(summary);
        if (state == TESTCASE || state == TEARDOWN)
            return true;
        else {
            switch (state) {
            case SETUP:
                System.out
                        .println("testfile execution ended in illegal state SETUP");
                return false;
            case UNDEFINED:
                System.out
                        .println("testfile execution ended in illegal state UNDEFINED");
                return false;
            }
        }
        return false;
    }

    /* 
     * yield from testfile or @resultfile
     * 
     */
    private boolean executeTest(String optcommand, String yield, int flag) {
        String result = "";
        if (execute(optcommand)) {
            if (solutions.length >= 0) {
                for (int i = 0; i < solutions.length; i++) {
                    Hashtable sol = solutions[i];
                    if (sol.size() <= 0) {
                    } else {
                        Enumeration vars = solutions[i].keys();
                        int varnum = 0;
                        while (vars.hasMoreElements()) {
                            Object v = vars.nextElement();
                            result += (v + " = " + solutions[i].get(v));
                        }
                        result += "\n";
                    }
                }
                // if solution is "" then compare to Solution
                if (result.equals("")) {
                    //System.out.println("SOLUTION WRITE OUTPUT:" + Solution);
                    if (!compareResult(Solution, yield, flag)) {
                        summary += "Results from " + optcommand
                                + " did not match!!!\n";
                    }
                } else {
                    //System.out.println("SOLUTION UNIFIED VARS:" + result);
                    if (!compareResult(result, yield, flag)) {
                        summary += "Results from " + optcommand
                                + " did not match!!!\n";
                    }
                }
            } else {
                // compare to false
                if (compareResult("false", yield, flag)) {
                    summary += "Results from " + optcommand
                            + " did not match!!!\n";
                }
                return true;
            }
            return true;
        } else {
            return false;
        }
    }

    private String readTestfile(String filename) {
        String line;
        try {
            // Open the file that is the first
            // command line parameter
            FileInputStream fstream = new FileInputStream(filename);
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));

            // read file
            String file = "";
            while ((line = br.readLine()) != null) {
                // Print the content on the console
                // System.out.println(line);
                file += line + "\n";
            }
            // close file
            in.close();
            // parsing the input as a whole takes place in a separate method
            return file;
        } catch (Exception e) {
            System.out.println("could not open or read testfile : " + filename);
            e.printStackTrace();
            return "";
        }
    }

    private boolean compareResult(String result, String yield, int flag) {
        /*
        System.out.println("CompareResult flag : " + flag);
        System.out.println("___________________________________________");
        System.out.println("Comparing result:\n" + result);
        System.out.println("___________________________________________");
        System.out.println("Comparing yield:\n" + yield);
        System.out.println("++++++++++++++++++++++++++++++++++++++++++++");
        */

        //force default heavior if none was set
        if (flag == UNDEFINED) {
            flag = STRINGCOMPARE;
        }
        switch (flag) {
        case NOCOMPARE:
            return true;
            // compare incoming results char by char
        case STRINGCOMPARE:
            if (result.trim().contentEquals(yield.trim())) {
                System.out.println("Info : results do match");
                return true;
            } else {
                summary += "\n-------------- result : ---------------------\n";
                summary += result;
                summary += "\n-------------- given yield : ---------------------\n";
                summary += yield;
                System.out.println("Error : results do NOT match");
                return false;
            }
            /* in SQLRESULTCOMPARE yield is compared starting from
               "Command succeeded, result:"*/
        case SQLRESULTCOMPARE:
            if (result.length() > 0 && yield.length() > 0) {
                if (result.indexOf("Command succeeded, result:") != -1) {
                    result = result.trim().substring(
                            result.indexOf("Command succeeded, result:"));
                }
                if (yield.indexOf("Command succeeded, result:") != -1) {
                    yield = yield.trim().substring(
                            yield.indexOf("Command succeeded, result:"));
                }
                if (result.trim().contentEquals(yield.trim())) {
                    System.out.println("Info : results do match");
                    return true;
                } else {
                    System.out.println("Error : results do NOT match");
                    summary += "\n-------------- result : ---------------------\n";
                    summary += result;
                    summary += "\n-------------- given yield : ---------------------\n";
                    summary += yield;
                    return false;
                }
            } else {
                System.out.println("Error : results do NOT match");
                return false;
            }

            // in OPTCOMPARE all the selectivities are being removed
        case OPTCOMPARE:
            String pattern = "\\{.*\\}";
            result = result.replaceAll(pattern, "");
            yield = result.replaceAll(pattern, "");
            if (result.trim().contentEquals(yield.trim())) {
                System.out.println("Info : results do match");
                return true;
            } else {
                System.out.println("Error : results do NOT match");
                summary += "\n-------------- result : ---------------------\n";
                summary += result;
                summary += "\n-------------- given yield : ---------------------\n";
                summary += yield;
                summary += "\n-----------------------------------------------------------\n";
                return false;
            }
        }
        System.out.print("Error check failed somehow");
        return false;
    }

    private boolean createTestfile(String filename) {
        String commandlist = readTestfile(filename);
        if (commandlist.isEmpty()) {
            System.out.println("Error : Testfile " + commandlist + " is empty");
            return false;
        }
        // split does remove emptylines
        String linebuffer[] = commandlist.split("[\\r\\n]+");
        int numberoflines = linebuffer.length;
        String currentline = "";
        int state = SETUP;

        System.out.println("analyzing " + numberoflines + " lines from "
                + filename);

        String resultfile = "";
        String testname = "";
        if (filename.indexOf(".") != -1) {
            testname = filename.substring(0, filename.indexOf("."));
        } else {
            testname = filename;
        }
        String testfile = "# This is a by OptTest automatically generated testfile.\n"
                + "# It was generated from file: "
                + filename
                + "\n"
                + "#setup " + testname + "\n";

        int testno = 0;
        String optcommand = "";
        String description = "";
        for (int lineno = 0; lineno < numberoflines; lineno++) {
            currentline = linebuffer[lineno];
            if (currentline.startsWith("# ") || currentline.equals("#")
                    || currentline.trim().isEmpty()) {
                // System.out.println("ignoring Comment");
            } else {
                switch (state) {
                case SETUP:
                    if (currentline.trim().startsWith("#testcase")) {
                        state = TESTCASE;
                        //System.out.println("changing state to TESTCASE");
                    } else {
                        if (currentline.endsWith(";")) {
                            optcommand += currentline.substring(0,
                                    currentline.length() - 1);
                            if (execute(optcommand)) {
                                //DisplaySolutions();
                                testfile += optcommand + ";\n";
                                optcommand = "";
                            } else {
                                System.out
                                        .println("Error : setup section failed due to error ");
                                return false;
                            }
                        } else {
                            optcommand += currentline + " ";
                        }
                    }
                    break;
                case TESTCASE:
                    if (currentline.startsWith("#teardown")) {
                        testfile += "#teardown\n";
                        state = TEARDOWN;
                    } else {
                        if (currentline.endsWith(";")) {
                            optcommand += currentline.substring(0,
                                    currentline.length() - 1)
                                    + "\n";
                            if (execute(optcommand)) {
                                resultfile = "";
                                testno++;
                                testfile += "#testcase No" + testno + " "
                                        + description + "\n";

                                String result = "";
                                if (solutions.length > 0) {
                                    for (int i = 0; i < solutions.length; i++) {
                                        Hashtable sol = solutions[i];
                                        if (sol.size() <= 0) {

                                        } else {
                                            Enumeration vars = solutions[i]
                                                    .keys();
                                            int varnum = 0;
                                            while (vars.hasMoreElements()) {
                                                Object v = vars.nextElement();
                                                result += (v + " = " + solutions[i]
                                                        .get(v));
                                            }
                                            result += "\n";
                                        }

                                    }
                                    if (result.equals("")) {
                                        result = Solution;
                                    } else {
                                        System.out.println("solutions : "
                                                + result);
                                    }

                                    int comparemode = STRINGCOMPARE;
                                    if (optcommand.trim().startsWith("sql select")) {
                                        comparemode = SQLRESULTCOMPARE;
                                    }
                                    if (optcommand.trim()
                                            .startsWith("optimize")) {
                                        comparemode = OPTCOMPARE;
                                    }
                                    if (result.length() > 70) {
                                        resultfile = result;
                                        String rfilename = testname + "_No"
                                                + testno + ".result";
                                        // testcase distinction

                                        testfile += "#yields " + comparemode
                                                + " @" + rfilename + "\n";
                                        try {
                                            FileWriter rstream = new FileWriter(
                                                    rfilename);
                                            BufferedWriter rout = new BufferedWriter(
                                                    rstream);
                                            rout.write(resultfile);
                                            //Close the output stream
                                            rout.close();
                                        } catch (IOException e) {

                                        }

                                    } else {
                                        testfile += "#yields " + comparemode
                                                + " " + result;
                                    }

                                } else {
                                    testfile += "#yields false\n";
                                }
                                testfile += optcommand.substring(0,
                                        optcommand.length() - 1)
                                        + ";\n\n";
                                optcommand = "";
                                description = "";

                            } else {
                                System.out
                                        .println("Error : creating testfile failed while executing : "
                                                + optcommand);
                                return false;
                            }

                        } else {
                            if (currentline.startsWith("% ")
                                    || currentline.startsWith("# ")) {
                                testfile += "# "
                                        + currentline.substring(1).trim()
                                        + "\n";
                            } else {
                                optcommand += currentline + "\n";
                            }
                        }
                    }
                    break;
                case TEARDOWN:
                    if (currentline.endsWith(";")) {
                        optcommand += currentline.substring(0,
                                currentline.length() - 1);
                        if (execute(optcommand)) {
                            //DisplaySolutions();
                            testfile += optcommand + ";\n";
                            optcommand = "";
                        } else {
                            System.out
                                    .println("Error : setup section failed due to error ");
                            return false;
                        }
                    } else {
                        optcommand += currentline + " ";
                    }
                    break;
                }
            }
        }
        try {
            String tfilename = filename.substring(0, filename.indexOf("."))
                    + ".test";
            FileWriter tstream = new FileWriter(tfilename);

            BufferedWriter tout = new BufferedWriter(tstream);
            tout.write(testfile);
            //Close the output stream
            tout.close();
            return true;
        } catch (IOException e) {
            System.out.println(e.getMessage());
            return false;
        }

    }

    /**
     * init prolog register the secondo predicate loads the optimizer prolog
     * code
     */
    private boolean initialize() {
        // later here is to invoke the init function which
        // registers the Secondo(command,Result) predicate to prolog
        if (registerSecondo() != 0) {
            System.err.println("error in registering the secondo predicate ");
            return false;
        }
        // cout.println("registerSecondo successful");
        try {

            String[] plargs = { "-L256M", "-G256M" };
            boolean ok = JPL.init(plargs);

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
            args[1] = jpl.Util.termArrayToList(new Term[] { new Atom("x") });
            Query q = new Query("member", args);
            if (!q.hasSolution()) {
                System.out.println("error in the member call'");
                return false;
            }

            args = new Term[1];
            args[0] = new Atom(auxiliarydest);
            q = new Query("consult", args);
            if (!q.hasSolution()) {
                System.out.println("error in loading 'auxiliary.pl'");
                return false;
            }

            args = new Term[1];
            args[0] = new Atom(calloptimizerdest);
            q = new Query("consult", args);
            if (!q.hasSolution()) {
                System.out.println("error in loading 'calloptimizer.pl'");
                return false;
            }

            q = new Query("set_prolog_flag(debug_on_error, false)");
            if (!q.hasSolution()) {
                System.out.println("error setting debug_on_error to false");
                return false;
            }
            return true;
        } catch (Exception e) {
            System.out.println("Exception in initialization " + e);
            return false;
        }
    }

    /**
     * analyzed the given string and extract the command and the argumentlist <br>
     * then the given command is executed all Solutions are stored into the
     * Vector Solution
     */
    private boolean execute(String command) {
        if (Solution != "") {
            Solution = "";
        }
        if (solutions != null) {
            solutions = null;
        }
        command = command.trim();
        System.out.println("Optimizer command: " + command);
        command = "tell('" + resultfilename + "')," + command + ",told";
        command = encode(command);
        //System.out.println("Command to execute : " + command);
        try {
            Query pl_query = new Query(command);
            if (pl_query == null) {
                System.out.println("Error : error in parsing command: ");
                return false;
            }
            solutions = pl_query.allSolutions();
            /* debug */
            //System.out.println("Number of solutions : " + solutions.length);
            //System.out.println("");
            /* end debug */
            if (solutions.length > 0) {
                Solution = readTestfile(resultfilename);
                //System.out.println("File Solution : " + Solution);
                return true;
            } else {
                Solution = "false";
                return true;
            }
        } catch (Exception e) {
            return false;
        }
    }

    private void DisplaySolutions() {
        System.out.println("Display Solutions");
        System.out.println("Solution : " + Solution);
        String result = "";
        for (int i = 0; i > solutions.length; i++) {
            Hashtable solution = solutions[i];
            Enumeration vars = solution.keys();
            int varnum = 0;
            while (vars.hasMoreElements()) {
                Object v = vars.nextElement();
                result += (v + " = " + solution.get(v));
            }
        }
        System.out.println("+++++++++++++++++");
        System.out.println("result :" + result);
        System.out.println("?????????????????");
    }

    /*
    private String readFile(String file) {
        try {
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String line = null;
            StringBuilder stringBuilder = new StringBuilder();
            String ls = System.getProperty("line.separator");

            while ((line = reader.readLine()) != null) {
                stringBuilder.append(line);
                stringBuilder.append(ls);
            }
            return stringBuilder.toString();

        } catch (IOException e) {
            System.out.println(e.getStackTrace());
            System.out.println("Could not open or read " + file);
            return "";
        }
    }
    */
    /** Converts a string into OS_pl_encoding **/
    private String encode(String src) {
        if (OS_pl_encoding == null) {
            return src;
        }
        try {
            byte[] encodedBytes = src.getBytes(OS_pl_encoding);
            return new String(encodedBytes, "UTF-8");
        } catch (Exception e) {
            System.err.println("Used encoding not supported\n" + e);
            return src;
        }
    }

    /** Converts a string from OS_pl_encoding **/
    private String decode(String src) {
        if (OS_pl_encoding == null) {
            return src;
        }
        try {
            byte[] encodedBytes = src.getBytes("UTF-8");
            return new String(encodedBytes, OS_pl_encoding);
        } catch (Exception e) {
            System.err.println("Used encoding not supported\n" + e);
            return src;
        }
    }

    public static void main(String[] args) {
        OptTest opttest = new OptTest();
        if (args.length == 1) {
            try {
                Class.forName("jpl.fli.Prolog"); // ensure to load the jpl library
            } catch (Exception e) {
                System.err.println("loading prolog class failed");
                System.exit(1);
            }

            if (!opttest.initialize()) {
                System.out.println("initialization failed");
                System.exit(1);
            }
            System.out.println("Testfile : " + args[0]);
            System.out.println(opttest.parseTestfile(args[0]));

            /*
            opttest.execute("current_prolog_flag(debug_on_error, X)");
            opttest.execute("set_prolog_flag(debug_on_error, false)");
            opttest.execute("current_prolog_flag(debug_on_error, X)");

            opttest.execute("open database opt");
            //opttest.execute("tell('" + opttest.resultfilename + "')");
            String command = "sqdsdxl select count(*) from orte";
            opttest.execute("tell('" + opttest.resultfilename + "')," + command
                    + ",told");

            command = "sql select * from orte";
            opttest.execute("tell('" + opttest.resultfilename + "')," + command + ",told");
            command = "sql select bevt from orte";
            opttest.execute("tell('" + opttest.resultfilename + "')," + command + ",told");
            command = "optimize(select bevt from orte where [bevt < 100 ,bevt > 50])";
            opttest.execute("tell('" + opttest.resultfilename + "')," + command + ",told");
            */
            //opttest.execute("sql select * from plz");
            //opttest.execute("told");
        } else {
            if (args.length == 2) {
                System.out.println("args.length == 2, arg[1] =" + args[1]);
                if (args[0].equals("-c")) {
                    try {
                        Class.forName("jpl.fli.Prolog"); // ensure to load the jpl library
                    } catch (Exception e) {
                        System.err.println("loading prolog class failed");
                        System.exit(1);
                    }

                    if (!opttest.initialize()) {
                        System.out.println("initialization failed");
                        System.exit(1);
                    }
                    System.out.println("Creating testfile from " + args[1]);
                    if (opttest.createTestfile(args[1])) {
                        System.out.println("creating testfile successful");
                    } else {
                        System.out.println("creating testfile failed");
                    }

                }
            } else {
                System.out
                        .println("OptTest \n  Usage : \n "
                                + "execute testfile : \n OptTest <testfilename>\n"
                                + "create testfile from commands: \n OptTest -c <testfilename>\n");
            }
        }
    }
}
