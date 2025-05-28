# clox

clox is a single-pass bytecode compiler for Lox written in C. Lox is a full-featured, efficient scripting language from Robert Nystrom's book, [Crafting Interpreters](https://craftinginterpreters.com/). <br>

The documentation for the Lox programming language itself can be found [here](https://craftinginterpreters.com/the-lox-language.html).

## Requirements

* C

## Installation guide

* Download the latest version of the `main.out` file from the releases section here: https://github.com/paudsu01/clox/releases

## Running the compiler

* Open up your terminal.
```
cd path/to/directory/of/main.out
```
* To run the compiler, run:
```
./main.out
```
* To run a .lox file, run:
```
./main.out file.lox
```
> You can also download some sample `.lox` files from the [sampleFiles/](sampleFiles/) directory and run them.

### Running clox from anywhere in the terminal

* I would suggest adding an alias from the terminal to run the `main.out` file. 
  
  ```
  cd path/to/directory/of/main.out
  dir=$(pwd)
  alias clox="$(pwd)/main.out"
  ```

  > You can now just type `clox` to use the compiler for the remaining terminal session.

* If you want to add a permanent alias, you should add the alias command with an absolute path for `main.out` to your shell config file like the `~/.bashrc` file if you use bash, `~/.zshrc` file if you use zsh.

