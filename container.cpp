#include <iostream>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

char* stack_memory() {
	const int stackSize = 65536;
	auto *stack = new (std::nothrow) char[stackSize];
	
	if (stack == NULL) {
		printf("Cannot allocate memory!\n");
		exit(EXIT_FAILURE);
	}
	
	return stack + stackSize; // move the pointer to the end of the array because the stack grows backward.
}

template <typename... P>
int run(P...params) {
	char *args[] = {(char *)params..., (char *)0};
	return execvp(args[0], args);
}

int jail(void *args) {
	clearenv(); // remove all env variables for this process		

	run("/bin/sh");
	return (EXIT_SUCCESS);
}

int main() {
	printf("Hello, World!, (parent) \n");

	clone(jail, stack_memory(), CLONE_NEWUTS |  SIGCHLD, 0);
	wait(NULL);
	return EXIT_SUCCESS;
}


