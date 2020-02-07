#include "NNGSocket.h"
#include "nng/protocol/pair0/pair.h"
#include "nng/transport/tls/tls.h"
#include "nng/supplemental/tls/tls.h"

NNGSocket::NNGSocket() {
	init();
}

NNGSocket::~NNGSocket() {
	close();
}

bool NNGSocket::Connect(const std::string& url_) {

	const std::string& url = urlCheckTLS(url_);

	nng_dialer_create(&_dialer, _socket, url.c_str());

#ifndef NO_TLS
	const bool tlsURL = needTLS(url);

	if (tlsURL) {
		const bool initTLSOk = initTLSClientCfg();
		if (!initTLSOk) {
			return false;
		}
	}
#endif

	const int flags = 0;
	const auto dialResult = nng_dialer_start(_dialer, flags);
	const bool dialOk = dialResult == 0;

	if (!dialOk) {

#ifndef NO_TLS
		if (tlsURL && dialResult == NNG_ECRYPTO) {
			reportError("TLS error occured (ECRYPTO), could be a certificate problem", "NetSocket::Connect");
		}
#endif
		reportInternalError(dialResult, "dialer_start failed", "NetSocket::Connect");
	}

	return dialOk;
}

std::string NNGSocket::urlCheckTLS(const std::string& url) const {

#ifdef NO_TLS
	if (needTLS(url)) {

		const auto tlsPos = url.find(tlsStr);
		reportError(std::string("url ") + url + " wants TLS connection, but TLS not enabled, we will ignore it", "NetSocket::urlCheckTLS");

		std::string newURL = url;
		newURL.erase(tlsPos, tlsStr.length());
		return newURL;
	}
#endif

	return url;
}

bool NNGSocket::Listen(const std::string& url_, const Block block) {

	// todo make sure nonblocking listening works as well!

	const std::string& url = urlCheckTLS(url_);

	nng_listener_create(&_listener, _socket, url.c_str());

#ifndef NO_TLS
	if (needTLS(url)) {

		const bool initTLSOk = initTLSServerCfg();
		if (!initTLSOk) {
			return false;
		}
	}
#endif

	const int flags = 0;
	const auto listenResult = nng_listener_start(_listener, flags);
	const bool listenOk = listenResult == 0;

	if (!listenOk) {
		reportInternalError(listenResult, "listener_start failed", "NetSocket::Listen");
	}

	return listenOk;
}

std::string NNGSocket::Receive(const Block block) {

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

bool NNGSocket::Send(const std::string& msg, const Block block) {

	const int flags = block == Block::BLOCKING ? 0 : NNG_FLAG_NONBLOCK;
	const auto sendResult = nng_send(_socket, (void*)msg.c_str(), msg.size(), flags);
	const bool sendOk = sendResult == 0;

	if (!sendOk) {
		reportInternalError(sendResult, "nng_send failed", "NetSocket::Send");
	}

	return sendOk;
}

bool NNGSocket::init() {

	const auto reqOpenResult = nng_pair0_open(&_socket);
	const bool succeed = (reqOpenResult == 0);
	if (!succeed) {
		reportInternalError(reqOpenResult);
	}

#ifndef NO_TLS
	const bool registerOk = nng_tls_register() == 0;
	if (!registerOk) {
		reportError("unable to init TLS", "NetSocket::init");
	}
#endif

	return succeed;
}

bool NNGSocket::close() {

	const bool socketInitialized = nng_socket_id(_socket) > 0;

	if (!socketInitialized) {

		reportError("socket seems destroyed already", "NetSocket::close");
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

#ifndef NO_TLS
bool NNGSocket::initTLSServerCfg() {

	if (_certificate.empty() || _privateKey.empty()) {
		reportError("unable to init TLS config, certificate or private key not specified", "NetSocket::initTLSServerCfg");
		return false;
	}

	nng_tls_config* cfg;
	const bool tlsAllocOk = nng_tls_config_alloc(&cfg, NNG_TLS_MODE_SERVER);


	const auto tlsCertErr = nng_tls_config_own_cert(cfg, _certificate.c_str(), _privateKey.c_str(), NULL);

	if (tlsCertErr != 0) {
		reportInternalError(tlsCertErr, "tls certificate init failed", "NetSocket::initTLSServerCfg");
		return false;
	}

	nng_listener_setopt_ptr(_listener, NNG_OPT_TLS_CONFIG, cfg);
	nng_tls_config_free(cfg);
	return true;
}

bool NNGSocket::initTLSClientCfg() {

	if (_certificate.empty()) {
		reportError("unable to init TLS config, certificate not specified", "NetSocket::initTLSClientCfg");
		return false;
	}

	nng_tls_config* cfg;
	const bool tlsAllocOk = nng_tls_config_alloc(&cfg, NNG_TLS_MODE_CLIENT);

	const auto tlsCertErr = nng_tls_config_ca_chain(cfg, _certificate.c_str(), NULL);

	if (tlsCertErr != 0) {
		reportInternalError(tlsCertErr, "tls certificate init failed", "NetSocket::initTLSClientCfg");
		return false;
	}

	nng_tls_config_auth_mode(cfg, NNG_TLS_AUTH_MODE_NONE);
	nng_dialer_setopt_ptr(_dialer, NNG_OPT_TLS_CONFIG, cfg);
	nng_tls_config_free(cfg);

	return true;
}
#endif