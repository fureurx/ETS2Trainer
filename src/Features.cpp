#include "Features.h"
#include "SDK/Memory/Memory.h"

template <typename T>
static constexpr auto relativeToAbsolute(uintptr_t address, int addressOffset, int instructionCount) noexcept
{
	return (T)(address + instructionCount + *reinterpret_cast<std::int32_t*>(address + addressOffset));
}

static std::uint8_t* teleportFuncAddress = nullptr;
static std::uint8_t* damageFuncAddress = nullptr;
static std::uint8_t* displayDamageFuncAddress = nullptr;

static std::uint8_t* velocityMultAddress = nullptr;
static void* allocatedDisplacementMem = nullptr;
static BYTE velocityMultJmp[5];
static bool velocityMultEnabled = false;

static Vector3* cameraCoords = nullptr;
static Vector3* truckCoords = nullptr;
static int64_t* teleportObj = nullptr;

Features::Features()
{
	g_Features = this;
	moduleBase = GetModuleHandle(NULL);
	
	profileBase = relativeToAbsolute<uintptr_t>(
		(uintptr_t)Memory::SigScan(moduleBase, "48 8b 05 ? ? ? ? 48 8b d9 8b 90 ? ? ? ? 48 8b 80 ? ? ? ? 48 8b 88 ? ? ? ? e8 ? ? ? ? 48 8b 4b ? 3b 81 ? ? ? ? 0f 92 c0"), 3, 7);

	currentTruck = *(Truck**)Memory::FindDMAAddy(profileBase, { 0x18, 0x80 });
	moneyAddress = (int64_t*)Memory::FindDMAAddy(profileBase, { 0x10, 0x10 });
	xpAddress = (int64_t*)Memory::FindDMAAddy(profileBase, { 0x196C });

	cameraCoords = (Vector3*)Memory::FindDMAAddy(relativeToAbsolute<uintptr_t>(
		(uintptr_t)Memory::SigScan(moduleBase, "48 8b 05 ? ? ? ? 33 ff 4d 8b f8"), 3, 7), 
		{ 0x38, 0x0, 0x40 });
	
	//truckCoords = (Vector3*)Memory::FindDMAAddy((uintptr_t)moduleBase + 0x1B0BF00, { 0x0, 0x68, 0x140, 0x0, 0x14 });

	auto teleportOffset = *(int32_t*)(Memory::SigScan(moduleBase, "48 8b 89 ? ? ? ? e8 ? ? ? ? 48 8b 5c 24 ? 48 8b 74 24 ? 48 83 c4 ? 5f c3 cc cc cc cc cc cc cc cc cc cc 40 55") + 3);
	teleportObj = (int64_t*)(Memory::FindDMAAddy(relativeToAbsolute<uintptr_t>(
		(uintptr_t)Memory::SigScan(moduleBase, "48 8b 05 ? ? ? ? f3 44 0f 10 25"), 3, 7), 
		{ (unsigned int)teleportOffset }));

	teleportFuncAddress = Memory::SigScan(moduleBase, "48 81 ec ? ? ? ? 48 83 79 ? ? 45 0f b6 f9") - 17;
	damageFuncAddress = Memory::SigScan(moduleBase, "0f 28 ce e8 ? ? ? ? 41 c7 87 ?") + 3;
	displayDamageFuncAddress = Memory::SigScan(moduleBase, "? ? ? ? ? f3 0f 10 15 ? ? ? ? 48 8d 54 24 ? 48 8b 4b");

	velocityMultAddress = Memory::SigScan(moduleBase, "f3 0f 11 0c b0 48 3b b5 ? ? ? ? 0f 83 ? ? ? ? 48 8b 85 ? ? ? ? 49 3b b7");

	void* allocateNearThisAddress = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(moduleBase) + 0x578EFD);
	allocatedDisplacementMem = AllocateNearAddress(allocateNearThisAddress, 19);
	Memory::Patch(static_cast<BYTE*>(allocatedDisplacementMem), (BYTE*)"\xD7\xA3\x80\x3F\xD7\xA3\x80\x3F\xD7\xA3\x80\x3F\x00\x00\x80\x3F", 16);
}

ULONGLONG startTime = GetTickCount64();
bool keyDown = false;
void Features::Tick()
{
	if (g_Options.headLight)
	{
		switch (g_Menu->selectedItemIndex)
		{
		case 0:
			if (keyDown && GetTickCount64() - startTime > g_Options.headLightFreq)
			{
				KeyUp(0x24);
				KeyUp(0x1B);
				KeyDown(0x1A);
				startTime = GetTickCount64();
				keyDown = false;
			}
			else if (!keyDown && GetTickCount64() - startTime > g_Options.headLightFreq)
			{
				KeyDown(0x24);
				KeyUp(0x1A);
				KeyDown(0x1B);
				startTime = GetTickCount64();
				keyDown = true;
			}
			break;
		case 1:
			if (GetTickCount64() - startTime > g_Options.headLightFreq)
			{
				KeyDown(0x26);
				KeyUp(0x26);
				startTime = GetTickCount64();
			}
			break;
		default:
			break;
		}
	}
	else if (g_Options.velocityMult)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		if (XInputGetState(0, &state) == ERROR_SUCCESS)
		{
			if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			{
				Memory::Patch(velocityMultAddress, velocityMultJmp, sizeof(velocityMultJmp));
				velocityMultEnabled = true;
			}
			else if (velocityMultEnabled == true)
			{
				Memory::Patch(velocityMultAddress, (BYTE*)"\xF3\x0F\x11\x0C\xB0", 5);
			}
		}
	}
}

void Features::Release()
{
	VelocityMult(false);

	if (!VirtualFree(allocatedDisplacementMem, 0, MEM_RELEASE)) {
		std::cerr << "Memory free failed" << std::endl;
		return;
	}
}

void Features::Repair()
{
	currentTruck->ChassiDmg = 0;
	currentTruck->EngineDmg = 0;
	currentTruck->TransmissionDmg = 0;
	currentTruck->CabinDmg = 0;
	currentTruck->WheelDmg = 0;
}

void Features::Refuel()
{
	currentTruck->Fuel = 1;
}
void Features::SetMoney(int64_t money)
{
	*moneyAddress = money;
}

void Features::SetXP(int64_t xp)
{
	*xpAddress = xp;
}

int64_t& Features::GetMoney()
{
	return *moneyAddress;
}

int64_t& Features::GetXp()
{
	return *xpAddress;
}

void Features::DisableDamage(bool enable)
{
	if (enable) 
	{
		Memory::Nop(damageFuncAddress, 5);
		Memory::Nop(displayDamageFuncAddress, 5);
	}
	else 
	{
		Memory::Patch(damageFuncAddress, (BYTE*)"\xE8\x93\xE3\xD5\xFF", 5);
		Memory::Patch(displayDamageFuncAddress, (BYTE*)"\xE8\xCF\xCF\xB2\xFF", 5);
	}
}

void Features::TeleportToCameraCoords()
{
	TeleportToCoords(*cameraCoords);
}

void Features::TeleportToCoords(Vector3& coords)
{
	typedef char(__fastcall* teleport_t) (int64_t teleportPtr, Vector3* cameraCoords, int64_t teleportFlag, int64_t extraCalculations);
	teleport_t TeleportTruck = (teleport_t)(teleportFuncAddress);

	TeleportTruck(*teleportObj, &coords, 0, 0);
}

void* allocatedMemory;
void Features::VelocityMult(bool enable)
{
	if (enable)
	{
		//std::cout << "Generated displacement: ";
		//for (int i = 0; i < 12; ++i) {
		//	std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(test[i] & 0xFF) << " ";
		//}
		//std::cout << std::endl;

		// Attempt to allocate memory near the specified address
		uintptr_t allocateNearThisAddress = reinterpret_cast<uintptr_t>(moduleBase) + 0x578EFD;
		allocatedMemory = AllocateNearAddress(reinterpret_cast<void*>(allocateNearThisAddress), 20);

		uintptr_t targetAddress = reinterpret_cast<uintptr_t>(allocatedMemory);
		uintptr_t currentAddress = reinterpret_cast<uintptr_t>(velocityMultAddress);

		// Generate the displacement offset
		int32_t dispOffset = reinterpret_cast<uintptr_t>(allocatedDisplacementMem) - (targetAddress + 7);

		// Create the byte array for the patch
		BYTE patch[17] = {
			0x0F, 0x59, 0x0D,
			static_cast<BYTE>((dispOffset >> 0) & 0xFF),
			static_cast<BYTE>((dispOffset >> 8) & 0xFF),
			static_cast<BYTE>((dispOffset >> 16) & 0xFF),
			static_cast<BYTE>((dispOffset >> 24) & 0xFF),
			0xF3, 0x0F, 0x11, 0x0C, 0xB0
		};

		// Generate the jump from mulps to original func
		BYTE jmpBinaryFromMulps[5];
		GenerateJmpBinary(jmpBinaryFromMulps, targetAddress, currentAddress - 7);

		// Write mulps logic to allocated memory
		std::memcpy(&patch[12], jmpBinaryFromMulps, 5);
		Memory::Patch(static_cast<BYTE*>(allocatedMemory), patch, sizeof(patch));

		// Generate the jump from original func to mulps
		GenerateJmpBinary(velocityMultJmp, currentAddress, targetAddress);
	}
	else
	{
		Memory::Patch(velocityMultAddress, (BYTE*)"\xF3\x0F\x11\x0C\xB0", 5);

		if (!VirtualFree(allocatedMemory, 0, MEM_RELEASE)) {
			std::cerr << "Memory free failed" << std::endl;
			return;
		}
	}
}

void Features::UpdateVelocityMultiplier(float value)
{
	// Convert the float parameter to an array of bytes
	BYTE* byteArray = reinterpret_cast<BYTE*>(&value);

	BYTE patch[16] = {
		byteArray[0],
		byteArray[1],
		byteArray[2],
		byteArray[3],
		byteArray[0],
		byteArray[1],
		byteArray[2],
		byteArray[3],
		byteArray[0],
		byteArray[1],
		byteArray[2],
		byteArray[3],
		0x00, 0x00, 0x80, 0x3F
	};

	Memory::Patch(static_cast<BYTE*>(allocatedDisplacementMem), patch, sizeof(patch));
}