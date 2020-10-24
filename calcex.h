// calcex.h

#ifndef _CALCEX_H_
#define _CALCEX_H_

#include "calc.h"

int calcext_init_extensions(calc_symboltable_t** ext);
void calcext_free_extensions(calc_symboltable_t* ext);

#endif
