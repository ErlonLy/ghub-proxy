# Guia de Implementação e Uso em C# (.NET)

Este guia detalha como importar e utilizar a biblioteca `ghubmouse.dll` em aplicações C# (.NET Framework, .NET Core, .NET 6/7/8/9+), enviando comandos de mouse através do driver de kernel do Logitech G HUB.

---

## 📌 Sumário
1. [Pré-requisitos e Configuração do Projeto](#-pré-requisitos-e-configuração-do-projeto)
2. [Como Importar a DLL (P/Invoke)](#-como-importar-a-dll-pinvoke)
3. [Mapeamento Completo da API](#-mapeamento-completo-da-api)
4. [Wrapper Orientado a Objetos (GHubMouse.cs)](#-wrapper-orientado-a-objetos-ghubmousecs)
5. [Exemplo Completo de Uso](#-exemplo-completo-de-uso)
6. [Resolução de Problemas Comuns](#-resolução-de-problemas-comuns)

---

## ⚙️ Pré-requisitos e Configuração do Projeto

1. **Logitech G HUB** instalado e em execução em segundo plano.
2. **Arquitetura x64 Obrigatória**:
   - A `ghubmouse.dll` é compilada exclusivamente para **64-bit (x64)**.
   - Em seu projeto C# (`.csproj`), configure o `PlatformTarget` para `x64`:
     ```xml
     <PropertyGroup>
       <PlatformTarget>x64</PlatformTarget>
       <Platforms>x64</Platforms>
     </PropertyGroup>
     ```
   - No Visual Studio, altere o seletor de compilação de `Any CPU` ou `x86` para **x64**.
3. **Localização da DLL**:
   - Coloque `ghubmouse.dll` na mesma pasta do executável gerado (`bin\x64\Debug\...` ou `bin\x64\Release\...`).
   - Ou inclua a DLL no seu arquivo `.csproj` para cópia automática:
     ```xml
     <ItemGroup>
       <None Update="ghubmouse.dll">
         <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
       </None>
     </ItemGroup>
     ```

---

## 🔌 Como Importar a DLL (P/Invoke)

Todas as funções da `ghubmouse.dll` utilizam a convenção de chamada standard do Windows (`__stdcall`). No C#, declare o atributo `[DllImport]` especificando `CallingConvention = CallingConvention.StdCall`.

```csharp
using System;
using System.Runtime.InteropServices;

public static class GHubNative
{
    private const string DllName = "ghubmouse.dll";

    // Ciclo de vida do Driver
    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool mouse_open();

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern void mouse_close();

    // Movimentação Relativa
    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int moveR(int dx, int dy);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int move_relative(int dx, int dy);

    // Movimentação Absoluta
    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int moveA(int x, int y);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int move_absolute(int x, int y);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool set_cursor_pos(int x, int y);

    // Botões e Cliques
    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int press(ushort button);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int release();

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int release_button(ushort button);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int mouse_down(ushort button);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int mouse_up(ushort button);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int mouse_click(ushort button, uint delayMs);

    // Scroll
    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int scroll(sbyte wheel);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern int scroll_h(sbyte pan);

    // Proxy Drop-in para APIs Win32
    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    public static extern void ghub_mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, UIntPtr dwExtraInfo);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool ghub_SetCursorPos(int x, int y);
}
```

---

## 🗺️ Mapeamento Completo da API

### Máscara de Botões (`ushort button`)

| Nome | Valor Hex | Valor Decimal | Descrição |
| :--- | :--- | :--- | :--- |
| `MouseButton.Left` | `0x0001` | `1` | Botão primário / esquerdo |
| `MouseButton.Right` | `0x0002` | `2` | Botão secundário / direito |
| `MouseButton.Middle` | `0x0004` | `4` | Botão do scroll (meio) |
| `MouseButton.XButton1` | `0x0008` | `8` | Botão lateral traseiro (Mouse 4) |
| `MouseButton.XButton2` | `0x0010` | `16` | Botão lateral dianteiro (Mouse 5) |

### Códigos de Retorno (`NTSTATUS`)
- `0` (`0x00000000` / `STATUS_SUCCESS`): Operação executada com sucesso no driver.
- Valores diferentes de 0 indicam código de erro do kernel NT (ex: `0xC0000001` ou falha de comunicação com o IOCTL).

---

## 📦 Wrapper Orientado a Objetos (GHubMouse.cs)

Para facilitar a integração sem precisar lidar diretamente com P/Invoke em todo o seu código, use esta classe com suporte a `IDisposable`:

```csharp
using System;
using System.Runtime.InteropServices;
using System.Threading;

public enum MouseButtons : ushort
{
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 4,
    X1 = 8,
    X2 = 16
}

public sealed class GHubMouse : IDisposable
{
    private bool _disposed;

    public bool IsConnected { get; private set; }

    public GHubMouse()
    {
        Connect();
    }

    public void Connect()
    {
        if (IsConnected) return;

        if (!GHubNative.mouse_open())
        {
            throw new InvalidOperationException(
                "Falha ao conectar com o driver Logitech G HUB. " +
                "Verifique se o software Logitech G HUB está em execução e se o aplicativo roda em x64.");
        }

        IsConnected = true;
    }

    public void MoveRelative(int dx, int dy)
    {
        EnsureConnected();
        GHubNative.moveR(dx, dy);
    }

    public void MoveAbsolute(int x, int y)
    {
        EnsureConnected();
        GHubNative.moveA(x, y);
    }

    public bool SetCursorPosition(int x, int y)
    {
        EnsureConnected();
        return GHubNative.set_cursor_pos(x, y);
    }

    public void Press(MouseButtons button)
    {
        EnsureConnected();
        GHubNative.mouse_down((ushort)button);
    }

    public void Release(MouseButtons button)
    {
        EnsureConnected();
        GHubNative.mouse_up((ushort)button);
    }

    public void ReleaseAll()
    {
        EnsureConnected();
        GHubNative.release();
    }

    public void Click(MouseButtons button, int delayMs = 25)
    {
        EnsureConnected();
        GHubNative.mouse_click((ushort)button, (uint)Math.Max(0, delayMs));
    }

    public void ScrollVertical(sbyte steps)
    {
        EnsureConnected();
        GHubNative.scroll(steps);
    }

    public void ScrollHorizontal(sbyte steps)
    {
        EnsureConnected();
        GHubNative.scroll_h(steps);
    }

    private void EnsureConnected()
    {
        if (!IsConnected)
            throw new InvalidOperationException("Driver G HUB não está conectado.");
    }

    public void Dispose()
    {
        if (_disposed) return;
        if (IsConnected)
        {
            GHubNative.mouse_close();
            IsConnected = false;
        }
        _disposed = true;
        GC.SuppressFinalize(this);
    }

    ~GHubMouse()
    {
        Dispose();
    }
}
```

---

## 🚀 Exemplo Completo de Uso

Crie uma aplicação de console (`Program.cs`) e execute:

```csharp
using System;
using System.Threading;

class Program
{
    static void Main()
    {
        Console.WriteLine("[*] Inicializando Logitech G HUB Proxy em C#...");

        try
        {
            using (var mouse = new GHubMouse())
            {
                Console.WriteLine("[+] Conexão com driver de kernel estabelecida!");

                // 1. Movimento Relativo (move 200 pixels para direita, 100 para baixo)
                Console.WriteLine("[*] Movendo cursor relativamente (+200, +100)...");
                mouse.MoveRelative(200, 100);
                Thread.Sleep(300);

                // Retorna à posição anterior
                mouse.MoveRelative(-200, -100);
                Thread.Sleep(300);

                // 2. Movimento Absoluto (leva o cursor para coordenadas específicas da tela)
                Console.WriteLine("[*] Movendo cursor absolutamente para (800, 600)...");
                mouse.MoveAbsolute(800, 600);
                Thread.Sleep(300);

                // 3. Clique do Mouse (Botão Esquerdo com 30ms de delay de pressão)
                Console.WriteLine("[*] Executando clique esquerdo...");
                mouse.Click(MouseButtons.Left, delayMs: 30);
                Thread.Sleep(300);

                // 4. Pressionar e Soltar Manualmente
                Console.WriteLine("[*] Segurando botão direito...");
                mouse.Press(MouseButtons.Right);
                Thread.Sleep(100);
                mouse.Release(MouseButtons.Right);
                Thread.Sleep(300);

                // 5. Scroll da Roda do Mouse (-3 = 3 passos para baixo, +3 = 3 passos para cima)
                Console.WriteLine("[*] Rolando scroll para baixo...");
                mouse.ScrollVertical(-3);
                Thread.Sleep(200);

                Console.WriteLine("[*] Rolando scroll para cima...");
                mouse.ScrollVertical(3);
                Thread.Sleep(200);

                Console.WriteLine("[+] Teste concluído com sucesso!");
            }
            // Ao sair do bloco 'using', mouse_close() é chamado automaticamente
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"[-] Erro: {ex.Message}");
            Console.ResetColor();
        }
    }
}
```

---

## 🛠️ Resolução de Problemas Comuns

| Erro | Causa Mais Frequente | Como Resolver |
| :--- | :--- | :--- |
| `BadImageFormatException: An attempt was made to load a program with an incorrect format` | Aplicação rodando em 32-bit (x86) tentando carregar a DLL de 64-bit (x64). | Mude a configuração do projeto C# para `x64` nas propriedades do projeto ou adicione `<PlatformTarget>x64</PlatformTarget>`. |
| `DllNotFoundException: Unable to load DLL 'ghubmouse.dll'` | A DLL não está no diretório de execução ou no PATH. | Copie `ghubmouse.dll` para o mesmo diretório do arquivo `.exe` compilado. |
| `mouse_open() retorna false` | O Logitech G HUB não está instalado, não está rodando, ou seus serviços de driver foram interrompidos. | Inicie o Logitech G HUB e certifique-se de que o processo `lghub.exe` e `lghub_agent.exe` estão ativos. |
