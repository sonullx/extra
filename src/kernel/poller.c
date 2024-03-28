
#include <errno.h>
#include <malloc.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "poller.h"
#include "list.h"
#include "rbtree.h"

#ifndef MAX_FD
#define MAX_FD 1024
#endif

struct piece
{
	int seq;
	struct timespec timeout;
	struct rb_node in_handle;
	struct rb_node in_poller;
};

struct handle
{
	struct handle_param data;
	struct rb_root pieces;
	struct rb_node in_poller;
};

struct poller
{
	int pfd;
	struct rb_root handles;
	struct rb_root pieces;
};


static struct handle * handle_create(const struct handle_param * param)
{
	struct handle * handle;

	if ((unsigned) param->fd > MAX_FD)
	{
		errno = param->fd < 0 ? EBADF : EMFILE;
		return NULL;
	}

	handle = (struct handle *) malloc(sizeof(struct handle));
	if (handle)
	{
		handle->data = *param;
		// TODO: list, timeout
	}
	return handle;
}

static void handle_destroy(struct handle * handle)
{
	free(handle);
	// TODO: list
}

static void handle_read(struct handle * handle)
{
	// TODO
}

static void handle_write(struct handle * handle)
{
	// TODO
}

poller_t * poller_create()
{
	struct poller * poller = (struct poller *) malloc(sizeof(struct poller));
	if (poller)
	{
		poller->pfd = epoll_create(1);
		if (poller->pfd >= 0)
			return poller;

		free(poller);
	}
	return poller;
}

void poller_destroy(poller_t * poller)
{
	close(poller->pfd);
	free(poller);
}


handle_t * poller_add(const struct handle_param * param, poller_t * poller)
{
	struct epoll_event event;
	struct handle * handle;

	handle = handle_create(param);
	if (handle)
	{
		event.events = EPOLLIN | EPOLLOUT | EPOLLET;
		event.data.ptr = handle;

		if (!epoll_ctl(poller->pfd, EPOLL_CTL_ADD, handle->data.fd, &event))
			return handle;

		handle_destroy(handle);
	}
	return NULL;
}

#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

void poller_go(poller_t * poller)
{
	struct epoll_event events[MAX_EVENTS];
	struct handle * handle;
	uint32_t event_type;
	int event_count;
	int i;

	event_count = epoll_wait(poller->pfd, events, MAX_EVENTS, 0);
	for (i = 0; i < event_count; i++)
	{
		event_type = events[i].events;
		handle = (struct handle *) events[i].data.ptr;
		if ((event_type & EPOLLOUT) == EPOLLOUT)
			handle_write(handle);

		if ((event_type & EPOLLIN) == EPOLLIN)
			handle_read(handle);
	}
}
