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

// Função para determinar a direção com base nos valores do joystick (girado 90°)
const char* direcao_joystick(uint16_t x, uint16_t y) {
    const int DEADZONE = 1000;
    int centro = 2048;

    int delta_x = x - centro; // novo eixo X (GPIO27)
    int delta_y = y - centro; // novo eixo Y (GPIO26)

    if (abs(delta_x) < DEADZONE && abs(delta_y) < DEADZONE)
        return "Centro 🎯";

    if (delta_x > DEADZONE && abs(delta_y) < DEADZONE)
        return "Leste ➡️";
    if (delta_x < -DEADZONE && abs(delta_y) < DEADZONE)
        return "Oeste ⬅️";
    if (delta_y > DEADZONE && abs(delta_x) < DEADZONE)
        return "Norte ⬆️";
    if (delta_y < -DEADZONE && abs(delta_x) < DEADZONE)
        return "Sul ⬇️";

    if (delta_x > DEADZONE && delta_y > DEADZONE)
        return "Nordeste ↗️";
    if (delta_x < -DEADZONE && delta_y > DEADZONE)
        return "Noroeste ↖️";
    if (delta_x > DEADZONE && delta_y < -DEADZONE)
        return "Sudeste ↘️";
    if (delta_x < -DEADZONE && delta_y < -DEADZONE)
        return "Sudoeste ↙️";

    return "Indefinido ❓";
}

// Função de resposta para o cliente
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Lê valores do joystick com eixo invertido
    adc_select_input(1); // GPIO27 -> novo eixo X
    uint16_t eixo_x = adc_read();

    adc_select_input(0); // GPIO26 -> novo eixo Y
    uint16_t eixo_y = adc_read();

    const char* direcao = direcao_joystick(eixo_x, eixo_y);

    // Leitura dos botões A e B (ativo em LOW)
    bool botao_a = !gpio_get(5);
    bool botao_b = !gpio_get(6);

    // Gera resposta HTML bonitona
    char response[1024];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>"
        "<html><head><meta charset=\"UTF-8\">"
        "<style>"
        "body{background:#000;color:#0f0;font-family:'Courier New',monospace;text-align:center;padding-top:40px}"
        "h2{color:#00ffff}"
        ".info{font-size:22px;margin-top:10px}"
        ".direcao{color:#ff0;font-size:26px;font-weight:bold;margin:20px 0}"
        ".botao{margin:8px;font-size:20px;color:#fff}"
        ".on{color:#0f0} .off{color:#f00}"
        "</style></head><body>"
        "<h2>Painel de Controle</h2>"
        "<div class=\"info\">Eixo X : %d</div>"
        "<div class=\"info\">Eixo Y : %d</div>"
        "<div class=\"direcao\">🧭 Direção: %s</div>"
        "<div class=\"botao\">Botão A : <span class=\"%s\">%s</span></div>"
        "<div class=\"botao\">Botão B : <span class=\"%s\">%s</span></div>"
        "<p style=\"color:#888\">F5 para atualizar a página para ler novamente.</p>"
        "</body></html>",
        eixo_x, eixo_y, direcao,
        botao_a ? "on" : "off", botao_a ? "PRESSIONADO" : "SOLTO",
        botao_b ? "on" : "off", botao_b ? "PRESSIONADO" : "SOLTO"
    );

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
    adc_gpio_init(26); // ADC0 - GPIO26
    adc_gpio_init(27); // ADC1 - GPIO27

    // Configura botões A e B
    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5); // Botão A

    gpio_init(6);
    gpio_set_dir(6, GPIO_IN);
    gpio_pull_up(6); // Botão B

    // Inicializa Wi-Fi
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
