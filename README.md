# NFA (regex) to DFA converter

this is a simple tool to  convert a regular expression into the graph
representation of a Deterministic Finite-state Automaton, or a Direct-Coded
scanner for that regex in `C`.

## Usage

### to build the project:

```sh
make bin/dfa
```

### to use the program:

create a text file (here `regex.txt`) in the following format:

```
foo foo
number 0|(1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*
```

where the first word of each line is an identifier, and the string starting after the
whitespace is the regular expression associated with it.


```sh
./bin/dfa regex.txt
```

will produce the file `regex.txt.c` with function definitions for `scan_foo`
and `scan_number` which take a string `s` and return the length of the longest
prefix of `s` that matches the given regex.

In order to produce the graphs the flags `-g` or `-a` can be used; the former
only generates the graph for the minimal DFA, while the latter produces 3
graphs per regex, respectively representing the NFA, naive DFA, and minimal
DFA.

Code generation can be skipped with the `-n` flag.

## Supported regex syntax:
- `foo|bar`  matches either "`foo`" or "`bar`".
- `bar*` matches "`ba`" followed by any number of "`r`"s.
- `()` parentheses explicitly encode associativity:
    - `a(b|c)*` matches "`a`" followed by any string of "`b`"s and/or "`c`"s.
    - `a(b|c*)` matches "`ab`" followed by either a single "`b`" or any number of "`c`"s.
- any other character is interpreted as a literal.

## Caveats:

- no escaping of parentheses.
- no character ranges 
- no complement operation.

... yet.
