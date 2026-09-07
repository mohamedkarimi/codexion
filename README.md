*This project has been created as part of the 42 curriculum by mokarimi.*

## Description

Codexion is a POSIX-thread simulation of coders sharing dongles in a circle.
Each coder must acquire its required dongles to compile, then debug and
refactor. The simulation stops when every coder has completed the requested
number of compiles or when one coder burns out.

The `fifo` scheduler grants compile requests in arrival order. The `edf`
scheduler grants the request with the earliest burnout deadline, using arrival
order to break equal deadlines. Both policies use the custom binary heap in
`src/scheduler.c`.

## Instructions

Compile the project:

```sh
make
```

Run it:

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Arguments are non-negative integers, except `number_of_coders`, which must be
positive. `scheduler` must be either `fifo` or `edf`.

Example:

```sh
./codexion 3 2000 100 100 100 3 50 edf
```

## Resources

- POSIX threads: `man pthread_create`, `man pthread_mutex_lock`, and `man pthread_cond_wait`.
- Mutexes and condition variables: *The Linux Programming Interface*, Michael Kerrisk.
- Deadlocks and Coffman's conditions: *Operating Systems: Three Easy Pieces*.
- Scheduling: *Operating System Concepts*, Silberschatz, Galvin, and Gagne.
- Binary heaps and priority queues: *Introduction to Algorithms*, Cormen et al.

AI was used as a pair-programming aid to review the synchronization design,
help implement the manually managed heap, and propose tests. The final source
code was inspected, compiled, and tested in this repository.

## Blocking Cases Handled

- Deadlock is prevented by always acquiring the lower-ID dongle before the higher-ID dongle.
- Holding and waiting cannot form a circular wait because the acquisition order is global.
- FIFO uses an arrival counter, so earlier queued requests are served first.
- EDF compares burnout deadlines and then arrival order, making equal deadlines deterministic.
- The active heap request owns scheduler priority until it has acquired its dongles, preventing scheduler bypass.
- A released dongle remains unavailable until its configured cooldown deadline.
- The monitor checks burnout every millisecond and stops the simulation when a deadline is reached.
- `log_mutex` serializes output so individual log lines never interleave.
- Shared coder state is protected by `state_mutex`; each dongle state is protected by its own mutex.

## Thread Synchronization Mechanisms

`pthread_mutex_t` protects shared state. `state_mutex` protects simulation
termination, compile counts, and compile start times. Every dongle has its own
mutex and condition variable; its mutex remains owned while a coder compiles,
and waiting coders sleep until release or cooldown expiry.

`scheduler_mutex` protects the request heap and `scheduler_cond` wakes queued
coder threads whenever a request is granted or the simulation ends. The monitor
uses the state mutex to decide termination, then broadcasts the scheduler and
dongle condition variables so no waiting coder remains blocked. `log_mutex`
protects the required timestamped output.
