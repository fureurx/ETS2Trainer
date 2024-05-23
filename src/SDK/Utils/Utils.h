#pragma once
#include "../../pch.h"

template <typename T>
void LogError(T msg)
{
    std::cout << "[Error] " << msg << std::endl;
}

template <typename T>
void LogInfo(T msg)
{
    std::cout << "[Info] " << msg << std::endl;
}

void AttachConsole();

void CloseConsole();

void PrintHexDump(const void* data, size_t size);

void KeyDown(int key);
void KeyUp(int key);

void GenerateJmpBinary(BYTE* buffer, uintptr_t currentAddress, uintptr_t targetAddress);
void* AllocateNearAddress(void* allocateNearThisAddress, SIZE_T size, SIZE_T range = 0x20000000);