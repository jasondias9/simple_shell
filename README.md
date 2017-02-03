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

## Limitations
* Interrupt fails ob background process that have been foregrounded. This limitation
  exists since I chose to ignore SIGINT signals on background processes as signal 
  was being transmitted to the foreground as well as process in the same group 
  (background processes)

*Note: In order to fix this we could have ensured that background processes 
run in a seperate process group*. 


## Installation

```shell
git clone git@github.com:jasondias9/simple_shell.git
cd simple_shell
make
./simple_shell
```
