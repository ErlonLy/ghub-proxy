# ghub-proxy

DLL em C (x64) para envio de comandos de mouse via driver virtual da Logitech (G HUB / LGS).

Permite movimentação relativa, cliques e scroll enviando IOCTLs diretamente pro driver de kernel, contornando restrições que bloqueiam `SendInput` ou `mouse_event`.

## Como funciona

A DLL carrega dinamicamente as funções da NTAPI (`NtCreateFile`, `NtDeviceIoControlFile`) da `ntdll.dll` e envia os pacotes diretamente para o driver pelo IOCTL `0x2A2010`.

- **Detecção do driver**: Localiza automaticamente a interface ativa varrendo instâncias `0000` a `0015` nos GUIDs do G HUB e LGS, salvando o caminho válido em cache.
- **Movimentação linear**: Movimentos que passam de 127 pixels são fracionados proporcionalmente nos eixos X e Y juntos, mantendo a trajetória em linha reta.
- **Estado de botões**: Mantém uma máscara dos botões pressionados para que chamadas a `moveR` não soltem cliques que estejam sendo segurados.

## Requisitos

- Windows x64
- Logitech G HUB ou LGS em execução
- Processo chamador executado como Administrador

## Funções Exportadas

```c
BOOL mouse_open(void);
void mouse_close(void);
NTSTATUS moveR(int dx, int dy);
NTSTATUS press(BYTE button);
NTSTATUS release(void);
NTSTATUS release_button(BYTE button);
NTSTATUS mouse_down(BYTE button);
NTSTATUS mouse_up(BYTE button);
NTSTATUS mouse_click(BYTE button, DWORD delay_ms);
NTSTATUS scroll(char wheel);
```

### Máscara de Botões

| Código | Botão |
| :--- | :--- |
| `1` | Botão Esquerdo |
| `2` | Botão Direito |
| `4` | Botão do Meio |
| `8` | Lateral Traseiro (Mouse 4) |
| `16` | Lateral Dianteiro (Mouse 5) |

## Compilação

Abra `ghubmouse.vcxproj` no Visual Studio 2022+ e compile em **Release x64**. A saída será gerada em `x64/Release/ghubmouse.dll`.
