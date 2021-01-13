/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
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

*/
#include "catch.hh"

#include "Host.hpp"

using namespace DBService;
using namespace std;

TEST_CASE("Constructing Hosts")
{
  SECTION("Constructing a simple Host")
  {
    Host host;
    REQUIRE(host.getHostname() == "");

    // Testing the overloaded != operator
    REQUIRE(!(host.getHostname() != ""));
  }

  SECTION("Two empty hosts should be equal")
  {
    Host host1;
    Host host2;

    // They are string-equal and object-equal
    REQUIRE(host1.getHostname() == host2.getHostname());
    REQUIRE(host1 == host2);
  }

  SECTION("An empty host and a non-empty host shouldn't be equal")
  {
    Host host1;
    Host host2("localhost");
    
    // They are string-unequal and object-unequal
    REQUIRE(host1.getHostname() != host2.getHostname());
    REQUIRE(host1 != host2);
  }

  SECTION("Two equal non-empty dns hosts should be considered equal")
  {
    Host host1("localhost");
    Host host2("localhost");

    REQUIRE(host1.getHostname() == host2.getHostname());
    REQUIRE(host1 == host2);
  }

  SECTION("Two equal non-empty IP hosts should be considered equal")
  {
    Host host1("127.0.0.1");
    Host host2("127.0.0.1");

    REQUIRE(host1.getHostname() == host2.getHostname());
    REQUIRE(host1 == host2);
  }

  SECTION("Two hosts should be considered equal if they resolve to the same IP \
    address")
  {
    Host host1("localhost");
    Host host2("127.0.0.1");

    // They are string-unequal but object-equal due to the DNS resolving.
    REQUIRE(host1.getHostname() != host2.getHostname());
    REQUIRE(host1 == host2);
  }

  SECTION("Two hosts should be considered unequal if they are different and \
    one of them doesn't resolve")
  {
    Host host1("localhost");
    Host host2("nonlocalhost");

    // They are string-unequal but object-equal due to the DNS resolving.
    REQUIRE(host1.getHostname() != host2.getHostname());
    REQUIRE(host1 != host2);
  }
}