#ifndef NETSOCKET_H
#define NETSOCKET_H

#include "NetworkLog.h"
#include "utils.h"

static const std::string tlsStr = "tls+";

enum class Block {
	NON_BLOCKING = 0,
	BLOCKING = 1
};

class NetSocket {

public:
	NetSocket();
	virtual ~NetSocket();

	virtual bool Connect(const std::string& url_);
	virtual bool Listen(const std::string& url_, const Block block = Block::BLOCKING);

	virtual std::string Receive(const Block block = Block::BLOCKING);
	virtual bool Send(const std::string& msg, const Block block = Block::NON_BLOCKING);

	const ErrorMessage::Ptr& GetError() const;

#ifndef NO_TLS
	void SetPrivateKey(const std::string& key);
	void SetCertificate(const std::string& cert);
#endif

	static bool needTLS(const std::string& url) {
		const bool need = url.find(tlsStr) != std::string::npos;
		return need;
	}

protected:
	virtual bool close();

	std::string getStrId() const;

	void reportInternalError(const int errSubCode, const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL) const;
	void reportError(const int code, const int errSubCode, const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL) const;
	void reportError(const std::string& what = "", const std::string& where = "", const int priority = LogMessage::NORMAL) const ;

protected:
#ifndef NO_TLS
	std::string _privateKey;
	std::string _certificate;
#endif

private:
	mutable ErrorMessage::Ptr _latestErr;
	Utils::IdType _id{0};
};

#endif