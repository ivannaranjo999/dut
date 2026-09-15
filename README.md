# dut
 
Short for "du, threaded." A small, fast command-line tool that tells you how much space your files and folders are taking up, written from scratch in C.
 
## What it does
 
Point `dut` at one or more files or directories, and it reports the size of each one in a clean, human-readable format:
 
```
$ ./dut *
36KB       ../dut
772KB      ../eventReader
15.57MB    ../shellcheck-v0.11.0
3.68MB     ../spare
```
 
No digging through nested folders by hand, no squinting at raw byte counts. Just a quick, readable summary of what's using your disk space.
 
## Why it's fast
 
Most disk usage tools check one file at a time, in order, from top to bottom. `dut` does something a bit smarter: it looks at how many processor cores your machine has, and splits the list of files and folders across all of them, so multiple items get measured in parallel instead of one after another. On a modern laptop or server with several cores, that can mean measuring a whole batch of large directories in a fraction of the time a single-threaded tool would take.
 
For folders, it also digs in on its own, walking through every file and subfolder inside, however deeply nested, and adding it all up into one total.
 
## How to use it
 
Build it once:
 
```
gcc dut.c -o dut -lpthread
```
 
Then run it against any files or folders:
 
```
./dut myfolder report.pdf notes.txt
```
 
Or measure everything in the current directory:
 
```
./dut *
```
 
## A note on the numbers
 
Sizes reflect actual space used on disk, not just the "logical" file size, which is the more accurate number when you're trying to figure out what's really eating into your storage.
 
## About this project
 
This tool was built as a hands-on exercise in systems-level programming: working directly with the file system, managing multiple threads safely, and thinking carefully about performance and edge cases along the way. It's a compact example of turning a simple, everyday problem, "what's taking up my disk space?", into a well-considered piece of software.

