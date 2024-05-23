#include "../../pch.h"
#include "Utils.h"
FILE* fStream;
void AttachConsole()
{
    if (AttachConsole(GetCurrentProcessId()) == false)
        AllocConsole();

    freopen_s(&fStream, "CONOUT$", "w", stdout);
    SetConsoleTitle(L"ETS2 Menu");
}

void CloseConsole() 
{
    fclose(fStream);
    FreeConsole();
}

void PrintHexDump(const void* data, size_t size) {
    const unsigned char* buffer = static_cast<const unsigned char*>(data);

    // Create a string stream to hold the formatted output
    std::ostringstream output;

    for (size_t i = 0; i < size; i += 16) {
        // Print address
        output << std::setw(8) << std::setfill('0') << std::hex << i << ": ";

        // Print hex values
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size)
                output << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buffer[i + j]) << " ";
            else
                output << "   ";

            // Print an extra space after 8 bytes for better readability
            if (j == 7)
                output << " ";
        }

        // Print ASCII representation
        output << "  ";
        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            char c = buffer[i + j];
            if (c >= 32 && c < 127)
                output << c;
            else
                output << ".";
        }

        output << std::endl;
    }

    // Print the entire formatted output at once
    std::cout << output.str() << std::flush;
}

void KeyDown(int key)
{
    INPUT input[1];
    ::ZeroMemory(input, sizeof(input));

    input[0].type = INPUT_KEYBOARD;

    input[0].ki.wScan = key;
    input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    do {} while (SendInput(1, input, sizeof(INPUT)) < 1);
}

void KeyUp(int key)
{
    INPUT input[1];
    ::ZeroMemory(input, sizeof(input));

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wScan = key;
    input[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    do {} while (SendInput(1, input, sizeof(INPUT)) < 1);
}

void GenerateJmpBinary(BYTE* buffer, uintptr_t currentAddress, uintptr_t targetAddress) {
    const BYTE jmpOpcode = 0xE9;

    // Calculate the offset
    int32_t offset = static_cast<int32_t>(targetAddress - (currentAddress + 5));

    // Encode the jmp instruction into the buffer
    buffer[0] = jmpOpcode;
    *reinterpret_cast<int32_t*>(&buffer[1]) = offset;
}

void* AllocateNearAddress(void* allocateNearThisAddress, SIZE_T size, SIZE_T range) {
    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(allocateNearThisAddress);
    uintptr_t startAddress = baseAddress - range / 2;
    uintptr_t endAddress = baseAddress + range / 2;

    // Align addresses to page boundaries
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    uintptr_t pageSize = sysInfo.dwPageSize;

    startAddress &= ~(pageSize - 1);
    endAddress &= ~(pageSize - 1);

    for (uintptr_t address = startAddress; address < endAddress; address += pageSize) {
        void* allocatedMemory = VirtualAlloc(reinterpret_cast<void*>(address), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (allocatedMemory != nullptr) {
            return allocatedMemory;
        }
    }

    // If no suitable address was found
    return nullptr;
}