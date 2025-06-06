# 🐍 Snake Game com ESP32 + Bluetooth + Firebase

Projeto desenvolvido para a disciplina de Sistemas Embarcados no Centro Universitário CESMAC.

Este é um jogo da cobrinha (Snake Game) controlado via **Bluetooth** com visualização em uma **matriz de LEDs WS2812B (5x5)**, com adição extra de um Buzzer. A melhor pontuação é armazenada no **Firebase Realtime Database**, garantindo persistência dos dados mesmo após desligar o dispositivo.

## 🎮 Funcionalidades

- ✅ Movimentação da cobrinha com comandos Bluetooth
- ✅ Crescimento ao comer comida
- ✅ Reinício ao colidir com o corpo ou limites
- ✅ Registro da **melhor pontuação** no Firebase
- ✅ Exibição gráfica com NeoPixels (WS2812B)

## 🧰 Tecnologias e Ferramentas

- ESP32 DevKit V1
- Arduino IDE + C++
- Biblioteca Adafruit NeoPixel
- Comunicação BluetoothSerial
- Firebase Realtime Database (via HTTP)
- Simulador Wokwi

## 👥 Equipe

| Nome                                     | GitHub                                |
|------------------------------------------|----------------------------------------|
| Bruno Malta Barros                       | [@brunomalta04](https://github.com/brunomalta04)       |
| David Santana Bulhões do Nascimento      | [@DavidSantanaB](https://github.com/DavidSantanaB)     |
| Italo Guilherme Cardoso Barbosa          | [@ShintaroK3](https://github.com/ShintaroK3)           |
| Luís Guilherme Aragão de Albuquerque     | [@LuisGuilherme1127](https://github.com/LuisGuilherme1127) |
| Kevin Lucas Vicente dos Santos           | [@luskas20](https://github.com/luskas20)               |


## 📁 Organização do Projeto

- `src/` – Código-fonte principal
- `lib/` – Bibliotecas auxiliares (se houver)
- `include/` – Arquivos de cabeçalho
- `test/` – Estrutura para testes
- `platformio.ini` – Configuração do projeto

---

**Desenvolvido com dedicação pela equipe CESMAC – 2025.**
