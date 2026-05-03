*This project has been created as part of the 42 curriculum by mhadir.*
# Codexion
## Description
Codexion is a C concurrency simulation inspired by the Dining Philosophers problem.
Several coders sit in a circular co-working hub.
There is one USB dongle between each pair of adjacent coders.
A coder needs the left and right dongles at the same time to compile.
Each coder is represented by one POSIX thread.
The coder cycle is compile, debug, refactor, then repeat.
A separate monitor thread checks burnout and stops the simulation.
A coder burns out if they do not start compiling before `time_to_burnout` expires.
The timer starts from the beginning of the last compile or from the simulation start.
The simulation stops when one coder burns out.
It also stops when all coders reach `number_of_compiles_required`.
Main goals:
- avoid deadlock
- avoid starvation when possible
- respect dongle cooldown
- serialize logs
- implement FIFO and EDF scheduling
- use a custom heap priority queue
- clean allocated memory and synchronization objects
---
## Instructions
### Build
Compile the project with:
```bash
make
```
This creates:
```bash
./codexion
```
Other rules:
```bash
make clean
make fclean
make re
```
### Usage
```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```
All arguments are mandatory.
| Argument | Meaning |
|---|---|
| `number_of_coders` | number of coders and dongles |
| `time_to_burnout` | maximum time before burnout |
| `time_to_compile` | time spent compiling |
| `time_to_debug` | time spent debugging |
| `time_to_refactor` | time spent refactoring |
| `number_of_compiles_required` | stop count for all coders |
| `dongle_cooldown` | cooldown after release |
| `scheduler` | must be `fifo` or `edf` |
---
## Output Format
Each state change is printed as:
```text
timestamp_in_ms coder_id message
```
Possible messages:
```text
X has taken a dongle
X is compiling
X is debugging
X is refactoring
X burned out
```
Example:
```text
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
400 1 is refactoring
402 2 has taken a dongle
402 2 has taken a dongle
402 2 is compiling
602 2 is debugging
802 2 is refactoring
1000 3 burned out
```
A compile action always needs two dongle messages before `is compiling`.
---
## Blocking Cases Handled
### Deadlock Prevention
Deadlock can happen when coders wait for each other in a circle.
The program avoids holding one dongle while waiting forever for the other.
A coder starts compiling only after both needed dongles are available.
`find_first()` orders the two dongle indexes before locking dongle mutexes.
This gives a deterministic lock order for the two dongles.
### Hold and Wait
The check-and-take step is done as one protected operation.
If one dongle is busy or cooling down, the coder waits again.
The coder does not enter the compiling state with only one dongle.
### Dongle Cooldown
After compiling, both dongles are released.
Each released dongle gets a new availability timestamp:
```text
dongle_free_at = current_time + dongle_cooldown
```
The code uses:
```text
dongle_taken[]
dongle_free_at[]
```
A coder can take a dongle only when it is free and cooldown is finished.
### FIFO Scheduling
With `fifo`, the oldest waiting request has priority.
The field `waiting_since` stores when the coder entered waiting state.
The heap compares waiting times to select the next coder.
### EDF Scheduling
With `edf`, the closest burnout deadline has priority.
The deadline is:
```text
deadline = last_compile + time_to_burnout
```
If deadlines are equal, `waiting_since` is used.
If still equal, the smaller coder id wins.
### Starvation Prevention
The custom heap avoids random selection between waiting coders.
FIFO favors the coder that waited the longest.
EDF favors the coder closest to burnout.
### Burnout Detection
The monitor thread checks all coders repeatedly.
For each coder, it computes:
```text
time_passed = now_time - last_compile
```
If the time passed reaches `time_to_burnout`, the monitor stops the simulation.
The burnout log is printed within the required precision.
### Log Serialization
All output is protected by `print_lock`.
This prevents two threads from mixing messages on the same line.
### Clean Stop
The shared flag `sim->stop` tells every thread to exit.
When stopping, the monitor broadcasts `sim->cond`.
This wakes coders waiting in `pthread_cond_timedwait()`.
The main thread joins coder threads and the monitor thread.
---
## Thread Synchronization Mechanisms
| Primitive | Purpose |
|---|---|
| `sim->lock` | protects shared simulation state |
| `sim->print_lock` | protects output |
| `sim->dongle_lock[i]` | protects dongle `i` |
| `sim->cond` | wakes waiting coders |
Important coder functions:
```text
take_dongles()
lock_dongles()
unlock_dongles()
coder_cycle()
coder_thread()
```
Important monitor function:
```text
monitor_thread()
```
Important heap functions:
```text
heap_add()
heap_top()
heap_rm_top()
is_higher()
```
For FIFO, `is_higher()` compares `waiting_since`.
For EDF, it compares `deadline`, then `waiting_since`, then coder id.
---
## Resources
### Documentation
- POSIX Threads Programming: https://hpc-tutorials.llnl.gov/posix/
- POSIX pthread specification: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html
- Linux man pages: https://man7.org/linux/man-pages/
### References
- Dining Philosophers Problem: https://en.wikipedia.org/wiki/Dining_philosophers_problem
- Deadlock and Coffman Conditions: https://en.wikipedia.org/wiki/Deadlock
### AI Usage
AI tools were used to understand the subject and discuss synchronization.
AI was also used to review possible race conditions.
AI helped improve this README structure and wording.
The final code was written and is understood by the project author.
