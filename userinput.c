// userinput.c

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "userinput.h"


static char* _histfilename(const char* apppath) {
	char* s;
	const char* p;

	p = strrchr(apppath, '/');
	if( p )
		++p;
	else
		p = apppath;

	asprintf(&s, "%s/.%s.hist", getenv("HOME"), p);

	return s;
}

int ui_loop(const ui_session_t* opt) {
	int err=0, done=0;
	char* histfile = _histfilename(opt->apppath);
	int interactive = isatty(fileno(stdin));

	if( opt->flags & ufUseHistory ) {
		using_history();
		read_history(histfile);
	}

	while( ! done ) {
		char *line, *p;

		line = readline(opt->prompt);
		if( ! line )
			break;

		for(p=line; isspace(*p); ++p)
			++p;
		if( ! *p ) {
			free(line);
			continue;
		}

		add_history(p);

		err = opt->handler(p, opt->context);

		free(line);

		done = err<0 || (err && ((opt->flags & ufAbortOnError) || ! interactive));
	}

	if( opt->flags & ufUseHistory ) {
		write_history(histfile);
	}

	free(histfile);
	return err<0 ? 0 : err;
}
