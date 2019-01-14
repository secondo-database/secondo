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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This file defines the members of class Metrics

*/
#include "Metrics.h"

namespace pregel {
 Metrics Metrics::singleton = Metrics();

 Metrics::Report Metrics::report() {
  using namespace std::chrono;
  auto report = Report();
  auto superstep = SuperstepCounter::get();
  report.messagesSentOverNetwork = messagesSentOverNetwork;

  report.messagesQueuedDirectly = messagesQueuedDirectly;

  report.messagesSentPerSuperstep =
   (superstep != 0) ?
   (messagesSentOverNetwork + messagesQueuedDirectly) / (double) superstep : 0;

  report.messagesReceived = messagesReceived;

  report.messagesReceivedPerSuperstep =
   (superstep != 0) ?
   messagesReceived / (double) superstep : 0;

  report.messagesDiscarded = messagesDiscarded;

  report.timeIdle =
   duration_cast<milliseconds>(timeIdle).count() / (double) 1000;

  report.timeProductive =
   duration_cast<milliseconds>(timeProductive).count() / (double) 1000;

  auto timeTotal = timeProductive + timeIdle;

  report.productivity =
   (timeTotal != TIME_ZERO) ?
   timeProductive.count() / (double) timeTotal.count() : 0;

  return report;
 }

 void Metrics::sentMessage() {
  ++messagesSentOverNetwork;
 }

 void Metrics::queuedMessage() {
  ++messagesQueuedDirectly;
 }

 void Metrics::receivedMessage() {
  ++messagesReceived;
 }

 void Metrics::discardedMessage() {
  ++messagesDiscarded;
 }

 void Metrics::startClock(bool productive) {
  using namespace std::chrono;
  if (stopWatch.clock != TIME_ZERO) {
   std::cerr << "Was already counting something\n";
  }

  stopWatch.countingProductiveTime = productive;
  stopWatch.clock = duration_cast<milliseconds>(
   system_clock::now().time_since_epoch()
  );
 }

 void Metrics::stopClock() {
  using namespace std::chrono;
  if (stopWatch.clock == TIME_ZERO) {
   std::cerr << "Wasn't counting\n";
   return;
  }
  auto now = duration_cast<milliseconds>(
   system_clock::now().time_since_epoch()
  );

  auto duration = now - stopWatch.clock;
  if (stopWatch.countingProductiveTime) {
   timeProductive += duration;
  } else {
   timeIdle += duration;
  }

  stopWatch.clock = TIME_ZERO;
 }

 Metrics &Metrics::get() {
  return singleton;
 }

 void Metrics::Report::print(std::stringstream &sstream) {
  sstream << "  == Metrics ==" << std::endl;
  sstream << "    Messages sent over network : " <<
          messagesSentOverNetwork << std::endl;
  sstream << "               queued directly : " <<
          messagesQueuedDirectly << std::endl;
  sstream << "                 per Superstep : " <<
          messagesSentPerSuperstep << std::endl;
  sstream << "    Messages received          : " <<
          messagesReceived << std::endl;
  sstream << "                 per Superstep : " <<
          messagesReceivedPerSuperstep << std::endl;
  sstream << "    Messages discarded         : " <<
          messagesDiscarded << std::endl;
  sstream << std::endl;
  sstream << "    Time productive            : " <<
          timeProductive << "s" << std::endl;
  sstream << "    Time idle                  : " <<
          timeIdle << "s" << std::endl;
  sstream << "    Productivity               : " <<
          (100.0 * productivity) << "%" << std::endl;
  sstream << std::endl;
 }

 void Metrics::Report::fill(PregelStatus2Helper& ps2h){
    ps2h.setMessagesSent(messagesSentOverNetwork);
    ps2h.setMessagesDirect(messagesQueuedDirectly);
    ps2h.setMessagesSentPerSuperstep(messagesSentPerSuperstep);
    ps2h.setMessagesReceived(messagesReceived);
    ps2h.setMessagesReceivedPerSuperStep(messagesReceivedPerSuperstep);
    ps2h.setMessagesDiscarded(messagesDiscarded);
    ps2h.setTimeProductive(timeProductive);
    ps2h.setTimeIdle(timeIdle);
    ps2h.setProductivity(100.0 * productivity);
 }


}
