# ghub-proxy

Este projeto consiste em uma biblioteca de vínculo dinâmico (DLL) desenvolvida em C para interagir com o driver do Logitech G Hub. O objetivo principal é simular comandos de mouse (movimentação e cliques) através do dispositivo virtual oficial do G Hub, permitindo a emulação de hardware que é frequentemente necessária para contornar restrições de software ou sistemas de proteção que bloqueiam entradas sintéticas convencionais.

## Visão Geral Técnica

A biblioteca utiliza funções nativas da API do Windows (NTAPI) para se comunicar diretamente com o objeto de dispositivo do driver G Hub. Ao enviar comandos via IOCTL (Input/Output Control), a DLL instrui o driver a realizar ações como se fossem originadas de um dispositivo físico de hardware.

## Estrutura do Projeto

Abaixo estão os principais componentes do código-fonte:

- [dllmain.c](dllmain.c): Ponto de entrada da DLL. Gerencia a inicialização das funções NT e o ciclo de vida da conexão com o dispositivo.
- [device.c](device.c): Responsável por abrir e fechar a comunicação com o driver do G Hub através de caminhos de dispositivo específicos.
- [ioctl.c](ioctl.c): Implementa o protocolo de comunicação de baixo nível, definindo como os comandos de mouse são formatados e enviados ao driver.
- [move.c](move.c): Contém a lógica de abstração para movimentação do cursor, incluindo o fracionamento de movimentos grandes em passos menores aceitos pelo driver.
- [ntapi.c](ntapi.c): Realiza o carregamento dinâmico das funções nativas do Windows (`NtCreateFile`, `NtDeviceIoControlFile`, etc.) para evitar dependências estáticas pesadas.
- [exports.c](exports.c): Define as funções que são exportadas pela DLL para serem utilizadas por aplicações externas.

## API de Exportação

A DLL exporta as seguintes funções para integração:

### mouse_open
```c
BOOL mouse_open(void);
```
Inicializa a conexão com o driver do G Hub. Deve ser chamada antes de qualquer operação de movimento ou clique. Retorna `TRUE` se a conexão for bem-sucedida.

### mouse_close
```c
void mouse_close(void);
```
Encerra a conexão com o driver e libera os recursos alocados.

### moveR
```c
NTSTATUS moveR(int dx, int dy);
```
Move o cursor de forma relativa. 
- `dx`: Deslocamento no eixo X.
- `dy`: Deslocamento no eixo Y.

### press
```c
NTSTATUS press(BYTE button);
```
Simula o pressionamento de um botão do mouse.
- `button`: Identificador do botão (ex: 1 para botão esquerdo).

### release
```c
NTSTATUS release(void);
```
Simula a liberação do botão do mouse anteriormente pressionado.

## Requisitos de Compilação

Para compilar este projeto, é necessário:
- Microsoft Visual Studio 2026 (com suporte a C/C++).
- Windows SDK.

O projeto utiliza um arquivo `.vcxproj` padrão que pode ser aberto diretamente no Visual Studio para compilação em arquiteturas x64.

## Notas de Implementação

A movimentação é processada em etapas (`MAX_MOVE_STEP`) para garantir a precisão e a compatibilidade com os buffers de entrada do driver. A biblioteca também implementa uma lógica de reconexão automática caso o handle do dispositivo se torne inválido durante a execução.
