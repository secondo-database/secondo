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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of class LRUCache.

[toc]

1 LRUCache implementation

*/

#ifndef _LRUCACHE_H_
#define _LRUCACHE_H_

#include <iostream>
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "StandardTypes.h"

namespace cstream {

    class LRUCache {

        public:
/*
1.1 Constructor

Creates a new cache with size 1

*/
            LRUCache() : _maxsize(1) {

            }

/*
1.2 Constructor

Creates a new cache with a given size

*/
            LRUCache(const size_t maxsize) : _maxsize(maxsize) {
                
            }

/*
1.3 Destructor

*/
            ~LRUCache() {
                _list.clear();
            }

/*
1.3 insert

Insert a new value at the front of the cache. If the value is already in 
the cache the value is only stored once at the front. The old value will be 
deleted. If the cache is full and a new value is insert at the front and the 
cache deletes the last value.

*/
            bool insert(std::string value) {
                boost::unique_lock<boost::mutex> lock(_cacheGuard);

                if(_list.empty()) {
                    _list.push_front(value);
                    return true;
                }

                std::list<std::string>::iterator it = 
                    std::find(_list.begin(), _list.end(), value);
                if(it == _list.end()) {
                    if(_list.size() == _maxsize)
                        _list.pop_back();
                    _list.push_front(value);
                    return true;
                }
                else {
                    _list.erase(it);
                    _list.push_front(value);
                }

                return false;
            }
/*
1.4 sendCache

Sends n values to a given iostream.

*/
            void sendCache(std::iostream& io, int number) {
                boost::unique_lock<boost::mutex> lock(_cacheGuard);
                if(number == -1 || (unsigned)number >= _list.size()) {
                    for (std::list<std::string>::iterator it = _list.begin() ; 
                                it != _list.end() ; ++it)
                        DBService::CommunicationUtils::sendLine(io, (*it));
                }
                else {
                    for (std::list<std::string>::iterator it = _list.begin() ; 
                                it != _list.end() && number>0 ; ++it) {
                        DBService::CommunicationUtils::sendLine(io, (*it));
                        number = number - 1;
                    }
                }
            }

        private:
/*
2 Member Definitions

2.1 maxsize

Maximal size of the cache.

*/
            const size_t _maxsize;
/*
2.2 list

List for the cache.

*/
            std::list<std::string> _list;
/*
2.3 cacheGuard

Mutex to make the cache threadsafe.

*/
            boost::mutex _cacheGuard;
    };

} /* namespace cstream */

#endif // _LRUCACHE_H_