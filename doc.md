# GHub Mouse DLL Wrapper & Proxy

Interface universal para o driver de mouse virtual do Logitech G Hub e LGS com comunicação kernel-mode direta via IOCTL.

## Funções Exportadas (__stdcall)

### Conexão e Ciclo de Vida
- `BOOL __stdcall mouse_open(void)`: Abre a comunicação com o barramento do driver e garante a criação da instância do mouse virtual.
- `void __stdcall mouse_close(void)`: Libera botões pressionados e fecha o handle do dispositivo.

### Movimentação
- `NTSTATUS __stdcall moveR(int dx, int dy)`: Movimento relativo fracionado e linearmente proporcional.
- `NTSTATUS __stdcall move_relative(int dx, int dy)`: Alias para `moveR`.
- `NTSTATUS __stdcall moveA(int x, int y)`: Movimento absoluto calculando delta em relação à posição atual do cursor.
- `NTSTATUS __stdcall move_absolute(int x, int y)`: Alias para `moveA`.
- `BOOL __stdcall set_cursor_pos(int x, int y)`: Equivalente ao `SetCursorPos` do Windows, mas operando 100% via driver Logitech.

### Botões e Cliques
- `NTSTATUS __stdcall press(WORD button)` / `mouse_down(WORD button)`: Segura o botão informado.
- `NTSTATUS __stdcall release(void)`: Solta todos os botões ativos.
- `NTSTATUS __stdcall release_button(WORD button)` / `mouse_up(WORD button)`: Solta exclusivamente o botão informado.
- `NTSTATUS __stdcall mouse_click(WORD button, DWORD delay_ms)`: Executa clique completo (down + delay + up).

### Scroll
- `NTSTATUS __stdcall scroll(char wheel)`: Scroll vertical (positivo para cima, negativo para baixo).
- `NTSTATUS __stdcall scroll_h(char pan)`: Scroll horizontal / pan (positivo para direita, negativo para esquerda).

### Proxy Windows Universal
- `void __stdcall ghub_mouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)`: Drop-in replacement para `mouse_event` da `user32.dll`. Converte flags de movimento, botões e scroll diretamente para comandos do driver do G HUB.
- `BOOL __stdcall ghub_SetCursorPos(int X, int Y)`: Drop-in replacement para `SetCursorPos`.

---

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

ghub.moveA.restype = ctypes.c_long
ghub.moveA.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.press.restype = ctypes.c_long
ghub.press.argtypes = [ctypes.c_ushort]

ghub.release.restype = ctypes.c_long
ghub.release.argtypes = []

ghub.mouse_click.restype = ctypes.c_long
ghub.mouse_click.argtypes = [ctypes.c_ushort, ctypes.c_ulong]

ghub.scroll.restype = ctypes.c_long
ghub.scroll.argtypes = [ctypes.c_byte]

if not ghub.mouse_open():
    print("Falha ao abrir driver G HUB.")
    exit(1)

# Movimento relativo (X=100, Y=50)
ghub.moveR(100, 50)
time.sleep(0.02)

# Movimento absoluto (para X=800, Y=600)
ghub.moveA(800, 600)
time.sleep(0.02)

# Clique esquerdo com delay de 25ms
ghub.mouse_click(1, 25)

# Scroll para baixo
ghub.scroll(-2)

ghub.mouse_close()
```

### C++

```cpp
#include <windows.h>
#include <iostream>

typedef BOOL(__stdcall* pfn_mouse_open)(void);
typedef void(__stdcall* pfn_mouse_close)(void);
typedef NTSTATUS(__stdcall* pfn_moveR)(int, int);
typedef NTSTATUS(__stdcall* pfn_moveA)(int, int);
typedef NTSTATUS(__stdcall* pfn_mouse_click)(WORD, DWORD);
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
    auto moveA       = (pfn_moveA)GetProcAddress(dll, "moveA");
    auto mouse_click = (pfn_mouse_click)GetProcAddress(dll, "mouse_click");
    auto scroll      = (pfn_scroll)GetProcAddress(dll, "scroll");

    if (!mouse_open()) {
        std::cerr << "Falha ao abrir driver G HUB.\n";
        FreeLibrary(dll);
        return 1;
    }

    moveR(150, 75);
    moveA(500, 500);
    mouse_click(1, 30);
    scroll(-2);

    mouse_close();
    FreeLibrary(dll);
    return 0;
}
```
