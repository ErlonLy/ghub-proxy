# Guia de Implementação e Uso em Python

Este guia detalha como importar e utilizar a biblioteca `ghubmouse.dll` em Python 3 através do módulo nativo `ctypes`, enviando comandos de mouse com nível de hardware via driver de kernel do Logitech G HUB.

---

## 📌 Sumário
1. [Pré-requisitos e Verificação de Ambiente](#-pré-requisitos-e-verificação-de-ambiente)
2. [Como Importar a DLL via `ctypes`](#-como-importar-a-dll-via-ctypes)
3. [Definições de Tipos e Assinaturas](#-definições-de-tipos-e-assinaturas)
4. [Classe Wrapper Pythonica (`ghub_mouse.py`)](#-classe-wrapper-pythonica-ghub_mousepy)
5. [Exemplos Práticos de Uso](#-exemplos-práticos-de-uso)
   - [Uso Básico com Context Manager](#uso-básico-com-context-manager)
   - [Movimento Suave / Humanizado (Interpolação Linear)](#movimento-suave--humanizado-interpolação-linear)
   - [Automação de Cliques e Macros](#automação-de-cliques-e-macros)
6. [Tabela de Códigos e Botões](#-tabela-de-códigos-e-botões)
7. [Resolução de Problemas](#-resolução-de-problemas)

---

## ⚙️ Pré-requisitos e Verificação de Ambiente

1. **Logitech G HUB** instalado e em execução no Windows.
2. **Python 64-bit Obrigatório**:
   Como a DLL foi compilada para arquitetura `x64`, seu interpretador Python **deve** ser 64-bit. Verifique no terminal:
   ```bash
   python -c "import struct; print(struct.calcsize('P') * 8)"
   ```
   *Se o resultado for `64`, seu ambiente está correto. Se for `32`, instale a versão 64-bit do Python.*
3. **Arquivo da DLL**:
   Certifique-se de que `ghubmouse.dll` esteja na mesma pasta do seu script `.py` ou no `PATH`.

---

## 🔌 Como Importar a DLL via `ctypes`

No Windows, funções compiladas com `__stdcall` devem ser carregadas utilizando `ctypes.windll.LoadLibrary` ou instanciadas através de `ctypes.WinDLL`.

```python
import ctypes
import os

dll_path = os.path.abspath("ghubmouse.dll")
ghub = ctypes.windll.LoadLibrary(dll_path)
```

---

## 📐 Definições de Tipos e Assinaturas

Configurar `argtypes` e `restype` para cada função exportada evita corrupção de memória e garante que os tipos numéricos (como `short`, `int` e `NTSTATUS`) sejam convertidos corretamente:

```python
import ctypes

# 1. Ciclo de Vida
ghub.mouse_open.restype = ctypes.c_bool
ghub.mouse_open.argtypes = []

ghub.mouse_close.restype = None
ghub.mouse_close.argtypes = []

# 2. Movimento
ghub.moveR.restype = ctypes.c_long
ghub.moveR.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.move_relative.restype = ctypes.c_long
ghub.move_relative.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.moveA.restype = ctypes.c_long
ghub.moveA.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.move_absolute.restype = ctypes.c_long
ghub.move_absolute.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.set_cursor_pos.restype = ctypes.c_bool
ghub.set_cursor_pos.argtypes = [ctypes.c_int, ctypes.c_int]

# 3. Botões e Cliques
ghub.press.restype = ctypes.c_long
ghub.press.argtypes = [ctypes.c_ushort]

ghub.release.restype = ctypes.c_long
ghub.release.argtypes = []

ghub.release_button.restype = ctypes.c_long
ghub.release_button.argtypes = [ctypes.c_ushort]

ghub.mouse_down.restype = ctypes.c_long
ghub.mouse_down.argtypes = [ctypes.c_ushort]

ghub.mouse_up.restype = ctypes.c_long
ghub.mouse_up.argtypes = [ctypes.c_ushort]

ghub.mouse_click.restype = ctypes.c_long
ghub.mouse_click.argtypes = [ctypes.c_ushort, ctypes.c_ulong]

# 4. Scroll
ghub.scroll.restype = ctypes.c_long
ghub.scroll.argtypes = [ctypes.c_byte]

ghub.scroll_h.restype = ctypes.c_long
ghub.scroll_h.argtypes = [ctypes.c_byte]
```

---

## 🐍 Classe Wrapper Pythonica (`ghub_mouse.py`)

Abaixo está uma classe pronta e segura que implementa Context Manager (`with`), permitindo gerenciar automaticamente o ciclo de vida da conexão com o driver:

```python
"""
Wrapper de alto nível para Logitech G HUB Kernel Mouse Proxy
"""

import ctypes
import os
import time
from enum import IntEnum

class MouseButton(IntEnum):
    LEFT   = 1
    RIGHT  = 2
    MIDDLE = 4
    X1     = 8   # Mouse 4
    X2     = 16  # Mouse 5

class GHubMouse:
    def __init__(self, dll_path="ghubmouse.dll"):
        # Tenta localizar a DLL no caminho informado ou relativo
        candidates = [
            dll_path,
            os.path.join(os.path.dirname(__file__), dll_path),
            os.path.join(os.path.dirname(__file__), "x64", "Release", dll_path)
        ]
        chosen = next((p for p in candidates if os.path.exists(p)), dll_path)
        
        self.dll = ctypes.windll.LoadLibrary(chosen)
        self._setup_bindings()
        self._connected = False

    def _setup_bindings(self):
        self.dll.mouse_open.restype = ctypes.c_bool
        self.dll.mouse_open.argtypes = []

        self.dll.mouse_close.restype = None
        self.dll.mouse_close.argtypes = []

        self.dll.moveR.restype = ctypes.c_long
        self.dll.moveR.argtypes = [ctypes.c_int, ctypes.c_int]

        self.dll.moveA.restype = ctypes.c_long
        self.dll.moveA.argtypes = [ctypes.c_int, ctypes.c_int]

        self.dll.set_cursor_pos.restype = ctypes.c_bool
        self.dll.set_cursor_pos.argtypes = [ctypes.c_int, ctypes.c_int]

        self.dll.press.restype = ctypes.c_long
        self.dll.press.argtypes = [ctypes.c_ushort]

        self.dll.release.restype = ctypes.c_long
        self.dll.release.argtypes = []

        self.dll.mouse_down.restype = ctypes.c_long
        self.dll.mouse_down.argtypes = [ctypes.c_ushort]

        self.dll.mouse_up.restype = ctypes.c_long
        self.dll.mouse_up.argtypes = [ctypes.c_ushort]

        self.dll.mouse_click.restype = ctypes.c_long
        self.dll.mouse_click.argtypes = [ctypes.c_ushort, ctypes.c_ulong]

        self.dll.scroll.restype = ctypes.c_long
        self.dll.scroll.argtypes = [ctypes.c_byte]

        self.dll.scroll_h.restype = ctypes.c_long
        self.dll.scroll_h.argtypes = [ctypes.c_byte]

    def open(self):
        if self._connected:
            return True
        if not self.dll.mouse_open():
            raise RuntimeError(
                "Falha ao conectar ao driver Logitech G HUB. "
                "Verifique se o G HUB esta aberto e rodando em segundo plano."
            )
        self._connected = True
        return True

    def close(self):
        if self._connected:
            self.dll.mouse_close()
            self._connected = False

    def move_rel(self, dx: int, dy: int):
        """Move o mouse relativamente (dx, dy)."""
        return self.dll.moveR(int(dx), int(dy))

    def move_abs(self, x: int, y: int):
        """Move o mouse para coordenadas absolutas (x, y) na tela."""
        return self.dll.moveA(int(x), int(y))

    def set_cursor_pos(self, x: int, y: int):
        """Define posicao do cursor diretamente."""
        return self.dll.set_cursor_pos(int(x), int(y))

    def press(self, button: MouseButton = MouseButton.LEFT):
        """Segura o botão indicado."""
        return self.dll.mouse_down(int(button))

    def release(self, button: MouseButton = MouseButton.LEFT):
        """Solta o botão indicado."""
        return self.dll.mouse_up(int(button))

    def release_all(self):
        """Solta todos os botões."""
        return self.dll.release()

    def click(self, button: MouseButton = MouseButton.LEFT, delay_ms: int = 25):
        """Executa clique completo com delay entre down e up."""
        return self.dll.mouse_click(int(button), int(delay_ms))

    def scroll(self, steps: int):
        """Scroll vertical (+ para cima, - para baixo)."""
        return self.dll.scroll(int(steps))

    def scroll_h(self, steps: int):
        """Scroll horizontal (+ para direita, - para esquerda)."""
        return self.dll.scroll_h(int(steps))

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
```

---

## 💡 Exemplos Práticos de Uso

### Uso Básico com Context Manager

```python
import time
from ghub_mouse import GHubMouse, MouseButton

with GHubMouse() as mouse:
    print("[+] Conectado ao driver!")

    # Movimenta relativamente 150px p/ direita e 80px p/ baixo
    mouse.move_rel(150, 80)
    time.sleep(0.1)

    # Executa clique esquerdo
    mouse.click(MouseButton.LEFT, delay_ms=30)
    time.sleep(0.1)

    # Move para posição absoluta (X=500, Y=500)
    mouse.move_abs(500, 500)
    time.sleep(0.1)

    # Scroll para baixo
    mouse.scroll(-3)
```

### Movimento Suave / Humanizado (Interpolação Linear)

Para mira suave ou automações que exigem traçado contínuo:

```python
import time
from ghub_mouse import GHubMouse

def smooth_move(mouse: GHubMouse, target_dx: int, target_dy: int, steps: int = 20, delay: float = 0.005):
    step_x = target_dx / steps
    step_y = target_dy / steps

    acc_x = 0.0
    acc_y = 0.0

    for _ in range(steps):
        acc_x += step_x
        acc_y += step_y
        
        move_x = int(acc_x)
        move_y = int(acc_y)
        
        if move_x != 0 or move_y != 0:
            mouse.move_rel(move_x, move_y)
            acc_x -= move_x
            acc_y -= move_y
        
        time.sleep(delay)

with GHubMouse() as mouse:
    # Movimento suave de 300 pixels para o lado
    smooth_move(mouse, 300, 0, steps=30, delay=0.004)
```

---

## 📊 Tabela de Códigos e Botões

| Constante | Valor Numérico | Descrição |
| :--- | :--- | :--- |
| `MouseButton.LEFT` | `1` | Botão Esquerdo |
| `MouseButton.RIGHT` | `2` | Botão Direito |
| `MouseButton.MIDDLE` | `4` | Botão do Scroll (Meio) |
| `MouseButton.X1` | `8` | Botão Lateral Traseiro (Mouse 4) |
| `MouseButton.X2` | `16` | Botão Lateral Dianteiro (Mouse 5) |

---

## ❓ Resolução de Problemas

| Problema | Causa | Solução |
| :--- | :--- | :--- |
| `OSError: [WinError 193] %1 não é um aplicativo Win32 válido` | Você está executando um interpretador Python 32-bit com uma DLL 64-bit. | Instale e execute o script com **Python 64-bit**. |
| `FileNotFoundError: Could not find module 'ghubmouse.dll'` | O Python não encontrou o arquivo no diretório atual. | Passe o caminho absoluto para `LoadLibrary` ou coloque a DLL na mesma pasta do script. |
| `RuntimeError: Falha ao conectar ao driver Logitech G HUB` | O aplicativo Logitech G HUB não está aberto ou os processos do driver não responderam ao IOCTL. | Inicie o aplicativo **Logitech G HUB** e aguarde ele carregar os perfis antes de rodar o script. |
