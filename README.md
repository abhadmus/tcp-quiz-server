## OVERVIEW

This project is a TCP-based client-server quiz application written in C. It allows a client to connect to a server over the network and take a short quiz consisting of five questions.

* The server hosts the quiz and sends questions.
* The client connects to the server, receives questions, submits answers, and gets immediate feedback and a final score.

---

## FEATURES

* Line-based TCP communication using sockets
* Randomized questions selected from a predefined quiz database
* Simple and robust user input/response handling
* Error handling for socket communication
* Modular, commented, and readable code

---

## FILE STRUCTURE

* `client.c` : TCP client that connects to server and takes the quiz
* `server.c` : TCP server that waits for clients and serves quiz
* `QuizDB.h` : Header file containing quiz questions and answers arrays

---

## COMPILATION

Use `gcc` to compile the client and server:

```bash
gcc -o client client.c
gcc -o server server.c
```

Ensure `QuizDB.h` is in the same directory when compiling `server.c`.

---

## RUNNING THE APPLICATION

### Start the Server

Run on the server machine or terminal:

```bash
./server <IP_ADDRESS> <PORT>
```

Example:

```bash
./server 127.0.0.1 8888
```

### Start the Client

Run on the client machine or terminal:

```bash
./client <SERVER_IP_ADDRESS> <PORT>
```

Example:

```bash
./client 127.0.0.1 8888
```

---

## QUIZ FLOW

1. The client connects and receives a welcome message.
2. The user is prompted to enter:

   * `Y` to begin the quiz
   * `q` to quit
3. If the quiz begins:

   * 5 random questions are asked
   * The user answers each
   * Feedback is given after each answer
4. After 5 questions, the final score is shown and the connection closes.

---

## DEPENDENCIES

* Standard C libraries (`stdio.h`, `stdlib.h`, `string.h`)
* Networking headers (`sys/socket.h`, `netinet/in.h`, `arpa/inet.h`, `unistd.h`)
* Random number generation (`time.h`)

---

## NOTES

* `QuizDB.h` must be implemented with two arrays: `QuizQ[]` and `QuizA[]` of matching size.
* Ensure both the server and client use the same protocol (newline-delimited messages).
* The server currently handles one client at a time (sequential handling).

---

## EXAMPLE OUTPUT

```
Welcome to Unix Programming Quiz! The quiz comprises five questions...

To start the quiz, press Y...

Enter Y to start or q to quit: Y

Q: What does TCP stand for?

A: Transmission Control Protocol

Right Answer.

...

Your quiz score is 4/5.

Goodbye!
```