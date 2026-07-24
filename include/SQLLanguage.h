/*
----
This file is part of SECONDO.

Copyright (C) 2026,
University in Hagen,
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

1 Which language is a typed command written in?

SECONDO understands three input languages: nested lists, the SOS text syntax,
and the SQL handled by the optimizer. Which one a typed command belongs to used
to be decided by every client for itself -- the TTYs, the JavaGUI and
the JDBC driver each carried their own copy of the rules -- and the copies had
to be kept in step, or the same command started behaving differently in the GUI
and in a TTY talking to the same server.

The classification is a property of the *language*, not of the client, so it
lives here, next to the dispatch it feeds. Clients ask the server to classify by
sending the command level ~CMD\_LEVEL\_AUTO~; the server answers with the level
it resolved to (see ~SecondoServer::CallSecondo~). The one client that has no
server to ask -- the standalone SecondoBDB, which runs the optimizer in-process
-- calls the functions below directly. Two call sites, one rule set.

*/

#ifndef SEC_SQL_LANGUAGE_H
#define SEC_SQL_LANGUAGE_H

#include <string>

/*
1.1 Command levels

The levels 0 to 2 are the ones a command is actually executed at and the only
ones a client may send besides ~CMD\_LEVEL\_AUTO~, the sentinel that asks the
server to classify the command. ~CMD\_LEVEL\_OPT\_DIRECTIVE~ only ever appears
in the server's answer, never in a request.

Whether the user addressed the optimizer explicitly is not a language but an
intent about execution, so it rides on the per-command protocol flags of the
level line next to ~planonly~, not on a level of its own.

*/
const int CMD_LEVEL_NESTED_LIST    =  0;  // nested list syntax
const int CMD_LEVEL_TEXT           =  1;  // SOS text syntax
const int CMD_LEVEL_SQL            =  2;  // SQL, optimized and executed
const int CMD_LEVEL_OPT_DIRECTIVE  =  3;  // optimizer control goal (reply only)

const int CMD_LEVEL_AUTO           = -1;  // server, you decide

/*
1.2 The rules

~looksLikeSql~ tells whether a command is written in SQL and therefore has to go
to the optimizer. Deciding this is not a matter of the first keyword alone:
~delete~, ~create~, ~update~ and ~let~ exist in both languages and are told
apart by the following token(s). Anything not recognized falls through to the
kernel -- the fallback is always the kernel.

~isOptimizerDirective~ recognizes the optimizer's control goals (~showOptions~,
~setOption(X)~, ~updateCatalog~, ...). They are neither SQL nor kernel commands:
they are Prolog goals run by the optimizer, and what they produce is the text
they print.

~deriveKernelCommandLevel~ picks 0 or 1 for a command that is neither: a leading
~(~ means nested list syntax, anything else is text. It is inline because a pure
client -- one that never classifies, but still has to recognize the ~save~ and
~restore~ commands it carries out itself -- needs this and nothing else; that
way such a client links without the rule set below (see ~SecondoInterfaceCS~,
which the C++ API packages on its own).

~resolveCommandLevel~ combines the three and is what the server calls. With
~optimizerAddressed~ (the client sent the ~optimizer~ flag because the user
wrote the ~optimizer~ prefix) anything that is not SQL is taken to be a
directive, so arbitrary optimizer goals can be run; without it only the known
directive names are.

*/
bool looksLikeSql(const std::string& cmd);

bool isOptimizerDirective(const std::string& cmd);

inline int deriveKernelCommandLevel(const std::string& cmd)
{
  size_t s = cmd.find_first_not_of(" \t\n\r\f\v\b\a");
  if ( s != std::string::npos && cmd[s] == '(' ) return CMD_LEVEL_NESTED_LIST;
  return CMD_LEVEL_TEXT;
}

int resolveCommandLevel(const std::string& cmd, bool optimizerAddressed);

/*
1.3 The ~optimizer~ prefix

Strips a leading ~optimizer~ keyword -- the way a user addresses the optimizer
explicitly -- and puts the remainder into ~rest~. Returns false and leaves
~rest~ = ~cmd~ if the prefix is not there. What follows the prefix decides what
it means: SQL is optimized but not executed, anything else is run as a
directive.

*/
bool stripOptimizerPrefix(const std::string& cmd, std::string& rest);

#endif
