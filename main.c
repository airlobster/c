// main.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "userinput.h"
#include "calc.h"
#include "calcex.h"
#include "_version.h"

static int _enumvars_cb(const char* name, number_t value, void* context) {
	printf("\t%-15s\t%Lg\n", name, value);
	return 0;
}

static int _process_user_input(const char* line, void* context) {
	int err = 0;
	number_t v;
	calc_symboltable_t* ext = (calc_symboltable_t*)context;

	if( strcmp(line, "/l")==0 ) {
		printf("Variables:\n");
		ext->enumvars(ext, _enumvars_cb, 0);
	} else if( strcmp(line, "/c")==0 ) {
		ext->resetvars(ext);
	} else if( strcmp(line, "/q")==0 ) {
		err = -1;
	} else if( strcmp(line, "/v")==0 ) {
		printf("Version: %s\n", appversion());
	} else if( (err=calc_evaluate(line, ext, ext, &v)) == CS_OK ) {
		printf("%.20LG\n", v);
	} else {
		cfprintf(stderr, "\e[91m%s (%d)\e[0m\n", calc_get_error(err), err);
	}

	return err;
}

int main(int argc, char** argv) {
	calc_symboltable_t* ext;
	int err=0;

	calcext_init_extensions(&ext);

	ui_session_t ui = {
		.apppath=argv[0],
		.prompt=getenv("CALC_PROMPT"),
		.flags=ufUseHistory,
		.handler=_process_user_input,
		.context=ext,
	};

	err = ui_loop(&ui);

	calcext_free_extensions(ext);
	return err;
}

