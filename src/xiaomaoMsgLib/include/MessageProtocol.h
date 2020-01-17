#ifndef CLASSES_NETWORK_MESSAGEPROTOCOL_H
#define CLASSES_NETWORK_MESSAGEPROTOCOL_H

#include <rabbit/rabbit.hpp>

class NetworkMessage {
public:
	NetworkMessage();
	virtual ~NetworkMessage() = default;

	std::string toString() const;

protected:
	virtual void buildJson() const {};

	std::string _id;

	mutable rabbit::object _json;
	mutable std::string _strDump;
};

class LoginMessage : public NetworkMessage {

public:
	explicit LoginMessage(const std::string& login, const std::string& pass);
	~LoginMessage() = default;
	void buildJson() const override;

private:
	std::string _loginStr;
	std::string _passStr;
};

#endif