
#ifndef EXTRA_POLLER_H
#define EXTRA_POLLER_H

typedef struct poller poller_t;

typedef struct handle handle_t;

struct handle_param
{
	int fd;
};


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @return NULL in case of error
 */
poller_t * poller_create();

void poller_destroy(poller_t * poller);

handle_t * poller_add(const struct handle_param * param, poller_t * poller);

void poller_del(int fd, poller_t * poller);

void poller_go(poller_t * poller);

int poller_mark(int ack, int timeout, handle_t * handle, poller_t * poller);

#ifdef __cplusplus
}
#endif

#endif //EXTRA_POLLER_H
