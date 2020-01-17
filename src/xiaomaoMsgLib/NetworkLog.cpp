#include "NetworkLog.h"
#include <iostream>
#include <fstream>

//#include "base/CCConsole.h"

#define logFile "network.log"

#ifdef RELEASE_BUILD
constexpr size_t errorsToKeep = 0;
#else
constexpr size_t errorsToKeep = 100;
#endif

LogMessage::LogMessage(
	const std::string& message,
	const utils::Time& time,
	const std::string& where,
	const int priority) :

	_message (message),
	_time    (time),
	_where   (where),
	_priority(priority)
{
	init();
}

LogMessage::LogMessage(const std::string& message, const std::string& where) :
	_message(message),
	_where(where) {

	init();
}

const std::string& LogMessage::getString() const {
	if (_str.empty()) {
		_str = generateStr();
	}

	return _str;
}

bool LogMessage::isError() const { return false; }

bool LogMessage::operator==(const LogMessage& LogMessage) const {

	const bool equals =
		_time     == LogMessage._time     &&
		_priority == LogMessage._priority &&
		_where    == LogMessage._where    &&
		_message  == LogMessage._message;

	return equals;
}

const LogMessage& LogMessage::nullLogMsg() {
	static const LogMessage nullInfo;
	return nullInfo;

}
bool LogMessage::isNull() const {
	const bool eq = (*this == nullLogMsg());
	return eq;
}

void LogMessage::init() {

	if (_time.isNull()) {
		_time = utils::Time::CurrentTime();
	}

	if (_where.empty()) {

		const auto printStack = []() ->std::string {
			//todo:optional platform-dependent stack trace code
			return "";
		};

		_where = printStack();
	}
}

std::string LogMessage::generateStr() const {

	const std::string& timeStr = std::string("(") + _time.toString(utils::Time::DATETIME_MS) + ")";

	const std::string& whereStr   =   _where.empty() ? "" : std::string(" <") + _where   + ">";
	const std::string& messageStr = _message.empty() ? "" : std::string(" [") + _message + "]";

	const std::string& priorityStr = (_priority != 0) ? std::string(" P=") + std::to_string(_priority) : "";

	const std::string& all = timeStr + priorityStr + whereStr + messageStr;
	return all;
}

ErrorMessage::ErrorMessage(
	const std::string& message,
	const int errCode,
	const int subErrCode,
	const utils::Time& time,
	const std::string& where,
	const int priority) :

	LogMessage(message, time, where, priority),
	_errCode(errCode),
	_subErrCode(subErrCode) {

	init();
}

ErrorMessage ErrorMessage::CreateForSubcode(const int subCode) {
	ErrorMessage ei("", ErrorMessage::NOT_SPECIFIED, subCode);
	return ei;
}

bool ErrorMessage::isError() const { return true; };

const ErrorMessage& ErrorMessage::nullErr() {
	static const ErrorMessage nullInfo;
	return nullInfo;
}

bool ErrorMessage::isNull() const {
	const bool eq = (*this == nullErr());
	return eq;
}

std::string ErrorMessage::generateStr() const {

	const std::string& logMsg = LogMessage::generateStr();

	const std::string& suberrCodeStr = (_subErrCode != 0) ? std::string(" ErrSubcode:") + std::to_string(_subErrCode) : "";
	const std::string& errCodeStr    =    (_errCode != 0) ? std::string(" Err:")        + std::to_string(   _errCode) : "";

	const std::string& all = logMsg + errCodeStr + suberrCodeStr + " {ERR}";
	return all;
}

bool ErrorMessage::operator==(const ErrorMessage& ei) const {

	const auto thisLogMessage = static_cast<LogMessage const*>(this);
	const auto thatLogMessage = static_cast<LogMessage const*>(&ei);

	const bool equalsBase = (*thisLogMessage == *thatLogMessage);

	const bool equals =
		equalsBase &&
		_errCode    == ei._errCode &&
		_subErrCode == ei._subErrCode;

	return equals;
}

NetworkLog* NetworkLog::Inst() {
	static auto* instance = new NetworkLog();
	return instance;
}

const std::string& NetworkLog::getLogFilename() {
	static const std::string& logFilename(logFile);
	return logFilename;
}

std::set<NetworkLog::Channel> NetworkLog::getEnabledChannels() const {

	static std::set<NetworkLog::Channel> channels = {
		Channel::ERR,
		Channel::MISC,
		Channel::VERBOSE
	};

	return channels;
}

void NetworkLog::PutMessage(const LogMessage::Ptr& msg) {
	if (!msg || msg->isNull()) {
		return;
	}

	const int priority = msg->getPriority();
	const bool isErr = msg->isError();

	Channel ch = Channel::MISC;
	if (isErr) {
		ch = Channel::ERR;

	} else if (priority < 0) {
		ch = Channel::VERBOSE;
	}

	const std::string& msgStr = msg->getString();

	const auto& enabledChannels = getEnabledChannels();
	const bool channelOk = enabledChannels.count(ch) > 0;

	if (channelOk) {

		std::lock_guard<std::mutex> authLock(_logMutex);

		putStr(msgStr);

		if (isErr) {
			const auto& errMsgPtr = std::static_pointer_cast<ErrorMessage>(msg);
			_errors.push_back(errMsgPtr);

			if (errorsToKeep > 0 && _errors.size() > errorsToKeep) {
				_errors.pop_front();
			}

#ifndef RELEASE_BUILD

			constexpr auto crashTolerance = LogMessage::Priority::NORMAL;

			if (priority > crashTolerance) {
				throw std::exception(msgStr.c_str());
				//abort();
			}
#endif
		}
	}
}

void NetworkLog::putStr(const std::string& msg) {

	const std::string& logFilename = getLogFilename();

	writeConsole(msg.c_str());

	if (!logFilename.empty()) {
		std::ofstream logFileStream;
		logFileStream.open(logFilename, std::ofstream::out | std::ofstream::app);

		if (logFileStream.good()) {
			logFileStream << msg << std::endl;
		}
		else {
			std::cout << "Log::putMsg error, unable to open " + logFilename + "\n";
		}
	}
}

void NetworkLog::writeConsole(const char* what) {
#ifdef CC_TARGET_PLATFORM
	cocos2d::log(what);
#else
	std::cout << "\n" << what;
#endif
}