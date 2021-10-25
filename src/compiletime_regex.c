#include "scanner_generator.h"
#include "automata.h"
#include "thompson.h"
#include "dfa.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct {
  unsigned nfa_graph     : 1;
  unsigned dfa_graph     : 1;
  unsigned minimal_graph : 1;
  unsigned generate_code : 1;
} options;

void usage(FILE *stream) {
  fprintf(
      stream,
      "Regex Compiler\n"
      "\n"
      "  USAGE: regex_compiler [OPTIONS] [INPUT FILES...]\n"
      "    takes a list of files containing entries in the form:\n"
      "\n"
      "      integer 0|(1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*\n"
      "      keyword new|const|static|do|if|else\n"
      "\n"
      "    and produces the functions:\n"
      "\n"
      "      `unsigned long parse_integer(const char *s)`\n"
      "      `unsigned long parse_keyword(const char *s)`\n"
      "\n"
      "    which accept a string as argument and return the length of\n"
      "    its longest prefix which matches the corresponding regex.\n"
      "    These function definitions are placed in a file with the same\n"
      "    name as the input file, adding the `.c` extension.\n"
      "\n"
      "  OPTIONS:\n"
      "    -n --no-code    Do not generate the code for parsing the regex.\n"
      "                    The regex is still conveted to DFA and errors are\n"
      "                    thrown if it is syntactically incorrect.\n"
      "\n"
      "    -g --graph      Generate a dot graph of the resulting DFA.\n"
      "                    the graph is written to the file:\n"
      "                    <input_filename>_<regex_name>.dot\n"
      "\n"
      "    -a --all-graphs Like -g, but also generates graphs for the NFA\n"
      "                    resulting from Thompson's construction and the \n"
      "                    naive DFA generated directly from that. these will\n"
      "                    have the extensions: '.nfa.dot' and '.naive.dot'\n");
}

int main(int argc, const char **argv) {
  if (argc < 2) {
    fprintf(stderr, "ERROR: No files provided.\n");
    usage(stderr);
    return 1;
  }

  options.nfa_graph = 0;
  options.dfa_graph = 0;
  options.minimal_graph = 0;
  options.generate_code = 1;

  const char *files[argc - 1];
  int file_count = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-')
      files[file_count++] = argv[i];
    else if (argv[i][1] != '-') { // parse as a set of flags
      for (const char *c = argv[i] + 1; *c; c++) {
        switch (*c) {
        case 'g':
          options.minimal_graph = 1;
          break;
        case 'a':
          options.nfa_graph = 1;
          options.dfa_graph = 1;
          options.minimal_graph = 1;
          break;
        case 'n':
          options.generate_code = 0;
          break;
        }
      }
    } else { // parse as a single flag
      if (strcmp(argv[i], "--all-graphs")) {
        options.nfa_graph = 1;
        options.dfa_graph = 1;
        options.minimal_graph = 1;
      } else if (strcmp(argv[i], "--graph")) {
        options.minimal_graph = 1;
      } else if (strcmp(argv[i], "--no-code")) {
        options.generate_code = 0;
      }
    }
  }

  char line[4096];
  char regex[4096];
  char name[4096];

  for (int i = 0; i < file_count; i++) {
    char fname[1024];
    snprintf(fname, 1024, "%s.c", files[i]);

    FILE *in = fopen(files[i], "r");
    FILE *out;
    if (options.generate_code)
      out = fopen(fname, "w");

    while (fgets(line, 4096, in) != NULL) {
      unsigned id_start = 0;
      for (; line[id_start] && isspace(line[id_start]); id_start++)
        ;
      unsigned id_end = id_start;
      for (; line[id_end] && isalnum(line[id_end]); id_end++)
        ;
      unsigned re_start = id_end;
      for (; line[id_end] && isspace(line[id_end]); id_end++)
        ;

      if (id_end - id_start == 0)
        fprintf(stderr, "ERROR: missing identifier in file \"%s\"", argv[i]);

      if (line[re_start] == '\0')
        fprintf(stderr, "ERROR: missing regex in file \"%s\"", argv[i]);
      unsigned l = strlen(line);

      memcpy(name, line + id_start, id_end - id_start);
      name[id_end - id_start - 1] = '\0';

      memcpy(regex, line + re_start + 1, l - re_start - 1);
      regex[l - re_start - 2] = '\0';


      nfa initial_nfa = regex_to_nfa(regex, l - re_start - 2);
      dfa *naive_dfa = to_dfa(&initial_nfa);
      dfa *minimal_dfa = minimize(naive_dfa);

      if (options.generate_code) {
          scanner_from_dfa(minimal_dfa, name, out);
      }

      if (options.nfa_graph) {
          char dot_name[1024];
          snprintf(dot_name, 1024, "%s_%s.nfa.dot", files[i], name);
          FILE * f = fopen(dot_name, "w");
          dump_nfa_to_dot(&initial_nfa, f);
          fclose(f);

      }
      if (options.dfa_graph) {
          char dot_name[1024];
          snprintf(dot_name, 1024, "%s_%s.naive.dot", files[i], name);
          FILE * f = fopen(dot_name, "w");
          dump_dfa_to_dot(naive_dfa, f);
          fclose(f);

      }
      if (options.minimal_graph) {
          char dot_name[1024];
          snprintf(dot_name, 1024, "%s_%s.dot", files[i], name);
          FILE * f = fopen(dot_name, "w");
          dump_dfa_to_dot(minimal_dfa, f);
          fclose(f);

      }

      delete_nfa(&initial_nfa);
      delete_dfa(naive_dfa);
      free(naive_dfa);
      delete_dfa(minimal_dfa);
      free(minimal_dfa);

    }

    fclose(in);
    if (options.generate_code)
      fclose(out);
  }
}
