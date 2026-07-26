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

The only thing crossing the language boundary is the nested list rendered
as *text* (~NestedList::ToString~). Turning that text into GeoJSON is done
in Python, where it is easy to fixture-test.

*/

#include <pybind11/pybind11.h>

#include <string>
#include <stdexcept>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "SQLLanguage.h"
#include "NestedList.h"

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
        nl = si->GetNestedList();
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
  py::str secondo(const std::string& command)
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    SecErrInfo err;
    std::string out;
    {
      // The call blocks on network I/O; let other Python threads run, and
      // let commands on other connections run alongside this one.
      py::gil_scoped_release release;
      ListExpr res = nl->TheEmptyList();
      si->Secondo(command, res, err);
      if (err.code == 0) {
        out = nl->ToString(res);
      }
    }
    if (err.code != 0) {
      throw secondoError(err.code, err.pos, err.msg, "");
    }
    return latin1(out);
  }

  // Asks the connected server whether it can run the SQL dialect (the optimizer
  // is compiled in and enabled). A property of *that server*, so the answer
  // must not be cached beyond the life of this connection.
  bool optimizer_available()
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
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
                        const bool optimizer_addressed)
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    AutoResult r;
    {
      // Both the call and the nested-list extraction are pure C++, so the GIL
      // stays released for all of it.
      py::gil_scoped_release release;
      ListExpr res = nl->TheEmptyList();
      si->SecondoAuto(command, optimizer_addressed, r.level, res,
                      r.errCode, r.errPos, r.errMsg);
      if (r.errCode == 0) {
        extract(res, r);
      } else if (r.level == CMD_LEVEL_SQL && nl->ListLength(res) >= 2) {
        // A plan that failed to execute still comes back with the answer; every
        // other client throws it away. Carry it on the exception so the console
        // can show what actually ran.
        r.plan = planOf(nl->First(res));
        r.hasPlan = true;
      }
    }
    if (r.errCode != 0) {
      throw secondoError(r.errCode, r.errPos, r.errMsg,
                         r.hasPlan ? r.plan : "");
    }

    py::dict out;
    out["level"] = r.level;
    out["text"] = latin1(r.text);
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
      out = si->optimizerCommand(directive);
    }
    return latin1(out);
  }

  void close()
  {
    if (si) {
      // Terminate talks to the server and deleting the interface frees this
      // connection's nested list; both are this connection's own business.
      py::gil_scoped_release release;
      si->Terminate();
      delete si;
      si = nullptr;
      nl = nullptr;
    }
  }

 private:
  // Splits the answer into the pieces Python needs, according to the level the
  // server resolved the command to.
  void extract(ListExpr res, AutoResult& r) const
  {
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
      r.text = nl->ToString(nl->Second(res));
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
      return;
    }
    r.text = nl->ToString(res);
  }

  // The plan half of an SQL answer. The server builds it as a text atom, so
  // ToString would yield "<text>...</text--->" instead of the plan itself.
  std::string planOf(ListExpr planExpr) const
  {
    return trimmed(nl->AtomType(planExpr) == TextType
                     ? nl->Text2String(planExpr)
                     : nl->ToString(planExpr));
  }

  SecondoInterfaceCS* si = nullptr;
  NestedList* nl = nullptr;
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
           "Execute a SECONDO command; return the result nested list as text.")
      .def("optimizer_available", &Connection::optimizer_available,
           "Whether this server can run the SQL dialect (optimizer).")
      .def("secondo_auto", &Connection::secondo_auto, py::arg("command"),
           py::arg("optimizer_addressed") = false,
           "Execute a command the server classifies itself; returns a dict "
           "with level/text/plan/costs/message.")
      .def("optimizer_command", &Connection::optimizer_command,
           py::arg("directive"),
           "Run an optimizer control directive; return the text it printed.")
      .def("close", &Connection::close, "Close the connection.");
}
