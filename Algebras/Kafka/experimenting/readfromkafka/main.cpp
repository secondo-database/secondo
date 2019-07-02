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


#include "librdkafka/rdkafkacpp.h"


static void metadata_print(const std::string &topic,
                           const RdKafka::Metadata *metadata) {
    std::cout << "Metadata for " << (topic.empty() ? "" : "all topics")
              << "(from broker " << metadata->orig_broker_id()
              << ":" << metadata->orig_broker_name() << std::endl;

    /* Iterate brokers */
    std::cout << " " << metadata->brokers()->size() << " brokers:" << std::endl;
    RdKafka::Metadata::BrokerMetadataIterator ib;
    for (ib = metadata->brokers()->begin();
         ib != metadata->brokers()->end();
         ++ib) {
        std::cout << "  broker " << (*ib)->id() << " at "
                  << (*ib)->host() << ":" << (*ib)->port() << std::endl;
    }
    /* Iterate topics */
    std::cout << metadata->topics()->size() << " topics:" << std::endl;
    RdKafka::Metadata::TopicMetadataIterator it;
    for (it = metadata->topics()->begin();
         it != metadata->topics()->end();
         ++it) {
        std::cout << "  topic \"" << (*it)->topic() << "\" with "
                  << (*it)->partitions()->size() << " partitions:";

        if ((*it)->err() != RdKafka::ERR_NO_ERROR) {
            std::cout << " " << err2str((*it)->err());
            if ((*it)->err() == RdKafka::ERR_LEADER_NOT_AVAILABLE)
                std::cout << " (try again)";
        }
        std::cout << std::endl;

        /* Iterate topic's partitions */
        RdKafka::TopicMetadata::PartitionMetadataIterator ip;
        for (ip = (*it)->partitions()->begin();
             ip != (*it)->partitions()->end();
             ++ip) {
            std::cout << "    partition " << (*ip)->id()
                      << ", leader " << (*ip)->leader()
                      << ", replicas: ";

            /* Iterate partition's replicas */
            RdKafka::PartitionMetadata::ReplicasIterator ir;
            for (ir = (*ip)->replicas()->begin();
                 ir != (*ip)->replicas()->end();
                 ++ir) {
                std::cout << (ir == (*ip)->replicas()->begin() ? "" : ",")
                          << *ir;
            }

            /* Iterate partition's ISRs */
            std::cout << ", isrs: ";
            RdKafka::PartitionMetadata::ISRSIterator iis;
            for (iis = (*ip)->isrs()->begin();
                 iis != (*ip)->isrs()->end(); ++iis)
                std::cout << (iis == (*ip)->isrs()->begin() ? "" : ",") << *iis;

            if ((*ip)->err() != RdKafka::ERR_NO_ERROR)
                std::cout << ", " << RdKafka::err2str((*ip)->err())
                          << std::endl;
            else
                std::cout << std::endl;
        }
    }
}

void
metadataMode(RdKafka::Conf *conf, RdKafka::Conf *tconf, std::string topic_str) {
/* Metadata mode */
    std::string errstr;

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
     * Create topic handle.
     */
    RdKafka::Topic *topic = NULL;
    if (!topic_str.empty()) {
        topic = RdKafka::Topic::create(producer, topic_str, tconf, errstr);
        if (!topic) {
            std::cerr << "Failed to create topic: " << errstr << std::endl;
            exit(1);
        }
    }

    class RdKafka::Metadata *metadata;

    /* Fetch metadata */
    RdKafka::ErrorCode err = producer->metadata(topic != NULL, topic,
                                                &metadata, 5000);
    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "%% Failed to acquire metadata: "
                  << RdKafka::err2str(err) << std::endl;
    } else {
        metadata_print(topic_str, metadata);
        delete metadata;
    }

}

static bool run = true;
static bool exit_eof = false;

void msg_consume(RdKafka::Message *message, void *opaque) {

    switch (message->err()) {
        case RdKafka::ERR__TIMED_OUT:
            break;

        case RdKafka::ERR_NO_ERROR:
            /* Real message */
            std::cout << "Read msg at offset " << message->offset()
                      << std::endl;
            if (message->key()) {
                std::cout << "Key: " << *message->key() << std::endl;
            }

            printf("%.*s\n",
                   static_cast<int>(message->len()),
                   static_cast<const char *>(message->payload()));
            break;

        case RdKafka::ERR__PARTITION_EOF:
            /* Last message */
            if (exit_eof) {
                run = false;
            }
            break;

        case RdKafka::ERR__UNKNOWN_TOPIC:
        case RdKafka::ERR__UNKNOWN_PARTITION:
            std::cerr << "Consume failed: " << message->errstr() << std::endl;
            run = false;
            break;

        default:
            /* Errors */
            std::cerr << "Consume failed: " << message->errstr() << std::endl;
            run = false;
    }
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::string brokers = "localhost";
    std::string errstr;
    std::string topic_str = "test";

    /*
     * Originally it was int32_t partition = RdKafka::Topic::PARTITION_UA;
     *
     * in this test we read only the first partition
     */

    int32_t partition = RdKafka::Topic::PARTITION_UA;
    int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;
    /*
     * Create configuration objects
     */
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    /*
    * Set configuration properties
    */
    conf->set("metadata.broker.list", brokers, errstr);

    /*
    ExampleEventCb ex_event_cb;
    conf->set("event_cb", &ex_event_cb, errstr);
     */


    metadataMode(conf, tconf, topic_str);

    /*
     * Consumer mode
     */

    conf->set("enable.partition.eof", "true", errstr);

    /*
     * Create consumer using accumulated global configuration.
     */
    RdKafka::Consumer *consumer = RdKafka::Consumer::create(conf, errstr);
    if (!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        exit(1);
    }

    std::cout << "% Created consumer " << consumer->name() << std::endl;

    /*
     * Create topic handle.
     */
    RdKafka::Topic *topic = RdKafka::Topic::create(consumer, topic_str,
                                                   tconf, errstr);
    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        exit(1);
    }

    /*
     * Start consumer for topic+partition at start offset
     */
    RdKafka::ErrorCode resp = consumer->start(topic, partition, start_offset);
    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to start consumer: " <<
                  RdKafka::err2str(resp) << std::endl;
        exit(1);
    }


    /*
     * Consume messages
     */
    while (run) {
        RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
        msg_consume(msg, NULL);
        delete msg;
    }

    /*
     * Stop consumer
     */
    consumer->stop(topic, partition);

    consumer->poll(1000);

    delete topic;
    delete consumer;


    delete conf;
    delete tconf;

    /*
     * Wait for RdKafka to decommission.
     * This is not strictly needed (when check outq_len() above), but
     * allows RdKafka to clean up all its resources before the application
     * exits so that memory profilers such as valgrind wont complain about
     * memory leaks.
     */
    RdKafka::wait_destroyed(5000);
    return 0;
}