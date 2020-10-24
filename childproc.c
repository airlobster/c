
#include <stdlib.h>
#include "childproc.h"

typedef enum _pipe_end_t {
	peREAD=0,
	peWRITE=1
} pipe_end_t;

#define def_max(type)	type max(type a, type b) { return a > b ? a : b; }
static def_max(int);

static size_t readFromChild(stream_type_t type, int fd, child_process_callback_t cb, void* user) {
	char buf[128];
	size_t t = 0;
	for(;;) {
		int n = read(fd, buf, sizeof(buf));
		if( n <= 0 )
			break;
		t += n;
		if( cb )
			cb(type, buf, n, user);
	}
	return t;
}

int child_process_exec(const char* cmd, FILE* fin, child_process_callback_t cb, void* user) {
#define CLOSE(fd) if(fd) {close(fd); (fd)=0;}
	char* args[] = {"/bin/sh", "-c", (char*)cmd, 0};
	pid_t pid=0;
	int status=0;
	int pipes[3][2] = {{0,0},{0,0},{0,0}};
	int rc = 0;
	extern char** environ;

	// use default shell if configured
	char* shell = getenv("SHELL");
	if( shell )
		args[0] = shell;

	// create pipes for STDIN, STDOUT and STDERR
	for(int i=0; i <= 2; i++) {
		pipe(pipes[i]);
	}

	// replace STDIN pipe's read end with the provided fin stream
	if( fin )
		dup2(fileno(fin), pipes[STDIN_FILENO][peREAD]);

	pid = fork();
	if( ! pid ) {
		// CHILD PROCESS
		// close unnecessary pipe ends
		close(pipes[STDIN_FILENO][peWRITE]);
		close(pipes[STDOUT_FILENO][peREAD]);
		close(pipes[STDERR_FILENO][peREAD]);
		// assign STDIN, STDOUT and STDERR
		dup2(pipes[STDIN_FILENO][peREAD], STDIN_FILENO);
		dup2(pipes[STDOUT_FILENO][peWRITE], STDOUT_FILENO);
		dup2(pipes[STDERR_FILENO][peWRITE], STDERR_FILENO);
		// execute
		exit(execve(args[0], args, environ));
	}

	// PARENT PROCESS
	// close unnecessary pipe ends
	CLOSE(pipes[STDIN_FILENO][peREAD]);
	CLOSE(pipes[STDOUT_FILENO][peWRITE]);
	CLOSE(pipes[STDERR_FILENO][peWRITE]);

	// intercept STDOUT and STDERR from child process
	int fdMax = max(pipes[STDOUT_FILENO][peREAD], pipes[STDERR_FILENO][peREAD]);
	for(;;) {
		fd_set read_set;
		size_t n = 0;
		FD_ZERO(&read_set);
		FD_SET(pipes[STDOUT_FILENO][peREAD], &read_set);
		FD_SET(pipes[STDERR_FILENO][peREAD], &read_set);
		int r = select(fdMax+1, &read_set, 0, 0, 0);
		if( r < 0 )
			break;
		for(int i=STDOUT_FILENO; i <= STDERR_FILENO; i++) {
			if( FD_ISSET(pipes[i][peREAD], &read_set) )
				n += readFromChild((stream_type_t)i, pipes[i][peREAD], cb, user);
		}
		if( ! n )
			break;
	}

	waitpid(pid, &status, 0);
	if( WIFEXITED(status) )
		rc = WEXITSTATUS(status);
	else
		rc = -1;

	CLOSE(pipes[STDIN_FILENO][peWRITE]);
	CLOSE(pipes[STDOUT_FILENO][peREAD]);
	CLOSE(pipes[STDERR_FILENO][peREAD]);

	return rc;
#undef CLOSE
}


