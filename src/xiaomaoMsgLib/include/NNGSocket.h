#ifndef NNGSOCKET_H
#define NNGSOCKET_H

#include "NetSocket.h"
#include "nng/nng.h"

class NNGSocket : public NetSocket {
	
public:

	NNGSocket();
	~NNGSocket();

	bool Connect(const std::string& url_) override;
	bool Listen(const std::string& url_, const Block block = Block::BLOCKING) override;

	std::string Receive(const Block block = Block::BLOCKING) override;
	bool Send(const std::string& msg, const Block block = Block::NON_BLOCKING) override;

protected:
	bool init();
	bool close() override;

	std::string urlCheckTLS(const std::string& url) const;

#ifndef NO_TLS
	bool initTLSServerCfg();
	bool initTLSClientCfg();
#endif

private:
	nng_socket _socket{ NNG_SOCKET_INITIALIZER };
	nng_dialer _dialer{ NNG_DIALER_INITIALIZER };
	nng_listener _listener{ NNG_LISTENER_INITIALIZER };
};

#endif