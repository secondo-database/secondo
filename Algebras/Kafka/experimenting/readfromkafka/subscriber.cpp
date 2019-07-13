/*

*/

#include "subscriber.h"

#include <iostream>
#include <fcntl.h>
#include <zconf.h>
#include "librdkafka/rdkafkacpp.h"
#include "easylogging++.hpp"

static int partition_cnt = 0;
static int eof_cnt = 0;

class ExampleRebalanceCb : public RdKafka::RebalanceCb {
private:
    static void
    part_list_print(const std::vector<RdKafka::TopicPartition *> &partitions) {
        for (unsigned int i = 0; i < partitions.size(); i++)
            std::cerr << partitions[i]->topic() <<
                      "[" << partitions[i]->partition() << "], ";
        std::cerr << "\n";
    }

public:
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
        eof_cnt = 0;
    }
};

static long msg_cnt = 0;
static int64_t msg_bytes = 0;
static int verbosity = 1;
static bool exit_eof = true; /* Control exit*/

static bool run = true;

void msg_consume_ss(RdKafka::Message *message, void *opaque) {
    switch (message->err()) {
        case RdKafka::ERR__TIMED_OUT:
            break;

        case RdKafka::ERR_NO_ERROR:
            /* Real message */
            msg_cnt++;
            msg_bytes += message->len();
            if (verbosity >= 3)
                std::cerr << "Read msg at offset " << message->offset()
                          << std::endl;
            RdKafka::MessageTimestamp ts;
            ts = message->timestamp();
            if (verbosity >= 2 &&
                ts.type !=
                RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
                std::string tsname = "?";
                if (ts.type ==
                    RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
                    tsname = "create time";
                else if (ts.type ==
                         RdKafka::MessageTimestamp::
                         MSG_TIMESTAMP_LOG_APPEND_TIME)
                    tsname = "log append time";
                std::cout << "Timestamp: " << tsname << " " << ts.timestamp
                          << std::endl;
            }
            if (verbosity >= 2 && message->key()) {
                std::cout << "Key: " << *message->key() << std::endl;
            }
            if (verbosity >= 1) {
                printf("%.*s\n",
                       static_cast<int>(message->len()),
                       static_cast<const char *>(message->payload()));
            }
            break;

        case RdKafka::ERR__PARTITION_EOF:
            /* Last message */
            if (exit_eof && ++eof_cnt == partition_cnt) {
                std::cerr << "%% EOF reached for all " << partition_cnt <<
                          " partition(s)" << std::endl;
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

std::string create_uuid() {
    std::string uuid;
    char buffer[128];

    const char *filename = "/proc/sys/kernel/random/uuid";
    FILE *file = fopen(filename, "r");

    // Does the proc file exists?
    if (access(filename, R_OK) == -1) {
        std::cerr << "Unable to get UUID from kernel" << std::endl;
        exit(-1);
    }

    if (file) {
        while (fscanf(file, "%s", buffer) != EOF) {
            uuid.append(buffer);
        }
    }

    fclose(file);
    return uuid;
}

int runSubscriberConsuming() {
    std::cout << "Hello, World!" << std::endl;
    LOG(INFO) << "My first info log using default logger";

    std::string brokers = "localhost";
    std::string errstr;
    std::vector<std::string> topics;


    /*
    * Create configuration objects
    */
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    /*
    * ?
    */
    ExampleRebalanceCb ex_rebalance_cb;
    conf->set("rebalance_cb", &ex_rebalance_cb, errstr);
    conf->set("enable.partition.eof", "true", errstr);

    /*    */
    conf->set("metadata.broker.list", brokers, errstr);


    /*
     * Optional
     * */
    if (conf->set("client.id", "kafka secondo client", errstr) !=
        RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }

    /*
     * should be generated e.g. UUID.randomUUID()
    std::string groupId = "id2";
     * */

    std::string groupId = create_uuid();
    std::cout << "groupId=" << groupId << std::endl;
    if (conf->set("group.id", groupId, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }


    /*
     *
    properties.put(ConsumerConfig.AUTO_OFFSET_RESET_CONFIG, "earliest");

     Works only with new group id

    earliest: automatically reset the offset to the earliest offset
    latest: automatically reset the offset to the latest offset
    none: throw exception to the consumer if no previous offset is found or the
     consumer's group
    anything else: throw exception to the consumer.

     */
    if (conf->set("auto.offset.reset", "earliest", errstr) !=
        RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }

    std::string topic_str = "test1";
    topics.push_back(topic_str);

    /*
    * Create consumer using accumulated global configuration.
    */
    RdKafka::KafkaConsumer *consumer = RdKafka::KafkaConsumer::create(conf,
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
        std::cerr << "Failed to subscribe to " << topics.size() << " topics: "
                  << RdKafka::err2str(err) << std::endl;
        exit(1);
    }

    std::cout << "Subscribed to topics" << std::endl;

    std::vector<RdKafka::TopicPartition *> partitions;
    consumer->assignment(partitions);
    std::cout << "Partitions:" << partitions.size() << std::endl;

    /*
     * Consume messages
     */
    while (run) {
        std::cout << "----------------" << std::endl;
        RdKafka::Message *msg = consumer->consume(1000);
        msg_consume_ss(msg, NULL);
        delete msg;
    }


    /*
    * Stop consumer
    */
    consumer->close();
    delete consumer;

    std::cerr << "% Consumed " << msg_cnt << " messages ("
              << msg_bytes << " bytes)" << std::endl;

    /*
     * Wait for RdKafka to decommission.
     * This is not strictly needed (with check outq_len() above), but
     * allows RdKafka to clean up all its resources before the application
     * exits so that memory profilers such as valgrind wont complain about
     * memory leaks.
     */
    RdKafka::wait_destroyed(5000);

    return 0;
}