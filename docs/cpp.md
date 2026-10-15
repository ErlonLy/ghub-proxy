# Guia de Implementação e Uso em C / C++

Este guia demonstra como importar e utilizar a biblioteca `ghubmouse.dll` em projetos C e C++ (MSVC, MinGW-w64, Clang-cl) no Windows x64.

---

## 📌 Sumário
1. [Visão Geral dos Métodos de Importação](#-visão-geral-dos-métodos-de-importação)
2. [Método 1: Carregamento Dinâmico em Tempo de Execução (Recomendado)](#-método-1-carregamento-dinâmico-em-tempo-de-execução-recomendado)
3. [Método 2: Vinculação em Tempo de Compilação (com exports.h e .lib)](#-método-2-vinculação-em-tempo-de-compilação-com-exportsh-e-lib)
4. [Wrapper RAII Moderno em C++ (`GHubMouse.hpp`)](#-wrapper-raii-moderno-em-c-ghubmousehpp)
5. [Exemplo Completo de Uso](#-exemplo-completo-de-uso)
6. [Drop-in Replacement para APIs do Windows (`mouse_event`)](#-drop-in-replacement-para-apis-do-windows-mouse_event)
7. [Como Compilar](#-como-compilar)

---

## 🔍 Visão Geral dos Métodos de Importação

Existem duas formas principais de consumir a `ghubmouse.dll` em C++:

| Método | Vantagens | Quando Usar |
| :--- | :--- | :--- |
| **Carregamento Dinâmico (`LoadLibrary` / `GetProcAddress`)** | Não exige o arquivo `.lib` na compilação. Falhas de ausência da DLL podem ser tratadas graciosamente em runtime. | Projetos portáteis, plugins, cheats, loaders ou ferramentas que distribuem a DLL dinamicamente. |
| **Vinculação com Arquivo `.lib` (`exports.h` + `#pragma comment`)** | Chamadas diretas de função sem boilerplate de ponteiros. Verificação estática de tipos pelo compilador. | Projetos que compilam a DLL junto na mesma solution do Visual Studio. |

---

## 🚀 Método 1: Carregamento Dinâmico em Tempo de Execução (Recomendado)

Neste método, você apenas precisa do binário compilado `ghubmouse.dll` na mesma pasta do seu executável.

```cpp
#include <windows.h>
#include <iostream>

// Definições de ponteiros de função compatíveis com a convenção __stdcall da DLL
typedef BOOL(__stdcall* pfn_mouse_open)(void);
typedef void(__stdcall* pfn_mouse_close)(void);
typedef NTSTATUS(__stdcall* pfn_moveR)(int dx, int dy);
typedef NTSTATUS(__stdcall* pfn_moveA)(int x, int y);
typedef BOOL(__stdcall* pfn_set_cursor_pos)(int x, int y);
typedef NTSTATUS(__stdcall* pfn_press)(WORD button);
typedef NTSTATUS(__stdcall* pfn_release)(void);
typedef NTSTATUS(__stdcall* pfn_release_button)(WORD button);
typedef NTSTATUS(__stdcall* pfn_mouse_down)(WORD button);
typedef NTSTATUS(__stdcall* pfn_mouse_up)(WORD button);
typedef NTSTATUS(__stdcall* pfn_mouse_click)(WORD button, DWORD delay_ms);
typedef NTSTATUS(__stdcall* pfn_scroll)(char wheel);
typedef NTSTATUS(__stdcall* pfn_scroll_h)(char pan);

int main() {
    // 1. Carrega o módulo da DLL em x64
    HMODULE hDll = LoadLibraryA("ghubmouse.dll");
    if (!hDll) {
        std::cerr << "[-] Erro ao carregar ghubmouse.dll (Cod: " << GetLastError() << ")\n";
        std::cerr << "[-] Certifique-se de que a DLL esta na mesma pasta do .exe e seu projeto e x64.\n";
        return 1;
    }

    // 2. Resolve os endereços das funções
    auto mouse_open  = (pfn_mouse_open)GetProcAddress(hDll, "mouse_open");
    auto mouse_close = (pfn_mouse_close)GetProcAddress(hDll, "mouse_close");
    auto moveR       = (pfn_moveR)GetProcAddress(hDll, "moveR");
    auto moveA       = (pfn_moveA)GetProcAddress(hDll, "moveA");
    auto mouse_click = (pfn_mouse_click)GetProcAddress(hDll, "mouse_click");
    auto scroll      = (pfn_scroll)GetProcAddress(hDll, "scroll");

    if (!mouse_open || !mouse_close || !moveR || !mouse_click) {
        std::cerr << "[-] Falha ao resolver funcoes exportadas na DLL.\n";
        FreeLibrary(hDll);
        return 1;
    }

    // 3. Inicializa comunicação com o kernel driver do G HUB
    if (!mouse_open()) {
        std::cerr << "[-] Falha ao inicializar mouse_open(). O Logitech G HUB esta rodando?\n";
        FreeLibrary(hDll);
        return 1;
    }

    std::cout << "[+] Conectado ao driver Logitech G HUB via C++!\n";

    // 4. Executa comandos
    moveR(100, 50);          // Move 100px para direita, 50px para baixo
    mouse_click(1, 30);      // Clique esquerdo (1) com 30ms de duracao
    scroll(-2);              // Scroll vertical para baixo

    // 5. Encerramento
    mouse_close();
    FreeLibrary(hDll);
    return 0;
}
```

---

## 🔗 Método 2: Vinculação em Tempo de Compilação (com exports.h e .lib)

Se você possui a biblioteca de importação `ghubmouse.lib` gerada na compilação:

1. Inclua o cabeçalho [`exports.h`](../exports.h) no seu código.
2. Adicione `#pragma comment(lib, "ghubmouse.lib")` ou configure as dependências adicionais no linker do Visual Studio.

```cpp
#include <windows.h>
#include <iostream>
#include "exports.h"

#pragma comment(lib, "ghubmouse.lib")

int main() {
    if (!mouse_open()) {
        std::cerr << "[-] Erro ao conectar ao driver Logitech.\n";
        return 1;
    }

    // Chamadas diretas
    moveR(100, 100);
    mouse_click(1, 25);
    mouse_close();

    return 0;
}
```

---

## 🛡️ Wrapper RAII Moderno em C++ (`GHubMouse.hpp`)

Para evitar vazamentos de recursos e chamadas manuais repetitivas de `mouse_close` e `FreeLibrary`, crie este header moderno em C++17+:

```cpp
#pragma once
#include <windows.h>
#include <stdexcept>
#include <string>

enum class GHubButton : WORD {
    Left    = 1,
    Right   = 2,
    Middle  = 4,
    Mouse4  = 8,
    Mouse5  = 16
};

class GHubMouse {
private:
    HMODULE m_hDll = nullptr;
    bool m_opened = false;

    typedef BOOL(__stdcall* pfn_mouse_open)(void);
    typedef void(__stdcall* pfn_mouse_close)(void);
    typedef NTSTATUS(__stdcall* pfn_moveR)(int, int);
    typedef NTSTATUS(__stdcall* pfn_moveA)(int, int);
    typedef BOOL(__stdcall* pfn_set_cursor_pos)(int, int);
    typedef NTSTATUS(__stdcall* pfn_press)(WORD);
    typedef NTSTATUS(__stdcall* pfn_release)(void);
    typedef NTSTATUS(__stdcall* pfn_release_button)(WORD);
    typedef NTSTATUS(__stdcall* pfn_mouse_down)(WORD);
    typedef NTSTATUS(__stdcall* pfn_mouse_up)(WORD);
    typedef NTSTATUS(__stdcall* pfn_mouse_click)(WORD, DWORD);
    typedef NTSTATUS(__stdcall* pfn_scroll)(char);
    typedef NTSTATUS(__stdcall* pfn_scroll_h)(char);

    pfn_mouse_open       m_mouse_open = nullptr;
    pfn_mouse_close      m_mouse_close = nullptr;
    pfn_moveR            m_moveR = nullptr;
    pfn_moveA            m_moveA = nullptr;
    pfn_set_cursor_pos   m_set_cursor_pos = nullptr;
    pfn_press            m_press = nullptr;
    pfn_release          m_release = nullptr;
    pfn_release_button   m_release_button = nullptr;
    pfn_mouse_down       m_mouse_down = nullptr;
    pfn_mouse_up         m_mouse_up = nullptr;
    pfn_mouse_click      m_mouse_click = nullptr;
    pfn_scroll           m_scroll = nullptr;
    pfn_scroll_h         m_scroll_h = nullptr;

    template <typename T>
    T LoadSymbol(const char* name) {
        FARPROC proc = GetProcAddress(m_hDll, name);
        if (!proc) {
            throw std::runtime_error(std::string("Simbolo nao encontrado na DLL: ") + name);
        }
        return reinterpret_cast<T>(proc);
    }

public:
    explicit GHubMouse(const char* dllPath = "ghubmouse.dll") {
        m_hDll = LoadLibraryA(dllPath);
        if (!m_hDll) {
            throw std::runtime_error("Nao foi possivel carregar ghubmouse.dll");
        }

        m_mouse_open       = LoadSymbol<pfn_mouse_open>("mouse_open");
        m_mouse_close      = LoadSymbol<pfn_mouse_close>("mouse_close");
        m_moveR            = LoadSymbol<pfn_moveR>("moveR");
        m_moveA            = LoadSymbol<pfn_moveA>("moveA");
        m_set_cursor_pos   = LoadSymbol<pfn_set_cursor_pos>("set_cursor_pos");
        m_press            = LoadSymbol<pfn_press>("press");
        m_release          = LoadSymbol<pfn_release>("release");
        m_release_button   = LoadSymbol<pfn_release_button>("release_button");
        m_mouse_down       = LoadSymbol<pfn_mouse_down>("mouse_down");
        m_mouse_up         = LoadSymbol<pfn_mouse_up>("mouse_up");
        m_mouse_click      = LoadSymbol<pfn_mouse_click>("mouse_click");
        m_scroll           = LoadSymbol<pfn_scroll>("scroll");
        m_scroll_h         = LoadSymbol<pfn_scroll_h>("scroll_h");

        if (!m_mouse_open()) {
            FreeLibrary(m_hDll);
            throw std::runtime_error("mouse_open() falhou. O driver do G HUB esta acessivel?");
        }
        m_opened = true;
    }

    ~GHubMouse() {
        Close();
    }

    // Move relativo fracionado e linearmente proporcional
    NTSTATUS MoveRelative(int dx, int dy) {
        return m_moveR(dx, dy);
    }

    // Move absoluto calculando a diferenca para a posicao atual
    NTSTATUS MoveAbsolute(int x, int y) {
        return m_moveA(x, y);
    }

    // Define posicao do cursor diretamente
    BOOL SetCursorPosition(int x, int y) {
        return m_set_cursor_pos(x, y);
    }

    // Clique completo com delay em ms
    NTSTATUS Click(GHubButton btn = GHubButton::Left, DWORD delayMs = 25) {
        return m_mouse_click(static_cast<WORD>(btn), delayMs);
    }

    // Segurar botao
    NTSTATUS MouseDown(GHubButton btn) {
        return m_mouse_down(static_cast<WORD>(btn));
    }

    // Soltar botao especifico
    NTSTATUS MouseUp(GHubButton btn) {
        return m_mouse_up(static_cast<WORD>(btn));
    }

    // Soltar todos os botoes
    NTSTATUS ReleaseAll() {
        return m_release();
    }

    // Scroll Vertical (+ cima, - baixo)
    NTSTATUS Scroll(char steps) {
        return m_scroll(steps);
    }

    // Scroll Horizontal (+ direita, - esquerda)
    NTSTATUS ScrollH(char steps) {
        return m_scroll_h(steps);
    }

    void Close() {
        if (m_opened && m_mouse_close) {
            m_mouse_close();
            m_opened = false;
        }
        if (m_hDll) {
            FreeLibrary(m_hDll);
            m_hDll = nullptr;
        }
    }
};
```

---

## 💻 Exemplo Completo de Uso

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include "GHubMouse.hpp"

int main() {
    try {
        std::cout << "[*] Conectando ao driver Logitech G HUB...\n";
        GHubMouse mouse;
        std::cout << "[+] Conexao estabelecida com sucesso!\n";

        // 1. Movimentacao Relativa
        std::cout << "[*] Movimento relativo (+150, +75)...\n";
        mouse.MoveRelative(150, 75);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 2. Movimentacao Absoluta
        std::cout << "[*] Movimento absoluto para coordenadas (500, 500)...\n";
        mouse.MoveAbsolute(500, 500);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 3. Clique do Mouse (Botao Esquerdo com 30ms de pressao)
        std::cout << "[*] Clique esquerdo...\n";
        mouse.Click(GHubButton::Left, 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 4. Scroll Vertical
        std::cout << "[*] Rolando scroll para baixo (-3)...\n";
        mouse.Scroll(-3);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "[+] Testes concluidos. Liberando recursos...\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "[-] Erro: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
```

---

## 🔄 Drop-in Replacement para APIs do Windows (`mouse_event`)

Se o seu código legado usa `mouse_event` da `user32.dll`, a biblioteca exporta `ghub_mouse_event` e `ghub_SetCursorPos`, que aceitam exatamente os mesmos parâmetros e flags do Windows (`MOUSEEVENTF_MOVE`, `MOUSEEVENTF_LEFTDOWN`, etc.), porém com despacho a nível de kernel:

```cpp
typedef void(__stdcall* pfn_ghub_mouse_event)(DWORD, DWORD, DWORD, DWORD, ULONG_PTR);
auto ghub_mouse_event = (pfn_ghub_mouse_event)GetProcAddress(hDll, "ghub_mouse_event");

// Substitui diretamente chamadas legadas:
ghub_mouse_event(MOUSEEVENTF_MOVE, 100, 50, 0, 0);
ghub_mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
Sleep(30);
ghub_mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
```

---

## 🔨 Como Compilar

### Com MSVC (Linha de Comando)
Certifique-se de usar o **x64 Native Tools Command Prompt for VS 2022**:
```cmd
cl.exe /EHsc /std:c++17 main.cpp /Fe:app.exe
```

### Com CMake (`CMakeLists.txt`)
```cmake
cmake_minimum_required(VERSION 3.15)
project(GHubExample CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(GHubExample main.cpp GHubMouse.hpp)

# Copia a DLL automaticamente para a pasta de saida do executavel
add_custom_command(TARGET GHubExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/ghubmouse.dll"
    $<TARGET_FILE_DIR:GHubExample>
)
```
