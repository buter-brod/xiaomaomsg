#include "ServerConnection.h"
#include <iostream>

int main() {

	ServerConnection::State lastServerStatus = ServerConnection::Instance()->GetState();
	ServerConnection::Instance()->SetAuth("user", "pass");

	std::cout << "\n";

	do {
		const auto currStatus = ServerConnection::Instance()->GetState();

		if (currStatus != lastServerStatus) {
			std::cout << "\nNew status: " << ServerConnection::Instance()->StateToString() << "\n\n";
			lastServerStatus = currStatus;
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	} 
	while (true);

	return 0;
}