#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10

void send_string(int client_socket, const char *str) {
    size_t length = strlen(str) + 1;

    if (send(client_socket, str, length, 0) != length) {
        perror("Ошибка в отправке строки клиенту");
        exit(EXIT_FAILURE);
    }
}

void receive_encoded_array(int client_socket, int *array, size_t num_elements) {
    ssize_t received_bytes = recv(client_socket, array, num_elements * sizeof(int), 0);

    if (received_bytes < 0) {
        perror("Ошибка в приеме массива int-ов от клиента");
        exit(EXIT_FAILURE);
    } else if (received_bytes == 0) {
        fprintf(stderr, "Соединение с клиентом закрыто\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Использование: %s <IP адрес сервера> <порт сервера> <количество клиентов>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int num_clients = atoi(argv[3]);

    if (num_clients <= 0 || num_clients > MAX_CLIENTS) {
        fprintf(stderr, "Количество клиентов должно быть положительным и не превышать %d\n", MAX_CLIENTS);
        exit(EXIT_FAILURE);
    }

    // Ввод строки от сервера
    printf("Введите строку: ");
    char input[1024];
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; // Удаление символа новой строки
    size_t length = strlen(input);
    if (length < num_clients) {
        printf("Кол-во клиентов не может быть больше кол-ва символов\n");
        exit(EXIT_FAILURE);
    }


    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(client_address);
    int i;

    // Создание сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Не удалось создать сокет");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);

    // Привязка сокета к адресу сервера
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Не удалось привязать сокет к адресу сервера");
        exit(EXIT_FAILURE);
    }

    // Прослушивание подключений на сервере
    if (listen(server_socket, num_clients) < 0) {
        perror("Ошибка в прослушивании подключений на сервере");
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен. Ожидание подключения %d клиентов...\n", num_clients);

    // Прием подключений от клиентов
    int encoded_array[length];
    for (i = 0; i < num_clients; i++) {
        if ((client_sockets[i] = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0) {
            perror("Ошибка в принятии подключения от клиента");
            exit(EXIT_FAILURE);
        }

        printf("Клиент %d подключен\n", i + 1);

        // Формирование строки для клиента
        int j, k;
        char client_string[length / num_clients + 1];
        k = 0;

        for (j = i; j < length; j += num_clients) {
            client_string[k++] = input[j];
        }

        client_string[k] = '\0';

        // Отправка строки клиенту
        send_string(client_sockets[i], client_string);

        // Получение массива от клиента
        int received_array[k];
        receive_encoded_array(client_sockets[i], received_array, k);

        int count = 0;
        for (int j = i; j < length; j += num_clients) {
            encoded_array[j] = received_array[count++];
        }

        // Вывод полученного массива
        printf("Полученный массив от клиента %d: ", i + 1);
        for (int j = 0; j < k; j++) {
            printf("%d ", received_array[j]);
        }
        printf("\n");
    }


    printf("Закодированный массив: ");
    for (int i = 0; i < length; i++) {
        printf("%d ", encoded_array[i]);
    }

    // Закрытие соединений с клиентами
    for (i = 0; i < num_clients; i++) {
        close(client_sockets[i]);
    }

    close(server_socket);

    return 0;
}