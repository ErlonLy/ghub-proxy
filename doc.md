# GHub Mouse DLL Wrapper

Interface para o driver de mouse virtual do Logitech G Hub e LGS. Comunicação direta em kernel-mode via IOCTL.

## Requisitos

1. G Hub ou LGS rodando no sistema.
2. Privilégios de Administrador no processo que carregar a DLL.
3. Arquitetura x64.

## Funções Exportadas (__stdcall)

### mouse_open
Abre a comunicação com o driver. Faz a varredura automática dos caminhos de dispositivo.
```c
BOOL __stdcall mouse_open(void);
```
Retorna `TRUE` em caso de sucesso.

### mouse_close
Encerra a conexão e reseta todos os botões.
```c
void __stdcall mouse_close(void);
```

### moveR
Movimenta o cursor relativamente. Valores maiores que 127 são divididos em passos mantendo proporção linear nos eixos X e Y.
```c
NTSTATUS __stdcall moveR(int dx, int dy);
```

### press / mouse_down
Pressiona e segura o botão informado. Preserva outros botões que já estejam pressionados.
```c
NTSTATUS __stdcall press(BYTE button);
NTSTATUS __stdcall mouse_down(BYTE button);
```

### release
Solta todos os botões que estiverem ativos.
```c
NTSTATUS __stdcall release(void);
```

### release_button / mouse_up
Solta exclusivamente o botão informado.
```c
NTSTATUS __stdcall release_button(BYTE button);
NTSTATUS __stdcall mouse_up(BYTE button);
```

### mouse_click
Executa clique completo com delay configurável em milissegundos.
```c
NTSTATUS __stdcall mouse_click(BYTE button, DWORD delay_ms);
```

### scroll
Movimenta o scroll vertical. Positivo sobe, negativo desce.
```c
NTSTATUS __stdcall scroll(char wheel);
```

## Códigos de Botão

- `1`: Botão Esquerdo
- `2`: Botão Direito
- `4`: Botão do Meio
- `8`: Mouse 4 (Lateral Traseiro)
- `16`: Mouse 5 (Lateral Dianteiro)

## Exemplos de Integração

### Python (ctypes)

```python
import ctypes
import time

ghub = ctypes.windll.LoadLibrary("ghubmouse.dll")

ghub.mouse_open.restype = ctypes.c_bool
ghub.mouse_open.argtypes = []

ghub.mouse_close.restype = None
ghub.mouse_close.argtypes = []

ghub.moveR.restype = ctypes.c_long
ghub.moveR.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.press.restype = ctypes.c_long
ghub.press.argtypes = [ctypes.c_ubyte]

ghub.release.restype = ctypes.c_long
ghub.release.argtypes = []

ghub.mouse_click.restype = ctypes.c_long
ghub.mouse_click.argtypes = [ctypes.c_ubyte, ctypes.c_ulong]

ghub.scroll.restype = ctypes.c_long
ghub.scroll.argtypes = [ctypes.c_byte]

if not ghub.mouse_open():
    print("Falha ao abrir driver. Execute como Administrador.")
    exit(1)

# Movimento diagonal proporcional
ghub.moveR(200, 100)
time.sleep(0.05)

# Clique esquerdo com delay de 20ms
ghub.mouse_click(1, 20)

# Scroll para baixo
ghub.scroll(-3)

ghub.mouse_close()
```

### C++

```cpp
#include <windows.h>
#include <iostream>

typedef BOOL(__stdcall* pfn_mouse_open)(void);
typedef void(__stdcall* pfn_mouse_close)(void);
typedef NTSTATUS(__stdcall* pfn_moveR)(int, int);
typedef NTSTATUS(__stdcall* pfn_mouse_click)(BYTE, DWORD);
typedef NTSTATUS(__stdcall* pfn_scroll)(char);

int main() {
    HMODULE dll = LoadLibraryA("ghubmouse.dll");
    if (!dll) {
        std::cerr << "Erro ao carregar ghubmouse.dll\n";
        return 1;
    }

    auto mouse_open  = (pfn_mouse_open)GetProcAddress(dll, "mouse_open");
    auto mouse_close = (pfn_mouse_close)GetProcAddress(dll, "mouse_close");
    auto moveR       = (pfn_moveR)GetProcAddress(dll, "moveR");
    auto mouse_click = (pfn_mouse_click)GetProcAddress(dll, "mouse_click");
    auto scroll      = (pfn_scroll)GetProcAddress(dll, "scroll");

    if (!mouse_open()) {
        std::cerr << "Falha ao abrir dispositivo. Execute como Administrador.\n";
        FreeLibrary(dll);
        return 1;
    }

    moveR(150, 75);
    mouse_click(1, 30);
    scroll(-2);

    mouse_close();
    FreeLibrary(dll);
    return 0;
}
```

### C#

```csharp
using System;
using System.Runtime.InteropServices;
using System.Threading;

class Program
{
    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern bool mouse_open();

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern void mouse_close();

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int moveR(int dx, int dy);

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int mouse_click(byte button, uint delay_ms);

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int scroll(sbyte wheel);

    static void Main()
    {
        if (!mouse_open())
        {
            Console.WriteLine("Erro ao abrir driver. Execute como Administrador.");
            return;
        }

        moveR(120, 60);
        mouse_click(1, 25);
        scroll(-1);

        mouse_close();
    }
}
```
