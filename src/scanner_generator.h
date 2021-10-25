#include "automata.h"

void scanner_from_dfa(dfa * D, const char *scanner_name, FILE *stream);
void scanner_from_regex(const char *regex, const char *scanner_name, FILE *stream);
