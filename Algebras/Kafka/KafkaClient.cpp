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

#include "KafkaClient.h"
#include "Utils.h"
#include "log.hpp"

namespace kafka {

    void KafkaProducerClient::Open(std::string brokers, std::string topic_str) {
        std::cout << "KafkaClient::Open" << std::endl;

        this->topic_str = topic_str;
        this->brokers = brokers;

        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

        conf->set("metadata.broker.list", brokers, errstr);

        producer = RdKafka::Producer::create(conf, errstr);
        if (!producer) {
            std::cerr << "Failed to create producer: " << errstr << std::endl;
            exit(1);
        }

        std::cout << "% Created producer " << producer->name() << std::endl;
    }

    void KafkaProducerClient::Write(std::string line) {
        this->Write(const_cast<char *>(line.c_str()), line.size());
    }

    void KafkaProducerClient::Write(void *payload, size_t len) {

        producer->poll(0);

        /*
        * Produce message
        */
        RdKafka::ErrorCode resp =
                producer->produce(topic_str, RdKafka::Topic::PARTITION_UA,
                                  RdKafka::Producer::RK_MSG_COPY,
                        /* Value */
                                  payload, len,
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
//            std::cerr << "% Produced message (" << len << " bytes)" <<
//                      std::endl;
        }

        producer->poll(0);
    }


    void KafkaProducerClient::Close() {

        while (producer->outq_len() > 0) {
            std::cerr << "Waiting for " << producer->outq_len() << std::endl;
            producer->poll(1000);
        }

        delete producer;
    }


    /*
     * Reader
     * */

    class ExampleRebalanceCb : public RdKafka::RebalanceCb {
    private:

        static void
        part_list_print(
                const std::vector<RdKafka::TopicPartition *> &partitions) {
            for (unsigned int i = 0; i < partitions.size(); i++)
                std::cerr << partitions[i]->topic() <<
                          "[" << partitions[i]->partition() << "], ";
            std::cerr << "\n";
        }

    public:
        int partition_cnt = 0;

        void rebalance_cb(RdKafka::KafkaConsumer *consumer,
                          RdKafka::ErrorCode err,
                          std::vector<RdKafka::TopicPartition *> &partitions) {
            std::cerr << "RebalanceCb: " << RdKafka::err2str(err) << ": ";

            part_list_print(partitions);

            if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
                consumer->assign(partitions);
                partition_cnt = (int) partitions.size();
            } else {
                consumer->unassign();
                partition_cnt = 0;
            }
        }
    };

    void KafkaReaderClient::Open(std::string brokers, std::string topic_str) {
        std::cout << "KafkaClient::Open" << std::endl;

        this->topic_str = topic_str;
        this->brokers = brokers;

        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

        ex_rebalance_cb = new ExampleRebalanceCb();
        conf->set("rebalance_cb", ex_rebalance_cb, errstr);

        conf->set("enable.partition.eof", "true", errstr);
        conf->set("metadata.broker.list", brokers, errstr);

        /*
         * Optional
         * */
        if (conf->set("client.id", "kafka secondo client", errstr) !=
            RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }

        std::string groupId = create_uuid();
        /* std::cout << "groupId=" << groupId << std::endl; */
        if (conf->set("group.id", groupId, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }

        if (conf->set("auto.offset.reset", "earliest", errstr) !=
            RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            exit(1);
        }

        std::vector<std::string> topics;
        topics.push_back(topic_str);

        consumer = RdKafka::KafkaConsumer::create(conf,
                                                  errstr);
        if (!consumer) {
            std::cerr << "Failed to create consumer: " << errstr << std::endl;
            exit(1);
        }

        delete conf;

        std::cout << "% Created consumer " << consumer->name() << std::endl;

        /*
        * Subscribe to topics
        */
        RdKafka::ErrorCode err = consumer->subscribe(topics);
        if (err) {
            std::cerr << "Failed to subscribe to " << topics.size()
                      << " topics: "
                      << RdKafka::err2str(err) << std::endl;
            exit(1);
        }
    }

    std::string *KafkaReaderClient::ReadSting() {
        bool run = true;
        char *payload;

        while (run) {
            RdKafka::Message *msg = consumer->consume(1000);
            KafkaMessage *message = msg_consume(msg,
                                                ex_rebalance_cb->partition_cnt);
            run = message->run && message->payload == NULL;
            payload = static_cast<char *>(message->payload);
            delete msg;
        }
        if (payload != NULL) {
            return new std::string(payload);
        } else {
            return NULL;
        }
    }

    KafkaMessage *KafkaReaderClient::msg_consume(RdKafka::Message *message,
                                                 int partition_cnt) {
        KafkaMessage *result = new KafkaMessage();
        result->run = true;

        switch (message->err()) {

            case RdKafka::ERR__TIMED_OUT:
                LOG(DEBUG) << "Consume failed(timeout): " << message->errstr();
                break;

            case RdKafka::ERR_NO_ERROR:
                /* Real message */
                msg_cnt++;
                msg_bytes += message->len();
                LOG(DEBUG) << "Read msg at offset " << message->offset();
//                result->key = message->key();
                result->len = message->len();
                result->payload = message->payload();
                break;

            case RdKafka::ERR__PARTITION_EOF:
                /* Last message */
                if (exit_eof && ++eof_cnt == partition_cnt) {
                    LOG(INFO) << "EOF reached for all " << partition_cnt <<
                              " partition(s)";
                    result->run = false;
                }
                break;

            case RdKafka::ERR__UNKNOWN_TOPIC:
            case RdKafka::ERR__UNKNOWN_PARTITION:
                LOG(ERROR) << "Consume failed: " << message->errstr();
                result->run = false;
                break;

            default:
                LOG(ERROR) << "Consume failed: " << message->errstr();
                result->run = false;
        }
        return result;
    }


    void KafkaReaderClient::Close() {
        consumer->close();
        delete consumer;
        delete ex_rebalance_cb;
        RdKafka::wait_destroyed(5000);
    }

    bool KafkaReaderClient::isExitEof() const {
        return exit_eof;
    }

    void KafkaReaderClient::setExitEof(bool exitEof) {
        exit_eof = exitEof;
    }


}

