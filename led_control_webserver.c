#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"

#define WIFI_SSID "ZTE-2"
#define WIFI_PASSWORD "shay241098"

// Função de resposta para o cliente
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Lê valores do joystick
    adc_select_input(0); // X - GPIO26
    uint16_t eixo_x = adc_read();
    adc_select_input(1); // Y - GPIO27
    uint16_t eixo_y = adc_read();

    // Gera resposta HTML
    char response[512];
    snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n\r\n"
            "<!DOCTYPE html>"
            "<html><head><meta charset=\"UTF-8\">"
            "<style>"
            "body{font-family:sans-serif;text-align:center;margin-top:40px}"
            ".info{font-size:22px;margin-top:10px}"
            "</style></head><body>"
            "<h2>🎮 Leitura do Joystick</h2>"
            "<div class=\"info\">Eixo X: %d</div>"
            "<div class=\"info\">Eixo Y: %d</div>"
            "<p>Atualize a página para ler novamente.</p>"
            "</body></html>", eixo_x, eixo_y);

    tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    pbuf_free(p);

    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

int main()
{
    stdio_init_all();

    // Inicializa ADC (para joystick)
    adc_init();
    adc_gpio_init(26); // ADC0 - X
    adc_gpio_init(27); // ADC1 - Y

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar no Wi-Fi\n");
        return -1;
    }

    printf("Conectado!\n");
    if (netif_default)
        printf("IP: %s\n", ipaddr_ntoa(&netif_default->ip_addr));

    // Configura servidor TCP
    struct tcp_pcb *server = tcp_new();
    tcp_bind(server, IP_ADDR_ANY, 80);
    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    while (true)
        cyw43_arch_poll();

    cyw43_arch_deinit();
    return 0;
}
