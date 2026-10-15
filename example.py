"""
Exemplo de uso da ghubmouse.dll em Python (ctypes)
Movimentação de cursor via kernel driver da Logitech (G HUB / LGS).
"""

import ctypes
import os
import time

# Procura a DLL em x64/Release ou na pasta atual
dll_name = "ghubmouse.dll"
dll_candidates = [
    os.path.join(os.path.dirname(__file__), "x64", "Release", dll_name),
    os.path.join(os.path.dirname(__file__), dll_name),
    dll_name,
]

dll_path = next((p for p in dll_candidates if os.path.exists(p)), dll_name)

print(f"[*] Carregando: {dll_path}")
ghub = ctypes.windll.LoadLibrary(dll_path)

# Assinaturas das funções
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

# Inicialização
if not ghub.mouse_open():
    print("[-] Falha ao inicializar o driver do G HUB.")
    print("[-] Certifique-se de que o Logitech G HUB está instalado e rodando.")
    exit(1)

print("[+] Conectado com sucesso ao driver de kernel da Logitech!")

try:
    print("[*] Testando movimento relativo (+150, +75)...")
    ghub.moveR(150, 75)
    time.sleep(0.3)

    print("[*] Retornando (-150, -75)...")
    ghub.moveR(-150, -75)
    time.sleep(0.3)

    print("[*] Testando clique esquerdo (25ms delay)...")
    ghub.mouse_click(1, 25)
    time.sleep(0.3)

    print("[*] Testando scroll (-2)...")
    ghub.scroll(-2)
    time.sleep(0.2)

finally:
    ghub.mouse_close()
    print("[+] Conexão encerrada.")
