# Tasks on Real Time Operating System 
## Introduction
This repository contains tasks for the university course of the real-time operating system. Here are solutions to such problems as:
- Multithreaded plaintext encryption with a one-time notepad (Vernam cipher) generated using a linear congruent RNG. 
- Plaintext encryption with The BBS(Blum-Blum-Shub) algorithm
- Single-threaded QNX Neutrino resource manager
- Multithreaded QNX Neutrino resource manager



## Task 1   One Time Pad
### Appointment

The "One Time Pad" software tool is designed for multithreaded plaintext encryption with a one-time notepad (Vernam cipher) generated using a linear congruent RNG.

### Mathematical apparatus

Realize LCG - a linear congruent pseudorandom number generator (PSC) that calculates a pseudorandom sequence (PSP) using a recompetitive formula: 

<img src="https://latex.codecogs.com/svg.image?X_{n&plus;1}\equiv&space;(aX_{n}&space;&plus;&space;c)\mod{M}" title="X_{n+1}\equiv (aX_{n} + c)\mod{M}" />


- [LCG](https://en.wikipedia.org/wiki/Linear_congruential_generator) - Linear congruential generator 
- [OTP](https://www.cryptomuseum.com/crypto/otp/index.htm) - One-Time Pad (Vernam cipher)

### Requirements

- launch platform — RTOS QNX Neutrino 6.6;
- development platform — IDE QNX Momentics;
- to work with files and streams, use POSIX API;
- LCG is realized in a separate thread;
- to work with terminal arguments, use the getopt() function of the system API;
- implement basic error and exception handling;
- encryption is carried out in N worker threads (workers);
- number of workers N = number of CPU cores;
- the workers and the main thread are synchronized using the POSIX barrier;
- get the number of CPU cores programmatically, using a special function.

## Additionally

- Measure the duration of each stage of the otp program.
- To estimate the order of costs for initialization of multithreading mechanisms.
- To estimate the limit of the value of the input sequence, after which the efficiency of multithreaded execution exceeds these costs.

### Input data

- the path to the plaintext file;
- LCG parameters;
- the path to the output file.


### Example of a startup string

`otp  -i /path/to/text.txt -o -/path/to/cypher.txt -x0 4212 -a 84589 -c 45989 -m 217728`

### Output data

- the ciphertext stored in the specified file.

### Algorithm

1. Read the command line parameters, unpack them into a structure.
2. Read a file with plain text in binary form, display it in RAM. Get the file size. Provide a limit on the file size.
3. Create a pseudo-random sequence based on the read parameters using LCG in a separate pthread API thread.
4. Synchronize the main thread with the worker by [joining](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Fp%2Fpthread_join.html).
5. [Create a barrier](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.lib_ref/topic/p/pthread_barrier_init.html).
6. Declare a context structure containing a barrier, input data for each worker (fragments of notepad and plaintext), as well as providing for receiving output data from the worker.
7. Create N workers using the _pthread_create()_ function, passing each instance of the context.
8. The main stream is [blocked by waiting for a barrier](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.lib_ref/topic/p/pthread_barrier_wait.html).
9. Each worker performs bitwise addition modulo 2 of its notepad and text fragments and is [blocked by waiting for a barrier](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.lib_ref/topic/p/pthread_barrier_wait.html).
10. The main stream merges and saves the data into an output file.
11. [The barrier is being destroyed](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.lib_ref/topic/p/pthread_barrier_destroy.html ).


### Test

- launch OTP, encrypt text, get ciphertext;
- run the OTP again, submit the ciphertext as the input text, get the plaintext;
- check the equality of files using [diff](https://www.qnx.com/developers/docs/6.3.0SP3/neutrino/utilities/d/diff.html).

## Task 2 Single-threaded resource manager (RM) "Pseudorandom number generator BBS”
### Theoretical information

Previously, a linear congruent generator was used to generate a pseudorandom sequence . This generator has good performance, but a short sequence repetition period and other anomalies, which makes it unsuitable for use in cryptographic tasks.
[The BBS(Blum-Blum-Shub) algorithm](https://en.wikipedia.org/wiki/Blum_Blum_Shub), on the contrary, allows to obtain a highly cryptographically stable sequence, but is characterized by slow operation. The calculation is performed using a recurrent formula:

<img src="https://latex.codecogs.com/svg.image?X_{n&plus;1}\equiv&space;X_{n}^2\mod{M}" title="X_{n+1}\equiv X_{n}^2\mod{M}" />,

where <img src="https://latex.codecogs.com/svg.image?M&space;=&space;pq" title="M = pq" />, <img src="https://latex.codecogs.com/svg.image?p" title="p" /> and <img src="https://latex.codecogs.com/svg.image?q" title="q" /> are prime numbers such that <img src="https://latex.codecogs.com/svg.image?M" title="M" /> is a [Blum number](https://oeis.org/A016105).

The output of the generator  <img src="https://latex.codecogs.com/svg.image?Y_{n&plus;1}" title="Y_{n+1}" />  will be the parity bit of the element <img src="https://latex.codecogs.com/svg.image?X_{n&plus;1}" title="X_{n+1}" /> :

<img src="https://latex.codecogs.com/svg.image?Y_{n&plus;1}&space;=&space;paritybit(X_{n&plus;1})" title="Y_{n+1} = paritybit(X_{n+1})" />

The generator parameters, thus, will be <img src="https://latex.codecogs.com/svg.image?X_{0}" title="X_{0}" /> (seed, initial state), as well as the values of <img src="https://latex.codecogs.com/svg.image?p" title="p" /> and <img src="https://latex.codecogs.com/svg.image?q" title="q" />.

### Description of the "Client-RM" interaction protocol

- The protocol should be described in the shared header file bbs.h.
- The command codes are defined using the macros [___DIOF and_ __DION_](https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/d/devctl.html).
- The "Client-RM" interaction is carried out via the POSIX interface using the [_open_](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Fo%2Fopen.html), [_devctl_](https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/d/devctl.html), [_close_](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Fo%2Fopen.html) functions.
- The requests described below are passed through the devctl function using a valid descriptor obtained by successfully calling the _open_ function:

  | Request 1  | Setting the generator parameters |
  | ------ | ------ |
  | Request Code | 1 |
  | Input data | structure containing generator parameters: `struct BBSParams {uint32_t seed;  uint32_t p; uint32_t q;} `|
  | Output data | No |

  | Request 2  | Getting the pseudorandom sequence element |
  | ------ | ------ |
  | Request Code | 2 |
  | Input data | No|
  | Output data | `uint32_t` |


### Client development
Realize the cryptobbs-client client program for the QNX Neutrino 6.6 (x86) platform. The primacy of client development can be understood in the context of the TDD (test-driven-development) concept, when top-level code using a certain interface is developed first, before implementing the interface itself. This makes it possible to think better in advance about solving a number of problems, including the specifics of interface usage scenarios and error handling.

#### Requirements:
1.	Open a connection with the RM using the `open("/dev/cryptobbs")` function.
2.	Set the parameters of the BBS generator (<img src="https://latex.codecogs.com/svg.image?p&space;=&space;3,&space;q&space;=&space;263,&space;seed&space;=&space;866" title="p = 3, q = 263, seed = 866" />) using the[`devctl()`](https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/d/devctl.html) function.
3.	In the loop, send requests to the RM using the [`devctl()`](https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/d/devctl.html) function with the requirement to receive the next element of the pseudorandom sequence.
4.  Save the next element to a fixed-length 1024 vector, working on the principle of a ring buffer.
5.	The cycle ends when the user presses Ctrl + C (the _SIGINT_ process receives).
6.	The contents of the vector are output to `stdout`.
7.  The connection to the RM is closed using the [`close()`](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Fo%2Fopen.html) function.


### Resource Manager Development

Realize a cryptobbs resource manager for the _QNX Neutrino 6.6 (x86)_ platform that meets the following requirements,
#### Requirements
- uses the [skeleton of a single-threaded RM](http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.resmgr/topic/skeleton_SIMPLE_ST_EG.html) as a basis;
- [registered](http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.lib_ref/topic/r/resmgr_attach.html) and functions using the _QNX Neutrino OS resmgr_library_ , the mount point is _"/dev/cryptobbs”_;
- realizes the processing of the client request using the [`io_devctl()`](https://it.wikireading.ru/2461) function, passed to the _io_funcs_ library through the corresponding pointer structure in the skeleton;
- it works according to the "Client-RM” interaction protocol , uses the bbs.h file;
- extracts the data structure from the message (for Protocol request 1), converts the `void*` type to the BBSParams structure type using `reinterpret_cast`;
- validates the data, in case of an error, passes it to the client;	
- upon request 2 of the Protocol, it generates another element of the pseudorandom sequence using the BBS algorithm according to the parameters passed in request 1 and sends it to the client.

### Test
- RM launch.
- client launch.
- Client completion by SIGINT.
- Checking the client's output.

## Task 3 Development of a multithreaded QNX Neutrino resource manager
### Problem
Develop a multithreaded version of the BBS PRNG resource Administrator cryptobbs v2.0 that supports correct parallel work with multiple clients.
### Requirements
- [The QNX Neutrino thread pool](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_resmgr%2Fmultithread.html) should be used as a tool for organizing a multithreaded scheme;
- the protocol of interaction "cryptobbs-client" v 2.0 should support working with sessions, each client should be able to generate “his" pseudorandom sequence with individual parameters;
- the sender ID of the incoming message is used as the client ID;
- to provide storage of interaction contexts with each client inside the RM, allocation of such a context (_io_open_) and destruction (_io_close_).

## License

MIT


