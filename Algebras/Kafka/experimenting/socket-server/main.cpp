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

#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>


class SimpleSocket {
    int sockfd;
    struct sockaddr_in address;
    int port;
    std::thread *sockThread;
    bool socketShutdown = false;
public:

    bool signalReceived = false;

    SimpleSocket(int portParam) : port(portParam) {
    }

    int open() {
        // Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            return 1;
        }

        // Forcefully attaching socket to the port 8080
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("setsockopt");
            return 2;
        }

        // Forcefully attaching socket to the port 8080
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr *) &address,
                 sizeof(address)) < 0) {
            perror("bind failed");
            return 3;
        }
        if (listen(sockfd, 3) < 0) {
            perror("listen");
            return 4;
        }
        sockThread = new std::thread([this] { this->waitForConnection(); });
    }

    void waitForConnection() {
        std::cout << "Starting waiting for connection" << std::endl;
        int new_socket, valread;
        int addrlen = sizeof(address);
        char const *expected = "Finish";
        char const *responseAccepted = "Finish accepted";
        char const *responseIgnored = "False request";

        while (!signalReceived) {
            if ((new_socket = accept(sockfd, (struct sockaddr *) &address,
                                     (socklen_t *) &addrlen)) < 0) {
                if (socketShutdown)
                    return;
                perror("accept");
                exit(EXIT_FAILURE);
            }
            char buffer[1024] = {0};
            valread = read(new_socket, buffer, 1024);
            printf("Received signal: %s\n", buffer);
            signalReceived = strcmp(expected, buffer) == 0;
            if (signalReceived)
                send(new_socket, responseAccepted, strlen(responseAccepted), 0);
            else
                send(new_socket, responseIgnored, strlen(responseAccepted), 0);
        }
    }

    int close() {
        socketShutdown = true;
        shutdown(sockfd, SHUT_RD);
        ::close(sockfd);
        sockThread->join();
        delete sockThread;
    }

    ~SimpleSocket() {
    }

};

int main() {

    std::cout << "Starting socket server self stop" << std::endl;
    SimpleSocket simpleSocket1(8080);
    simpleSocket1.open();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    simpleSocket1.close();
    std::cout << "Finished 1" << std::endl;

    std::cout << "Starting socket server for waiting" << std::endl;
    SimpleSocket simpleSocket2(8080);
    simpleSocket2.open();
    while (!simpleSocket2.signalReceived) {
        std::cout << "Waiting" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    simpleSocket2.close();
    std::cout << "Finished 2" << std::endl;
    return 0;
}