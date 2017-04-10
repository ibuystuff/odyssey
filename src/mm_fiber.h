#ifndef MM_FIBER_H_
#define MM_FIBER_H_

/*
 * machinarium.
 *
 * cooperative multitasking engine.
*/

typedef struct mm_fiber_t mm_fiber_t;

typedef void (*mm_function_t)(void *arg);

typedef enum {
	MM_FIBER_NEW,
	MM_FIBER_READY,
	MM_FIBER_ACTIVE,
	MM_FIBER_FREE
} mm_fiberstate_t;

struct mm_fiber_t {
	uint64_t        id;
	mm_fiberstate_t state;
	mm_call_t       call;
	int             cancel;
	void           *function_arg;
	mm_function_t   function;
	void           *context;
	mm_fiber_t     *resume;
	mm_timer_t      timer;
	int             condition;
	int             condition_status;
	void           *scheduler;
	void           *data;
	mm_list_t       waiters;
	mm_list_t       link_wait;
	mm_list_t       link;
};

mm_fiber_t*
mm_fiber_allocate(int);

void mm_fiber_init(mm_fiber_t*);
void mm_fiber_free(mm_fiber_t*);

static inline int
mm_fiber_is_cancelled(mm_fiber_t *fiber) {
	return fiber->cancel;
}

static inline void
mm_fiber_cancel(mm_fiber_t *fiber)
{
	if (fiber->cancel)
		return;
	fiber->cancel++;
	mm_call_cancel(&fiber->call, fiber);
}

#endif
