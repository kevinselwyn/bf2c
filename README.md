#bf2c

Utility for converting brainfuck programs into C programs

##Description

The [brainfuck](http://en.wikipedia.org/wiki/Brainfuck) programming language commands are directly analogous to C commands and can be converted easily.

##Installation

```bash
make && sudo make install
```

##Usage

```bash
bf2c <program.bf> [<output.c>]
```

If no output file is supplied, the program will write to stdout.
