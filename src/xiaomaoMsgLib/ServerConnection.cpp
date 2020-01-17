#include "ServerConnection.h"
#include "MessageProtocol.h"
#include <map>

static std::string defaultAuthURL() {return "tcp://127.0.0.1:50051";}

ServerConnection* ServerConnection::_instance = nullptr;

std::string ServerConnection::StateToString() const {
	return StateToString(_state);
}

std::string ServerConnection::StateToString(ServerConnection::State st) const {

	static const std::map<ServerConnection::State, std::string> stateStrMap = {

		{State::INIT,                      "INIT"},
		{State::READY_TO_CONNECT,          "READY_TO_CONNECT"},
		{State::CONNECTING_TO_AUTH_SERVER, "CONNECTING_TO_AUTH_SERVER"},
		{State::AUTH_WILL_RETRY_TO_CONNECT,  "AUTH_RETRYING_TO_CONNECT"},
		{State::AUTH_CONNECTED,            "AUTH_CONNECTED"},
		{State::WAITING_FOR_AUTH_INPUT,    "WAITING_FOR_AUTH_INPUT"},
		{State::TRYING_AUTH,               "TRYING_AUTH"},
		{State::AUTH_FAILED,               "AUTH_FAILED"},
		{State::AUTH_SUCCESSFUL,           "AUTH_SUCCESSFUL"},
		{State::GAME_CONNECTING,           "GAME_CONNECTING"},
		{State::GAME_WILL_RETRY_TO_CONNECT,  "GAME_RETRYING_TO_CONNECT"},
		{State::GAME_CONNECTED,            "GAME_CONNECTED"},
		{State::GAME_RETRIEVING_INFO,      "GAME_RETRIEVING_INFO"},
		{State::CONNECTED,                 "CONNECTED"}
	};

	std::string result;

	const auto& it = stateStrMap.find(st);
	if (it != stateStrMap.end()) {
		result = it->second;
	} else {
		logErr("unknown state " + std::to_string((int)st) + "ServerConnection::StateToString");
	}

	return result;	
}

bool ServerConnection::destroySockets() {
	
	logMsg("will now destroy sockets", "ServerConnection::destroySockets");

	if (_sockAuth) {
		_sockAuth.reset();
	}

	if (_sockGame) {
		_sockGame.reset();
	}

	return true;
}

bool ServerConnection::initSockets() {

	if (_sockAuth) {
		logErr("auth socket already created", "ServerConnection::initSockets");
		return false;
	}

	if (_sockGame) {
		logErr("game socket already created", "ServerConnection::initSockets");
		return false;
	}

	_sockAuth = std::make_unique<NetSocket>();
	_sockGame = std::make_unique<NetSocket>();

	return true;
}

void ServerConnection::start() {
	
	if (_worker.joinable()) {
		logErr("worker thread already started", "ServerConnection::start");
		return;
	}

	_worker = std::thread([this] {process(); });
}

bool ServerConnection::setState(const State newState) {

	const std::string& toLog = "state changed from " + StateToString(_state) + " to " + StateToString(newState);
	_state = newState;

	logMsg(toLog, "ServerConnection::setState");
	return true;
}

void ServerConnection::waitForAuth() {

	const auto isLoginProvided = [this]() -> bool {
		const bool credentialsProvided = !_loginStr.empty() && !_passStr.empty();
		return credentialsProvided;
	};

	std::unique_lock<std::mutex> authLock(_authMutex);

	const bool alreadyProvided = isLoginProvided();
	if (!alreadyProvided) {
		logMsg("will wait for user login input now", "ServerConnection::waitForAuth");
		_authCondition.wait(authLock, isLoginProvided);
		logMsg("looks like auth info is provided, stop waiting", "ServerConnection::waitForAuth");
	}
}

bool ServerConnection::tryStateAdvance() {

	const State prevState = _state;

	if (_state == State::INIT) {
		setState(State::READY_TO_CONNECT);
	} 

	if (_state == State::READY_TO_CONNECT) {
		setState(State::CONNECTING_TO_AUTH_SERVER);
		const bool socketConnected = connectAuth(Block::BLOCKING);
		if (socketConnected) {
			_secondsToRetry = secondsToRetryInitial;
			setState(State::AUTH_CONNECTED);
		} else {
			setState(State::AUTH_WILL_RETRY_TO_CONNECT);
		}
	}

	if (_state == State::AUTH_WILL_RETRY_TO_CONNECT) {
		logMsg("will retry auth connection in " + std::to_string(_secondsToRetry) + "s.", "ServerConnection::tryStateAdvance");
		std::this_thread::sleep_for(std::chrono::seconds(_secondsToRetry));
		++_secondsToRetry;
		setState(State::READY_TO_CONNECT);
	}

	if (_state == State::AUTH_CONNECTED) {
		setState(State::WAITING_FOR_AUTH_INPUT);
	}

	if (_state == State::WAITING_FOR_AUTH_INPUT) {
		
		waitForAuth();
		setState(State::TRYING_AUTH);
		const bool authSuccess = tryAuth(Block::BLOCKING);
		setState(authSuccess ? State::AUTH_SUCCESSFUL : State::AUTH_FAILED);
	}

	if (_state == State::AUTH_FAILED) {

		//todo: notify obesrvers credentials wrong
		resetCredentials();
		setState(State::WAITING_FOR_AUTH_INPUT);
	}

	if (_state == State::GAME_WILL_RETRY_TO_CONNECT) {
		logMsg("will retry game connection in " + std::to_string(_secondsToRetry) + "s.", "ServerConnection::tryStateAdvance");
		std::this_thread::sleep_for(std::chrono::seconds(_secondsToRetry));
		++_secondsToRetry;
		setState(State::AUTH_SUCCESSFUL);
	}

	if (_state == State::AUTH_SUCCESSFUL) {
		setState(State::GAME_CONNECTING);

		const bool gameConnected = connectGame();
		if (gameConnected) {
			_secondsToRetry = secondsToRetryInitial;
			setState(State::GAME_CONNECTED);
		} else {
			setState(State::GAME_WILL_RETRY_TO_CONNECT);
		}
	}

	if (_state == State::GAME_CONNECTED) {
		setState(State::GAME_RETRIEVING_INFO);
	}

	else if (_state == State::GAME_RETRIEVING_INFO) {
	}
	else if (_state == State::CONNECTED) {
	} 

	const bool changed = prevState != _state;
	return changed;
}

void ServerConnection::resetCredentials() {
	
	logMsg("will now reset credentials" , "ServerConnection::resetCredentials");
	std::lock_guard<std::mutex> authLock(_authMutex);
	_loginStr.clear();
	_passStr.clear();
}

void ServerConnection::logMsg(const std::string& what, const std::string& where, const int priority) const {
	
	const auto& logMessage = std::make_shared<LogMessage>(what, utils::Time::CurrentTime(), where, priority);
	NetworkLog::Inst()->PutMessage(logMessage);
}

void ServerConnection::logErr(const std::string& what, const std::string& where, const int priority) const {

	const auto& errInfo = std::make_shared<ErrorMessage>(what, ErrorMessage::NOT_SPECIFIED, ErrorMessage::NO_SUBERROR, utils::Time::CurrentTime(), where, priority);
	NetworkLog::Inst()->PutMessage(errInfo);

	_latestErr = errInfo;
}

const ErrorMessage::Ptr& ServerConnection::getError() const {
	return _latestErr;
}

void ServerConnection::process() {

	//todo: remember start time

	while(!workerEndRequested.load()) {
		const bool connectAdvance = tryStateAdvance();

		if (!connectAdvance) {
			std::this_thread::yield();
		}
	}
}

ServerConnection::ServerConnection() {

	logMsg("new ServerConnection created", "ServerConnection");

	initConfig();
	initSockets();
	start();
}

ServerConnection::~ServerConnection() {
	logMsg("will now destroy connection", "~ServerConnection");
	disconnectAll();
}

void ServerConnection::Reset() {

	delete _instance;
	_instance = nullptr;

	ServerConnection::Instance();
}

void ServerConnection::SetAuth(const std::string& login, const std::string& pass) {

	logMsg("will now try to set credentials", "ServerConnection::SetAuth");

	std::lock_guard<std::mutex> guard(_authMutex);

	_loginStr = login;
	_passStr = pass;

	logMsg("new credentials set " + login + "," + pass, "ServerConnection::SetAuth");

	_authCondition.notify_one();
}

void ServerConnection::disconnectAll() {
	
	logMsg("will now disconnect everything", "ServerConnection::disconnectAll");

	if (_worker.joinable()) {
		workerEndRequested = true;
		_worker.join();
	}
	else {
		logErr("worker was supposed to be busy now, why it ended earlier? ", "ServerConnection::disconnectAll", LogMessage::Priority::LOW);
	}

	destroySockets();
}

void ServerConnection::initConfig() {
	
	// todo: read from config instead?
	_authURL = defaultAuthURL();

	//_loginStr = "test_user";
	//_passStr = "pass123";
}

const std::string& ServerConnection::getAuthURL() const {
	return _authURL;
}

const std::string& ServerConnection::getGameURL() const {
	
	if (_gameURL.empty()) {
		return getAuthURL(); // separate auth & game servers not implemented yet?
	}

	return _gameURL;
}

bool ServerConnection::sendAuthMsg(const std::string& msg, const Block block) const {
	
	logMsg("will send auth msg", "ServerConnection::sendAuthMsg", LogMessage::Priority::LOW);
	const bool ok = _sockAuth->Send(msg, block);
	return ok;
}

bool ServerConnection::sendGameMsg(const std::string& msg, const Block block) const {

	logMsg("will send game msg", "ServerConnection::sendGameMsg", LogMessage::Priority::LOW);
	const bool ok = _sockGame->Send(msg, block);
	//todo: logme
	return ok;
}

std::string ServerConnection::receiveAuthMsg(const Block block) const {

	logMsg("will now try to receive auth msg", "ServerConnection::receiveAuthMsg", LogMessage::Priority::LOW);
	
	const auto& msg = _sockAuth->Receive(block);
	return msg;
}
std::string ServerConnection::receiveGameMsg(const Block block) const {

	logMsg("will now try to receive game msg", "ServerConnection::receiveAuthMsg", LogMessage::Priority::LOW);

	const auto& msg = _sockGame->Receive(block);
	return msg;
}

bool ServerConnection::tryAuth(const Block block) {
	
	const LoginMessage loginMsg(_loginStr, _passStr);
	const std::string& loginMsgStr = loginMsg.toString();

	const bool sendOk = sendAuthMsg(loginMsgStr, block);

	if (!sendOk) {
		logErr("cannot send auth message", "ServerConnection::tryAuth");
		return false;
	}

	const std::string& authReplyStr = receiveAuthMsg(block);

	if (authReplyStr.empty()) {
		logErr("unable to get auth server reply", "ServerConnection::tryAuth");
		return false;
	}
	
	rabbit::document authReplyJson;
	authReplyJson.parse(authReplyStr);

	const std::string& replyMsg = authReplyJson.has("message") ? (authReplyJson["message"].as_string()) : ("");
	const bool replyOk = replyMsg == "OK";

	if (!replyOk) {

		const bool invalidCred = replyMsg == "INVALID_CREDENTIALS";
		if (invalidCred) {
			logMsg("auth server thinks that login/password are invalid", "ServerConnection::tryAuth");
		} else {
			logErr("auth server declined authorization with the following reason: " + replyMsg, "ServerConnection::tryAuth");
		}

		return false;
	}

	const bool hasGameURL = authReplyJson.has("gameURL");
	const bool hasToken = authReplyJson.has("token");

	if (hasGameURL) {
		_gameURL = authReplyJson["gameURL"].as_string();
	} else {
		logErr("gameURL not received from server", "ServerConnection::tryAuth", LogMessage::Priority::LOW);
		// todo: will return false later
	}

	if (hasToken) {
		_authToken = authReplyJson["token"].as_string();
	} else {
		logErr("token not received from server", "ServerConnection::tryAuth", LogMessage::Priority::NORMAL);
		// todo: will return false later
	}

	return true;
}

bool ServerConnection::connectAuth(const Block block) {
	logMsg("will try connect to auth server", "ServerConnection::connectAuth");
	const auto& authURL = getAuthURL();
	const bool connected = _sockAuth->Connect(authURL, block);
	
	if (!connected) {
		logErr("auth not connected", "ServerConnection::connectGame");
	}
	
	return connected;
}

bool ServerConnection::connectGame(const Block block) {
	
	logMsg("will try connect to game server", "ServerConnection::connectGame");
	const auto& gameURL = getGameURL();
	const bool connected = _sockGame->Connect(gameURL, block);

	if (!connected) {
		logErr("game not connected", "ServerConnection::connectGame");
	}

	return connected;
}

ServerConnection* ServerConnection::Instance() {

	if (!_instance) {
		_instance = new ServerConnection();
	}

	return _instance;
}