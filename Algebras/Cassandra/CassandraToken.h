/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2014, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 Jan Kristof Nidzwetzki

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 A representation class for the cassandra token 
 
 1 Includes and defines

*/

#ifndef _CASSANDRA_TOKEN_H
#define _CASSANDRA_TOKEN_H


/*
1.1 Namspace

*/
namespace cassandra {


/*
2.0 Result objects for tokens

*/
class CassandraToken {
  
public:

/*
2.0.1 Default constructor

*/
  CassandraToken(long long myToken, string myIp) 
    : token(myToken), ip(myIp) {
      
  }
  
/*
2.0.2 Is IP local?

*/  
  bool isLocalToken() const {
    return ip.compare("127.0.0.1");
  }
  
/*
2.0.3 Getter for token

*/
  long long getToken() const {
    return token;
  }

/*
2.0.4 Getter for ip

*/
  string getIp() const {
    return ip;
  }

/*
2.0.5 Operator <

*/
  bool operator<( const CassandraToken& val ) const { 
        return getToken() < val.getToken(); 
  }

private:
  long long token;
  string ip;
};

/*
2.0.6 Implementation for "toString"

*/
inline std::ostream& operator<<(std::ostream &strm, 
                         const cassandra::CassandraToken &cassandraToken) {
  
  return strm << "CassandraToken[" << cassandraToken.getToken() 
              << " / " << cassandraToken.getIp()  << "]" << endl;
}

} // Namespace


#endif
