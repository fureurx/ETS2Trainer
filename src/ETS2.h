class Truck
{
public:
	char pad_0000[120]; //0x0000
	float ChassiDmg; //0x0078
	char pad_007C[164]; //0x007C
	float WheelDmg; //0x0120
	char pad_0124[36]; //0x0124
	float EngineDmg; //0x0148
	float TransmissionDmg; //0x014C
	float CabinDmg; //0x0150
	char pad_0154[12]; //0x0154
	float Fuel; //0x0160
	char pad_0164[796]; //0x0164
};