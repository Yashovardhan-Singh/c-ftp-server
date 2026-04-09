#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 2121
#define BUFFER_SIZE 1024

void send_response(int sock, const char *msg) {
    send(sock, msg, strlen(msg), 0);
}

void handle_list(int sock) {
    DIR           *d = opendir(".");
    struct dirent *dir;
    char           buffer[BUFFER_SIZE];

    if (!d) {
        send_response(sock, "550 Failed\n");
        return;
    }

    send_response(sock, "150 Listing\n");

    while ((dir = readdir(d)) != NULL) {
        snprintf(buffer, sizeof(buffer), "%s\n", dir->d_name);
        send(sock, buffer, strlen(buffer), 0);
    }

    closedir(d);
    send_response(sock, "226 Done\n");
}

void handle_get(int sock, char *filename) {
    FILE *fp = fopen(filename, "rb");
    char  buffer[BUFFER_SIZE];

    if (!fp) {
        send_response(sock, "550 File not found\n");
        return;
    }

    send_response(sock, "150 Sending file\n");

    while (!feof(fp)) {
        int n = fread(buffer, 1, BUFFER_SIZE, fp);
        send(sock, buffer, n, 0);
    }

    fclose(fp);
    send_response(sock, "\n226 Transfer complete\n");
}

void handle_put(int sock, char *filename) {
    FILE *fp = fopen(filename, "wb");
    char  buffer[BUFFER_SIZE];

    if (!fp) {
        send_response(sock, "550 Cannot create file\n");
        return;
    }

    send_response(sock, "150 Ready to receive\n");

    while (1) {
        int n = recv(sock, buffer, BUFFER_SIZE, 0);
        if (n <= 0)
            break;

        if (strstr(buffer, "EOF\n"))
            break;

        fwrite(buffer, 1, n, fp);
    }

    fclose(fp);
    send_response(sock, "226 Upload complete\n");
}

void handle_client(int sock) {
    char buffer[BUFFER_SIZE];

    send_response(sock, "220 FTP Server Ready\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sock, buffer, BUFFER_SIZE, 0);
        if (n <= 0)
            break;

        printf("Client: %s", buffer);

        if (strncmp(buffer, "USER", 4) == 0) {
            send_response(sock, "331 OK\n");
        } else if (strncmp(buffer, "PASS", 4) == 0) {
            send_response(sock, "230 Logged in\n");
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            handle_list(sock);
        } else if (strncmp(buffer, "GET", 3) == 0) {
            char *file                = buffer + 4;
            file[strcspn(file, "\n")] = 0;
            handle_get(sock, file);
        } else if (strncmp(buffer, "PUT", 3) == 0) {
            char *file                = buffer + 4;
            file[strcspn(file, "\n")] = 0;
            handle_put(sock, file);
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            send_response(sock, "221 Bye\n");
            break;
        } else {
            send_response(sock, "502 Unknown\n");
        }
    }

    close(sock);
}

int main() {
    int                server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t          len = sizeof(client);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        exit(2);
    }

    listen(server_sock, 5);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *) &client, &len);
        printf("Client connected\n");
        handle_client(client_sock);
    }
}
