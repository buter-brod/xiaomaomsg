#include "MessageProtocol.h"
//#include <json/stringbuffer.h>
//#include <json/writer.h>

std::string newId() {return "";}

NetworkMessage::NetworkMessage() {
	_id = newId();
}

std::string NetworkMessage::toString() const {

	if (!_json.has("req")) {
		buildJson();
	}

	if (_strDump.empty()) {
		_strDump = _json.str();
	}

	return _strDump;
}

void LoginMessage::buildJson() const {

	_json["req"] = "auth";
	rabbit::object params = _json["params"];
	params["login"] = _loginStr;
	params["pass"] = _passStr;
}

LoginMessage::LoginMessage(const std::string& login, const std::string& pass) : _loginStr(login), _passStr(pass) {
}
