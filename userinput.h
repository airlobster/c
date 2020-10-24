// userinput.h

#ifndef _USERINPUT_H_
#define _USERINPUT_H_

typedef enum _ui_flags_t {
	ufAbortOnError=0x1,
	ufUseHistory=0x2,
} ui_flags_t;

typedef struct _ui_session_t {
	const char* apppath;
	const char* prompt;
	unsigned int flags;
	int(*handler)(const char* line, void* context);
	void* context;
} ui_session_t;

int ui_loop(const ui_session_t* opt);

#endif
