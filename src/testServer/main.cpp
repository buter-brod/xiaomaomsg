#include <iostream>
#include "NetSocket.h"

#include <fstream>
#include <streambuf>

//#define NO_TLS

std::string loadFile(const std::string& filename) {
	std::ifstream t(filename);

	const bool openOk = t.is_open();

	if (!openOk) {
		return "";
	}

	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	return str;
}

int server(const char* url)
{
	auto sockAuth = std::make_unique<NetSocket>();

	const std::string& certificate = loadFile("../xiaomao.crt");
	const std::string& privateKey = loadFile("../xiaomao.key");

	sockAuth->SetCertificate(certificate);
	sockAuth->SetPrivateKey(privateKey);

	const bool listening = sockAuth->Listen(url);

	if (listening) {
		std::cout << std::string("\nlistening on ") + url << std::endl;
	}

	for (;;) {
		const std::string& message = sockAuth->Receive();
		std::cout << "IN: " << message << std::endl;
		const std::string answer = "{\"message\":\"OK\"}";
		sockAuth->Send(answer);
	}
}

int main() {

	std::string url = "tcp://127.0.0.1:50051";

#ifndef NO_TLS
	url = std::string("tls+") + url;
#endif

  server(url.c_str());

  return 0;
}
