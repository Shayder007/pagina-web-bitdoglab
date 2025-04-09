# 🔹 BitDogLab Web Joystick Dashboard

Projeto desenvolvido para a placa **BitDogLab (Raspberry Pi Pico W)** que cria uma página web embarcada para monitorar em tempo real os dados do **joystick analógico** e dos **botões físicos A e B**.

> Feito com muito café ☕, amor ❤️ e uns `#define` bem colocados.

---

## 🚀 Funcionalidades

- 🌐 Servidor web embarcado rodando direto na placa
- 🤹 Leitura dos eixos X e Y do joystick
- 🤭 Cálculo da direção (ex: Norte, Sul, Nordeste...)
- 🔘 Detecção de botões pressionados (A e B)
- 🎨 Página estilizada no modo hacker (verde no preto)

---

## 🧰 Tecnologias e bibliotecas

- C com SDK da Raspberry Pi Pico
- lwIP para rede
- HTML embutido no código C
- Wi-Fi (cyw43 driver)

---

## 🔌 Conexões

| Componente | GPIO | Função |
|------------|------|--------|
| Joystick X | 27   | ADC1   |
| Joystick Y | 26   | ADC0   |
| Botão A    | 5    | Digital com pull-up |
| Botão B    | 6    | Digital com pull-up |

---

## 📸 Captura de Tela (Exemplo)

![Captura de tela 2025-04-09 104925](https://github.com/user-attachments/assets/f3dde21f-61c9-489c-9b1d-2e2d8d21de26)

## 📲 Como usar

1. Clone este repositório
2. Compile com o SDK do Raspberry Pi Pico
3. Configure seu Wi-Fi no código:
   ```c
   #define WIFI_SSID "SeuWiFi"
   #define WIFI_PASSWORD "Senha123"
   ```
4. Flash na placa
5. Veja o IP no terminal e acesse via navegador

---

## 💡 Melhorias Futuras

- Atualização automática com WebSockets
- API REST para integrar com outras aplicações
- Integração com OLED ou LEDs para feedback visual

---

## 👨‍💻 Autor

**Shayder Faustino do Nascimento**  
Estudante de ADS - IFPI  
Participante do projeto Embarca Tech  

---

## ⚠️ Aviso

Este projeto é educacional. Use com responsabilidade e evite conectar a redes Wi-Fi públicas sem segurança.

---

