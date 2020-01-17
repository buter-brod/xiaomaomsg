

//#include "nng/nng.h"
//#include "nng/protocol/reqrep0/rep.h"
//#include <cstdio>
//#include <ctime>
//
//int server(const char* url)
//{
//	nng_socket sock;
//	int        rv;
//
//	if ((rv = nng_rep0_open(&sock)) != 0) {
//		fatal("nng_rep0_open", rv);
//	}
//	if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
//		fatal("nng_listen", rv);
//	}
//	for (;;) {
//		char* buf = NULL;
//		size_t   sz;
//		uint64_t val;
//		if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
//			fatal("nng_recv", rv);
//		}
//		if ((sz == sizeof(uint64_t)) &&
//			((GET64(buf, val)) == DATECMD)) {
//			time_t now;
//			printf("SERVER: RECEIVED DATE REQUEST\n");
//			now = time(&now);
//			printf("SERVER: SENDING DATE: ");
//			showdate(now);
//
//			// Reuse the buffer.  We know it is big enough.
//			PUT64(buf, (uint64_t)now);
//			rv = nng_send(sock, buf, sz, NNG_FLAG_ALLOC);
//			if (rv != 0) {
//				fatal("nng_send", rv);
//			}
//			continue;
//		}
//		// Unrecognized command, so toss the buffer.
//		nng_free(buf, sz);
//	}
//}

#include "ServerConnection.h"

int main() {

    ServerConnection::State lastServerStatus = ServerConnection::Instance()->GetState();

  return 0;
}
