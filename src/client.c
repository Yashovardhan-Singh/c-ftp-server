#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 2121
#define BUFFER_SIZE 1024

void receive_response(int sock) {
    char buffer[BUFFER_SIZE];
    int  n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
}

void handle_get(int sock, char *filename) {
    FILE *fp = fopen(filename, "wb");
    char  buffer[BUFFER_SIZE];

    while (1) {
        int n = recv(sock, buffer, BUFFER_SIZE, 0);
        if (n <= 0)
            break;

        if (strstr(buffer, "226"))
            break;

        fwrite(buffer, 1, n, fp);
    }

    fclose(fp);
}

void handle_put(int sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    char  buffer[BUFFER_SIZE];

    if (!fp) {
        printf("File not found\n");
        return;
    }

    while (!feof(fp)) {
        int n = fread(buffer, 1, BUFFER_SIZE, fp);
        send(sock, buffer, n, 0);
    }

    send(sock, "EOF\n", 4, 0);
    fclose(fp);
}

int main() {
    int                sock;
    struct sockaddr_in server;
    char               buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port   = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        exit(2);
    }

    receive_response(sock);

    while (1) {
        printf("ftp> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "GET", 3) == 0) {
            char *file                = buffer + 4;
            file[strcspn(file, "\n")] = 0;
            handle_get(sock, file);
        } else if (strncmp(buffer, "PUT", 3) == 0) {
            char *file                = buffer + 4;
            file[strcspn(file, "\n")] = 0;
            handle_put(sock, file);
        } else {
            receive_response(sock);
        }

        if (strncmp(buffer, "QUIT", 4) == 0)
            break;
    }

    close(sock);
}
