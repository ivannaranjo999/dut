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

## How it works

`dut` splits the work across every processor core on the machine, so several files and folders get measured at the same time instead of one after another.

The tricky part with disk usage is that work isn't evenly sized. One folder might hold three small text files, another might hold a hundred thousand. If the workload was split up front and handed out evenly, one thread could end up stuck measuring a huge folder alone while every other thread finishes early and sits idle.

To avoid that, `dut` keeps a shared queue of everything left to measure, instead of handing out a fixed list to each thread. Every core pulls the next item off that queue whenever it's free. If a thread opens a folder and finds more files or subfolders inside, it drops those back onto the same queue rather than working through them by itself. That way, if one folder turns out to be enormous, the other cores jump in and help chip away at it instead of standing around waiting. The whole thing keeps running until the queue is empty and every core has finished what it picked up.

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

It also works with piped input, if you'd rather feed it a list of paths than type them out:

```
echo "myfolder report.pdf notes.txt" | ./dut
```

## A note on the numbers

Sizes reflect actual space used on disk, not just the "logical" file size, which is the more accurate number when you're trying to figure out what's really eating into your storage.

## About this project

This tool was built as a hands-on exercise in systems-level programming: working directly with the file system, managing multiple threads safely, and thinking carefully about performance and edge cases along the way. It's a compact example of turning a simple, everyday problem, "what's taking up my disk space?", into a well-considered piece of software.
