# Work
A small command-line tool written in C that allows Neovim users to predefine tasks they are working on as well as the files necessary to complete those tasks.

## Prerequisites 
- Make
- GCC 
- Git

## Installation
```sh
$ git clone https://github.com/jacob-ramsey-2/Work.git
$ cd work
$ make
```

## Usage
```sh
$ work [OPTIONS] [TASK_NAME] 
	    -h		Print help message
```

## Workfile Format
A **Workfile** defines one or more tasks. Each task has a:
- A task name with no white space before it and ends with a colon (`TaskName:`)
- One or more file paths starting with a tab
- Zero or more Neovim flags starting with a tab and a following `-`

See an example below:
```sh
$ cat Workfile
Task1:
	File1.c
	File2.c

Task2:
	-O
	File3.py
	File4.py
```

With the above **Workfile**, the command: ```work Task1``` would open File1.c and File2.c as two tabs in a single Neovim window. If you don't specify a task, it will default to the first in your **Workfile**. You can also specify Neovim flags inside of a task and the flags will be applied to the Neovim open command that ```work``` runs.
Your **Workfile** must be in the root directory of your project, with all file paths relative to the root.
