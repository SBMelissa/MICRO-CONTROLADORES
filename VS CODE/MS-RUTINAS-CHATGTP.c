//LIBRERIAS 
#include <stdio.h>
#include <mqtt.h>
#include <string.h>
//ELEMENTOS DEL MQTT
#define BROKER_ADDRESS "mqtt://test.mosquitto.org"
#define TOPIC "ITLA/MICRO/MS/PUERTA"
#define CLIENT_ID "ESP_MS"

////ESTADOS DE LA MS
//#define ESTADO_INIT      0
//#define ESTADO_CERRADO   1
//#define ESTADO_ABIERTO   2
//#define ESTADO_CERRANDO  3
//#define ESTADO_ABRIENDO  4
//#define ESTADO_ERROR     5

// Estados
enum estados { INIT, CERRADO, ABIERTO, CERRANDO, ABRIENDO, ERROR } estado_actual, estado_siguiente;

// Configuración MQTT
mqtt_client_t client;
uint8_t sendbuf[256];
uint8_t recvbuf[256];
int mqtt_connected = 0;

// Función simulada para verificar conexión Wi-Fi
int wifi_is_connected() {
    // Implementar verificación de conexión a la red Wi-Fi
    // Retorna 1 si está conectado, 0 si no lo está.
    return 1;  // Cambiar a la lógica real de conexión en el dispositivo STM32
}

// Callback para recibir mensajes MQTT
void message_callback(void** unused, struct mqtt_response_publish *published) {
    printf("Mensaje recibido en %s: %.*s\n", published->topic_name, (int)published->application_message_size, (const char*)published->application_message);

    if (strncmp((const char*)published->application_message, "ABRIR", published->application_message_size) == 0) {
        estado_siguiente = ABRIENDO;
    } else if (strncmp((const char*)published->application_message, "CERRAR", published->application_message_size) == 0) {
        estado_siguiente = CERRANDO;
    } else if (strncmp((const char*)published->application_message, "RESET", published->application_message_size) == 0) {
        estado_siguiente = INIT;
    }
}

// Conectar al broker MQTT
void mqtt_connect() {
    int sockfd = connect_to_broker(BROKER_ADDRESS, 1883);  // conecta al broker
    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), message_callback);
    mqtt_connect_client(&client, CLIENT_ID, NULL, NULL, 0, NULL, NULL, 0, 400);

    if (client.error == MQTT_OK) {
        mqtt_connected = 1;
        mqtt_subscribe(&client, TOPIC, 1);
    } else {
        mqtt_connected = 0;
    }
}

// Publicar el estado actual
void publicar_estado(const char* estado) {
    if (mqtt_connected) {
        mqtt_publish(&client, TOPIC, estado, strlen(estado), MQTT_PUBLISH_QOS_0);
    }
}

// Máquina de estados
void maquina_de_estados() {
    switch (estado_actual) {
        case INIT:
            printf("Estado: INIT\n");
            publicar_estado("INIT");
            estado_siguiente = CERRADO;
            break;

        case CERRADO:
            printf("Estado: CERRADO\n");
            publicar_estado("CERRADO");
            if (estado_siguiente == ABRIENDO) {
                estado_actual = ABRIENDO;
            }
            break;

        case ABIERTO:
            printf("Estado: ABIERTO\n");
            publicar_estado("ABIERTO");
            if (estado_siguiente == CERRANDO) {
                estado_actual = CERRANDO;
            }
            break;

        case CERRANDO:
            printf("Estado: CERRANDO\n");
            publicar_estado("CERRANDO");
            estado_actual = CERRADO;
            break;

        case ABRIENDO:
            printf("Estado: ABRIENDO\n");
            publicar_estado("ABRIENDO");
            estado_actual = ABIERTO;
            break;

        case ERROR:
            printf("Estado: ERROR\n");
            publicar_estado("ERROR");
            if (estado_siguiente == INIT) {
                estado_actual = INIT;
            }
            break;
    }
}

int main() {
    estado_actual = INIT;

    // Bucle de conexión Wi-Fi
    while (!wifi_is_connected()) {
        printf("Esperando conexión Wi-Fi...\n");
        delay(1000);  // Espera 1 segundo antes de reintentar
    }
    printf("Conexión Wi-Fi establecida.\n");

    // Conecta al broker MQTT
    mqtt_connect();

    while (1) {
        if (!wifi_is_connected()) {
            printf("Conexión Wi-Fi perdida. Intentando reconectar...\n");
            while (!wifi_is_connected()) {
                delay(1000);
            }
            printf("Reconexión Wi-Fi exitosa.\n");

            mqtt_connect();  // Reintenta conexión MQTT si se perdió Wi-Fi
        }

        if (mqtt_connected) {
            mqtt_sync(&client);  // Procesa MQTT
            maquina_de_estados();  // Ejecuta máquina de estados
        } else {
            mqtt_connect();  // Reintentar conexión MQTT si no está conectado
        }

        delay(100);  // Breve espera antes del próximo ciclo
    }
}
