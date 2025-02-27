## **Transmissor FM com RP2040 (BitDogLab)**

Este projeto implementa um **Transmissor FM** usando a **RP2040** com microfone embutido na  **BitDogLab** , transmitindo na faixa de  **88.0 a 108.0 MHz** .

🔊  **Entrada de Áudio** : Microfone no **GPIO 28 (ADC2)**
📻  **Saída FM** : **GPIO 15 (PWM)** para antena
🎛️  **Ajuste de Frequência** :

* **Botão UP (GPIO 5)** : Aumenta a frequência em **0.1 MHz**
* **Botão DOWN (GPIO 6)** : Diminui a frequência em **0.1 MHz**

⚠️  **Aviso Legal** : Transmissões FM são **regulamentadas** na maioria dos países. Não conecte uma antena ao GPIO sem um  **Filtro Passa-Baixa (LPF)** . Consulte as leis locais antes de transmitir qualquer sinal.

---

## ⚙️ **Funcionalidades**

* **Faixa de Frequência** : **88.0 a 108.0 MHz**
* **Modulação FM** usando PWM para transmitir áudio do microfone embutido.
* **Ajuste de Frequência Dinâmico** com botões físicos.
* **Média Móvel** para suavização do áudio.
* **Debounce de Botões** por hardware e software para ajustes precisos.

---

## 🧰 **Componentes Necessários**

* **BitDogLab (RP2040 com microfone embutido)**
* **Botão (Push Button)** x 2 (para ajuste de frequência)
* **Resistores Pull-Up Internos** (configurados no código)
* **Antena (Opcional e com LPF)** : Um pedaço de fio de **75 cm** (1/4 de onda para FM)
* **Filtro Passa-Baixa (LPF)** : Recomendado para eliminar harmônicos.

### **Filtro Passa-Baixa (LPF) Recomendado**

Para **100 MHz** (faixa FM), use:

* **Indutores (L1, L2, L3)** : 56 nH
* **Capacitores (C1, C2)** : 22 pF

Esquema:

GPIO 15 ---> L1 ---> C1 ---> L2 ---> C2 ---> L3 ---> Antena

---

## 🔨 **Montagem e Conexões**

### **Conexões de Hardware**

* **Microfone** : Já embutido na BitDogLab (GPIO 28 - ADC2)
* **Botões** :
* **Botão UP** : GPIO 5 (PULL-UP Interno)
* **Botão DOWN** : GPIO 6 (PULL-UP Interno)
* **Saída FM (Antena)** : GPIO 15
* **Conecte o GPIO 15 ao LPF** antes de conectar a antena.

---

### **Diagrama de Ligação**

```


+---------------------+
|     BitDogLab        |
|  (RP2040 com Mic)    |
|                     |
|    GPIO 28 (ADC2)    | <-- Microfone embutido
|    GPIO 15 (PWM)     | --> LPF --> Antena|    GPIO 5 (Botão UP) | <-- Botão para Aumentar Frequência
|    GPIO 6 (Botão DN) | <-- Botão para Diminuir Frequência
+---------------------+

```

---

## 🚀 **Configuração e Compilação**

### **Pré-requisitos**

* **Pico SDK** : Certifique-se de ter o **Pico SDK** instalado e atualizado.
* [Guia de Instalação do Pico SDK](https://github.com/raspberrypi/pico-sdk)

### **Clone o Repositório**

```bash
git clone https://github.com/hsantosdias/BitDogLab-FM.git
cd BitDogLab-FM

```

### **Configure o CMake**

mkdir build
cd build
cmake ..
make

### **Flash no RP2040**

1. Conecte a BitDogLab no modo  **BOOTSEL** .
2. Copie o arquivo `.uf2` gerado na pasta `build` para a unidade USB que aparecerá.

---

## 📝 **Como Usar**

1. Ligue a BitDogLab com o código carregado.
2. **Ajuste a frequência** usando os botões:
   * **UP (GPIO 5)** : Aumenta 0.1 MHz por clique.
   * **DOWN (GPIO 6)** : Diminui 0.1 MHz por clique.
3. **Sintonize um rádio FM** na frequência definida.
4. **Fale no microfone embutido** e ouça sua voz no rádio.

---

## ⚠️ **Avisos e Considerações Legais**

* **NUNCA conecte uma antena diretamente ao GPIO** sem um **LPF adequado** para evitar interferências e problemas legais.
* Verifique as **leis locais** sobre transmissões de rádio FM.
* A transmissão de sinais na faixa de **88-108 MHz** é **altamente regulada** e **não é permitida sem licença** na maioria dos países.
* **Use apenas para fins educacionais** e  **dentro da sua propriedade** .

---

## 📘 **Referências e Créditos**

* **RP2040 Datasheet** : [https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html]()
* **Pico SDK** : [https://github.com/raspberrypi/pico-sdk](https://github.com/raspberrypi/pico-sdk)
* **BitDogLab Documentation** : Documentação oficial da placa.
* **Código Base** desenvolvido a partir de experimentos com **PWM** e **ADC** no RP2040.

---

## 📜 **Licença**

Este projeto é distribuído sob a  **Licença MIT** . Consulte o arquivo `LICENSE` para mais detalhes.

---

## 💬 **Contribuições e Suporte**

* **Pull Requests** são bem-vindos para melhorias no código e documentação.
* Para sugestões ou problemas, abra uma **Issue** no GitHub.
