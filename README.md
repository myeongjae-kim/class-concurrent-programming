# Subject: Concurrent Programming

This repository is a result of projects which I designed and implemented in Concurrent Programming class at Hanyang University. I conducted first three projects very well, but it was very hard to solve last project. I am solving a same problem of last project in different database system, not MariaDB.

## Table of Contents

* [Project 1: Signal, Hyunseok the Astronomer](#project-1-signal-hyunseok-the-astronomer)
* [Project 2: Simple Two\-phase Locking with Readers\-writer Lock](#project-2-simple-two-phase-locking-with-readers-writer-lock)
* [Project 3: Wait\-Free Snapshot](#project-3-wait-free-snapshot)
* [Project 4: Scalable Lock Manager](#project-4-scalable-lock-manager)

## [Project 1: Signal, Hyunseok the Astronomer](#table-of-contents)

[Wiki](https://github.com/hrzon/Class_ConcurrentProgramming/wiki/project1)

[Codes](https://github.com/hrzon/Class_ConcurrentProgramming/tree/master/project1)

This project is solving 'Order-Preserving Multiple Pattern Matching' problem using multithread programming. I used Trie structure.

Among 36 students, my ranking was #6. [Here](https://archive.is/at7oY) is score board.

## [Project 2: Simple Two-phase Locking with Readers-writer Lock](#table-of-contents)

[Wiki](https://github.com/hrzon/Class_ConcurrentProgramming/wiki/project2)

[Codes](https://github.com/hrzon/Class_ConcurrentProgramming/tree/master/project2)

### Readers-Writer Lock

It is a lock that many readers can acquire or only one writer acquire. In a transaction, read operation does not change a value of records, so read operation does not violate ACID property of transacntion even though plural readers access the record simultaneously. The readers-writer lock can improve throughput via permitting the access of plural readers.

### Two-phase Locking

The Two Phase Locking (2PL) is a kind of locking policy to guarantee atomicity, consistency, and isolation property of transaction in the multicore environment. It has two phase like its name.

First phase is locking phase. Records that have to be changed are locked before being modified. In a transaction, plural records can be locked. These locks are not released until the end of locking phase.

Second phase is releasing phase. In this phase, locks are only released. No record is locked in releasing phase. By this algorithm, a transaction's atomicity, consistency, and isolation property will be guaranteed.

But there is a problem when a transaction is aborted. Assume that a transaction is aborted at the end of releasing phase, and other transactions used records that the transaction modified. Since the transaction has been aborted, the other transactions also must be aborted. It makes huge overhead. This problem is called **cascading abort**.

### Strict Two-phase Locing

This locking algorithm does not release any locks until the end of a transaction. If the transaction modified some records, other transaction cannot read the records. It removes the situation that cascading abort problem occurs.

## [Project 3: Wait-Free Snapshot](#table-of-contents)

[Wiki](https://github.com/hrzon/Class_ConcurrentProgramming/wiki/project3)

[Codes](https://github.com/hrzon/Class_ConcurrentProgramming/tree/master/project3)

In the text book(The Art of Multiprocessor Programming, Maurice Herlihy & Sir Shavit), There are codes of implementing wait-free snapshot. These codes are wrtten in Java.

This project is writing wait-free snapshot code in C++. The biggest obstacle was a cause of memory leakage.

## [Project 4: Scalable Lock Manager](#table-of-contents)

[Wiki](https://github.com/hrzon/Class_ConcurrentProgramming/wiki/project4)

[Codes](https://github.com/hrzon/Class_ConcurrentProgramming/tree/master/project4/mariadb)

In the lock manager in MariaDB, there is bottleneck point. Because of the point, transaction's throughput is not increased even though CPU's core number is increased.

This project is removing the bottleneck point by modifying coarse-grained lock to fine-grained lock guaranteeing ACID property.

I did not solved the problem yet. Still I am solving it because my graduation project is this problem.