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

#ifndef KAFKA_SIGNALINGSOCKETS_H
#define KAFKA_SIGNALINGSOCKETS_H

#include <iostream>

#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <thread>
#include <arpa/inet.h>

class SignallingSocket {
private:
    int sockfd;
    struct sockaddr_in address;
    std::thread *sockThread;
    bool socketShutdown = false;
    bool signalReceived = false;

public:

    SignallingSocket() {
    }

    ~SignallingSocket() {
    }


    int open(int port) {
        // Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            return 1;
        }

        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("setsockopt");
            return 2;
        }

        // Forcefully attaching socket to the port
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
        return 0;
    }

    int close() {
        socketShutdown = true;
        shutdown(sockfd, SHUT_RD);
        ::close(sockfd);
        sockThread->join();
        delete sockThread;
        return 0;
    }

    bool isSignalReceived() {
        return signalReceived;
    }

private:
    void waitForConnection() {
        std::cout << "Starting waiting for connection" << std::endl;
        int new_socket;
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
            int valread = read(new_socket, buffer, 1024);
            printf("Received signal: %s\n", buffer);
            signalReceived = strncmp(expected, buffer, valread) == 0;
            if (signalReceived)
                send(new_socket, responseAccepted, strlen(responseAccepted), 0);
            else
                send(new_socket, responseIgnored, strlen(responseIgnored), 0);
        }
    }

};


class SignallingClient {

public:
    int sendSignal(std::string host, int port) {
        int sock = 0;
        struct sockaddr_in serv_addr;
        char const *message = "Finish";
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return 1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            return 2;
        }

        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
            0) {
            printf("\nConnection Failed \n");
            return 3;
        }
        send(sock, message, strlen(message), 0);
        printf("Hello message sent\n");

        char buffer[1024] = {0};
        read(sock, buffer, 1024);
        printf("Response from server: %s\n", buffer);
        return 0;
    }
};


#endif //KAFKA_SIGNALINGSOCKETS_H