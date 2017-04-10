#ifndef MM_TIMER_H_
#define MM_TIMER_H_

/*
 * machinarium.
 *
 * cooperative multitasking engine.
*/

typedef struct mm_timer_t mm_timer_t;

typedef void (*mm_timer_callback_t)(mm_timer_t*);

struct mm_timer_t {
	int                  active;
	int                  timeout;
	int                  interval;
	int                  seq;
	mm_timer_callback_t  callback;
	void                *arg;
	void                *clock;
};

static inline void
mm_timer_init(mm_timer_t *timer, mm_timer_callback_t cb, void *arg, int interval)
{
	timer->active = 0;
	timer->interval = interval;
	timer->timeout = 0;
	timer->seq = 0;
	timer->callback = cb;
	timer->arg = arg;
	timer->clock = NULL;
}

#endif
