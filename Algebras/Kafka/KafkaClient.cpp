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

#define KAFKA_CLIENT_PRINT_DEBUG_INFO 0

#define KAFKA_VERSION_REQUEST_FIX 1

namespace kafka {

    void KafkaWriterClient::Open(std::string brokers, std::string topic_str) {
        LOG(DEBUG) << "KafkaWriterClient::Open called";

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

        LOG(DEBUG) << "KafkaWriterClient::Open finished. Created producer "
                   << producer->name();
    }

    void KafkaWriterClient::Write(std::string line) {
        LOG(TRACE) << "KafkaWriterClient::Write line: " << line;
        this->Write(const_cast<char *>(line.c_str()), line.size());
    }

    void KafkaWriterClient::Write(void *payload, size_t len) {
        RdKafka::ErrorCode resp;
        do {
            producer->poll(0);
            resp =
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
            if (resp == RdKafka::ERR__QUEUE_FULL) {
                kafka::sleepMS(500);
            } else if (resp != RdKafka::ERR_NO_ERROR) {
                std::cerr << "% Produce failed: " <<
                          RdKafka::err2str(resp) << std::endl;
            } else {
//            std::cerr << "% Produced message (" << len << " bytes)" <<
//                      std::endl;
            }
            producer->poll(0);
        } while (resp == RdKafka::ERR__QUEUE_FULL);
        written_count++;
        if ((LOG_PROGRESS_INTERVAL > 0)
            && (written_count % LOG_PROGRESS_INTERVAL == 0))
            std::cout << "." << std::flush;
    }


    void KafkaWriterClient::Close() {

        while (producer->outq_len() > 0) {
            LOG(DEBUG) << "Waiting for " << producer->outq_len();
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

            if (KAFKA_CLIENT_PRINT_DEBUG_INFO) {
                std::cerr << "RebalanceCb: " << RdKafka::err2str(err) << ": ";
                part_list_print(partitions);
            }

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
        LOG(DEBUG) << "KafkaReaderClient::Open";

        this->topic_str = topic_str;
        this->brokers = brokers;

        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

        if (KAFKA_VERSION_REQUEST_FIX) {
            if (conf->set("api.version.request", "true", errstr) !=
                RdKafka::Conf::CONF_OK) {
                std::cerr << errstr << std::endl;
            }
            if (conf->set("api.version.fallback.ms", "0", errstr) !=
                RdKafka::Conf::CONF_OK) {
                std::cerr << errstr << std::endl;
            }
        }

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

        LOG(INFO) << "KafkaReaderClient::Open. Created consumer "
                  << consumer->name();

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
        std::string *result = NULL;
        bool run = true;
        while (run) {
            LOG(TRACE) << "Starting consuming next message from kafka";
            RdKafka::Message *msg = consumer->consume(1000);
            KafkaMessage *message = msg_consume(msg,
                                                ex_rebalance_cb->partition_cnt);
            run = message->run && message->payload == NULL;
            if (message->payload) {
                char *payload = static_cast<char *>(message->payload);
                LOG(TRACE) << "KafkaReaderClient::ReadSting res:  " << payload;
                result = new std::string(payload, message->len);
            } else {
                LOG(TRACE)
                        << "KafkaReaderClient::ReadSting res: payload is NULL ";
            }
            delete msg;
        }
        read_count++;
        if ((LOG_PROGRESS_INTERVAL > 0)
            && (read_count % LOG_PROGRESS_INTERVAL == 0))
            std::cout << "." << std::flush;

        return result;
    }

    KafkaMessage *KafkaReaderClient::msg_consume(RdKafka::Message *message,
                                                 int partition_cnt) {
        KafkaMessage *result = new KafkaMessage();
        result->run = true;

        switch (message->err()) {

            case RdKafka::ERR__TIMED_OUT:
                LOG(DEBUG) << "Consume failed(timeout): " << message->errstr();
                if (exit_on_timeout)
                    result->run = false;
                break;

            case RdKafka::ERR_NO_ERROR:
                /* Real message */
                msg_cnt++;
                msg_bytes += message->len();
                LOG(TRACE) << "Read msg at offset " << message->offset();
//                result->key = message->key();
                result->len = message->len();
                result->payload = message->payload();
                break;

            case RdKafka::ERR__PARTITION_EOF:
                /* Last message */
                if (exit_eof && ++eof_cnt == partition_cnt) {
                    LOG(DEBUG) << "EOF reached for all " << partition_cnt <<
                               " partition(s)";
                    eof_cnt = 0;
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

}

