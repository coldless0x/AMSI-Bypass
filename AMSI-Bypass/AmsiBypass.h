#pragma once
#include <Windows.h>
#include <vector>

class AmsiBypass {
public:
    AmsiBypass();
    ~AmsiBypass();

    // Main bypass methods
    bool BypassAmsiScanBuffer();
    bool BypassAmsiScanBufferAlternative(); // Alternative method
    bool BypassAmsiOpenSession();
    bool BypassAll();
    
    // Verification
    bool IsAmsiBypassed();
    bool TestBypass();
    
    // Restore
    bool RestoreAmsi();
    
private:
    struct PatchInfo {
        void* address;
        std::vector<BYTE> originalBytes;
        bool isPatched;
    };
    
    PatchInfo m_scanBufferPatch;
    PatchInfo m_openSessionPatch;
    
    // Helper functions
    void* GetAmsiScanBufferAddress();
    void* GetAmsiOpenSessionAddress();
    bool PatchMemory(void* address, const BYTE* patch, size_t size, PatchInfo& info);
    bool RestorePatch(PatchInfo& info);
    bool ChangeProtection(void* address, size_t size, DWORD newProtect, DWORD* oldProtect);
};
