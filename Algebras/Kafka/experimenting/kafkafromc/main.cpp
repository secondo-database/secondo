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

#include <librdkafka/rdkafkacpp.h>

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::string brokers = "localhost";
    static bool run = true;
    std::string errstr;
    std::string topic_str = "test";
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    int32_t partition = RdKafka::Topic::PARTITION_UA;



    /*
    * Set configuration properties
    */
    conf->set("metadata.broker.list", brokers, errstr);

    //    ExampleDeliveryReportCb ex_dr_cb;
    //
    //    /* Set delivery report callback */
    //    conf->set("dr_cb", &ex_dr_cb, errstr);
    //
    //    conf->set("default_topic_conf", tconf, errstr);


    /*
     * Create producer using accumulated global configuration.
     */
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        exit(1);
    }

    std::cout << "% Created producer " << producer->name() << std::endl;

    /*
     * Read messages from stdin and produce to broker.
     */
    for (std::string line; run && std::getline(std::cin, line);) {
        if (line.empty()) {
            producer->poll(0);
            continue;
        }
        run = line.compare("exit");

        /*
        * Produce message
        */
        RdKafka::ErrorCode resp =
                producer->produce(topic_str, partition,
                                  RdKafka::Producer::RK_MSG_COPY,
                        /* Value */
                                  const_cast<char *>(line.c_str()), line.size(),
                        /* Key */
                                  NULL, 0,
                        /* Timestamp (defaults to now) */
                                  0,
                        /* Per-message opaque value passed to
                         * delivery report */
                                  NULL);
        if (resp != RdKafka::ERR_NO_ERROR) {
            std::cerr << "% Produce failed: " <<
                      RdKafka::err2str(resp) << std::endl;
        } else {
            std::cerr << "% Produced message (" << line.size() << " bytes)" <<
                      std::endl;
        }

        producer->poll(0);

    }
    run = true;

    while (run && producer->outq_len() > 0) {
        std::cerr << "Waiting for " << producer->outq_len() << std::endl;
        producer->poll(1000);
    }

    delete producer;


    return 0;
}