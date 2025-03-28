#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define BUFFER_SIZE 1024

void send_progress(int client_socket, int percentage)
{
    send(client_socket, &percentage, sizeof(int), 0);
}

int convert_file(const char *input_file, const char *output_file, const char *format, int client_socket)
{
    FILE *fin = fopen(input_file, "rb");
    FILE *fout = fopen(output_file, "wb");

    if (!fin || !fout)
    {
        perror("Error opening file");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read, total_bytes = 0, progress = 0;

    fseek(fin, 0, SEEK_END);
    int file_size = ftell(fin);
    rewind(fin);

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fin)) > 0)
    {
        fwrite(buffer, 1, bytes_read, fout);
        total_bytes += bytes_read;
        progress = (total_bytes * 100) / file_size;
        send_progress(client_socket, progress);
        usleep(50000); // Simulate processing delay
    }

    fclose(fin);
    fclose(fout);
    return 0;
}

void handle_client(int client_socket)
{
    char filename[256], format[10], converted_filename[256];

    recv(client_socket, filename, sizeof(filename), 0);
    recv(client_socket, format, sizeof(format), 0);

    sprintf(converted_filename, "converted.%s", format);

    send_progress(client_socket, 10);

    if (convert_file(filename, converted_filename, format, client_socket) == 0)
    {
        send_progress(client_socket, 100);
        send(client_socket, converted_filename, sizeof(converted_filename), 0);
    }
    else
    {
        send_progress(client_socket, -1); // Indicate failure
    }

    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket >= 0)
        {
            printf("Client connected!\n");
            handle_client(client_socket);
        }
    }

    close(server_socket);
    return 0;
}
