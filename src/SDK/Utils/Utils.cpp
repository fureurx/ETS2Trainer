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