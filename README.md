*This project has been created as part of the 42 curriculum by \<mhadir\>.*

---

# Codexion

## Description

Codexion is a concurrency simulation inspired by the classic **Dining Philosophers** problem. Multiple coders sit in a circle around a shared Quantum Compiler. Between each pair of adjacent coders lies a USB dongle. To compile their quantum code, a coder must simultaneously hold **two dongles** — the one on their left and the one on their right.

The goal is to coordinate access to these shared resources so that:
- No coder ever starves (burns out from lack of compiling)
- No deadlock ever occurs
- Dongle cooldown periods are respected
- Fair scheduling is enforced via FIFO or EDF policy

Each coder runs as an independent POSIX thread and cycles through three phases: **compile → debug → refactor**, then repeats. A separate monitor thread watches all coders and stops the simulation the moment any coder burns out, or when every coder has reached the required compile count.

---

## Instructions

### Compilation

```bash
make
```

This produces the `codexion` binary using `cc -Wall -Wextra -Werror -pthread`.

Other Makefile rules:

```bash
make clean
make fclean
make re
```

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are mandatory. All time values are in milliseconds.

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders (and dongles) in the simulation |
| `time_to_burnout` | Max ms a coder can go without starting a compile |
| `time_to_compile` | Time in ms a coder spends compiling |
| `time_to_debug` | Time in ms a coder spends debugging |
| `time_to_refactor` | Time in ms a coder spends refactoring |
| `number_of_compiles_required` | Simulation stops when all coders reach this count (0 = exit immediately) |
| `dongle_cooldown` | Ms a dongle is unavailable after being released |
| `scheduler` | `fifo` (first come first served) or `edf` (earliest deadline first) |

### Examples

```bash

./codexion 5 1000 200 200 200 3 100 fifo

./codexion 5 1000 200 200 200 3 100 edf

./codexion 2 800 200 200 200 3 0 fifo

./codexion 1 500 200 200 200 3 0 fifo
```

### Expected Output Format

Every state change is printed as:
```
timestamp_ms  coder_id  message
```

Example:
```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
402 2 has taken a dongle
403 2 has taken a dongle
403 2 is compiling
603 2 is debugging
803 2 is refactoring
1204 3 burned out
```

## Blocking Cases Handled

### Deadlock Prevention — Coffman's Conditions

Deadlock requires all four Coffman conditions to hold simultaneously:

1. **Mutual exclusion** — dongles can only be held by one coder at a time
2. **Hold and wait** — a coder holds one dongle while waiting for the other
3. **No preemption** — a dongle cannot be forcibly taken
4. **Circular wait** — each coder waits for a dongle held by its neighbor

This implementation breaks **Circular Wait** and **Hold and Wait** by requiring each coder to acquire **both dongles atomically**. A coder only transitions to COMPILING when `can_grab()` confirms both its left and right dongles are free simultaneously. No coder ever holds one dongle while waiting for the other.

### Starvation Prevention

Both schedulers prevent indefinite starvation:

- **FIFO**: the coder that has been WAITING the longest gets priority. A coder that started waiting before its neighbor will always be served first, ensuring bounded wait time.
- **EDF** (Earliest Deadline First): the coder whose burnout deadline is closest gets priority. This directly prevents the most at-risk coder from burning out while others keep compiling.

Priority is checked in `has_priority()` by comparing `waiting_since` (FIFO) or `last_compile_start` (EDF) between neighbors. A coder only grants access to itself when it beats both its left and right neighbor.

### Cooldown Handling

After a coder releases both dongles, each dongle is marked unavailable for `dongle_cooldown` milliseconds via `dongle_free_at[]`. The `can_grab()` function checks `now_ms() >= dongle_free_at[idx]` before granting access. Coders use `pthread_cond_timedwait` with a 2ms timeout so they automatically recheck cooldown expiry without busy-waiting forever.

### Precise Burnout Detection

A dedicated monitor thread polls all coders every 1ms. For each coder it computes:

```c
elapsed = now_ms(sim) - sim->coders[i].last_compile_start;
if (elapsed >= sim->burnout)  →  stop simulation
```

Polling every 1ms guarantees the burnout log appears within 10ms of the actual deadline as required by the subject.

### Log Serialization

All output goes through a single `print_log()` function protected by `print_lock` (a `pthread_mutex_t`). The mutex is locked before `printf` and unlocked immediately after, so no two messages can interleave on the same line regardless of how many threads are active simultaneously.

---

## Thread Synchronization Mechanisms

### Primitives Used

| Primitive | Name | Purpose |
|---|---|---|
| `pthread_mutex_t` | `lock` | Protects dongle state, coder state, and stop flag |
| `pthread_mutex_t` | `print_lock` | Serializes all log output |
| `pthread_cond_t` | `cond` | Signals waiting coders when a dongle is released |
| `pthread_cond_timedwait` | — | Wakes coders periodically to recheck cooldown |

### How Shared Resources Are Protected

**Dongle access** is controlled by the global `lock` mutex. When a coder wants to compile, it locks `lock`, marks itself as WAITING, then waits in a loop:

```c
pthread_mutex_lock(&sim->lock);
coder->state = WAITING;
coder->waiting_since = now_ms(sim);
while (!sim->stop)
{
    if (can_grab(sim, idx))
    {
        coder->state = COMPILING;
        sim->dongle_taken[left] = 1;
        sim->dongle_taken[right] = 1;
        pthread_mutex_unlock(&sim->lock);
        return (1);
    }
    pthread_cond_timedwait(&sim->cond, &sim->lock, &ts);
}
```

Both dongles are marked taken inside the same lock acquisition — this is the atomic grab that prevents any race condition between checking and taking.

**Dongle release** broadcasts to all waiting coders:

```c
pthread_mutex_lock(&sim->lock);
sim->dongle_taken[left] = 0;
sim->dongle_taken[right] = 0;
sim->dongle_free_at[left] = now + cooldown;
sim->dongle_free_at[right] = now + cooldown;
pthread_cond_broadcast(&sim->cond);
pthread_mutex_unlock(&sim->lock);
```

**Compile count and stop flag** are updated atomically in `inc_and_check()`:

```c
pthread_mutex_lock(&sim->lock);
coder->compiles++;
if (all_done) {
    sim->stop = 1;
    pthread_cond_broadcast(&sim->cond);
}
pthread_mutex_unlock(&sim->lock);
```

Incrementing and checking happen under the same lock so no coder can slip into a new compile cycle between the increment and the stop check.

**Thread-safe communication between coders and monitor** is achieved by the monitor reading `last_compile_start` and `compiles` which are only written by their respective coder thread. The monitor sets `stop = 1` under `lock` and broadcasts to wake all coders blocked in `pthread_cond_timedwait`. Because `grab_dongles()` checks `sim->stop` at the top of every iteration, all threads exit cleanly within one poll cycle.

### Race Condition Prevention Summary

| Risk | Prevention |
|---|---|
| Two coders grabbing the same dongle | Atomic check-and-take under `lock` |
| Coder starting new cycle after stop | `stop` checked at top of every loop iteration |
| Burnout missed or late | Monitor polls every 1ms, guaranteed within 10ms |
| Log lines interleaving | `print_lock` held for entire `printf` call |
| Compile count over/under counted | Increment and completion check share the same `lock` acquisition |

---

## Resources

### Documentation
- [POSIX Threads Programming — Lawrence Livermore National Laboratory](https://hpc-tutorials.llnl.gov/posix/)
- [The Open Group — pthread.h specification](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html)
- [Linux man pages — pthread_mutex_t, pthread_cond_t](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html)

### References on the Problem
- Dijkstra, E.W. — *Cooperating Sequential Processes* (1965) — original Dining Philosophers formulation
- [Wikipedia — Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Wikipedia — Earliest Deadline First Scheduling](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- Coffman, E.G. et al. — *System Deadlocks* (1971) — the four necessary conditions for deadlock

### AI Usage
Claude (Anthropic) was used during this project for the following tasks:

- **Understanding the subject** — explaining the Dining Philosophers analogy and how the project maps to it
- **Architecture discussion** — thinking through the struct layout, locking strategy, and atomic dongle acquisition approach before writing any code
- **Debugging** — identifying why coders continued running after reaching `number_of_compiles_required`, tracing the missing `pthread_cond_broadcast` after setting `stop_flag`
- **README writing** — structuring and drafting this document

All generated content was reviewed, tested, and understood before being used. No code was copy-pasted without full comprehension of what it does and why.