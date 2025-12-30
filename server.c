/*
*
* [server.c]
*
* Author: Abdus'Samad Bhadmus
*
* This program implements a TCP server that hosts a quiz application. 
* It binds to a specified IPv4 address and port, listens for client 
* connections, and serves clients iteratively. For each client, it 
* sends a welcome message, allowing the client to start the quiz 
* with 'Y' or quit with 'q'. If started, it randomly selects 
* five questions from QuizDB.h, sends each question, evaluates 
* the client's answer, and provides feedback. After five questions, 
* it sends the final score and closes the connection. Error handling 
* ensures robust socket operations.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "QuizDB.h"

#define MAX_LINES 256

/*
 * read_line: Reads a line from a socket until a newline character, storing it in a buffer.
 * This function receives data one byte at a time from the specified socket, building a string until it encounters a newline or reaches the buffer's maximum length. It excludes the newline from the buffer, null-terminates the string, and returns the number of bytes read or -1 on error.
 */
int read_line(int sock, char* buffer, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        /* Receive one byte from socket */
        int n = recv(sock, &c, 1, 0);
        /* Return -1 if connection closed or error occurs */
        if (n <= 0) return -1;
        /* Stop at newline, null-terminate buffer */
        if (c == '\n') {
            buffer[i] = '\0';
            return i;
        }
        buffer[i] = c;
        i++;
    }
    /* Null-terminate buffer if max length reached */
    buffer[i] = '\0';
    return i;
}

/*
 * send_message: Sends a message followed by a newline to a socket.
 * This function transmits a given string to the specified socket and appends a newline character to ensure proper line-based communication. It uses the send() system call to handle the transmission, making it suitable for sending questions, feedback, and score messages to the client.
 */
void send_message(int sock, const char* message) {
    /* Send the message content */
    send(sock, message, strlen(message), 0);
    /* Append newline for line-based protocol */
    send(sock, "\n", 1, 0);
}

/*
 * main: Implements the TCP quiz server logic.
 * This function sets up a TCP server that binds to a user-specified IP address and port, listens for client connections, and handles the quiz process for each client iteratively. It sends a welcome message, processes the client's response ('Y' to start or 'q' to quit), selects five random questions, conducts the quiz by sending questions and evaluating answers, and sends the final score. Error handling is implemented for all socket operations.
 */
int main(int argc, char** argv) {
    /* Validate command-line arguments */
    if (argc != 3) {
        fprintf(stderr, "Error - Incorrect number of arguments. Use as follows: %s <IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* ip = argv[1];
    /* Convert port string to integer */
    int port = atoi(argv[2]);
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    /* Create TCP socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Initialize server address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    /* Convert IP address to binary */
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address\n");
        exit(EXIT_FAILURE);
    }

    /* Bind socket to IP and port */
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* Listen for incoming connections */
    if (listen(server_sock, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* Print listening status */
    printf("<Listening on %s:%d>\n", ip, port);
    printf("<Press ctrl-C to terminate>\n");

    /* Main loop to handle clients */
    while (1) {
        client_len = sizeof(client_addr);
        /* Accept client connection */
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        /* Send quiz preamble */
        const char* preamble = "Welcome to Unix Programming Quiz!\n"
                               "The quiz comprises five questions posed to you one after the other.\n"
                               "You have only one attempt to answer a question.\n"
                               "Your final score will be sent to you after conclusion of the quiz.\n"
                               "To start the quiz, press Y and <enter>.\n"
                               "To quit the quiz, press q and <enter>.\n";
        send(client_sock, preamble, strlen(preamble), 0);

        /* Read client's response (Y or q) */
        char response[MAX_LINES];
        if (read_line(client_sock, response, sizeof(response)) <= 0) {
            /* Close connection on read error */
            close(client_sock);
            continue;
        }

        /* Check if client wants to quit */
        if (strcmp(response, "q") == 0) {
            close(client_sock);
            continue;
        }
        /* Validate response is Y */
        if (strcmp(response, "Y") != 0) {
            close(client_sock);
            continue;
        }

        /* Calculate number of available questions */
        int num_questions = sizeof(QuizQ) / sizeof(QuizQ[0]);
        int selected[5];
        int count = 0;
        /* Seed random number generator */
        srand(time(NULL));
        /* Select five unique question indices */
        while (count < 5) {
            int idx = rand() % num_questions;
            int found = 0;
            for (int i = 0; i < count; i++) {
                if (selected[i] == idx) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                selected[count] = idx;
                count++;
            }
        }

        /* Conduct quiz for client */
        int score = 0;
        char feedback[256];
        for (int i = 0; i < 5; i++) {
            int q_idx = selected[i];
            /* Send question to client */
            send_message(client_sock, QuizQ[q_idx]);

            /* Read client's answer */
            char answer[MAX_LINES];
            if (read_line(client_sock, answer, sizeof(answer)) <= 0) {
                /* Break loop on read error */
                break;
            }

            /* Evaluate answer */
            if (strcmp(answer, QuizA[q_idx]) == 0) {
                score++;
                /* Send positive feedback */
                send_message(client_sock, "Right Answer.");
            } else {
                /* Prepare and send negative feedback */
                snprintf(feedback, sizeof(feedback), "Wrong Answer. Right answer is %s.", QuizA[q_idx]);
                send_message(client_sock, feedback);
            }
        }

        /* Send final score to client */
        char score_message[256];
        snprintf(score_message, sizeof(score_message), "Your quiz score is %d/5. Goodbye!", score);
        send_message(client_sock, score_message);

        /* Close client connection */
        close(client_sock);
    }

    /* Close server socket (unreachable due to infinite loop) */
    close(server_sock);
    return 0;
}