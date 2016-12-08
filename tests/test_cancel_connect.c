
/*
 * machinarium.
 *
 * Cooperative multitasking engine.
*/

#include <machinarium.h>
#include <uv.h>
#include <assert.h>

static void
test_connect(void *arg)
{
	mm_t env = arg;
	printf("child started\n");
	mm_io_t client = mm_io_new(env);
	struct sockaddr_in sa;
	uv_ip4_addr("8.8.8.8", 1324, &sa);
	int rc;
	rc = mm_connect(client, (struct sockaddr*)&sa, 0);
	printf("child resumed\n");
	assert(rc < 0);
	mm_close(client);
	if (mm_is_cancel(env))
		printf("child marked as cancel\n");
	printf("child end\n");
}

static void
test_waiter(void *arg)
{
	mm_t env = arg;

	printf("waiter started\n");

	int id = mm_create(env, test_connect, env);
	mm_sleep(env, 0);
	mm_cancel(env, id);
	mm_wait(env, id);

	printf("waiter 1 ended \n");
	mm_stop(env);
}

int
main(int argc, char *argv[])
{
	mm_t env = mm_new();
	mm_create(env, test_waiter, env);
	mm_start(env);
	printf("shutting down\n");
	mm_free(env);
	return 0;
}
