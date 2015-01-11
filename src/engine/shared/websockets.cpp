/* (c) eeeee */

#include <string.h>
#include <stdlib.h>

#include "engine/external/libwebsockets/libwebsockets.h"
#include "base/system.h"
#include "ringbuffer.h"
#include <netinet/in.h>
#include <arpa/inet.h>

extern "C" {

typedef TStaticRingBuffer<unsigned char, 640*1024, CRingBufferBase::FLAG_RECYCLE> TRecvBuffer;
typedef TStaticRingBuffer<unsigned char, 64*1024, CRingBufferBase::FLAG_RECYCLE> TSendBuffer;

typedef struct {
  size_t size;
  size_t read;
  sockaddr_in addr;
  unsigned char data[0];
} websocket_chunk;

struct per_session_data {
	struct libwebsocket *wsi;
	int port;
	sockaddr_in addr;
	TSendBuffer send_buffer;
};

#define WS_CLIENTS 2048

struct context_data {
	per_session_data* port_map[WS_CLIENTS];
	TRecvBuffer recv_buffer;
	int last_used_port;
};

void netaddr_to_sockaddr_in(const NETADDR *src, struct sockaddr_in *dest)
{
	mem_zero(dest, sizeof(struct sockaddr_in));
	if(src->type != NETTYPE_IPV4 && src->type != NETTYPE_WEBSOCKET_IPV4)
	{
		dbg_msg("system", "couldn't convert NETADDR of type %d to ipv4", src->type);
		return;
	}

	dest->sin_family = AF_INET;
	dest->sin_port = htons(src->port);
	mem_copy(&dest->sin_addr.s_addr, src->ip, 4);
}

static int
websocket_callback(struct libwebsocket_context *context,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason, void *user,
	void *in, size_t len)
{
	websocket_chunk* chunk;
	struct per_session_data *pss = (struct per_session_data *)user;

	context_data* ctx_data = (context_data*)libwebsocket_context_user(context);

	int n;

	switch (reason) {

		case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: {
			int port = -1;
			for (int i = 0; i < WS_CLIENTS; i++) {
				int j = (ctx_data->last_used_port + i + 1) % WS_CLIENTS;
				if (ctx_data->port_map[j] == NULL) {
					port = j;
					break;
				}
			}
			if (port == -1) {
				return -1;
			}
		}
		break;

		case LWS_CALLBACK_ESTABLISHED: {
			int port = -1;
			for (int i = 0; i < WS_CLIENTS; i++) {
				int j = (ctx_data->last_used_port + i + 1) % WS_CLIENTS;
				if (ctx_data->port_map[j] == NULL) {
					port = j;
					break;
				}
			}
			if (port == -1) {
				return -1;
			}
			ctx_data->last_used_port = port;
			pss->wsi = wsi;
			int fd = libwebsocket_get_socket_fd(wsi);
			socklen_t addr_size = sizeof(pss->addr);
			getpeername(fd, (struct sockaddr *)&pss->addr, &addr_size);
			int orig_port = ntohs(pss->addr.sin_port);
			pss->addr.sin_port = htons(port);
			pss->send_buffer.Init();
			pss->port = port;
			ctx_data->port_map[port] = pss;
			char addr_str[NETADDR_MAXSTRSIZE];
			inet_ntop(AF_INET, &pss->addr.sin_addr, addr_str, sizeof(addr_str));
			dbg_msg("websockets", "connection established with %s:%d , assigned fake port %d", addr_str, orig_port, port);
		}
		break;

		case LWS_CALLBACK_CLOSED: {
			dbg_msg("websockets", "connection with fake port %d closed", pss->port);
			pss->wsi = 0;
			ctx_data->port_map[pss->port] = NULL;
		}
		break;

		case LWS_CALLBACK_SERVER_WRITEABLE: {
			chunk = (websocket_chunk*)pss->send_buffer.First();
			if (chunk == NULL) {
				// libwebsocket_callback_on_writable(context, wsi);
				break;
			}
			int len = chunk->size - chunk->read;
			n = libwebsocket_write(wsi, &chunk->data[LWS_SEND_BUFFER_PRE_PADDING + chunk->read], chunk->size - chunk->read, LWS_WRITE_BINARY);
			if (n < 0) {
				return 1;
			}
			if (n < len) {
				chunk->read += n;
				libwebsocket_callback_on_writable(context, wsi);
				break;
				// return -1;
			}
			pss->send_buffer.PopFirst();
			libwebsocket_callback_on_writable(context, wsi);
		}
		break;


		case LWS_CALLBACK_RECEIVE: {
			chunk = (websocket_chunk*)ctx_data->recv_buffer.Allocate(len + sizeof(websocket_chunk));
			if (chunk == 0) {
				return 1;
			}
			chunk->size = len;
			chunk->read = 0;
			memcpy(&chunk->addr, &pss->addr, sizeof(sockaddr_in));
			memcpy(&chunk->data[0], in, len);
		}
		break;

		default:
		break;
	}

	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	{
                "binary",                         /* name */
                websocket_callback,               /* callback */
                sizeof(struct per_session_data)   /* per_session_data_size */
	},
	{
                NULL, NULL, 0           /* End of list */
	}
};

static libwebsocket_context* contexts[640]; // 640 contexts ought to be enough for anybody

int websocket_create(const char* addr, int port) {
	struct libwebsocket_context *context;
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof(info));
	info.port = port;
	info.iface = addr;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	// find me some context
	int first_free = -1;
	for (int i = 0; i < sizeof(contexts) / sizeof(libwebsocket_context*); i++) {
		if (contexts[i] == NULL) {
			first_free = i;
			break;
		}
	}
	if (first_free == -1) {
		return -1;
	}

	context_data* ctx_data = (context_data*)malloc(sizeof(context_data));
	info.user = (void*)ctx_data;

	contexts[first_free] = libwebsocket_create_context(&info);
	if (contexts[first_free] == NULL) {
		free(ctx_data);
		return -1;
	}
	memset(ctx_data->port_map, NULL, sizeof(ctx_data->port_map));
	ctx_data->recv_buffer.Init();
	ctx_data->last_used_port = 0;
	return first_free;
}

int websocket_destroy(int socket) {
	if (contexts[socket] == NULL) {
		return -1;
	}
	free(libwebsocket_context_user(contexts[socket]));
	libwebsocket_context_destroy(contexts[socket]);
	contexts[socket] = NULL;
}

int websocket_recv(int socket, unsigned char* data, size_t maxsize, struct sockaddr_in *sockaddrbuf, size_t fromLen) {

	libwebsocket_context* context = contexts[socket];
	if (context == NULL)
		return -1;
	int n = libwebsocket_service(context, 0);
	if (n < 0)
		return n;
	context_data* ctx_data = (context_data*)libwebsocket_context_user(context);
	websocket_chunk* chunk = (websocket_chunk*)ctx_data->recv_buffer.First();
	if (chunk == 0) {
		return 0;
	}
	if (maxsize >= chunk->size - chunk->read) {
		int len = chunk->size - chunk->read;
		memcpy(data, &chunk->data[chunk->read], len);
		memcpy(sockaddrbuf, &chunk->addr, fromLen);
		ctx_data->recv_buffer.PopFirst();
		return len;
	} else {
		memcpy(data, &chunk->data[chunk->read], maxsize);
		memcpy(sockaddrbuf, &chunk->addr, fromLen);
		chunk->read += maxsize;
		return maxsize;
	}
}

int websocket_send(int socket, const unsigned char* data, size_t size, int port) {
	libwebsocket_context* context = contexts[socket];
	if (context == NULL)
		return -1;
	context_data* ctx_data = (context_data*)libwebsocket_context_user(context);
	struct per_session_data *pss = ctx_data->port_map[port];
	if (pss == NULL)
		return -1;
	websocket_chunk* chunk = (websocket_chunk*)pss->send_buffer.Allocate(size + sizeof(websocket_chunk) + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);
	if (chunk == NULL)
		return -1;
	chunk->size = size;
	chunk->read = 0;
	memcpy(&chunk->addr, &pss->addr, sizeof(sockaddr_in));
	memcpy(&chunk->data[LWS_SEND_BUFFER_PRE_PADDING], data, size);
	libwebsocket_callback_on_writable(context, pss->wsi);
	return size;
}

}
