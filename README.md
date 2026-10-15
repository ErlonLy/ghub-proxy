# ghub-proxy

[![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue.svg)](https://github.com/ErlonLy/ghub-proxy)
[![Language](https://img.shields.io/badge/language-C%20%2F%20C%2B%2B-00599C.svg)](https://github.com/ErlonLy/ghub-proxy)
[![Release](https://img.shields.io/github/v/release/ErlonLy/ghub-proxy?include_prereleases&color=brightgreen)](https://github.com/ErlonLy/ghub-proxy/releases)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

DLL em C (x64) de alta performance para envio de comandos de mouse diretamente via driver de kernel virtual da Logitech (**G HUB / LGS**).

Permite movimentação relativa e absoluta, cliques e scroll enviando IOCTLs diretamente para o barramento do driver de kernel (`logi_joy_bus_enum.x64.sys`), garantindo que os eventos de mouse entrem na pilha do Windows como **entradas de hardware genuínas** (`MSLLHOOKSTRUCT.flags = 0x00`, `LLMHF_INJECTED = 0`).

---

## 🚀 Download da DLL Pronta

Acesse a aba de **[Releases](https://github.com/ErlonLy/ghub-proxy/releases)** para baixar a versão compilada (`ghubmouse.dll`) pronta para uso.

---

## 🔬 Engenharia Reversa (Descobertas do Driver G HUB)

Através da desmontagem e análise do driver de kernel `logi_joy_bus_enum.x64.sys`:
1. **GUID Correto**: O barramento do G HUB expõe o GUID `{dfbedcdd-2148-416d-9e4d-cecc2424128c}` (com `cdd`, não `cdb`). O GUID `{1abc...}` pertence ao `logi_joy_xlcore` (driver de volante/force feedback).
2. **Criação do PDO do Mouse**: O driver não inicializa o PDO do mouse virtual até que receba o IOCTL `0x2A2040` com payload `(8, 'm')`. A DLL agora envia isso automaticamente na inicialização, registrando o mouse virtual `LGHUBDevice\VID_046D&PID_C231`.
3. **Estrutura do Pacote HID**: O descritor HID do mouse virtual define:
   - 16 bits para máscara de botões (`WORD button`)
   - 16 bits signed para X relativo (`short dx`)
   - 16 bits signed para Y relativo (`short dy`)
   - 8 bits signed para Scroll Vertical (`char wheel`)
   - 8 bits signed para Scroll Horizontal / Pan (`char pan`)
   Tamanho total do pacote: **8 bytes**.
4. **Resolução Dinâmica**: Utiliza `CM_Get_Device_Interface_ListW` (da `cfgmgr32.dll`) para localizar dinamicamente a interface do driver em qualquer número de instância (`ROOT#SYSTEM#...`), com fallback de varredura.
5. **Indetectabilidade**: Como a injeção ocorre em nível de driver de barramento, os eventos são despachados diretamente pelo subsistema HID. Hooks de baixo nível (`WH_MOUSE_LL`) registram `LLMHF_INJECTED = False`, contornando bloqueios de `SendInput` ou `mouse_event`.

---

## 📋 Requisitos

- Windows 10/11 x64
- **Logitech G HUB** instalado e em execução em segundo plano

---

## 📚 Tutoriais de Implementação e Uso

Documentação completa passo a passo, wrappers prontos e exemplos práticos para sua linguagem:

| Linguagem | Documentação e Guia Completo | Método de Importação |
| :--- | :--- | :--- |
| **C# (.NET)** | 👉 [**Guia de Uso em C# (.NET)**](docs/csharp.md) | `[DllImport("ghubmouse.dll")]` (P/Invoke x64) |
| **C / C++** | 👉 [**Guia de Uso em C / C++**](docs/cpp.md) | `LoadLibraryA` ou `exports.h` + `.lib` |
| **Python** | 👉 [**Guia de Uso em Python**](docs/python.md) | `ctypes.windll.LoadLibrary` (Python 64-bit) |

---

## ⚡ Como Importar e Usar nas 3 Linguagens

### 🔹 C# (.NET)
```csharp
using System;
using System.Runtime.InteropServices;

class Program {
    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern bool mouse_open();

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern void mouse_close();

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int moveR(int dx, int dy);

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int mouse_click(ushort button, uint delayMs);

    static void Main() {
        if (!mouse_open()) return;
        moveR(100, 50);          // Move relativamente
        mouse_click(1, 25);      // Clique esquerdo (1) com 25ms delay
        mouse_close();
    }
}
```
> Veja o [Guia Completo de C# com Wrapper IDisposable](docs/csharp.md).

### 🔹 C / C++
```cpp
#include <windows.h>

typedef BOOL(__stdcall* pfn_mouse_open)(void);
typedef void(__stdcall* pfn_mouse_close)(void);
typedef NTSTATUS(__stdcall* pfn_moveR)(int, int);
typedef NTSTATUS(__stdcall* pfn_mouse_click)(WORD, DWORD);

int main() {
    HMODULE dll = LoadLibraryA("ghubmouse.dll");
    if (!dll) return 1;

    auto mouse_open  = (pfn_mouse_open)GetProcAddress(dll, "mouse_open");
    auto mouse_close = (pfn_mouse_close)GetProcAddress(dll, "mouse_close");
    auto moveR       = (pfn_moveR)GetProcAddress(dll, "moveR");
    auto mouse_click = (pfn_mouse_click)GetProcAddress(dll, "mouse_click");

    if (mouse_open()) {
        moveR(100, 50);      // Move relativamente
        mouse_click(1, 25);  // Clique esquerdo (1) com 25ms delay
        mouse_close();
    }
    FreeLibrary(dll);
    return 0;
}
```
> Veja o [Guia Completo de C++ com Classe RAII](docs/cpp.md).

### 🔹 Python
```python
import ctypes
import time

ghub = ctypes.windll.LoadLibrary("ghubmouse.dll")

ghub.mouse_open.restype = ctypes.c_bool
ghub.mouse_close.restype = None
ghub.moveR.argtypes = [ctypes.c_int, ctypes.c_int]
ghub.mouse_click.argtypes = [ctypes.c_ushort, ctypes.c_ulong]

if ghub.mouse_open():
    ghub.moveR(100, 50)      # Move relativamente
    ghub.mouse_click(1, 25)  # Clique esquerdo (1) com 25ms delay
    ghub.mouse_close()
```
> Veja o [Guia Completo de Python com Context Manager](docs/python.md) e o script [`example.py`](example.py).

---

## 🛠️ Funções Exportadas

```c
// Conexão e Ciclo de Vida
BOOL      __stdcall mouse_open(void);
void      __stdcall mouse_close(void);

// Movimento
NTSTATUS  __stdcall moveR(int dx, int dy);
NTSTATUS  __stdcall move_relative(int dx, int dy);
NTSTATUS  __stdcall moveA(int x, int y);
NTSTATUS  __stdcall move_absolute(int x, int y);
BOOL      __stdcall set_cursor_pos(int x, int y);

// Botões
NTSTATUS  __stdcall press(WORD button);
NTSTATUS  __stdcall release(void);
NTSTATUS  __stdcall release_button(WORD button);
NTSTATUS  __stdcall mouse_down(WORD button);
NTSTATUS  __stdcall mouse_up(WORD button);
NTSTATUS  __stdcall mouse_click(WORD button, DWORD delay_ms);

// Scroll
NTSTATUS  __stdcall scroll(char wheel);
NTSTATUS  __stdcall scroll_h(char pan);

// Proxy Universal (Drop-in replacements para Windows APIs)
void      __stdcall ghub_mouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
BOOL      __stdcall ghub_SetCursorPos(int X, int Y);
```

### Máscara de Botões

| Código | Botão |
| :--- | :--- |
| `1` | Botão Esquerdo |
| `2` | Botão Direito |
| `4` | Botão do Meio |
| `8` | Lateral Traseiro (Mouse 4) |
| `16` | Lateral Dianteiro (Mouse 5) |

---

## 🔨 Compilação a partir do Código-Fonte

Abra `ghubmouse.slnx` ou `ghubmouse.vcxproj` no Visual Studio 2022+ e compile em **Release x64**.

Ou via linha de comando (PowerShell):
```powershell
MSBuild.exe ghubmouse.vcxproj /p:Configuration=Release /p:Platform=x64
```
A DLL será gerada em `x64/Release/ghubmouse.dll`.

---

## 📄 Licença

Distribuído sob a licença MIT. Consulte [`LICENSE`](LICENSE) para mais detalhes.
