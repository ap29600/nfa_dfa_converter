#ifndef DFA_H_
#define DFA_H_

#include "automata.h"

void dump_nfa_to_dot(nfa *N, FILE *stream);
void dump_dfa_to_dot(dfa *D, FILE *stream);

#endif // DFA_H_
