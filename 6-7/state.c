#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void receive_string(int server_socket, char *buffer, size_t buffer_size) {
    ssize_t received_bytes = recv(server_socket, buffer, buffer_size - 1, 0);

    if (received_bytes < 0) {
        perror("Ошибка в приеме строки от сервера");
        exit(EXIT_FAILURE);
    } else if (received_bytes == 0) {
        fprintf(stderr, "Соединение с сервером закрыто\n");
        exit(EXIT_FAILURE);
    }

    buffer[received_bytes] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <IP адрес сервера> <порт сервера>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int state_socket;
    struct sockaddr_in server_address;
    char buffer[1024];

    // Создание сокета для state программы
    if ((state_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Не удалось создать сокет для state программы");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера для state программы
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &(server_address.sin_addr)) <= 0) {
        perror("Ошибка в адресе сервера для state программы");
        exit(EXIT_FAILURE);
    }

    // Подключение state программы к серверу
    if (connect(state_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Не удалось подключить state программу к серверу");
        exit(EXIT_FAILURE);
    }

    printf("Установлено соединение со state программой\n");

    while (1) {
        // Прием сообщения от сервера
        receive_string(state_socket, buffer, sizeof(buffer));

        printf("Получено сообщение от сервера: %s\n", buffer);

        // Проверка условия завершения работы сервера
        if (strcmp(buffer, "SERVER_EXIT") == 0) {
            printf("Сервер завершил работу. State программа завершается.\n");
            break;
        }
    }

    close(state_socket);

    return 0;
}