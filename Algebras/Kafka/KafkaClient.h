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

#ifndef KAFKA_KAFKACLIENT_H
#define KAFKA_KAFKACLIENT_H

#include <iostream>
#include <librdkafka/rdkafkacpp.h>

namespace kafka {

#define LOG_PROGRESS_INTERVAL 10000

    class KafkaWriterClient {
    private:
        std::string brokers;
        std::string topic_str;
        RdKafka::Producer *producer;
        unsigned long written_count = 0;
    public:
        void Open(std::string brokers, std::string topic_str);

        void Write(std::string line);

        void Write(void *payload, size_t len);

        void Close();
    };

    struct KafkaMessage {
        bool run = true;
        std::string *key = NULL;
        size_t len = 0;
        void *payload = NULL;
    };

    class ExampleRebalanceCb;

    class KafkaReaderClient {
    private:
        std::string brokers;
        std::string topic_str;
        RdKafka::KafkaConsumer *consumer;
        long msg_cnt = 0;
        int64_t msg_bytes = 0;
        /* Control exit*/
        bool exit_eof = true;
        ExampleRebalanceCb *ex_rebalance_cb;
        int eof_cnt = 0;
        bool exit_on_timeout = false;
        unsigned long read_count = 0;
    public:
        void Open(std::string brokers, std::string topic_str);

        std::string *ReadSting();

        void Close();

        bool isExitOnTimeout() const {
            return exit_on_timeout;
        }

        void setExitOnTimeout(bool exitOnTimeout) {
            exit_on_timeout = exitOnTimeout;
        }

    private:
        KafkaMessage *msg_consume(RdKafka::Message *message, int partition_cnt);
    };


}

#endif //KAFKA_KAFKACLIENT_H
