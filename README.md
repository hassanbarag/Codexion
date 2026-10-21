# Codexion

**A C Concurrency Simulation with FIFO and EDF Scheduling**

## Description

Codexion is a systems programming project written in C that simulates multiple coders competing for shared USB dongles. Each coder runs in a POSIX thread and must acquire two dongles before starting a compilation.

The simulation combines concurrent execution with resource scheduling and deadline management. If a coder exceeds the permitted time between compilation starts, the simulation stops.

## ✨ Key Features

- POSIX threads for concurrent coder execution
- Mutexes and condition variables for thread synchronization
- FIFO and EDF resource scheduling
- Binary min-heap priority queues
- Deadlock prevention through ordered resource acquisition
- Dongle cooldown management
- Deadline monitoring and synchronized logging
- Coordinated thread termination and resource cleanup

## 🚀 Quick Start

**Build the project**

```bash
make
```

**Run the simulation**

```bash
./codexion <coders> <t_burnout> <t_compile> <t_debug> <t_refactor> <n_compiles> <cooldown> <scheduler>
```

**Example — FIFO**

```bash
./codexion 5 800 200 200 200 7 100 fifo
```

**Example — EDF**

```bash
./codexion 5 800 200 200 200 7 100 edf
```

## ⚙️ Arguments

| Parameter | Description |
|---|---|
| `coders` | Number of coders and dongles |
| `t_burnout` | Maximum time between compilation starts (ms) |
| `t_compile` | Compilation duration (ms) |
| `t_debug` | Debugging duration (ms) |
| `t_refactor` | Refactoring duration (ms) |
| `n_compiles` | Required compilations per coder |
| `cooldown` | Dongle cooldown duration (ms) |
| `scheduler` | `fifo` or `edf` |

## 🔄 Simulation Flow

```text
        ┌──────────────────┐
        │  Request Dongles │
        └────────┬─────────┘
                 ▼
        ┌──────────────────┐
        │ Acquire Both     │
        └────────┬─────────┘
                 ▼
        ┌──────────────────┐
        │     Compile      │
        └────────┬─────────┘
                 ▼
        ┌──────────────────┐
        │ Release Dongles  │
        └────────┬─────────┘
                 ▼
        ┌──────────────────┐
        │ Debug / Refactor │
        └────────┬─────────┘
                 │
                 └──────► Repeat

   Monitor: checks deadlines and completion
```

## 🛡️ Concurrency Challenges

**Deadlock prevention**

Codexion enforces a consistent order when acquiring dongles to prevent circular waiting between coder threads.

**Scheduling policies**

FIFO serves requests by arrival order, while EDF prioritizes requests with earlier burnout deadlines. A binary min-heap manages the waiting queue according to the selected policy.

**Resource availability**

Each dongle observes a cooldown period after release. Synchronization primitives coordinate access and notify waiting threads when they need to re-evaluate availability.

**Deadline monitoring**

A dedicated monitor checks coder compilation deadlines and detects when the simulation must terminate, including when all required compilations have been completed.

**Safe termination**

Condition-variable notifications wake waiting threads when necessary. Threads are joined before shared resources and synchronization objects are cleaned up.

## 🔒 Synchronization

| Mechanism | Responsibility |
|---|---|
| `pthread_mutex_t` | Protects shared data and resource state |
| `pthread_cond_t` | Coordinates waiting and notifications |
| Timed condition waits | Handle cooldown-related waiting |
| Binary min-heap | Maintains scheduling priorities |
| `pthread_join()` | Coordinates thread completion |

## 🧰 Requirements

- C compiler with POSIX thread support
- GNU Make
- Linux or another compatible POSIX environment

## 📚 References

- [42 subject](https://cdn.intra.42.fr/pdf/pdf/205753/en.subject.pdf)
- [pthread mutexes](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html)
- *The Little Book of Semaphores* — Allen B. Downey

---

*Exploring concurrency, resource coordination, and deadline-aware scheduling through C.*
