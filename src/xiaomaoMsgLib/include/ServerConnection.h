#ifndef CLASSES_NETWORK_SERVERCONNECTION_H
#define CLASSES_NETWORK_SERVERCONNECTION_H

#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>

constexpr unsigned int secondsToRetryInitial = 1;

//#define NO_TLS
#include "NNGSocket.h"

class ServerConnection {
public:
	enum class State {
		INIT,
		READY_TO_CONNECT,
		CONNECTING_TO_AUTH_SERVER,
		AUTH_WILL_RETRY_TO_CONNECT,
		AUTH_CONNECTED,
		WAITING_FOR_AUTH_INPUT,
		TRYING_AUTH,
		AUTH_FAILED,
		AUTH_SUCCESSFUL,
		GAME_CONNECTING,
		GAME_WILL_RETRY_TO_CONNECT,
		GAME_CONNECTED,
		GAME_RETRIEVING_INFO,
		CONNECTED
	};

	static ServerConnection* Instance();
	static void Reset();

	void SetAuth(const std::string& login, const std::string& pass);
	State GetState() const {return _state.load();}

	std::string StateToString() const;
	std::string StateToString(ServerConnection::State st) const;

private:

	ServerConnection();
	~ServerConnection();

	void initConfig();
	void start();
	bool initSockets();
	bool setState(const State newState);

	bool connectAuth(const Block block = Block::BLOCKING);
	bool connectGame(const Block block = Block::BLOCKING);

	bool destroySockets();
	void disconnectAll();

	bool tryAuth(const Block block = Block::BLOCKING);
	void resetCredentials();

	bool sendAuthMsg(const std::string& msg, const Block block = Block::BLOCKING) const;
	bool sendGameMsg(const std::string& msg, const Block block = Block::BLOCKING) const;

	std::string receiveAuthMsg(const Block block = Block::NON_BLOCKING) const;
	std::string receiveGameMsg(const Block block = Block::NON_BLOCKING) const;

	const std::string& getAuthURL() const;
	const std::string& getGameURL() const;

	const ErrorMessage::Ptr& getError() const;

	//LOG
	void logMsg(const std::string& what, const std::string& where = "", const int priority = LogMessage::NORMAL) const;
	void logErr(const std::string& what, const std::string& where = "", const int priority = LogMessage::NORMAL) const;

	static ServerConnection* _instance;

	mutable ErrorMessage::Ptr _latestErr;

	std::unique_ptr<NNGSocket> _sockAuth;
	std::unique_ptr<NNGSocket> _sockGame;
	// todo: add 'observer' socket. Having just 1 req/rep socket is not sufficient as server needs a way to notify us about some events

	std::string _authURL;
	std::string _gameURL;
	
	std::string _authToken;
	
	std::string _loginStr;
	std::string _passStr;

	unsigned int _secondsToRetry{secondsToRetryInitial};

	//threading
	void process();
	bool tryStateAdvance();
	void waitForAuth();

	std::thread _worker;

	std::atomic<State> _state{ State::INIT };
	std::atomic<bool> workerEndRequested{ false };

	std::condition_variable _authCondition;
	std::mutex _authMutex;
};


#endif