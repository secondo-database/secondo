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
#include "NestedList.h"
#include "NList.h"

namespace py = pybind11;

// One SECONDO client session. Not thread-safe on its own: callers must
// serialize access to a single Connection (the FastAPI layer keeps one
// Connection per browser session and guards it with a lock).
class Connection
{
 public:
  Connection(const std::string& host,
             const std::string& port,
             const std::string& user,
             const std::string& passwd,
             const std::string& config)
  {
    si = new SecondoInterfaceCS(true, 0, false);
    si->InitRTFlags(config);

    std::string errMsg;
    const bool multiUser = true;  // host/port take precedence over config
    bool ok;
    {
      // Connecting blocks on network I/O and can hang if the SECONDO server is
      // slow or wedged. Without releasing the GIL that would freeze the whole
      // Python process (even endpoints that never touch SECONDO).
      py::gil_scoped_release release;
      ok = si->Initialize(user, passwd, host, port, config, "",
                          errMsg, multiUser);
    }
    if (!ok) {
      delete si;
      si = nullptr;
      throw std::runtime_error("Cannot connect to SECONDO server at " +
                               host + ":" + port + " - " + errMsg);
    }
    nl = si->GetNestedList();
    NList::setNLRef(nl);
  }

  ~Connection() { close(); }

  // Run one SECONDO command and return the result nested list as text.
  // Raises RuntimeError on a SECONDO error (non-zero error code).
  //
  // SECONDO stores strings in Latin-1 (ISO-8859-1), so the nested-list text
  // may contain bytes (e.g. 0xfc for 'u"'/umlaut) that are invalid UTF-8.
  // pybind11's default std::string -> str conversion assumes UTF-8 and would
  // raise UnicodeDecodeError on such results (Kinos, WFlaechen, ...). Decode
  // Latin-1 explicitly: it is lossless for any byte and yields correct Unicode.
  py::str secondo(const std::string& command)
  {
    if (!si) {
      throw std::runtime_error("connection is closed");
    }
    ListExpr res = nl->TheEmptyList();
    SecErrInfo err;
    {
      // The call blocks on network I/O; let other Python threads run.
      py::gil_scoped_release release;
      si->Secondo(command, res, err);
    }
    if (err.code != 0) {
      throw std::runtime_error("SECONDO error " + std::to_string(err.code) +
                               " (pos " + std::to_string(err.pos) + "): " +
                               err.msg);
    }
    std::string out = nl->ToString(res);
    return py::reinterpret_steal<py::str>(
        PyUnicode_DecodeLatin1(out.data(), out.size(), "replace"));
  }

  void close()
  {
    if (si) {
      {
        // Terminate talks to the server too; same reasoning as above.
        py::gil_scoped_release release;
        si->Terminate();
      }
      delete si;
      si = nullptr;
      nl = nullptr;
    }
  }

 private:
  SecondoInterfaceCS* si = nullptr;
  NestedList* nl = nullptr;
};

PYBIND11_MODULE(secondo_native, m)
{
  m.doc() = "Thin pybind11 wrapper over the SECONDO C++ client (libsecondo.a)";

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
      .def("close", &Connection::close, "Close the connection.");
}
