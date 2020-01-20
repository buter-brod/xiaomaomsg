#include <iostream>
#include "NetSocket.h"

int server(const char* url)
{
	auto sockAuth = std::make_unique<NetSocket>();
	const bool listening = sockAuth->Listen(url);

	if (listening) {
		std::cout << std::string("listening on ") + url << std::endl;
	}

	for (;;) {
		const std::string& message = sockAuth->Receive();
		std::cout << "IN: " << message << std::endl;
		const std::string answer = "{\"message\":\"OK\"}";
		sockAuth->Send(answer);
	}
}

int main() {

  server("ws://127.0.0.1:50051");

  return 0;
}
