*This project has been created as part of the 42 curriculum by <hassamoh>.*

# Codexion

## Description

Codexion is a concurrency project developed as part of the 42 curriculum.

The goal of the project is to simulate multiple coders competing for a limited number of shared resources called **dongles**. Each coder runs in its own thread and must acquire two dongles simultaneously before starting a compilation.

The simulation focuses on thread synchronization, resource scheduling, race-condition prevention, deadlock prevention, starvation prevention, and precise deadline monitoring.

Each coder repeatedly follows this lifecycle:

1. Acquire the required dongles.
2. Start compiling.
3. Release the dongles.
4. Debug.
5. Refactor.
6. Try to acquire the dongles again.

A coder **burns out** if it does not start a new compilation before its burnout deadline.

The simulation terminates when either:

* every coder has completed the required number of compilations; or
* at least one coder burns out.

Two scheduling policies are supported:

* **FIFO** — requests are served according to their arrival order.
* **EDF (Earliest Deadline First)** — requests are prioritized according to the earliest burnout deadline.

The project also handles the special case where there is only one coder and therefore only one dongle.

---

## Features

* One POSIX thread per coder.
* Dedicated monitor thread for simulation termination.
* FIFO resource scheduling.
* EDF resource scheduling.
* Priority queue implemented as a binary heap.
* Deadlock prevention through ordered dongle acquisition.
* Starvation prevention through deadline-aware scheduling.
* Mandatory dongle cooldown after release.
* Thread-safe logging.
* Thread-safe access to shared coder state.
* Precise burnout monitoring.
* Condition variables for efficient waiting and notification.
* Graceful simulation shutdown.
* Proper dynamic memory cleanup.

---

## Instructions

### Compilation

The project is compiled using the provided `Makefile`.

```bash
make
```

The compiler flags used by the project are:

```text
-Wall -Wextra -Werror -pthread
```

### Cleaning

Remove object files:

```bash
make clean
```

Remove object files and the executable:

```bash
make fclean
```

Recompile the project from scratch:

```bash
make re
```

### Execution

The program expects exactly 8 arguments:

```text
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

The arguments are:

| Argument                      | Description                                                     |
| ----------------------------- | --------------------------------------------------------------- |
| `number_of_coders`            | Number of coder threads and number of dongles                   |
| `time_to_burnout`             | Maximum time in milliseconds allowed between compilation starts |
| `time_to_compile`             | Compilation duration in milliseconds                            |
| `time_to_debug`               | Debugging duration in milliseconds                              |
| `time_to_refactor`            | Refactoring duration in milliseconds                            |
| `number_of_compiles_required` | Number of successful compilations required per coder            |
| `dongle_cooldown`             | Mandatory cooldown period after releasing a dongle              |
| `scheduler`                   | Scheduling policy: `fifo` or `edf`                              |

### Example

```bash
./codexion 3 3000 200 200 200 1 200 fifo
```

Using EDF:

```bash
./codexion 3 3000 200 200 200 1 200 edf
```

### Output

The program reports coder actions using the required format:

```text
0 2 has taken a dongle
0 2 has taken a dongle
0 2 is compiling
201 2 is debugging
401 2 is refactoring
```

The first value is the elapsed time in milliseconds, followed by the coder identifier and the action.

A burnout is reported as:

```text
1234 3 burned out
```

---

## Blocking cases handled

Concurrency introduces several possible blocking and synchronization problems. Codexion explicitly addresses the main cases required by the project.

### Deadlock prevention

A coder needs two dongles to compile. If every coder acquires one dongle and waits indefinitely for the second one, a circular wait can occur.

To prevent this, each coder acquires its two dongles in a deterministic order based on the dongle identifiers.

For example, if a coder needs `D3` and `D4`, it always requests:

```text
D3 -> D4
```

and never:

```text
D4 -> D3
```

This removes the circular-wait condition from the Coffman deadlock conditions.

The solution therefore prevents the classic situation where:

```text
Coder 1 holds D1 and waits for D2
Coder 2 holds D2 and waits for D1
```

### Coffman's conditions

A deadlock requires four conditions:

1. Mutual exclusion.
2. Hold and wait.
3. No preemption.
4. Circular wait.

Dongles naturally provide mutual exclusion because only one coder can use a dongle at a time.

The implementation specifically breaks the **circular wait** condition by enforcing a global acquisition order for dongles.

### Starvation prevention

A coder waiting for a dongle must not be indefinitely ignored by other coders.

The implementation uses a custom binary heap as a priority queue for each dongle.

With FIFO scheduling, requests are ordered by their arrival sequence.

With EDF scheduling, requests are ordered by their burnout deadline:

```text
deadline = last_compile_start + time_to_burnout
```

This allows the scheduler to prioritize coders whose deadlines are closer.

### Dongle cooldown

A dongle cannot immediately be reused after being released.

When a coder releases a dongle, the implementation stores the earliest time at which that dongle becomes available:

```text
available_at = current_time + dongle_cooldown
```

A waiting coder can therefore be selected by the scheduler but still has to wait until the cooldown expires.

A condition-variable timed wait is used while waiting for this availability time.

### Precise burnout detection

A dedicated monitor thread continuously checks every coder's latest compilation start time.

For each coder, it evaluates the elapsed time since the last compilation started.

If the burnout limit is exceeded, the monitor:

1. Logs the burnout.
2. Sets the shared simulation-stop flag.
3. Wakes threads waiting on dongles.
4. Terminates the monitor thread.

The monitor checks the simulation state at a short interval so burnout detection remains precise.

### Graceful shutdown of blocked threads

A coder can be blocked inside a condition-variable wait while waiting for a dongle.

When the monitor stops the simulation, it broadcasts on every dongle condition variable.

This wakes blocked coders so they can re-check:

```text
simulation_stop
```

and exit instead of remaining blocked indefinitely.

This is particularly important because the main thread joins every coder before cleaning up the simulation resources.

### Log serialization

Multiple coder threads can attempt to print at the same time.

All logging therefore uses a dedicated `pthread_mutex_t`.

The logging sequence is effectively:

```text
lock log mutex
print message
unlock log mutex
```

This prevents messages from different threads from being interleaved.

---

## Thread synchronization mechanisms

Codexion uses POSIX threads and synchronization primitives to coordinate the coder threads, monitor thread, dongles, and shared simulation state.

### `pthread_mutex_t`

Mutexes protect shared state from simultaneous access.

The implementation uses mutexes for several purposes.

#### Dongle state

Every dongle has its own mutex protecting information such as:

* whether the dongle is currently in use;
* its cooldown availability time;
* its scheduling queue;
* its request sequence number.

A coder must hold the dongle mutex while modifying or checking this state.

This prevents two threads from simultaneously deciding that the same dongle is available.

#### Simulation state

A dedicated state mutex protects shared coder information such as:

* `last_compile_start`;
* `compiles_done`.

For example, the coder updates its compilation start time while holding the state mutex, while the monitor reads the value under the same mutex.

This prevents data races between the coder and monitor threads.

#### Stop state

The simulation-stop flag is protected by a separate mutex.

The monitor writes:

```text
simulation_stop = 1
```

while holding the stop mutex.

Coder threads read the same flag while holding that mutex.

This provides synchronized communication between the monitor and coder threads.

#### Logging

A dedicated logging mutex guarantees that only one thread writes a log message at a time.

---

### `pthread_cond_t`

Condition variables allow coder threads to wait efficiently for a dongle instead of continuously polling.

Each dongle has its own condition variable.

A waiting coder:

1. Locks the dongle mutex.
2. Enters the dongle's scheduling queue.
3. Checks whether it is allowed to acquire the dongle.
4. If it cannot proceed, it waits on the condition variable.

`pthread_cond_wait()` temporarily releases the dongle mutex while the thread sleeps and automatically reacquires it when the thread wakes up.

The condition is then checked again.

This is implemented using a loop because condition variables may wake spuriously.

### Timed condition waits

When a coder is the next scheduled requester but the dongle is still cooling down, the coder uses a timed condition wait.

This allows the thread to sleep until the dongle's `available_at` time instead of repeatedly checking the clock.

The thread then re-evaluates the scheduling and availability conditions.

### Condition broadcasts

When a dongle is released, its condition variable is broadcast so waiting coders can re-check the queue.

When the monitor stops the simulation, all dongle condition variables are broadcast.

This ensures that coders blocked inside condition waits can wake up, observe the stop condition, and terminate cleanly.

---

## Scheduling

Codexion supports two scheduling policies.

### FIFO

FIFO means **First In, First Out**.

Each request receives a monotonically increasing sequence number:

```text
seq = 0
seq = 1
seq = 2
...
```

The smallest sequence number has priority.

For example:

```text
Request A -> seq 0
Request B -> seq 1
Request C -> seq 2
```

The scheduler serves:

```text
A -> B -> C
```

FIFO controls the order in which coders waiting for a particular dongle are served. It does not control which operating-system thread gets CPU time first.

### EDF

EDF means **Earliest Deadline First**.

Each request is assigned a deadline:

```text
deadline = last_compile_start + time_to_burnout
```

The request with the earliest deadline receives priority.

For example:

```text
Coder A -> deadline 5200
Coder B -> deadline 4800
Coder C -> deadline 6100
```

The scheduler prioritizes:

```text
Coder B -> Coder A -> Coder C
```

The implementation uses the same binary heap structure for both policies. The difference is the value used as the heap priority key.

---

## Priority queue

Each dongle contains its own scheduling queue.

The queue is implemented as a binary min-heap.

A waiter contains:

```text
key
seq
```

For FIFO:

```text
key = seq
```

For EDF:

```text
key = deadline
```

The heap maintains the highest-priority waiter at its root.

This gives efficient insertion and removal while keeping the scheduling logic independent for every dongle.

---

## Resource management

Each coder is associated with two dongle pointers.

For normal simulations:

```text
coder->left
coder->right
```

refer to two different dongles.

The coder acquires both before compiling and releases both immediately after compilation.

For the single-coder case, there is only one dongle. Both pointers therefore refer to the same dongle object.

The implementation must acquire and release that single dongle only once in this case.

All dynamically allocated arrays are released during cleanup, and synchronization primitives are destroyed before the allocated environment is freed.

---

## Program architecture

The project is divided into several responsibilities:

```text
main
 ├── argument validation
 ├── environment initialization
 ├── coder threads
 │    └── coder routine
 │         ├── acquire dongles
 │         ├── compile
 │         ├── release dongles
 │         ├── debug
 │         └── refactor
 │
 └── monitor thread
      ├── detect burnout
      ├── check compilation completion
      └── stop simulation
```

The main thread is responsible for creating and joining the threads.

The coder threads perform the actual simulation.

The monitor thread determines when the simulation must stop.

---

## Resources

### Official documentation

* POSIX Threads documentation:

  * `pthread_create`
  * `pthread_join`
  * `pthread_mutex_*`
  * `pthread_cond_*`

* Linux/POSIX manual pages:

  * `pthread_mutex_lock`
  * `pthread_cond_wait`
  * `pthread_cond_timedwait`
  * `gettimeofday`
  * `usleep`

### Classic concurrency references

* *The Little Book of Semaphores* — Allen B. Downey
* *Operating System Concepts* — Abraham Silberschatz, Peter B. Galvin, Greg Gagne
* *The Art of Multiprocessor Programming* — Maurice Herlihy and Nir Shavit

These references were useful for understanding:

* mutual exclusion;
* condition variables;
* race conditions;
* deadlocks;
* starvation;
* resource allocation;
* thread synchronization.

### 42 resources

The 42 project subject and the official 42 documentation were used as the primary specification for the required behavior, argument format, output format, and project constraints.

### AI usage

AI was used as a learning and development assistant throughout the project.

It was used for:

* understanding POSIX thread concepts;
* explaining mutexes and condition variables;
* reasoning about race conditions;
* analyzing deadlock and Coffman conditions;
* understanding FIFO and EDF scheduling;
* designing and checking the priority-queue logic;
* tracing concurrent execution step by step;
* reviewing synchronization logic;
* identifying potential edge cases such as the single-coder/single-dongle case;


---

## Technical choices

### One thread per coder

Each coder is represented by a dedicated POSIX thread.

This allows multiple coders to compete for dongles concurrently while sharing the same simulation environment.

### Dedicated monitor thread

A separate monitor thread is responsible for detecting:

* coder burnout;
* completion of the required number of compilations.

This keeps termination detection independent from the coder execution loops.

### Per-dongle synchronization

Each dongle owns its own mutex, condition variable, and scheduling queue.

This avoids using one global lock for every resource and keeps synchronization localized to the resource being accessed.

### Binary heap scheduler

A binary heap was chosen because it provides an efficient way to maintain the highest-priority waiting request.

The same data structure supports both FIFO and EDF by changing the priority key.

---

## Error handling and cleanup

The program validates its command-line arguments before starting the simulation.

Memory allocated for the coder and dongle arrays is released during cleanup.

Before freeing the environment, the program destroys the mutexes and condition variables associated with the simulation.

The main thread waits for the monitor and coder threads to finish before cleanup, preventing resources from being destroyed while worker threads are still using them.

---

## Conclusion

Codexion is a practical concurrency exercise focused on controlled access to shared resources.

The project combines:

* POSIX threads;
* mutexes;
* condition variables;
* priority queues;
* FIFO and EDF scheduling;
* deadline monitoring;
* deadlock prevention;
* starvation prevention;
* resource cooldowns;
* synchronized logging.

The main objective is not simply to make multiple threads run simultaneously, but to make them **share resources safely, predictably, and without violating their deadlines**.
