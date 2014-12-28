/* (c) eeeee */

#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

int websocket_create(const char* addr, int port);
int websocket_destroy(int socket);
int websocket_recv(int socket, unsigned char* data, size_t maxsize, struct sockaddr_in *sockaddrbuf, size_t fromLen);
int websocket_send(int socket, const unsigned char* data, size_t size, int port);

#endif