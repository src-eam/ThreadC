#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#define TIME_ONE 1
#define TIME_TWO 4
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void print1() {
	printf("1\n");
}

void print2() {
	printf("2\n");
}
static ucontext_t uctx_handler1, uctx_handler2;

static void handler1(void) {
	for (;;) {
		signal(SIGALRM, print1);
		alarm(TIME_ONE);
		pause();
		if (swapcontext(&uctx_handler1, &uctx_handler2) == -1)
			handle_error("swapcontext");
	}
}

static void handler2(void) {
	for (;;) {
		signal(SIGALRM, print2);
		alarm(TIME_TWO);
		pause();
		if (swapcontext(&uctx_handler2, &uctx_handler1) == -1)
			handle_error("swapcontext");
	}
}

int main(int argc, char *argv[]) {
	char handler1_stack[SIGSTKSZ];
	char handler2_stack[SIGSTKSZ];

	if (getcontext(&uctx_handler1) == -1)
		handle_error("getcontext");
	uctx_handler1.uc_stack.ss_sp = handler1_stack;
	uctx_handler1.uc_stack.ss_size = sizeof(handler1_stack);
	uctx_handler1.uc_link = &uctx_handler2;
	makecontext(&uctx_handler1, handler1, 0);

	if (getcontext(&uctx_handler2) == -1)
		handle_error("getcontext");
	uctx_handler2.uc_stack.ss_sp = handler2_stack;
	uctx_handler2.uc_stack.ss_size = sizeof(handler2_stack);
	uctx_handler2.uc_link = &uctx_handler1;
	makecontext(&uctx_handler2, handler2, 0);

	setcontext(&uctx_handler1);

	exit(EXIT_SUCCESS);
}
