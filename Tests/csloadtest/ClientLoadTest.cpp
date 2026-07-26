
/*
----
This file is part of SECONDO.

Copyright (C) 2021,
Faculty of Mathematics and Computer Science,
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


Drive the client library from many threads at once, so that the shared state it
still keeps is actually exercised rather than argued about. Every check has an
expected answer and a mismatch makes the program exit non-zero, which is what
lets this run unattended -- under ThreadSanitizer in particular, where the point
is to have a workload that is both concurrent and verifiable.

Two workloads, because they stress different things:

  * ~connect~ opens and closes connections from several threads while running
    commands on them. This is the path with the process-wide state: the runtime
    flags, the message centre's handler list, the global nested list reference.
    Needs a running SecondoMonitor.

  * ~parse~ builds and re-reads nested lists from several threads, each on its
    own NestedList. That is the path through NLParser, which serializes on a
    static mutex today; it needs no server, so it stays fast and deterministic
    under a sanitizer.

The list transfer mode is not selectable here: since the format is negotiated,
the *server* decides it (see csp::BINARY\_TRANSFER\_TAG). To cover the textual
path end to end, point ~--port~ at a monitor whose SecondoConfig.ini has
~Server:BinaryTransfer~ switched off; ~--expect-binary~/~--expect-text~ makes
the run assert which one it got, so a mis-configured server fails loudly
instead of quietly testing the same path twice.

1.1 Includes

*/

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "NestedList.h"
#include "Profiles.h"
#include "LogMsg.h"

using namespace std;

/*
1.2 Reporting

Failures are counted rather than thrown: a thread that gives up early would
reduce the concurrency the run is meant to produce, and the exit code is what
the caller looks at anyway.

*/

static atomic<int> failures(0);
static atomic<long> checks(0);

static void fail(const string& what)
{
  failures++;
  // One write per failure, so the lines do not interleave.
  cerr << ("FAIL: " + what + "\n");
}

static void check(bool ok, const string& what)
{
  checks++;
  if (!ok) {
    fail(what);
  }
}

/*
1.3 Configuration

*/

struct Options
{
  string config = "Config.ini";
  string host = "";       // filled from the config file when not given
  string port = "";       // ditto
  string db = "BERLINTEST";
  string mode = "both";   // connect | parse | both
  int threads = 10;
  int rounds = 10;
  int expectBinary = -1;  // -1 = do not care, 0 = text, 1 = binary
};

static void usage(const char* prog)
{
  cerr << "usage: " << prog << " [options]\n"
       << "  --host H          server host   (default: from Config.ini)\n"
       << "  --port P          server port   (default: from Config.ini)\n"
       << "  --db NAME         database      (default: BERLINTEST)\n"
       << "  --config FILE     config file   (default: Config.ini)\n"
       << "  --threads N       concurrent threads      (default: 10)\n"
       << "  --rounds N        connections per thread  (default: 10)\n"
       << "  --mode M          connect | parse | both  (default: both)\n"
       << "  --expect-binary   require the server to transfer lists binary\n"
       << "  --expect-text     require the server to transfer lists as text\n";
}

static bool parseOptions(int argc, char** argv, Options& o)
{
  for (int i = 1; i < argc; i++) {
    const string a = argv[i];
    const bool hasValue = (i + 1 < argc);
    if (a == "--host" && hasValue) { o.host = argv[++i]; }
    else if (a == "--port" && hasValue) { o.port = argv[++i]; }
    else if (a == "--db" && hasValue) { o.db = argv[++i]; }
    else if (a == "--config" && hasValue) { o.config = argv[++i]; }
    else if (a == "--threads" && hasValue) { o.threads = atoi(argv[++i]); }
    else if (a == "--rounds" && hasValue) { o.rounds = atoi(argv[++i]); }
    else if (a == "--mode" && hasValue) { o.mode = argv[++i]; }
    else if (a == "--expect-binary") { o.expectBinary = 1; }
    else if (a == "--expect-text") { o.expectBinary = 0; }
    else { usage(argv[0]); return false; }
  }
  if (o.mode != "connect" && o.mode != "parse" && o.mode != "both") {
    usage(argv[0]);
    return false;
  }
  // Taking host and port from the same file the client reads keeps this from
  // drifting out of step with the installation, which is how the hard-coded
  // port here went stale before.
  if (o.host.empty()) {
    o.host = SmiProfile::GetParameter("Environment", "SecondoHost",
                                      "localhost", o.config);
  }
  if (o.port.empty()) {
    o.port = SmiProfile::GetParameter("Environment", "SecondoPort",
                                      "1234", o.config);
  }
  return o.threads > 0 && o.rounds > 0;
}

/*
1.4 The connection workload

Each thread owns the objects it creates, so what comes back is predictable and
can be asserted. The threads still contend for the same database, which is the
server-side half of what this test was written for.

*/

static string trim(const string& s)
{
  const size_t b = s.find_first_not_of(" \t\n\r");
  if (b == string::npos) return "";
  return s.substr(b, s.find_last_not_of(" \t\n\r") - b + 1);
}

static bool command(SecondoInterface* si, NestedList* nl,
                    const string& cmd, string& out, bool mustSucceed = true)
{
  ListExpr res = nl->TheEmptyList();
  SecErrInfo err;
  si->Secondo(cmd, res, err);
  if (err.code != 0) {
    if (mustSucceed) {
      fail("command '" + cmd + "' failed: " + err.msg);
    }
    return false;
  }
  out = trim(nl->ToString(res));
  return true;
}

static void connectWorker(const Options& o, int id)
{
  const string obj = "cslt_x" + to_string(id);

  for (int r = 0; r < o.rounds; r++) {
    SecondoInterface* si = new SecondoInterfaceCS(true, 0, false);
    si->InitRTFlags(o.config);

    string errMsg;
    if (!si->Initialize("", "", o.host, o.port, o.config, "", errMsg, true)) {
      fail("thread " + to_string(id) + " cannot connect to " +
           o.host + ":" + o.port + ": " + errMsg);
      delete si;
      return;
    }

    if (o.expectBinary >= 0) {
      // What this connection actually agreed with the server, not the
      // Server:BinaryTransfer flag -- that only says what this client would
      // have picked, so asking it would pass whatever the server answered.
      const bool binary =
          static_cast<SecondoInterfaceCS*>(si)->usesBinaryTransfer();
      check(binary == (o.expectBinary == 1),
            string("server transfers lists ") +
            (binary ? "binary" : "as text") +
            ", which is not what was asked for");
    }

    NestedList* nl = si->GetNestedList();
    // Deliberately no NList::setNLRef here: that global holds one nested list
    // for the whole process, so a second connection would leave the first one
    // pointing at a list it does not own -- and at a freed one once that
    // connection closes.
    string out;

    command(si, nl, "open database " + o.db, out);

    // Left over from a previous round or an earlier run; failure is expected
    // the first time round, so it is not asserted.
    command(si, nl, "delete " + obj, out, false);

    if (command(si, nl, "let " + obj + " = " + to_string(id), out)) {
      command(si, nl, "query " + obj, out);
      check(out == "(int " + to_string(id) + ")",
            "thread " + to_string(id) + " read back '" + out + "'");
    }

    // A query the threads share, so they are not only touching their own
    // objects -- and one whose answer is known.
    if (command(si, nl, "query " + to_string(id) + " + 1", out)) {
      check(out == "(int " + to_string(id + 1) + ")",
            "thread " + to_string(id) + " computed '" + out + "'");
    }

    command(si, nl, "delete " + obj, out, false);

    si->Terminate();
    delete si;
  }
}

/*
1.5 The nested list workload

Round trips a list through the textual representation on each thread's own
NestedList, which is the path through NLParser and NLScanner. No server is
involved, so this stays quick enough to run under a sanitizer.

*/

static string sampleList(int id)
{
  ostringstream os;
  os << "(thread" << id << " (";
  for (int i = 0; i < 40; i++) {
    os << i << " " << (i + 0.5) << " \"s" << i << "\" ";
  }
  os << ") TRUE <text>some text " << id << "</text--->)";
  return os.str();
}

static void parseWorker(const Options& o, int id)
{
  // Its own list, as every connection has: the instance is what carries the
  // node tables, so sharing one here would be testing something else.
  NestedList* nl = new NestedList("");

  const string text = sampleList(id);
  for (int r = 0; r < o.rounds * 10; r++) {
    ListExpr list = nl->TheEmptyList();
    if (!nl->ReadFromString(text, list)) {
      fail("thread " + to_string(id) + " could not parse its own list");
      break;
    }
    const string again = nl->ToString(list);
    ListExpr second = nl->TheEmptyList();
    if (!nl->ReadFromString(again, second)) {
      fail("thread " + to_string(id) + " could not re-parse what it wrote");
      break;
    }
    check(nl->Equal(list, second),
          "thread " + to_string(id) + " lost information in a round trip");

    // A list that cannot be parsed, so that the error path is exercised too.
    // It runs through the parser's yyerror, and so through the global cmsg,
    // which every thread shares -- reporting a failure is the one thing a
    // concurrency test must not leave untested, because it is where the
    // shared message buffer is written.
    ListExpr broken = nl->TheEmptyList();
    check(!nl->ReadFromString("(unbalanced (list ", broken),
          "thread " + to_string(id) + " accepted a malformed list");
  }
  delete nl;
}

/*
1.6 Main

*/

static void run(void (*worker)(const Options&, int), const Options& o,
                const char* what)
{
  vector<thread> ts;
  for (int i = 0; i < o.threads; i++) {
    ts.push_back(thread(worker, o, i));
  }
  for (size_t i = 0; i < ts.size(); i++) {
    ts[i].join();
  }
  cout << what << ": " << o.threads << " threads done" << endl;
}

int main(int argc, char** argv)
{
  Options o;
  if (!parseOptions(argc, argv, o)) {
    return 2;
  }

  if (o.mode == "parse" || o.mode == "both") {
    run(parseWorker, o, "nested lists");
  }
  if (o.mode == "connect" || o.mode == "both") {
    cout << "connecting to " << o.host << ":" << o.port
         << ", database " << o.db << endl;
    run(connectWorker, o, "connections");
  }

  cout << checks.load() << " checks, " << failures.load() << " failures"
       << endl;
  return failures.load() == 0 ? 0 : 1;
}
