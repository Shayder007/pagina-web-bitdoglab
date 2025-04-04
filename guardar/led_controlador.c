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

#define LED_PIN CYW43_WL_GPIO_LED_PIN
#define LED_BLUE_PIN 12
#define LED_GREEN_PIN 11
#define LED_RED_PIN 13

bool blink_state = false;

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';
    printf("Request: %s\n", request);

    // Comandos de LED
    if (strstr(request, "GET /blue_on")) gpio_put(LED_BLUE_PIN, 1);
    else if (strstr(request, "GET /blue_off")) gpio_put(LED_BLUE_PIN, 0);
    else if (strstr(request, "GET /green_on")) gpio_put(LED_GREEN_PIN, 1);
    else if (strstr(request, "GET /green_off")) gpio_put(LED_GREEN_PIN, 0);
    else if (strstr(request, "GET /red_on")) gpio_put(LED_RED_PIN, 1);
    else if (strstr(request, "GET /red_off")) gpio_put(LED_RED_PIN, 0);
    else if (strstr(request, "GET /blink")) blink_state = !blink_state;
    else if (strstr(request, "GET /on")) cyw43_arch_gpio_put(LED_PIN, 1);
    else if (strstr(request, "GET /off")) cyw43_arch_gpio_put(LED_PIN, 0);

    // Temperatura
    adc_select_input(4);
    uint16_t raw = adc_read();
    float volts = raw * 3.3f / (1 << 12);
    float temp = 27.0f - (volts - 0.706f) / 0.001721f;

    // HTML enxuto com visual top
    char html[1024];
    snprintf(html, sizeof(html),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Pico LED</title>"
        "<style>body{background:#121212;color:#fff;text-align:center;font-family:sans-serif}"
        "button{margin:6px;padding:12px 24px;font-size:16px;border:none;border-radius:8px;cursor:pointer}"
        ".on{background:#4CAF50}.off{background:#f44336}.toggle{background:#2196F3}"
        "</style></head><body><h1>Controle de LEDs</h1>"
        "<p><a href='/blue_on'><button class='on'>Azul ON</button></a>"
        "<a href='/blue_off'><button class='off'>Azul OFF</button></a></p>"
        "<p><a href='/green_on'><button class='on'>Verde ON</button></a>"
        "<a href='/green_off'><button class='off'>Verde OFF</button></a></p>"
        "<p><a href='/red_on'><button class='on'>Vermelho ON</button></a>"
        "<a href='/red_off'><button class='off'>Vermelho OFF</button></a></p>"
        "<p><a href='/blink'><button class='toggle'>Piscar LEDs</button></a></p>"
        "<p>Temperatura: %.1f &deg;C</p></body></html>", temp);

    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    free(request);
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

    gpio_init(LED_BLUE_PIN); gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN); gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(LED_RED_PIN); gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    if (cyw43_arch_init()) return -1;
    cyw43_arch_gpio_put(LED_PIN, 0);
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        printf("Erro ao conectar\n");
        return -1;
    }
    printf("Conectado!\n");

    if (netif_default) printf("IP: %s\n", ipaddr_ntoa(&netif_default->ip_addr));

    struct tcp_pcb *server = tcp_new();
    if (!server) return -1;
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) return -1;

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    adc_init();
    adc_set_temp_sensor_enabled(true);

    while (true)
    {
        cyw43_arch_poll();
        if (blink_state)
        {
            gpio_put(LED_BLUE_PIN, 1);
            gpio_put(LED_GREEN_PIN, 1);
            gpio_put(LED_RED_PIN, 1);
            sleep_ms(300);
            gpio_put(LED_BLUE_PIN, 0);
            gpio_put(LED_GREEN_PIN, 0);
            gpio_put(LED_RED_PIN, 0);
            sleep_ms(300);
        }
    }

    cyw43_arch_deinit();
    return 0;
}
