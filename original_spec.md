This assignment requires you to make a basic UNIX shell with the following features:

It should accept the following commands, with proper functionality:
- ls
  - 'ls ~' should show you the home directory
- cat
- grep
- cd
- mkdir
- rmdir
- exit

Additionally, your shell should handle:
- piping
- show the current working directory, at every user prompt
- take any number of spaces between commands/arguments/semi-colons
- should not take in repeating semi-colons
- reject invalid arguments, use your built-in UNIX shell as reference

You are provided with a test program, your shell should pass all test cases.

Submissions that do not meet any of the above requirement will be given an automatic zero grade.

Enjoy!

________________________________________________

Charlie's take (from my understanding):
Describe the assignment in levels, so that students can choose to go above and beyond.

TODO: Describe the below in more detail

lvl 1: builtins (cd, exit, exec)
lvl 2: path finding and execution
lvl 3: parsing for sprecial chars
lvl 4: piping, forking
lvl 5: job processing
lvl 6: etc..

No tests for this proposal, must reach some minimal level (e.g. lvl 4). Clout is due if all levels are completed.

The problem with this proposal is that we might need to spend more time than necessary explaining the concepts behind building the shell. Maybe that's ok, but I thought that we wanted to spend as little time on this as possible.
