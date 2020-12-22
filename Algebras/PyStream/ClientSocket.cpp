/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

#include "ClientSocket.h"
#include "SocketException.h"


ClientSocket::ClientSocket ( std::string host, int port )
{
    if ( ! Socket::create() )
    {
        throw SocketException ( "Could not create client socket." );
    }
    if ( ! Socket::connect ( host, port ) )
    {
        throw SocketException ( "Could not bind to port." );
    }

}


const ClientSocket& ClientSocket::operator << ( const std::string& s ) const
{
    if ( ! Socket::send ( s ) )
    {
        throw SocketException ( "Could not write to socket." );
    }

    return *this;

}


const ClientSocket& ClientSocket::operator >> ( std::string& s ) const
{
    if ( Socket::recvLine ( s ) == -1)
    {
        throw SocketException ( "Could not read line from socket." );
    }

    return *this;
}
