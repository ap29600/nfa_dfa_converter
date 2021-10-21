# NFA (regex) to DFA converter

this is a simple tool to  convert a regular expression into the graph
representation of a Deterministic Finite-state Automaton.

## Usage

### to just build the project:

```sh
make dfa
```

### to convert the regex `foo|bar` (also rebuilds the project if the executable is outdated):

```sh
make pics
Please insert the regular expression to translate:
>>> foo|bar
```

this will generate three `dot` files, as well as three `svg` files representing
the generated NFA (`initial.*`), naive DFA (`intermediate.*`) and minimal DFA
(`minimised.*`).

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
