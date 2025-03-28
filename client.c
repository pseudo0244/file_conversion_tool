#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5001
#define BUFFER_SIZE 1024

void receive_progress(int client_socket)
{
    int progress;
    while (recv(client_socket, &progress, sizeof(int), 0) > 0)
    {
        if (progress == 100)
        {
            printf("\n✅ Conversion complete!\n");
            break;
        }
        else if (progress == -1)
        {
            printf("\n❌ Conversion failed!\n");
            break;
        }
        else
        {
            printf("\r🔄 Progress: %d%%", progress);
            fflush(stdout);
        }
    }
}

void send_file(int client_socket, const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error opening file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);
}

void receive_file(int client_socket, char *output_filename)
{
    char filename[256];
    recv(client_socket, filename, sizeof(filename), 0);

    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Error saving file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
    printf("✅ Converted file received: %s\n", filename);
}

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    char filename[256], format[10];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Enter the file to convert: ");
    scanf("%s", filename);
    printf("Enter the output format: ");
    scanf("%s", format);

    send(client_socket, filename, sizeof(filename), 0);
    send(client_socket, format, sizeof(format), 0);

    receive_progress(client_socket);
    receive_file(client_socket, filename);

    close(client_socket);
    return 0;
}
