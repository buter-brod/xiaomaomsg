#ifndef NETSOCKET_H
#define NETSOCKET_H

#include "nng/nng.h"
#include "NetworkLog.h"

enum class Block {
	NON_BLOCKING = 0,
	BLOCKING = 1
};

class NetSocket {

public:
	NetSocket();

	bool Connect(const std::string& url_);
	bool Listen(const std::string& url_, const Block block = Block::BLOCKING);

	std::string Receive(const Block block = Block::BLOCKING);
	bool Send(const std::string& msg, const Block block = Block::NON_BLOCKING);

	const ErrorMessage::Ptr& GetError() const;

#ifndef NO_TLS
	void SetPrivateKey(const std::string& key);
	void SetCertificate(const std::string& cert);
#endif

protected:
	bool init();
	bool close();

#ifndef NO_TLS
	bool initTLSServerCfg();
	bool initTLSClientCfg();
#endif

	std::string urlCheckTLS(const std::string& url) const;

	std::string getStrId() const;

	void reportInternalError(const int errSubCode, const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL) const;
	void reportError(const int code, const int errSubCode, const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL) const;
	void reportError(const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL) const ;

private:
	nng_socket _socket{ NNG_SOCKET_INITIALIZER };
	nng_dialer _dialer{ NNG_DIALER_INITIALIZER };
	nng_listener _listener {NNG_LISTENER_INITIALIZER };

#ifndef NO_TLS
	std::string _privateKey;
	std::string _certificate;
#endif

	mutable ErrorMessage::Ptr _latestErr;
};

#endif