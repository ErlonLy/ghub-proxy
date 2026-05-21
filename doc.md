# GHub Mouse DLL Wrapper

Esta biblioteca (DLL) fornece uma interface simples para interagir diretamente com o driver de mouse virtual do **Logitech G Hub**. Ela permite simular movimentação relativa do mouse e cliques de botões a nível de driver kernel-mode, facilitando a automação de entrada sem ser detectada por sistemas de proteção convencionais que bloqueiam APIs de user-mode (como `SendInput` ou `mouse_event`).

---

## 📋 Pré-requisitos

Para que a DLL funcione corretamente, os seguintes requisitos devem ser atendidos:

1. **Logitech G Hub Instalado**: O software oficial do Logitech G Hub deve estar instalado e rodando no sistema, pois a DLL se comunica diretamente com o driver virtual instalado por ele.
2. **Privilégios de Administrador**: O aplicativo que carrega esta DLL **precisa ser executado como Administrador**. Isso ocorre porque o acesso direto aos caminhos do dispositivo virtual no kernel (`\\??\\ROOT#SYSTEM...`) exige privilégios elevados.
3. **Arquitetura Compatível**: A DLL deve ser compilada e carregada na mesma arquitetura do processo chamador (geralmente **x64**).

---

## 🛠️ API Exportada (Convenção `__stdcall`)

A DLL expõe as seguintes funções para uso externo:

### 1. `mouse_open`
Inicializa e abre a conexão com o driver do G Hub.
```cpp
BOOL __stdcall mouse_open(void);
```
- **Retorno**: `TRUE` se a conexão com o driver foi aberta com sucesso; `FALSE` caso contrário (geralmente por falta de privilégios ou se o G Hub não estiver instalado).

### 2. `mouse_close`
Fecha a conexão ativa com o driver e libera o handle.
```cpp
void __stdcall mouse_close(void);
```

### 3. `moveR`
Move o cursor do mouse de forma relativa em relação à posição atual.
```cpp
NTSTATUS __stdcall moveR(int dx, int dy);
```
- **Parâmetros**:
  - `dx`: Distância horizontal (positivo para direita, negativo para esquerda).
  - `dy`: Distância vertical (positivo para baixo, negativo para cima).
- **Nota**: A DLL divide automaticamente movimentos grandes em passos menores (máximo de 127 por passo) para garantir a compatibilidade com o driver.
- **Retorno**: `STATUS_SUCCESS` (`0`) em caso de sucesso.

### 4. `press`
Simula o pressionamento (pressionar e segurar) de um botão do mouse.
```cpp
NTSTATUS __stdcall press(BYTE button);
```
- **Parâmetros**:
  - `button`: Máscara binária do botão. Valores comuns:
    - `1`: Botão Esquerdo
    - `2`: Botão Direito
    - `4`: Botão do Meio (Scroll)
    - `8`: Botão Lateral Traseiro (Mouse 4)
    - `16`: Botão Lateral Dianteiro (Mouse 5)
- **Retorno**: `STATUS_SUCCESS` (`0`) em caso de sucesso.

### 5. `release`
Solta todos os botões do mouse que foram pressionados anteriormente.
```cpp
NTSTATUS __stdcall release(void);
```
- **Retorno**: `STATUS_SUCCESS` (`0`) em caso de sucesso.

---

## 💻 Exemplos de Uso

Abaixo estão exemplos práticos de como usar a DLL em diferentes linguagens de programação.

### 1. Python (via `ctypes`)
Python é ideal para scripts rápidos de automação. Salve o script no mesmo diretório da DLL ou forneça o caminho completo para a DLL compiled.

```python
import ctypes
import time

# 1. Carregar a DLL (certifique-se de que a arquitetura do Python corresponda à da DLL)
try:
    ghub = ctypes.windll.LoadLibrary("ghubmouse.dll")
except Exception as e:
    print(f"Erro ao carregar DLL: {e}")
    exit(1)

# 2. Definir tipos de retorno e argumentos das funções exportadas
ghub.mouse_open.restype = ctypes.c_bool
ghub.mouse_open.argtypes = []

ghub.mouse_close.restype = None
ghub.mouse_close.argtypes = []

ghub.moveR.restype = ctypes.c_long  # NTSTATUS
ghub.moveR.argtypes = [ctypes.c_int, ctypes.c_int]

ghub.press.restype = ctypes.c_long  # NTSTATUS
ghub.press.argtypes = [ctypes.c_ubyte]

ghub.release.restype = ctypes.c_long  # NTSTATUS
ghub.release.argtypes = []

# Mapeamento de botões
MOUSE_LEFT = 1
MOUSE_RIGHT = 2

def main():
    print("Tentando conectar ao driver do Logitech G Hub...")
    # O script DEVE ser executado como Administrador
    if not ghub.mouse_open():
        print("Erro: Não foi possível abrir o dispositivo do G Hub.")
        print("Verifique se o G Hub está instalado e se você executou o console como Administrador.")
        return

    print("Conexão estabelecida com sucesso!")
    
    try:
        # Exemplo 1: Mover o mouse de forma relativa (+50 px direita, +50 px baixo)
        print("Movendo o mouse...")
        ghub.moveR(50, 50)
        time.sleep(0.5)

        # Exemplo 2: Clique no botão esquerdo (Press + Release)
        print("Pressionando botão esquerdo...")
        ghub.press(MOUSE_LEFT)
        time.sleep(0.1)
        
        print("Soltando botão...")
        ghub.release()
        time.sleep(0.5)
        
    finally:
        # Sempre feche a conexão ao finalizar
        print("Fechando conexão...")
        ghub.mouse_close()

if __name__ == "__main__":
    main()
```

### 2. C++ (Carregamento Dinâmico)
Ideal para projetos nativos ou cheats que injetam ou rodam em paralelo.

```cpp
#include <windows.h>
#include <iostream>

// Definição dos tipos dos ponteiros de função
typedef BOOL(__stdcall* pfnMouseOpen)(void);
typedef void(__stdcall* pfnMouseClose)(void);
typedef NTSTATUS(__stdcall* pfnMoveR)(int, int);
typedef NTSTATUS(__stdcall* pfnPress)(BYTE);
typedef NTSTATUS(__stdcall* pfnRelease)(void);

int main() {
    HMODULE hDll = LoadLibraryA("ghubmouse.dll");
    if (!hDll) {
        std::cerr << "Falha ao carregar ghubmouse.dll" << std::endl;
        return 1;
    }

    auto mouse_open = (pfnMouseOpen)GetProcAddress(hDll, "mouse_open");
    auto mouse_close = (pfnMouseClose)GetProcAddress(hDll, "mouse_close");
    auto moveR = (pfnMoveR)GetProcAddress(hDll, "moveR");
    auto press = (pfnPress)GetProcAddress(hDll, "press");
    auto release = (pfnRelease)GetProcAddress(hDll, "release");

    if (!mouse_open || !mouse_close || !moveR || !press || !release) {
        std::cerr << "Falha ao obter endereços das funções exportadas" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    if (mouse_open()) {
        std::cout << "Conectado ao driver do G Hub!" << std::endl;

        // Move 100 pixels para a direita
        moveR(100, 0);
        Sleep(100);

        // Clica com o botão direito (código 2)
        press(2); 
        Sleep(50);
        release();

        mouse_close();
    } else {
        std::cerr << "Falha ao abrir driver do mouse. Execute como administrador!" << std::endl;
    }

    FreeLibrary(hDll);
    return 0;
}
```

### 3. C# (P/Invoke)
Útil para interfaces em WinForms, WPF ou Unity.

```csharp
using System;
using System.Runtime.InteropServices;
using System.Threading;

class Program
{
    // Importações da DLL
    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern bool mouse_open();

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern void mouse_close();

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int moveR(int dx, int dy);

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int press(byte button);

    [DllImport("ghubmouse.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int release();

    static void Main()
    {
        Console.WriteLine("Abrindo driver do mouse...");
        if (!mouse_open())
        {
            Console.WriteLine("Erro ao conectar. O programa está executando como Administrador?");
            return;
        }

        try
        {
            // Move relativamente
            moveR(-50, -50);
            Thread.Sleep(200);

            // Botão esquerdo (1)
            press(1);
            Thread.Sleep(50);
            release();
        }
        finally
        {
            mouse_close();
            Console.WriteLine("Conexão com o driver encerrada.");
        }
    }
}
```

---

## 🛠️ Como Compilar

Este projeto usa uma solução do Visual Studio (`ghubmouse.slnx` / `ghubmouse.vcxproj`).

1. Abra o arquivo de solução no **Visual Studio 2022** ou superior.
2. Defina a configuração de compilação como **Release** e a plataforma como **x64**.
3. Compile a solução (Build -> Build Solution).
4. O arquivo `ghubmouse.dll` resultante será gerado na pasta de saída (`x64/Release/`).
