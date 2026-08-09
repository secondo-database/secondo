/*
----
This file is part of SECONDO.

Copyright (C) 2026, Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
----

1 secondo\_native -- pybind11 wrapper over the SECONDO C++ client

This module is a *thin* wrapper over the in-tree, prebuilt client-server
interface (~apis/api\_cpp/cs~, linking ~libsecondo.a~). It deliberately
does NOT reimplement any part of the SECONDO wire protocol: connecting,
binary nested-list decoding, framing and heartbeats all stay inside the
trusted C++ code that the rest of the system uses.

What crosses the language boundary is the answer's nested list, either
rendered as text (~NestedList::ToString~) or built straight into Python
objects, and for a relation one tuple at a time (~ResultTuples~). Turning
that into GeoJSON is done in Python, where it is easy to fixture-test.

*/

#include <pybind11/pybind11.h>

#include <cctype>
#include <cstdint>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <stdexcept>
#include <utility>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "SQLLanguage.h"
#include "NestedList.h"
#include "LogMsg.h"

namespace py = pybind11;

// SECONDO stores strings in Latin-1 (ISO-8859-1), so any text coming back from
// the server may contain bytes (e.g. 0xfc for 'u"'/umlaut) that are invalid
// UTF-8. pybind11's default std::string -> str conversion assumes UTF-8 and
// would raise UnicodeDecodeError on such results (Kinos, WFlaechen, ...).
// Decoding Latin-1 explicitly is lossless for any byte and yields correct
// Unicode. Only for text the *server* produced -- a string that came in from
// Python is already UTF-8 and must not be run through this.
static py::str latin1(const std::string& s)
{
  return py::reinterpret_steal<py::str>(
      PyUnicode_DecodeLatin1(s.data(), s.size(), "replace"));
}

// Everything one command produced, filled while the GIL is released and turned
// into Python objects afterwards.
struct AutoResult
{
  int level = CMD_LEVEL_TEXT;
  std::string text;        // the result nested list, as text
  // The same list as ~text~, still as nodes. Kept so the Python tree can be
  // built from it once the GIL is back; it stays valid until the next command
  // on this connection, which cannot start before this one has returned.
  ListExpr result = 0;
  std::string plan;        // level 2: the plan the optimizer generated
  std::string message;     // level 3: what the directive printed
  double costs = 0.0;      // level 2: estimated costs
  bool hasPlan = false;
  bool hasCosts = false;
  bool hasMessage = false;
  int errCode = 0;
  int errPos = 0;
  std::string errMsg;
};

static std::string trimmed(const std::string& s)
{
  const std::string ws = " \t\n\r\f\v";
  const size_t b = s.find_first_not_of(ws);
  if (b == std::string::npos) return "";
  return s.substr(b, s.find_last_not_of(ws) - b + 1);
}

// Whitespace runs (newlines included) squeezed to single spaces, then trimmed.
static std::string oneLine(const std::string& s)
{
  std::string out;
  out.reserve(s.size());
  bool space = false;
  for (const char c : s) {
    if (isspace(static_cast<unsigned char>(c))) {
      space = true;
      continue;
    }
    if (space && !out.empty()) out += ' ';
    space = false;
    out += c;
  }
  return out;
}

/*
1.1 Nested list -> Python objects

Builds the Python tree straight from the ~ListExpr~, in exactly the shape
~app/nlparser.parse~ produces from text: a list becomes a list, and an atom
becomes the matching Python scalar.

The GIL must be held: this allocates Python objects.

*/
typedef py::str (*Decoder)(const std::string&);

// What the server produced is Latin-1; what Python handed in (parse_nl) is
// already UTF-8 and must not be run through the Latin-1 decoder, or an umlaut
// comes back as two characters.
static py::str utf8(const std::string& s)
{
  return py::str(s);
}

static py::object treeOf(NestedList* nl, ListExpr list, Decoder decode = latin1)
{
  if (nl->IsEmpty(list)) {
    return py::list();
  }
  if (nl->IsAtom(list)) {
    switch (nl->AtomType(list)) {
      case IntType:    return py::int_(nl->IntValue(list));
      case RealType:   return py::float_(nl->RealValue(list));
      case BoolType:   return py::bool_(nl->BoolValue(list));
      // Strings, symbols and texts all arrive as str, as the text parser had
      // them: nothing downstream distinguishes a symbol from a quoted string.
      case StringType: return decode(nl->StringValue(list));
      case SymbolType: return decode(nl->SymbolValue(list));
      case TextType:   return decode(nl->Text2String(list));
      default:         return py::none();
    }
  }
  // Iterative over the spine, so only *nesting* costs C stack -- a relation of
  // a million tuples is one long list, not a million-deep one.
  py::list items;
  while (!nl->IsEmpty(list)) {
    items.append(treeOf(nl, nl->First(list), decode));
    list = nl->Rest(list);
  }
  return items;
}

/*
1.2 Text -> nested list, through SECONDO's own parser

For the one thing that arrives as text and has no ~ListExpr~ behind it: a value
the *user* typed into a table cell. It is checked here rather than by a parser
of our own because the only question worth asking is whether SECONDO will
accept it -- a second implementation of ~NLLex.l~ and ~NLParser.y~ can only ever
agree with them by accident, and the dangerous direction is the one where it is
more permissive: the value then fails on the server, mid-save, instead of at the
field.

Its own list, not a connection's: validating a cell has nothing to do with a
session, and this way it works before any connection exists. The list is reset
after every parse so a long-lived bridge does not accumulate the nodes.

The mutex covers all three of the shared things touched here -- the list, its
reset, and ~cmsg~'s process-wide error buffer, which is how the parser's own
diagnostic (token, line and column) is retrieved.

*/
static py::object parseNL(const std::string& text)
{
  static std::mutex mtx;
  static NestedList* nl = nullptr;
  std::lock_guard<std::mutex> guard(mtx);
  if (!nl) {
    nl = new NestedList("temp_nested_list");
  }

  ListExpr list = nl->TheEmptyList();
  cmsg.resetErrors();
  const bool ok = nl->ReadFromString(text, list);
  if (!ok) {
    // The parser lays its complaint out over several lines; this one ends up
    // beside a table cell, so it is folded onto one.
    const std::string why = oneLine(cmsg.getErrorMsg());
    cmsg.resetErrors();
    nl->initializeListMemory();
    throw std::invalid_argument(why.empty() ? "not a valid nested list" : why);
  }
  py::object tree = treeOf(nl, list, utf8);
  cmsg.resetErrors();
  nl->initializeListMemory();
  return tree;
}

// An error the SECONDO server reported. Surfaces in Python as
// secondo_native.SecondoError, which derives from RuntimeError so that every
// existing "except RuntimeError" keeps catching it.
struct SecondoCommandError : public std::runtime_error
{
  SecondoCommandError(int code, int pos, const std::string& msg,
                      const std::string& plan)
    : std::runtime_error("SECONDO error " + std::to_string(code) +
                         " (pos " + std::to_string(pos) + "): " + msg),
      code(code), pos(pos), plan(plan)
  {}

  int code;
  int pos;
  // The generated plan, when an SQL command failed while *executing* it. The
  // server sends the plan alongside the error; empty otherwise.
  std::string plan;
};

static SecondoCommandError secondoError(int code, int pos,
                                        const std::string& msg,
                                        const std::string& plan)
{
  return SecondoCommandError(code, pos, msg, plan);
}

// The registered Python exception type, set up in PYBIND11_MODULE.
static py::handle secondoErrorType;

/*
1.3 Handing a result out one tuple at a time

Turning a whole relation into Python objects at once is what made a large
answer expensive on this side: ~query roads~ is ~48M nodes, so ~treeOf~ over
the top of it builds ~48M Python objects and every one of them stays alive
until the last consumer is done with the tree. The tuples are read once each
and independently of one another, so they never have to exist at the same
time.

~ResultTuples~ walks the spine and builds one element's objects per
~\_\_next\_\_~. The payload builders in ~app/convert.py~ consume it in a single
pass and drop each tuple before asking for the next, so they see the whole
relation while only one tuple's worth of it is live.

The GIL is deliberately *not* released between elements. Everything here
either allocates Python objects -- which requires it -- or is a handful of
nested-list node reads, and a release/acquire pair per tuple would cost more
than the little it could overlap.

*/

// What a Connection shares with every iterator it has handed out. It outlives
// both, so an iterator can ask whether what it points at is still there
// instead of finding out by reading freed nodes.
struct ListState
{
  // The connection's nested list, or 0 once the connection has been closed.
  NestedList* nl = 0;
  // Bumped whenever that list is rolled back, which is at the start of every
  // command (see Connection::beginCommand). An iterator made before the
  // rollback points into nodes it has handed back, so it refuses to go on
  // rather than reading them.
  //
  // Deliberately not atomic: a connection is used by one thread at a time --
  // the FastAPI layer holds a per-session lock across the command and the
  // iteration that follows it -- so this orders two things that already
  // cannot overlap. It catches a use-after-free in program order, not a race.
  uint64_t generation = 0;
};

class ResultTuples
{
 public:
  ResultTuples(std::shared_ptr<ListState> listState, ListExpr rest)
    : state(std::move(listState)), rest(rest), generation(state->generation)
  {}

  py::object next()
  {
    NestedList* const nl = live();
    if (nl->IsEmpty(rest)) {
      throw py::stop_iteration();
    }
    py::object item = treeOf(nl, nl->First(rest));
    rest = nl->Rest(rest);
    return item;
  }

 private:
  // The list these elements are in, if it is still the one they were taken
  // from. Checked on every step: the cost is two loads against reading a node
  // that another command has already reused.
  NestedList* live() const
  {
    if (state->nl == 0) {
      throw std::runtime_error("connection is closed");
    }
    if (state->generation != generation) {
      throw std::runtime_error(
          "this result has been released: its tuples have to be read before "
          "the next command runs on the same connection");
    }
    return state->nl;
  }

  std::shared_ptr<ListState> state;
  // Where the walk stands. The elements before it are gone as far as this
  // object is concerned; the list itself is untouched.
  ListExpr rest;
  uint64_t generation;
};

/*
1.4 Reading a relation straight into Python

~ResultTuples~ keeps the answer from becoming one Python tree, but the client
has still built the whole nested list by the time it hands it over: some 48M
nodes for a "query roads", which is the larger half of what that query costs
this process.

~PyResultSink~ closes that half too. Installed on the connection for the
duration of one command (~SecondoInterfaceCS::SetResultSink~), it is handed
each tuple as the client decodes it off the socket, turns it into Python
objects, and lets the kernel take the nodes straight back -- so the client's
list holds one tuple rather than a relation.

Two things have to happen here because there is no answer left to do them from
afterwards.

The first is the result *text*. It goes through the same writer the whole list
would have gone through, ~NestedList::WriteStringTo~, with the punctuation that
list would have had around it: an opening parenthesis, the type half, a space,
a second opening parenthesis, the elements separated by single spaces, and two
closing ones. So the bytes are the ones ~ToString~ produced before. Without it
a streamed command could not also show its answer on the console, and that
would rule streaming out for every view the WebUI actually uses.

The second is a Python error, which is caught rather than let out: the client
is part way through a result frame and has to drain it before the connection
can be used for anything else, so the failure is remembered and raised once
the read is over.

The GIL is taken per tuple. That is some 212k acquire/release pairs for
~roads~ -- tens of milliseconds -- and it buys the opposite of what holding it
throughout would: the socket read and the nested-list work between tuples run
with it released, so other sessions keep going.

*/
class PyResultSink : public NestedList::BinaryListSink
{
 public:
  // Built with the GIL held, before the command releases it: looking the two
  // methods up once is also what keeps the per-tuple cost to a call.
  PyResultSink(NestedList* nl, const py::object& sink, const bool wantText)
    : nl(nl), onBegin(sink.attr("begin")), onElem(sink.attr("elem")),
      wantText(wantText)
  {}

  // Whether the reader used this at all. False for an answer whose shape it
  // could not stream -- the caller's cue to fall back to the whole list.
  bool used() const { return started; }

  // What the Python handler raised, or empty. Checked once the read is over.
  const std::string& failure() const { return error; }

  // The answer as text, in the ~want_text~ sense: "" when it was not asked
  // for. Only meaningful once the read has finished.
  std::string text() const
  {
    return wantText ? (rendered.str() + "))") : std::string();
  }

  void begin(ListExpr typeExpr) override
  {
    started = true;
    if (wantText) {
      rendered << "(";
      nl->WriteStringTo(typeExpr, rendered);
      rendered << " (";
    }
    call(onBegin, typeExpr);
  }

  bool elem(ListExpr element) override
  {
    if (wantText) {
      if (elems > 0) rendered << " ";
      nl->WriteStringTo(element, rendered);
    }
    elems++;
    return call(onElem, element);
  }

 private:
  // Hand one list to Python. Answering false stops the reading, which is what
  // an error has to do: going on would call a handler that has already failed
  // once per remaining tuple.
  bool call(const py::object& fn, ListExpr list)
  {
    if (!error.empty()) {
      return false;
    }
    py::gil_scoped_acquire gil;
    try {
      fn(treeOf(nl, list));
    } catch (py::error_already_set& e) {
      error = e.what();
      if (error.empty()) {
        error = "the result handler failed";
      }
      // Cleared here rather than left set: the C++ below this point makes
      // plenty of calls that would see a pending Python error and misread it
      // as their own.
      e.discard_as_unraisable("secondo_native result sink");
      return false;
    }
    return true;
  }

  NestedList* nl;
  py::object onBegin;
  py::object onElem;
  bool wantText;
  bool started = false;
  unsigned long elems = 0;
  std::ostringstream rendered;
  std::string error;
};

// Installs a sink on a connection for the length of one command and takes it
// off again however the command ends -- leaving one behind would point the
// next command's reader at a dead object.
class SinkGuard
{
 public:
  SinkGuard(SecondoInterfaceCS* si, NestedList::BinaryListSink* sink) : si(si)
  {
    si->SetResultSink(sink);
  }
  ~SinkGuard() { si->SetResultSink(0); }

 private:
  SecondoInterfaceCS* si;
};

// Connections are independent: opening, using and closing one runs alongside
// whatever the others are doing.
//
// This module used to hold a process-wide lock around every call, because the
// client library kept per-connection state in globals -- which threw away the
// parallelism the server offers, since it forks a process per connection. Those
// globals are gone: the list transfer mode moved onto the connection's
// CSProtocol, the runtime flags are filled once and then only read, the message
// centre's handler list is locked, the connection counter is atomic, the
// nested-list parser is reentrant, and the error reporter and message buffer
// are per thread. Tests/csloadtest opens, uses and closes connections from many
// threads at once under ThreadSanitizer, which reports nothing.
//
// What remains here is only the GIL: every call releases it, so the other
// Python threads keep running while this one waits on the server.

// One SECONDO client session. The FastAPI layer keeps one Connection per
// browser session and an asyncio lock per session, which keeps a single
// session's commands in order -- sessions are stateful -- and makes a second
// request on the same session wait in the event loop rather than in a worker
// thread. Different sessions do not wait for each other at all.
class Connection
{
 public:
  Connection(const std::string& host,
             const std::string& port,
             const std::string& user,
             const std::string& passwd,
             const std::string& config)
  {
    std::string errMsg;
    const bool multiUser = true;  // host/port take precedence over config
    bool ok;
    {
      // Connecting blocks on network I/O and can hang if the SECONDO server is
      // slow or wedged. Without releasing the GIL that would freeze the whole
      // Python process (even endpoints that never touch SECONDO).
      //
      // The runtime flags are left to Initialize, which fills them once per
      // process from the same config file every session uses.
      py::gil_scoped_release release;
      si = new SecondoInterfaceCS(true, 0, false);
      ok = si->Initialize(user, passwd, host, port, config, "",
                          errMsg, multiUser);
      if (ok) {
        // Each connection keeps its own nested list and nothing here builds
        // NList objects, so NList::setNLRef is deliberately not called: it
        // holds one list for the whole process, and a second session would
        // leave it pointing at a list another session owns -- at a freed one
        // once that session closes.
        state->nl = si->GetNestedList();
        // Where this connection's list stood before it had run anything: every
        // command rolls back to here (see beginCommand).
        connected = state->nl->mark();
      } else {
        // Tearing the half-built interface down again, still with the GIL
        // released: it talks to the server and frees its nested list.
        delete si;
        si = nullptr;
      }
    }
    if (!ok) {
      throw std::runtime_error("Cannot connect to SECONDO server at " +
                               host + ":" + port + " - " + errMsg);
    }
  }

  ~Connection() { close(); }

  // Run one SECONDO command in SOS text syntax and return the result nested
  // list as text. Raises RuntimeError on a SECONDO error (non-zero error
  // code).
  //
  // This is the path for commands the *backend* issues itself ("list objects",
  // and the relation-editing commands in app/updates.py): they are kernel
  // commands and must never involve the optimizer. What the user types goes
  // through secondo_auto.
  // Returns a dict: `text` (the result nested list as text, what the console
  // shows) and `tree` (the same list as Python objects, ready to convert). Both
  // come from the one answer; building the tree here rather than parsing the
  // text in Python is what keeps a large result from being walked twice.
  //
  // Each half is built only if it is asked for, because each is a walk of the
  // whole list and most callers want exactly one of them: `Session.run` reads
  // only the text, `Session.run_tree` only the tree, and a command run for its
  // effect (`let x = <a long track> consume`) wants neither. `query roads` on a
  // 212k-tuple relation is 81 MB of text and ~48M Python objects, so the half
  // nobody reads is not a rounding error.
  //
  // The text is "" rather than None when it was not built: it stays a str, so
  // no caller downstream needs a null check.
  //
  // `sink`, when it is not None, is an object with `begin` and `elem` that
  // takes the answer as it arrives instead of after it is built -- see
  // `PyResultSink` and `answer`.
  py::dict secondo(const std::string& command, const bool want_tree,
                   const bool want_text, const py::object& sink)
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    NestedList* const nl = state->nl;
    std::unique_ptr<PyResultSink> into(sinkFor(nl, sink, want_text));
    SecErrInfo err;
    std::string out;
    ListExpr res = nl->TheEmptyList();
    {
      // The call blocks on network I/O; let other Python threads run, and
      // let commands on other connections run alongside this one.
      py::gil_scoped_release release;
      beginCommand();
      SinkGuard guard(si, into.get());
      si->Secondo(command, res, err);
      if (err.code == 0 && want_text) {
        out = streamed(into) ? into->text() : nl->ToString(res);
      }
    }
    check(into);
    if (err.code != 0) {
      throw secondoError(err.code, err.pos, err.msg, "");
    }
    py::dict result;
    result["text"] = latin1(out);
    answer(result, res, want_tree, into);
    return result;
  }

  // Asks the connected server whether it can run the SQL dialect (the optimizer
  // is compiled in and enabled). A property of *that server*, so the answer
  // must not be cached beyond the life of this connection.
  bool optimizer_available()
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    // No beginCommand: this is a capability probe, not a command, and it
    // builds no list -- rolling back here would only invalidate iterators.
    py::gil_scoped_release release;
    return si->optimizerAvailable();
  }

  // Run one command without saying which language it is written in: the server
  // classifies it (see include/SQLLanguage.h) and reports back what it decided.
  // This is what the JavaGUI and the TTY do, so that the rules live in exactly
  // one place and no client carries a copy of them.
  //
  // Returns a dict:
  //   level    the level the server resolved the command to (0/1/2/3)
  //   text     the *result* nested list as text -- for SQL the result half of
  //            the answer, so it is byte-identical to what the same query would
  //            produce at level 1 and the GeoJSON/temporal conversion in Python
  //            works on it unchanged
  //   plan     level 2 only: the plan the optimizer generated, raw, including
  //            the literal "done" the server sends when it carried the command
  //            out itself while translating (a create/drop). Interpreting that
  //            sentinel is left to Python, where it is easy to fixture-test.
  //   costs    level 2 only, and only when the server appended them
  //   message  level 3 only: the text an optimizer directive printed
  //
  // With optimizer_addressed the "optimizer" protocol flag is sent along,
  // saying the user wrote the "optimizer " prefix: SQL is then only optimized
  // and not executed, and anything else is taken to be a directive.
  py::dict secondo_auto(const std::string& command,
                        const bool optimizer_addressed,
                        const bool want_tree,
                        const bool want_text,
                        const py::object& sink)
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    NestedList* const nl = state->nl;
    std::unique_ptr<PyResultSink> into(sinkFor(nl, sink, want_text));
    AutoResult r;
    {
      // Both the call and the nested-list extraction are pure C++, so the GIL
      // stays released for all of it -- except where the sink takes it back
      // for one tuple at a time.
      py::gil_scoped_release release;
      beginCommand();
      SinkGuard guard(si, into.get());
      ListExpr res = nl->TheEmptyList();
      si->SecondoAuto(command, optimizer_addressed, r.level, res,
                      r.errCode, r.errPos, r.errMsg);
      if (r.errCode == 0 && streamed(into)) {
        // The answer went to the sink; there is no list left to take apart,
        // and the text is the one the sink rendered as it went.
        r.text = into->text();
      } else if (r.errCode == 0) {
        extract(res, r, want_text);
      } else if (r.level == CMD_LEVEL_SQL && nl->ListLength(res) >= 2) {
        // A plan that failed to execute still comes back with the answer; every
        // other client throws it away. Carry it on the exception so the console
        // can show what actually ran.
        r.plan = planOf(nl->First(res));
        r.hasPlan = true;
      }
    }
    check(into);
    if (r.errCode != 0) {
      throw secondoError(r.errCode, r.errPos, r.errMsg,
                         r.hasPlan ? r.plan : "");
    }

    py::dict out;
    out["level"] = r.level;
    out["text"] = latin1(r.text);
    answer(out, r.result, want_tree, into);
    out["plan"] = r.hasPlan ? py::object(latin1(r.plan))
                            : py::object(py::none());
    out["costs"] = r.hasCosts ? py::object(py::cast(r.costs))
                              : py::object(py::none());
    out["message"] = r.hasMessage ? py::object(latin1(r.message))
                                  : py::object(py::none());
    return out;
  }

  // Run an optimizer control directive (a Prolog goal such as "showOptions" or
  // "setOption(subqueries)") and return the text it printed. Never raises: a
  // server-side failure comes back as the message text.
  py::str optimizer_command(const std::string& directive)
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    std::string out;
    {
      py::gil_scoped_release release;
      beginCommand();
      out = si->optimizerCommand(directive);
    }
    return latin1(out);
  }

  void close()
  {
    if (si) {
      // Any iterator still out there stops here rather than reading a list
      // that is about to be freed; set before the free, not after it.
      state->nl = 0;
      ++state->generation;
      // Terminate talks to the server and deleting the interface frees this
      // connection's nested list; both are this connection's own business.
      py::gil_scoped_release release;
      si->Terminate();
      delete si;
      si = nullptr;
    }
  }

 private:
  // Throws away what the previous command on this connection produced, at the
  // point where the next one starts.
  //
  // ~SecondoInterfaceCS~ never resets the client's nested list, so without this
  // a connection that lives as long as a browser session keeps every result it
  // has ever decoded: three identical ~query roads~ on one connection took the
  // node tables from 40 to 77 chunks and the process from 576 MB to 1.6 GB of
  // resident memory.
  //
  // The start of the next command is where the previous answer is provably
  // dead. Everything the call built has been turned into Python objects or
  // into a ~std::string~ before it returned (see ~AutoResult::result~); this
  // class keeps no ~ListExpr~ across commands and neither does the interface,
  // which keeps only locals; and ~NList::setNLRef~ is deliberately never
  // called, so no list object outside this connection points here either.
  //
  // The one thing that does outlive the call is a ~ResultTuples~, which is
  // still walking the answer. It is not made safe here but *stopped* here: the
  // generation below is what it checks, and the FastAPI layer reads it to the
  // end under the same session lock as the command that produced it, so
  // reaching this point with one still live is a caller's bug and is reported
  // as one rather than read past.
  //
  // Rolling back to ~connected~ rather than ~initializeListMemory~, which is
  // the other way to empty a list. Three reasons, in order of weight:
  //
  //   * ~initializeListMemory~ deletes the three tables and creates them
  //     again, and their backing files are created under a name every
  //     ~NestedList~ in the process shares. Doing that once per connection is
  //     already a race; doing it once per command would run it constantly.
  //   * Whatever the connect built stays: the rollback stops at the mark, so
  //     it cannot invalidate a list that is older than this command.
  //   * It reuses the mapped chunks instead of unmapping and mapping them
  //     again, which is the point -- the slots are meant to be used over.
  //
  // The tables stay as large as the largest single result, rather than
  // shrinking back between commands. That is the accumulation gone, which is
  // what was wrong; the pages are backed by a deleted file, so the kernel can
  // take them back when it needs to.
  //
  // The rollback is also the moment a ~ResultTuples~ from the previous command
  // becomes invalid -- the nodes it is walking are exactly the ones being
  // handed back -- so the generation is bumped here, in the same place, rather
  // than anywhere it could drift out of step.
  void beginCommand()
  {
    ++state->generation;
    if (state->nl != 0) {
      state->nl->release(connected);
    }
  }

  // A sink for this command, or nothing when the caller did not pass one.
  static PyResultSink* sinkFor(NestedList* nl, const py::object& sink,
                               const bool want_text)
  {
    return sink.is_none() ? 0 : new PyResultSink(nl, sink, want_text);
  }

  static bool streamed(const std::unique_ptr<PyResultSink>& into)
  {
    return into && into->used();
  }

  // Raise what the Python handler raised, now that the read is over and the
  // connection is back in a usable state. Raised in preference to whatever
  // error the interrupted read reported, which would be the symptom.
  static void check(const std::unique_ptr<PyResultSink>& into)
  {
    if (into && !into->failure().empty()) {
      throw std::runtime_error(into->failure());
    }
  }

  // Puts the answer into ~out~ in whichever of the three forms it took.
  //
  //   streamed   the sink already has it, tuple by tuple, and nothing was
  //              built here at all
  //   type+tuples  the answer is a list of the shape (type value) that was
  //              read whole -- a textual transfer, or the optimizer's
  //              (plan result costs), neither of which the reader streams --
  //              so at least the *Python* tree is still built one tuple at a
  //              time, on demand
  //   tree       anything else, whole
  //
  // Which of the last two it is turns on the *shape* alone, never on the type
  // name: a two-element list with a list on the right can be split, and every
  // relation is one. Which type names are relations stays in Python, where
  // the payload builders already know it (app/table.py); a caller that gets a
  // split it cannot use just puts the halves back together, which for
  // anything that is not a relation is a handful of nodes.
  //
  // With a tree asked for as well and a split possible, the split wins:
  // building the tree too would build the very thing this avoids.
  void answer(py::dict& out, ListExpr res, const bool want_tree,
              const std::unique_ptr<PyResultSink>& into) const
  {
    NestedList* const nl = state->nl;
    if (streamed(into)) {
      out["streamed"] = true;
      out["type"] = py::none();
      out["tuples"] = py::none();
      out["tree"] = py::none();
      return;
    }
    const bool want_tuples = static_cast<bool>(into);
    const bool split = want_tuples && nl->ListLength(res) == 2
                       && !nl->IsAtom(nl->Second(res));
    out["streamed"] = false;
    out["type"] = split ? treeOf(nl, nl->First(res))
                        : py::object(py::none());
    out["tuples"] = split
        ? py::cast(ResultTuples(state, nl->Second(res)))
        : py::object(py::none());
    out["tree"] = (want_tree || (want_tuples && !split))
        ? treeOf(nl, res)
        : py::object(py::none());
  }

  // Splits the answer into the pieces Python needs, according to the level the
  // server resolved the command to.
  //
  // ~want_text~ gates only ~r.text~, the result rendered as a nested list --
  // the walk a caller who will not show it should not pay for. The plan, the
  // costs and a directive's message are always extracted: they are small, and
  // they are the answer for the levels that produce them.
  void extract(ListExpr res, AutoResult& r, const bool want_text) const
  {
    NestedList* const nl = state->nl;
    const int len = nl->ListLength(res);
    if (r.level == CMD_LEVEL_SQL && len >= 2) {
      // The SQL answer is the list (plan result costs). Testing the length
      // rather than the level alone is the guard the JavaGUI uses too
      // (CommandPanel.unwrapSqlAnswer): an answer that is not of that shape is
      // passed through unchanged. It also covers a trap -- SecondoInterfaceCS
      // assigns resolvedCmdLevel only on its "usual command" branch, so a
      // save/restore that the client carries out itself reports the *previous*
      // command's level together with an empty result list.
      r.plan = planOf(nl->First(res));
      r.hasPlan = true;
      r.result = nl->Second(res);
      if (want_text) {
        r.text = nl->ToString(r.result);
      }
      // The costs were appended to the answer, so a server that does not send
      // them still works.
      if (len >= 3 && nl->AtomType(nl->Third(res)) == RealType) {
        r.costs = nl->RealValue(nl->Third(res));
        r.hasCosts = true;
      }
      return;
    }
    if (r.level == CMD_LEVEL_OPT_DIRECTIVE) {
      // A directive answers with the text it printed; there is no result.
      r.message = nl->AtomType(res) == TextType ? nl->Text2String(res)
                                                : nl->ToString(res);
      r.hasMessage = true;
      r.text = "()";
      r.result = nl->TheEmptyList();
      return;
    }
    r.result = res;
    if (want_text) {
      r.text = nl->ToString(res);
    }
  }

  // The plan half of an SQL answer. The server builds it as a text atom, so
  // ToString would yield "<text>...</text--->" instead of the plan itself.
  std::string planOf(ListExpr planExpr) const
  {
    NestedList* const nl = state->nl;
    return trimmed(nl->AtomType(planExpr) == TextType
                     ? nl->Text2String(planExpr)
                     : nl->ToString(planExpr));
  }

  SecondoInterfaceCS* si = nullptr;
  // The connection's nested list, held indirectly so that a ResultTuples this
  // connection handed out can tell whether it is still valid. Every method
  // below takes it out into a local named ~nl~ and uses that.
  std::shared_ptr<ListState> state = std::make_shared<ListState>();
  NestedList::Mark connected = {0, 0, 0};
};

PYBIND11_MODULE(secondo_native, m)
{
  m.doc() = "Thin pybind11 wrapper over the SECONDO C++ client (libsecondo.a)";

  static py::exception<SecondoCommandError> secErr(m, "SecondoError",
                                                PyExc_RuntimeError);
  secondoErrorType = secErr;
  py::register_exception_translator([](std::exception_ptr p) {
    try {
      if (p) std::rethrow_exception(p);
    } catch (const SecondoCommandError& e) {
      // Raise SecondoError with the server's own numbers attached, so a caller
      // that wants them does not have to parse the message back apart.
      py::object err = py::reinterpret_steal<py::object>(
          PyObject_CallFunction(secondoErrorType.ptr(), "s", e.what()));
      if (err) {
        err.attr("code") = e.code;
        err.attr("pos") = e.pos;
        err.attr("plan") = e.plan.empty() ? py::object(py::none())
                                          : py::object(latin1(e.plan));
        PyErr_SetObject(secondoErrorType.ptr(), err.ptr());
      }
    }
  });

  // The rule for the "optimizer " prefix the user may type, taken from the
  // kernel (include/SQLLanguage.h) rather than copied into Python: what follows
  // the prefix decides what it means, and that decision belongs to the one
  // place that defines the input languages. Returns (had_prefix, remainder).
  m.def(
      "strip_optimizer_prefix",
      [](const std::string& command) {
        std::string rest;
        const bool had = stripOptimizerPrefix(command, rest);
        // The remainder is a substring of what Python just handed in, so it
        // goes back through the default (UTF-8) conversion -- decoding it as
        // Latin-1 would mangle any non-ASCII the user typed.
        return py::make_tuple(had, rest);
      },
      py::arg("command"),
      "Strip a leading \"optimizer \" keyword; returns (had_prefix, rest).");

  // Needs no connection: it is the kernel's own parser, not a server call.
  m.def("parse_nl", &parseNL, py::arg("text"),
        "Parse nested-list text with SECONDO's own parser (NLParser/NLLex) and "
        "return it as Python objects. Raises ValueError, carrying the parser's "
        "message, if SECONDO would not accept the text.");

  // Handed out by secondo/secondo_auto, never built from Python: it only
  // means anything alongside the connection whose result it is walking.
  py::class_<ResultTuples>(m, "ResultTuples")
      .def("__iter__", [](py::object self) { return self; })
      .def("__next__", &ResultTuples::next);

  py::class_<Connection>(m, "Connection")
      .def(py::init<const std::string&, const std::string&,
                    const std::string&, const std::string&,
                    const std::string&>(),
           py::arg("host"),
           py::arg("port"),
           py::arg("user") = "",
           py::arg("passwd") = "",
           py::arg("config") = "",
           "Open a client-server connection to a running SecondoMonitor.")
      .def("secondo", &Connection::secondo, py::arg("command"),
           py::arg("want_tree") = true,
           py::arg("want_text") = true,
           py::arg("sink") = py::none(),
           "Execute a SECONDO command; returns a dict with the result nested "
           "list as text (unless want_text is off, when it is \"\") and as "
           "Python objects (unless want_tree is off, when it is None). Pass a "
           "sink -- an object with begin(type) and elem(tuple) -- to be given "
           "the answer as it is read instead; `streamed` says whether it was.")
      .def("optimizer_available", &Connection::optimizer_available,
           "Whether this server can run the SQL dialect (optimizer).")
      .def("secondo_auto", &Connection::secondo_auto, py::arg("command"),
           py::arg("optimizer_addressed") = false,
           py::arg("want_tree") = true,
           py::arg("want_text") = true,
           py::arg("sink") = py::none(),
           "Execute a command the server classifies itself; returns a dict "
           "with level/text/streamed/tree/type/tuples/plan/costs/message. "
           "want_tree, want_text and sink gate the forms of the result "
           "independently; see secondo for what sink does.")
      .def("optimizer_command", &Connection::optimizer_command,
           py::arg("directive"),
           "Run an optimizer control directive; return the text it printed.")
      .def("close", &Connection::close, "Close the connection.");
}
