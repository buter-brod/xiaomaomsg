#ifndef NETWORKLOG_H
#define NETWORKLOG_H

#include "TimeUtils.h"
#include <set>
#include <memory>
#include <deque>
#include <mutex>

class LogMessage {
public:

	enum Priority {
		LOWEST = -2,
		LOW = -1,
		NORMAL = 0,
		HIGH = 1,
		CRITICAL = 2
	};

	typedef std::shared_ptr<LogMessage> Ptr;

	LogMessage() = default;
	virtual ~LogMessage() = default;

	LogMessage(
		const std::string& message,
		const utils::Time& time = utils::Time::CurrentTime(),
		const std::string& where = "",
		const int priority = NORMAL);

	LogMessage(const std::string& message, const std::string& where = "");

	const std::string& getString() const;
	virtual bool isError() const;
	bool operator==(const LogMessage& logInfo) const;

	static const LogMessage& nullLogMsg();
	virtual bool isNull() const;

	const std::string& getMessage() const { return _message; }
	const std::string& getWhere()   const { return _where; }
	const utils::Time& getTime()    const { return _time; }
	int getPriority()               const { return _priority; }

protected:

	void init();
	virtual std::string generateStr() const;

	std::string _message;
	std::string _where;

	utils::Time _time;
	int _priority{ NORMAL };

	mutable std::string _str;
};

class ErrorMessage : public LogMessage {
public:

	enum ErrorCode {
		NOT_SPECIFIED = 0
	};

	enum SuberrorCode {
		NO_SUBERROR = 0
	};

	typedef std::shared_ptr<ErrorMessage> Ptr;

	ErrorMessage() = default;
	virtual ~ErrorMessage() = default;

	ErrorMessage(
		const std::string& message,
		const int errCode = NOT_SPECIFIED,
		const int subErrCode = NO_SUBERROR,
		const utils::Time& time = utils::Time::CurrentTime(),
		const std::string& where = "",
		const int priority = LogMessage::NORMAL);

	static ErrorMessage CreateForSubcode(const int subCode);

	bool isError() const override;
	static const ErrorMessage& nullErr();
	bool operator==(const ErrorMessage& ei) const;

	bool isNull() const override;

protected:

	std::string generateStr() const override;

	int _errCode{ 0 };
	int _subErrCode{ 0 };
};

class NetworkLog {
public:

	enum class Channel {
		ERR,
		MISC,		
		VERBOSE
	};

	static NetworkLog* Inst();
	void PutMessage(const LogMessage::Ptr& msg);

protected:

	static const std::string& getLogFilename();
	static void putStr(const std::string& msg);
	static void writeConsole(const char* what);

	std::set<Channel> getEnabledChannels() const;

private:
	NetworkLog() = default;

	std::deque<ErrorMessage::Ptr> _errors;
	std::mutex _logMutex;
};

#endif