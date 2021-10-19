#ifndef THOMPSON_H_
#define THOMPSON_H_
#include "automata.h"

struct nfa regex_to_nfa(const char *regex, size_t regex_len);

#endif // THOMPSON_H_
