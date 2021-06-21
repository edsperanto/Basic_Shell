# Basic_Shell

A rudimentary shell written in C for CPSC 3500 Computing Systems class.

### How to compile

```
make clean
make
```

### How to run
```
./myshell
```

### Supported features
- executes commands
- supports piping

### Unsupported features
- auto-complete with tab
- quotation handling
- color and chimes
- shell scripting

### Limitations
- can pipe a maximum of 10 commands
- each command can have at most 15 tokens, excluding the command name
- each token must be less that 20 characters long
