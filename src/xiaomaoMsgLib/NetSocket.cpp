#include "NetSocket.h"
#include "nng/protocol/reqrep0/req.h"
#include <string>

NetSocket::NetSocket() {
	init();
}

bool NetSocket::init() {

	const auto reqOpenResult = nng_req0_open(&_socket);
	const bool succeed = (reqOpenResult == 0);
	if (!succeed) {
		reportInternalError(reqOpenResult);
	}

	return succeed;
}

bool NetSocket::close() {

	const bool socketInitialized = nng_socket_id(_socket) > 0;

	if (!socketInitialized) {
		
		reportError("socket seems destroyed already" , "NetSocket::close");
		return false;
	}

	const int closeResult = nng_close(_socket);
	const bool closeOk = closeResult == 0;

	if (!closeOk) {
		// NNG_EBADF?
		reportInternalError(closeResult, "nng_close failed", "NetSocket::close");
		return false;
	}

	_socket = NNG_SOCKET_INITIALIZER;
	return true;
}

bool NetSocket::Connect(const std::string& url, const Block block) {

	const int flags = block == Block::BLOCKING ? 0 : NNG_FLAG_NONBLOCK;
	const auto dialResult = nng_dial(_socket, url.c_str(), &_dialer, flags);
	const bool dialOk = dialResult == 0;

	if (!dialOk) {
		reportInternalError(dialResult, "nng_dial failed", "NetSocket::Connect");
	}

	return dialOk;
}

std::string NetSocket::Receive(const Block block) {

	size_t sz;
	char* buf = nullptr;
	const int flags = block == Block::BLOCKING ? NNG_FLAG_ALLOC : (NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);

	const int result = nng_recv(_socket, &buf, &sz, flags);
	const bool recvOk = (result == 0);
	std::string msg;

	if (recvOk) {
		msg.assign(buf, sz);
	}
	else if (result != NNG_EAGAIN) {
		reportInternalError(result, "nng_recv failed", "NetSocket::Receive");
	}

	return msg;
}

bool NetSocket::Send(const std::string& msg, const Block block) {

	const int flags = block == Block::BLOCKING ? 0 : NNG_FLAG_NONBLOCK;
	const auto sendResult = nng_send(_socket, (void*)msg.c_str(), msg.size(), flags);
	const bool sendOk = sendResult == 0;

	if (!sendOk) {
		reportInternalError(sendResult, "nng_send failed", "NetSocket::Send");
	}

	return sendOk;
}

std::string NetSocket::getStrId() const {
	return std::to_string(_socket.id);
}

void NetSocket::reportInternalError(const int errSubCode, const std::string& what, const std::string& where, const int priority) {
	reportError(ErrorMessage::NOT_SPECIFIED, errSubCode, what, where, priority);
}

void NetSocket::reportError(const std::string& what, const std::string& where, const int priority) {
	reportError(ErrorMessage::NOT_SPECIFIED, ErrorMessage::NO_SUBERROR, what, where, priority);
}

void NetSocket::reportError(const int code, const int errSubCode, const std::string& what, const std::string& where, const int priority) {

	const std::string& whereWithSockId = where + "(id " + getStrId() + ")";
	const auto& errInfo = std::make_shared<ErrorMessage> (what, code, errSubCode, utils::Time::CurrentTime(), whereWithSockId, priority);

	NetworkLog::Inst()->PutMessage(errInfo);
	_latestErr = errInfo;
}

const ErrorMessage::Ptr& NetSocket::GetError() const {
	return _latestErr;
}