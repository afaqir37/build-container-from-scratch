#include <iostream>
#include <sys/prctl.h>
#include <sys/mount.h>
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

void setup_variables() {	
	clearenv(); // remove all env variables for this process		
	setenv("TERM", "xterm-256color", 0);
	setenv("PATH", "/bin/:/sbin/:usr/bin:/usr/sbin", 0);
}

void setup_root() {
	chroot("./root");
	chdir("/");
}

template <typename... P>
int run(P...params) {
	char *args[] = {(char *)params..., (char *)0};
	return execvp(args[0], args);
}

int jail(void *args) {
	printf("child process id: %d\n", getpid());
	setup_variables();
	setup_root();
		
	mount("proc", "/proc", "proc", 0, 0);
	prctl(PR_SET_NAME, (unsigned long)"messi", 0, 0, 0);
	printf("running ps: \n");
	system("ps");

	auto runThis = [](void *args) -> int {
		prctl(PR_SET_NAME, (unsigned long)"prctl", 0, 0, 0);
		printf("second child process id: %d\n", getpid());
		mount("proc", "/proc", "proc", 0, 0);
		
		printf("second running ps: \n");
		system("ps");
		run("/bin/sh");
		return EXIT_SUCCESS;
	};

	clone(runThis, stack_memory(), CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | SIGCHLD, 0);
	wait(NULL);	

	
	umount("/proc");
	return (EXIT_SUCCESS);
}

int main() {
	printf("Hello, World!, (parent) \n");
	printf("parent process id: %d\n", getpid());
	clone(jail, stack_memory(), CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | SIGCHLD, 0);
	wait(NULL);
	return EXIT_SUCCESS;
}


