import sys
from ctypes import windll, c_void_p, WinError
from ctypes.wintypes import HANDLE, LPCVOID, DWORD, HMODULE, LPCSTR, LPDWORD

sys.dont_write_bytecode = True

kernel32 = windll.kernel32
PVOID = LPVOID = ULONG_PTR = c_void_p
PROCESS_ALL_ACCESS = 0x1F0FFF
MEM_COMMIT = 0x1000
MEM_RESERVE = 0x2000
PAGE_READWRITE = 0x04

kernel32.VirtualAllocEx.restype = LPVOID
kernel32.WriteProcessMemory.argtypes = [HANDLE, LPVOID, LPCVOID, DWORD, PVOID]
kernel32.GetModuleHandleW.restype = HMODULE
kernel32.GetProcAddress.argtypes = [HMODULE, LPCSTR]
kernel32.GetProcAddress.restype = LPVOID
kernel32.CreateRemoteThread.argtypes = [HANDLE, LPVOID, ULONG_PTR, LPCVOID, LPCVOID, DWORD, LPDWORD]


def inject_dll(pid, dll_path):
    dll_path_bin = dll_path.encode()
    process_handle = kernel32.OpenProcess(PROCESS_ALL_ACCESS, False, pid)
    alloc_mem = kernel32.VirtualAllocEx(process_handle, None, len(dll_path_bin), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
    if not alloc_mem:
        raise WinError()
    res = kernel32.WriteProcessMemory(process_handle, alloc_mem, dll_path_bin, len(dll_path_bin), None)
    if not res:
        raise WinError()
    k32_module_addr = kernel32.GetModuleHandleW("Kernel32")
    if not k32_module_addr:
        raise WinError()
    load_lib_addr = kernel32.GetProcAddress(k32_module_addr, b"LoadLibraryA")
    if not load_lib_addr:
        raise WinError()
    thread_handle = kernel32.CreateRemoteThread(process_handle, None, None, load_lib_addr, alloc_mem, 0, None)
    if not thread_handle:
        raise WinError()
    if kernel32.WaitForSingleObject(thread_handle, 0xFFFFFFFF) == 0xFFFFFFFF:
        raise WinError()
