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

	bool Connect(const std::string& url);
	bool Listen(const std::string& url, const Block block = Block::BLOCKING);

	std::string Receive(const Block block = Block::BLOCKING);
	bool Send(const std::string& msg, const Block block = Block::NON_BLOCKING);

	const ErrorMessage::Ptr& GetError() const;

protected:
	bool init();
	bool close();

	std::string getStrId() const;

	void reportInternalError(const int errSubCode, const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL);
	void reportError(const int code, const int errSubCode, const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL);
	void reportError(const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL);

private:
	nng_socket _socket{ NNG_SOCKET_INITIALIZER };
	nng_dialer _dialer{ NNG_DIALER_INITIALIZER };

	ErrorMessage::Ptr _latestErr;
};

#endif