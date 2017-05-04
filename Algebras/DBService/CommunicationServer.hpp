#ifndef ALGEBRAS_DBSERVICE_CommunicationServer_HPP_
#define ALGEBRAS_DBSERVICE_CommunicationServer_HPP_

#include "Algebras/DBService/MultiClientServer.hpp"
#include "Algebras/DBService/TraceWriter.hpp"

class Socket;

namespace DBService {

class CommunicationServer: public MultiClientServer {
public:
    explicit CommunicationServer(int port);
    virtual ~CommunicationServer();
protected:
    int communicate(std::iostream& io);
    bool handleProvideReplicaRequest(std::iostream& io);
    bool handleTriggerFileTransferRequest(std::iostream& io);
    bool handleUseReplicaRequest(std::iostream& io);
private:
    std::auto_ptr<TraceWriter> traceWriter;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_CommunicationServer_HPP_ */
