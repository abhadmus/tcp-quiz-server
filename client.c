/*
*
* [client.c]
* 
* Author: Abdus'Samad Bhadmus
* 
* This program implements a TCP client that connects to a quiz 
* server to participate in a quiz. It takes the server's IPv4 
* address and port as arguments, connects, and receives a 
* welcome message. The user inputs 'Y' to start or 'q' to 
* quit. During the quiz, it receives five questions, 
* sends user-provided answers, and displays server feedback. 
* After the quiz, it receives and displays the final score 
* before closing the connection. Error handling ensures 
* reliable communication with the server.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LINES 256

/*
 * read_line: Reads a line from a socket until a newline character, storing it in a buffer.
 * This function reads one character at a time from the socket until it encounters a newline or the buffer is full, excluding the newline from the stored string. It returns the number of characters read or -1 on error or connection closure, ensuring reliable reading of server messages like questions or feedback.
 */
int read_line(int sock, char* buffer, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        /* Read one character from the socket */
        int n = recv(sock, &c, 1, 0);
        /* Return -1 if connection is closed or an error occurs */
        if (n <= 0) return -1; // Error or connection closed
        /* Stop reading at newline, null-terminate buffer */
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
 * send_message: Sends a message to the socket followed by a newline.
 * This function transmits a given string to the server and appends a newline character to ensure proper message delimitation. It is used to send user responses, such as 'Y', 'q', or quiz answers, maintaining the expected communication format with the server.
 */
void send_message(int sock, const char* message) {
    /* Send the message content */
    send(sock, message, strlen(message), 0);
    /* Append newline to delimit message */
    send(sock, "\n", 1, 0);
}

/*
 * main: Implements the core logic of the TCP quiz client.
 * This function handles the client's interaction with the quiz server. It parses command-line arguments for the server's IP and port, establishes a connection, receives the welcome message, and processes user input to start or quit the quiz. It then manages the quiz loop, handling questions, answers, and feedback, and finally displays the score before closing the connection. Error handling ensures robust operation.
 */
int main(int argc, char** argv) {
    /* Check for correct number of arguments */
    if (argc != 3) {
        fprintf(stderr, "Error - Incorrect number of arguments. Use as follows: %s <server IP> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* server_ip = argv[1];
    /* Convert port string to integer */
    int server_port = atoi(argv[2]);
    int sock;
    struct sockaddr_in server_addr;

    /* Create TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Set up server address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    /* Convert IP address to binary format */
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address\n");
        exit(EXIT_FAILURE);
    }

    /* Connect to the server */
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    /* Receive and display welcome message */
    char buffer[1024];
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    } else {
        /* Close socket on receive error */
        close(sock);
        exit(EXIT_FAILURE);
    }

    /* Read user response to start or quit */
    char response[MAX_LINES];
    printf("Enter Y to start or q to quit: ");
    if (fgets(response, sizeof(response), stdin) == NULL) {
        /* Close socket on input error */
        close(sock);
        exit(EXIT_FAILURE);
    }
    /* Remove trailing newline from response */
    response[strcspn(response, "\n")] = '\0';
    /* Send response to server */
    send_message(sock, response);

    /* Exit if user chooses to quit */
    if (strcmp(response, "q") == 0) {
        close(sock);
        exit(EXIT_SUCCESS);
    }

    /* Handle five quiz questions */
    for (int i = 0; i < 5; i++) {
        /* Receive and display question */
        char question[MAX_LINES];
        if (read_line(sock, question, sizeof(question)) <= 0) {
            printf("Connection lost.\n");
            break;
        }
        printf("Q: %s\n", question);

        /* Read user answer */
        char answer[MAX_LINES];
        printf("A: ");
        if (fgets(answer, sizeof(answer), stdin) == NULL) {
            break;
        }
        /* Remove trailing newline from answer */
        answer[strcspn(answer, "\n")] = '\0';
        /* Send answer to server */
        send_message(sock, answer);

        /* Receive and display feedback */
        char feedback[MAX_LINES];
        if (read_line(sock, feedback, sizeof(feedback)) <= 0) {
            printf("Connection lost.\n");
            break;
        }
        printf("%s\n", feedback);
    }

    /* Receive and display final score */
    char score_message[MAX_LINES];
    if (read_line(sock, score_message, sizeof(score_message)) > 0) {
        printf("%s\n", score_message);
    }

    /* Close socket and exit */
    close(sock);
    return 0;
}