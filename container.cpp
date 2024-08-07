#include <iostream>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int jail(void *args) {
	sleep(4);
	printf("Hello, World!, (child) \n");
	return (EXIT_SUCCESS);
}

char* stack_memory() {
	const int stackSize = 65536;
	auto *stack = new (std::nothrow) char[stackSize];
	
	if (stack == NULL) {
		printf("Cannot allocate memory!\n");
		exit(EXIT_FAILURE);
	}
	
	return stack + stackSize; // move the pointer to the end of the array because the stack grows backward.
}


int main() {
	printf("Hello, World!, (parent) \n");

	clone(jail, stack_memory(), SIGCHLD, 0);
	wait(NULL);
	return EXIT_SUCCESS;
}


