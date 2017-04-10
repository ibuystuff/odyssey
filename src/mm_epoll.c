
/*
 * machinarium.
 *
 * cooperative multitasking engine.
*/

#include <machinarium_private.h>
#include <machinarium.h>

typedef struct mm_epoll_t mm_epoll_t;

struct mm_epoll_t {
	mm_poll_t           poll;
	int                 fd;
	struct epoll_event *list;
	int                 size;
	int                 count;
};

static mm_poll_t*
mm_epoll_create(void)
{
	mm_epoll_t *epoll;
	epoll = malloc(sizeof(mm_epoll_t));
	if (epoll == NULL)
		return NULL;
	epoll->poll.iface = &mm_epoll_if;
	epoll->count = 0;
	epoll->size  = 1024;
	int size = sizeof(struct epoll_event) * epoll->size;
	epoll->list  = malloc(size);
	if (epoll->list == NULL) {
		free(epoll);
		return NULL;
	}
	memset(epoll->list, 0, size);
	epoll->fd = epoll_create(epoll->size);
	if (epoll->fd == -1) {
		free(epoll->list);
		free(epoll);
		return NULL;
	}
	return &epoll->poll;
}

static void
mm_epoll_free(mm_poll_t *poll)
{
	mm_epoll_t *epoll = (mm_epoll_t*)poll;
	if (epoll->list)
		free(epoll->list);
	free(poll);
}

static int
mm_epoll_shutdown(mm_poll_t *poll)
{
	mm_epoll_t *epoll = (mm_epoll_t*)poll;
	if (epoll->fd != -1) {
		close(epoll->fd);
		epoll->fd = -1;
	}
	return 0;
}

static int
mm_epoll_step(mm_poll_t *poll, int timeout)
{
	mm_epoll_t *epoll = (mm_epoll_t*)poll;
	if (epoll->count == 0)
		return 0;
	int count;
	count = epoll_wait(epoll->fd, epoll->list, epoll->count, timeout);
	if (count <= 0)
		return 0;
	int i = 0;
	while (i < count) {
		struct epoll_event *ev = &epoll->list[i];
		mm_fd_t *fd = ev->data.ptr;
		int events = 0;
		if (ev->events & EPOLLIN) {
			events = MM_R;
		}
		if (ev->events & EPOLLOUT ||
			ev->events & EPOLLERR ||
			ev->events & EPOLLHUP) {
			events |= MM_W;
		}
		fd->callback(fd, events);
		i++;
	}
	return count;
}

static int
mm_epoll_add(mm_poll_t *poll, mm_fd_t *fd, int mask)
{
	mm_epoll_t *epoll = (mm_epoll_t*)poll;
	if ((epoll->count + 1) > epoll->size) {
		int size = epoll->size * 2;
		void *ptr = realloc(epoll->list, sizeof(struct epoll_event) * size);
		if (ptr == NULL)
			return -1;
		free(epoll->list);
		epoll->list = ptr;
		epoll->size = size;
	}
	struct epoll_event ev;
	ev.events = 0;
	fd->mask = mask;
	if (fd->mask & MM_R)
		ev.events |= EPOLLIN;
	if (fd->mask & MM_W)
		ev.events |= EPOLLOUT;
	ev.data.ptr = fd;
	int rc = epoll_ctl(epoll->fd, EPOLL_CTL_ADD, fd->fd, &ev);
	if (rc == -1)
		return -1;
	epoll->count++;
	return 0;
}

static int
mm_epoll_modify(mm_poll_t *poll, mm_fd_t *fd, int mask)
{
	mm_epoll_t *epoll = (mm_epoll_t*)poll;
	struct epoll_event ev;
	ev.events = 0;
	if (mask & MM_R)
		ev.events |= EPOLLIN;
	if (mask & MM_W)
		ev.events |= EPOLLOUT;
	ev.data.ptr = fd;
	int rc = epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd->fd, &ev);
	if (rc == -1)
		return -1;
	fd->mask = mask;
	return 0;
}

static int
mm_epoll_del(mm_poll_t *poll, mm_fd_t *fd)
{
	mm_epoll_t *epoll = (mm_epoll_t*)poll;
	struct epoll_event ev;
	ev.events = 0;
	if (fd->mask & MM_R)
		ev.events |= EPOLLIN;
	if (fd->mask & MM_W)
		ev.events |= EPOLLOUT;
	ev.data.ptr = fd;
	epoll->count--;
	assert(epoll->count >= 0);
	return epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd->fd, &ev);
}

mm_pollif_t mm_epoll_if =
{
	.name     = "epoll",
	.create   = mm_epoll_create,
	.free     = mm_epoll_free,
	.shutdown = mm_epoll_shutdown,
	.step     = mm_epoll_step,
	.add      = mm_epoll_add,
	.modify   = mm_epoll_modify,
	.del      = mm_epoll_del
};
