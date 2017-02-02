Simple Shell - JSH (~pronounced JSell)
============

## Features

* A simple shell with minimal built in functionality, including:
    * `ls` : list items in given directory
    * `cd` : change to a given directory
    * `fg` : foreground a background job
    * `exit` : exit the simple_shell
    * `jobs` : list jobs running in the background
* Ability to run any external commands
* Signal handling
    * Interrupt (Ctrl-C) : Interrupt only foreground processes
    * Suspend (Ctrl-Z) : Ignored by jsh
* Output Redirection : Send the output of any external commend to disk
* Piping : Redirect the output from one external command to the input of another


## Installation

```shell
git clone git@github.com:jasondias9/simple_shell.git
cd simple_shell
make
./simple_shell
```
