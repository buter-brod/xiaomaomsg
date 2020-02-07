#include "NetSocket.h"

NetSocket::NetSocket() {
	_id = Utils::newID();
}

NetSocket::~NetSocket() {
}

std::string NetSocket::getStrId() const {
	return std::to_string(_id);
}

bool NetSocket::close() {
	reportError("not implemented", "NetSocket::close");
	return false;
}

void NetSocket::SetPrivateKey(const std::string& key) {
	_privateKey = key;
}

void NetSocket::SetCertificate(const std::string& cert) {
	_certificate = cert;
}

bool NetSocket::Listen(const std::string& url_, const Block block) {

	reportError("not implemented", "NetSocket::Listen");
	return false;
}

bool NetSocket::Connect(const std::string& url_) {
	reportError("not implemented", "NetSocket::Connect");
	return false;
}

std::string NetSocket::Receive(const Block block) {
	reportError("not implemented", "NetSocket::Receive");
	return "";
}

bool NetSocket::Send(const std::string& msg, const Block block) {
	reportError("not implemented", "NetSocket::Send");
	return false;
}

void NetSocket::reportInternalError(const int errSubCode, const std::string& what, const std::string& where, const int priority) const {
	reportError(ErrorMessage::NOT_SPECIFIED, errSubCode, what, where, priority);
}

void NetSocket::reportError(const std::string& what, const std::string& where, const int priority) const {
	reportError(ErrorMessage::NOT_SPECIFIED, ErrorMessage::NO_SUBERROR, what, where, priority);
}

void NetSocket::reportError(const int code, const int errSubCode, const std::string& what, const std::string& where, const int priority) const {

	const std::string& whereWithSockId = where + "(id " + getStrId() + ")";
	const auto& errInfo = std::make_shared<ErrorMessage> (what, code, errSubCode, utils::Time::CurrentTime(), whereWithSockId, priority);

	NetworkLog::Inst()->PutMessage(errInfo);
	_latestErr = errInfo;
}

const ErrorMessage::Ptr& NetSocket::GetError() const {
	return _latestErr;
}