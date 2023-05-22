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

void send_encoded_array(int server_socket, const int *array, size_t num_elements) {
    if (send(server_socket, array, num_elements * sizeof(int), 0) != num_elements * sizeof(int)) {
        perror("Ошибка в отправке массива int-ов серверу");
        exit(EXIT_FAILURE);
    }
}

void send_string(int client_socket, const char *str) {
    size_t length = strlen(str) + 1;

    if (send(client_socket, str, length, 0) != length) {
        perror("Ошибка в отправке строки клиенту");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <IP адрес сервера> <порт сервера>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_address;
    char buffer[1024];

    // Создание сокета
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Не удалось создать сокет");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &(server_address.sin_addr)) <= 0) {
        perror("Ошибка в адресе сервера");
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Не удалось подключиться к серверу");
        exit(EXIT_FAILURE);
    }

    printf("Установлено соединение с сервером %s:%d\n", server_ip, server_port);

    // Прием строки от сервера
    receive_string(client_socket, buffer, sizeof(buffer));

    printf("Получена строка от сервера: %s\n", buffer);

    // Вывод символов строки
    size_t i, length = strlen(buffer);

    printf("Символы строки: ");
    for (i = 0; i < length; i++) {
        printf("%c ", buffer[i]);
    }
    printf("\n");

    // Кодирование символов
    int int_array[length];
    for (i = 0; i < length; i++) {
        int_array[i] = (int)buffer[i];
    }


    //Отправка строки серверу
    char client_encoded_state[1500];
    sprintf(client_encoded_state, "Клиент успешно закодировал символы %s\n", buffer);
    send_string(client_socket, client_encoded_state);

    // Отправка закодированного массива серверу
    send_encoded_array(client_socket, int_array, length);


    close(client_socket);

    return 0;
}