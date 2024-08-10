#include <iostream>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#define CGROUP_FOLDER "/sys/fs/cgroup/pids/container/"
#define concat(a,b) (a"" b)

void writeRule(const char *path, const char *value) {
	int fp = open(path, O_WRONLY | O_APPEND);
	if (fp == -1)
		perror("write");
	write(fp, value, strlen(value));
	close(fp);
}

void limitProcessCreation() {
	mkdir(CGROUP_FOLDER, S_IRUSR | S_IWUSR);

	const char *pid = std::to_string(getpid()).c_str();
	writeRule(concat(CGROUP_FOLDER, "cgroup.procs"), pid);
	writeRule(concat(CGROUP_FOLDER, "notify_on_release"), "1");
	writeRule(concat(CGROUP_FOLDER, "pids.max"), "5");
	
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
	limitProcessCreation();
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

	clone(runThis, stack_memory(), CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, 0);
	wait(NULL);	
 
	
	umount("/proc");
	return (EXIT_SUCCESS);
}

int main() {
	printf("Hello, World!, (parent) \n");
	printf("parent process id: %d\n", getpid());
	clone(jail, stack_memory(), CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, 0);
	wait(NULL);
	umount("./root/proc");
	return EXIT_SUCCESS;
}


