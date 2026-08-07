
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

  * ~shared~ puts every thread on *one* NestedList at the same time, which is
    the case ~parse~ deliberately leaves out and the one that says whether
    ~NestedList~'s own recursive mutex is still carrying weight. Writers here
    grow the node table while readers walk it, which is the case the storage
    layer is built for: BigArray adds a chunk rather than moving what is
    already mapped, so a reader holding a pointer into an earlier chunk keeps
    a valid one. Needs no server.

    Reaching that at all takes ~--list-mem~, which sizes the tables far below
    what a NestedList takes by default. At the default sizes this workload
    fits inside the chunk BigArray maps at construction and never grows it
    once, so the mode used to run without touching the persistence layer it
    was written for. The run keeps going past ~--rounds~ until the table has
    left its first chunk, says how many chunks it reached, and fails if it
    never left the first -- so that cannot quietly come back, and no round
    count has to be re-tuned when the chunk size changes.

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
#include <cstddef>
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
  string mode = "both";   // connect | parse | shared | both
  int threads = 10;
  int rounds = 10;
  int expectBinary = -1;  // -1 = do not care, 0 = text, 1 = binary
  // Node/string/text memory for the lists this test builds, in kB, and
  // deliberately far below the 1024/512/512 a NestedList takes by default.
  // The tables then run out of room within the first few rounds and BigArray
  // has to grow -- extending the file and mapping another chunk of it while
  // other threads are reading the chunks before it. At the default sizes this
  // workload fits inside the first chunk and never reaches that code at all.
  int listMem = 1;
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
       << "  --mode M          connect | parse | shared | both | contract |\n"
       << "                    handover   (default: both)\n"
       << "                    contract breaks the threading contract on\n"
       << "                    purpose and must abort; needs a build with\n"
       << "                    -DNL_CHECK_CONCURRENCY\n"
       << "                    handover keeps it, passing one list from\n"
       << "                    thread to thread, and must not abort\n"
       << "  --list-mem KB     node/string/text memory per list (default: 1,\n"
       << "                    small on purpose so the tables have to grow)\n"
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
    else if (a == "--list-mem" && hasValue) { o.listMem = atoi(argv[++i]); }
    else if (a == "--expect-binary") { o.expectBinary = 1; }
    else if (a == "--expect-text") { o.expectBinary = 0; }
    else { usage(argv[0]); return false; }
  }
  if (o.mode != "connect" && o.mode != "parse" && o.mode != "shared" &&
      o.mode != "both" && o.mode != "contract" && o.mode != "handover") {
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
  return o.threads > 0 && o.rounds > 0 && o.listMem > 0;
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
  //
  // Sized down like the shared mode, so that these lists grow too. That used
  // to be impossible: BigArray grew by remapping, mremap unmaps the old range
  // as it moves, and TSan does not model that half of it -- the stale shadow
  // then collided with whichever thread's mapping the kernel next put at that
  // address, and the report named two threads holding two *different* lists'
  // mutexes. Growth never unmaps anything now, so the artifact has no cause
  // and this mode covers many lists growing at once.
  NestedList* nl = new NestedList("", o.listMem, o.listMem, o.listMem);

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
1.5b One list, every thread on it at once

Each thread owns the nodes it makes and only ever checks those, so the answers
stay deterministic while the writes of the other threads are landing in the same
tables. That is exactly the shape of the Distributed2 pattern: build into shared
list memory, then read back what you built while others keep building.

What it would catch if the locking were wrong: TSan reports the unsynchronized
access, and the value checks catch a list that came back as something other than
what this thread put in -- an index handed to two threads, or a slot written by
one thread and read by another without an ordering between them.

*/

static NestedList* sharedList = 0;

static void sharedWorker(const Options& o, int id)
{
  NestedList* nl = sharedList;

  // Most rounds keep their list rather than giving it straight back, so that
  // the tables grow through the run instead of settling at a steady state on
  // the free lists. Growing is the whole point: it is what makes BigArray map
  // another chunk while readers are walking the ones before it. Every fourth
  // round still destroys, so the free lists are written under contention too.
  vector<ListExpr> kept;

  // How much work growing takes is set by the *chunk size*, not by --rounds:
  // BigArray maps whole chunks, and one chunk is thousands of nodes however
  // small --list-mem is. So --rounds is a floor, and the workers keep going
  // past it until the table has actually left its first chunk. Tying it to the
  // observed state rather than to a tuned round count is what keeps this mode
  // honest when the chunk size changes -- picking numbers that happen to
  // exceed it today is how it came to pass while testing nothing.
  //
  // Bounded, so that a change which stops the table growing fails the
  // assertion after the run rather than hanging here.
  const int minRounds = o.rounds * 10;
  const int maxRounds = minRounds + 20000;

  for (int r = 0; r < maxRounds; r++) {
    if (r >= minRounds && sharedList->chunksOfNodeTable() > 1) {
      break;
    }
    // Build: atoms of every kind, so the string and text tables are written
    // too and not just the node table.
    ListExpr mine = nl->OneElemList(nl->IntAtom(id));
    ListExpr last = mine;
    last = nl->Append(last, nl->RealAtom(id + 0.5));
    last = nl->Append(last, nl->BoolAtom((id % 2) == 0));
    last = nl->Append(last, nl->StringAtom("s" + to_string(id)));
    last = nl->Append(last, nl->SymbolAtom("sym" + to_string(id)));
    last = nl->Append(last, nl->TextAtom("text of thread " + to_string(id)));
    // A nested sublist, so the reader walks more than one spine.
    last = nl->Append(last, nl->TwoElemList(nl->IntAtom(r),
                                            nl->IntAtom(id)));

    // Read back: the values must be this thread's own, whatever the others are
    // doing to the same tables meanwhile.
    check(nl->ListLength(mine) == 7,
          "thread " + to_string(id) + " lost elements from its own list");
    check(nl->IntValue(nl->First(mine)) == id,
          "thread " + to_string(id) + " read back another thread's int");
    ListExpr rest = nl->Rest(mine);
    check(nl->AtomType(nl->First(rest)) == RealType,
          "thread " + to_string(id) + " sees the wrong atom type");
    check(nl->StringValue(nl->Nth(4, mine)) == "s" + to_string(id),
          "thread " + to_string(id) + " read back another thread's string");
    check(nl->Text2String(nl->Nth(6, mine)) ==
              "text of thread " + to_string(id),
          "thread " + to_string(id) + " read back another thread's text");
    ListExpr sub = nl->Nth(7, mine);
    check(nl->IntValue(nl->First(sub)) == r &&
              nl->IntValue(nl->Second(sub)) == id,
          "thread " + to_string(id) + " read back the wrong sublist");

    // Whole-list traversals: ToString and Equal walk every node, which is the
    // read pattern that a concurrent grow would break.
    const string text = nl->ToString(mine);
    check(text.find("s" + to_string(id)) != string::npos,
          "thread " + to_string(id) + " could not find itself in ToString");
    check(nl->Equal(mine, mine),
          "thread " + to_string(id) + " list is not equal to itself");

    // IncReferences is the odd one out: every other method that touches a
    // table locks first, and this one did not, so it read the node table
    // while another thread was growing it. Kept here because nothing else
    // reaches it -- it is called from InMap/OutMap, not from list building.
    nl->IncReferences(mine);

    if ((r % 4) == 3) {
      // Destroy writes the free lists, which are shared state of their own.
      nl->Destroy(mine);
    } else {
      kept.push_back(mine);
    }
  }

  // Only now, so that the tables stayed under pressure for the whole run.
  for (size_t i = 0; i < kept.size(); i++) {
    nl->Destroy(kept[i]);
  }
}

/*
1.6 Main

*/

/*
1.6 The workload that is *supposed* to be rejected

Every other mode here obeys the threading contract: threads share one
~NestedList~ but never one list inside it. This one breaks it on purpose --
all threads append to the same node -- so that the checker built for exactly
this can be shown to fire, rather than assumed to.

Only meaningful under ~-DNL\_CHECK\_CONCURRENCY~, and it is expected to abort:
a run that *completes* is the failure. Without the checker compiled in this is
undefined behaviour and nothing should be concluded from it, which is why the
mode refuses to run unless the checker is there.

*/

#ifdef NL_CHECK_CONCURRENCY

static NestedList* contractList = 0;
static ListExpr    contractTail = 0;

static void contractWorker(const Options&, int id)
{
  NestedList* nl = contractList;

  for (int r = 0; r < 100000; r++) {
    // No lock, one shared tail: two threads read the same record and both
    // write their own node into its right son, so one of the two elements is
    // simply lost. The checker should stop the process on the first overlap.
    contractTail = nl->Append(contractTail, nl->IntAtom(id * 1000 + r));
  }
}

#endif // NL_CHECK_CONCURRENCY

/*
1.7 The workload that must *not* be rejected

The checker marks a node while a thread writes it and clears the mark when the
write is over. Forgetting to clear it does not look like a bug from the outside:
the mode above still aborts, and the mode that keeps the contract still passes
as long as no node index happens to reach a second thread. What it costs is
everything in between -- a mark left behind reports the *next* thread to touch
that node however long after, and a slot held forever hides every other node
that hashes to it. That is not hypothetical; it is what the checker did, and
nothing here noticed, because breaking a detector makes it quieter.

So this mode hands one list from thread to thread: a thread builds it and is
joined, then a *different* thread walks and frees it. Every node is written by
two threads with a happens-before between them, which is the contract kept, not
broken -- the shape ~Distributed2~ produces when a worker's result is copied
into the global list under ~copylistmutex~ and read back afterwards. A run that
aborts is the failure.

*/

static NestedList* handoverList  = 0;
static ListExpr    handoverBuilt = 0;

static void handoverBuilder(const Options& o, int id)
{
  NestedList* nl = handoverList;
  ListExpr l = nl->OneElemList(nl->IntAtom(id));
  ListExpr last = l;
  for (int r = 0; r < 200; r++) {
    last = nl->Append(last, nl->IntAtom(r));
  }
  check(nl->ListLength(l) == 201, "the handover list came out the wrong size");
  handoverBuilt = l;
}

static void handoverConsumer(const Options&, int)
{
  NestedList* nl = handoverList;
  // Destroy writes every node the builder wrote, which is what makes the two
  // threads meet on the same node records.
  nl->Destroy(handoverBuilt);
  handoverBuilt = 0;
}

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
  if (o.mode == "shared" || o.mode == "both") {
    sharedList = new NestedList("", o.listMem, o.listMem, o.listMem);
    run(sharedWorker, o, "one shared nested list");

    // Assert that the run really did grow the storage, rather than assuming
    // it. BigArray maps whole chunks, and one chunk is a good deal more than
    // the requested kB, so the entry count is not the thing to compare -- the
    // number of chunks is. Without this the mode keeps passing while quietly
    // testing nothing about growth, which is what it did until the tables
    // were made small.
    cout << "  node table grew to " << sharedList->sizeOfNodeTable()
         << " entries in " << sharedList->chunksOfNodeTable() << " chunks"
         << endl;
    check(sharedList->chunksOfNodeTable() > 1,
          "the node table never left its first chunk -- raise --threads or "
          "--rounds, or lower --list-mem, or this mode is not testing "
          "BigArray growth at all");

    delete sharedList;
    sharedList = 0;
  }
  if (o.mode == "handover") {
    handoverList = new NestedList("", o.listMem, o.listMem, o.listMem);
    int lists = 0;
    for (int r = 0; r < o.rounds; r++) {
      for (int t = 0; t < o.threads; t++) {
        // One at a time, on purpose: joining before the next starts is the
        // ordering that makes this legal.
        thread(handoverBuilder, o, t).join();
        thread(handoverConsumer, o, t).join();
        lists++;
      }
    }
    cout << "handover: " << lists
         << " lists built and freed by different threads" << endl;
    delete handoverList;
    handoverList = 0;
  }
  // Deliberately not part of "both": this one is meant to die.
  if (o.mode == "contract") {
#ifndef NL_CHECK_CONCURRENCY
    cerr << "--mode contract needs the build to define NL_CHECK_CONCURRENCY; "
            "without it this workload is undefined behaviour and proves "
            "nothing." << endl;
    return 2;
#else
    contractList = new NestedList("", o.listMem, o.listMem, o.listMem);
    contractTail = contractList->OneElemList(contractList->IntAtom(0));
    run(contractWorker, o, "one shared list, unsynchronised");
    cerr << "FAILURE: the concurrency checker did not fire -- " << o.threads
         << " threads appended to one node and the run completed." << endl;
    return 1;
#endif
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
