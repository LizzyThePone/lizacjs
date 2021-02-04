#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <thread>
#include <napi.h>
#include <map>

#include "Memory.h"
#include "Offsets.hpp"

#define f(x,y)*(int*)x^=*(int*)y
#define PI 3.1415927f

Memory* Mem;

int TriggerBind = 0x06;

struct GlowColor {
	float r, g, b, a;
};

struct GlowSettings {
    bool renderWhenOccluded, renderWhenUnoccluded, fullBloom;
};

struct Vec3 {
	float x, y, z;

    Vec3() { x = y = z = 0; }
	Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vec3 operator + (const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    Vec3 operator - (const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    Vec3 operator * (const float& rhs) const { return Vec3(x * rhs, y * rhs, z * rhs); }
    Vec3 operator / (const float& rhs) const { return Vec3(x / rhs, y / rhs, z / rhs); }
    Vec3& operator += (const Vec3& rhs) { return *this = *this + rhs; }
    Vec3& operator -= (const Vec3& rhs) { return *this = *this - rhs; }
    Vec3& operator *= (const float& rhs) { return *this = *this * rhs; }
    Vec3& operator /= (const float& rhs) { return *this = *this / rhs; }
    float dot() const { return x * x + y * y + z * z; }
    float Length() const { return sqrtf(dot()); }
    float Distance(const Vec3& rhs) const { return (*this - rhs).Length(); }

	void Normalize() {
		while (y < -180) {
			y += 360;
		};
		while (y > 180) {
			y -= 360;
		};
		if (x > 89) {
			x = 89;
		};
		if (x < -89) {
			x = -89;
		};
	}

};

Vec3 Subtract(Vec3 src, Vec3 dst)
{
    Vec3 diff;
    diff.x = src.x - dst.x;
    diff.y = src.y - dst.y;
    diff.z = src.z - dst.z;
    return diff;
}

struct SkinObject {
    int kit, stattrak, seed;
    float wear;
};

std::map<int, SkinObject> Skins { {7, {3,0,0,0.f} } };

float Magnitude(Vec3 vec)
{
    return sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
};

float Distance(Vec3 src, Vec3 dst)
{
    Vec3 diff = Subtract(src, dst);
    return Magnitude(diff);
};

Vec3 CalcAngle(Vec3 src, Vec3 dst)
{
    Vec3 vForward = dst - src;
    Vec3 vAngle;

    if (vForward.y == 0 && vForward.x == 0)
    {
        vAngle.y = 0;
        if (vForward.z > 0)
            vAngle.x = 270;
        else 
            vAngle.x = 90;
    }
    else
    {
        vAngle.y = (atan2(vForward.y, vForward.x) * 180 / PI);

        if (vAngle.y < 0)
            vAngle.y += 360;

        float dotproduct  = sqrt(vForward.x * vForward.x + vForward.y * vForward.y);
        vAngle.x = (atan2(-vForward.z, dotproduct) * 180 / PI);

        if (vAngle.x < 0)
            vAngle.x += 360;
    }

    vAngle.z = 0;

    return vAngle;
}



Vec3 RadToDeg(Vec3& radians)
{
    Vec3 degrees =  {0,0,0};
    degrees.x = radians.x * (180 / PI);
    degrees.y = radians.y * (180 / PI);
    degrees.z = radians.z * (180 / PI);
    return degrees;
};

bool TriggerToggled = false;

void Trigger()
{
    while (true)
    {

        if (!TriggerToggled || !GetAsyncKeyState(TriggerBind)) {
            Sleep(100);
        }

        else if (TriggerToggled && GetAsyncKeyState(TriggerBind)){
            DWORD LocalPlayer_Base = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
            int LocalPlayer_inCross = Mem->Read<int>(LocalPlayer_Base + hazedumper::netvars::m_iCrosshairId);
            int LocalPlayer_Team = Mem->Read<int>(LocalPlayer_Base + hazedumper::netvars::m_iTeamNum);
            DWORD Trigger_EntityBase = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwEntityList + ((LocalPlayer_inCross - 1) * 0x10));
            int Trigger_EntityTeam = Mem->Read<int>(Trigger_EntityBase + hazedumper::netvars::m_iTeamNum);
            bool Trigger_EntityDormant = Mem->Read<bool>(Trigger_EntityBase + hazedumper::signatures::m_bDormant);

            if ((LocalPlayer_inCross > 0 && LocalPlayer_inCross <= 64) && (Trigger_EntityBase != NULL) && (Trigger_EntityTeam != LocalPlayer_Team) && (!Trigger_EntityDormant))
            {
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceAttack, 5);
                Sleep(10);
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceAttack, 4);
                Sleep(25);
            }
        }
    }
}

bool AimToggled = true;

// Please no booly i am stoopid
/*
void Aim()
{
    while (true)
    {

        if (!AimToggled || !GetAsyncKeyState(VK_XBUTTON2)) {
            Sleep(100);
        }

        else if (AimToggled && GetAsyncKeyState(VK_XBUTTON2)){
            DWORD LocalPlayer_Base = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
            DWORD ClientState = Mem->Read<DWORD>(Mem->EngineDLLBase + hazedumper::signatures::dwClientState);
            Vec3 CurrentAngles = Mem->Read<Vec3>(ClientState + hazedumper::signatures::dwClientState_ViewAngles);
            int LocalPlayer_inCross = Mem->Read<int>(LocalPlayer_Base + hazedumper::netvars::m_iCrosshairId);
            int LocalPlayer_Team = Mem->Read<int>(LocalPlayer_Base + hazedumper::netvars::m_iTeamNum);
            DWORD Trigger_EntityBase = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwEntityList + ((LocalPlayer_inCross - 1) * 0x10));
            int Trigger_EntityTeam = Mem->Read<int>(Trigger_EntityBase + hazedumper::netvars::m_iTeamNum);
            bool Trigger_EntityDormant = Mem->Read<bool>(Trigger_EntityBase + hazedumper::signatures::m_bDormant);
            DWORD BoneMatrix = Mem->Read<DWORD>(Trigger_EntityBase + hazedumper::netvars::m_dwBoneMatrix);
            Vec3 VecOrigin = Mem->Read<Vec3>(LocalPlayer_Base + hazedumper::netvars::m_vecOrigin);
            Vec3 ViewOffset = Mem->Read<Vec3>(LocalPlayer_Base + hazedumper::netvars::m_vecViewOffset);

            Vec3 HeadPos = {0,0,0};
            HeadPos.x = Mem->Read<float>(BoneMatrix + 0x30 * 8 + 0x0C);
            HeadPos.y = Mem->Read<float>(BoneMatrix + 0x30 * 8 + 0x1C);
            HeadPos.z = Mem->Read<float>(BoneMatrix + 0x30 * 8 + 0x2C);

            if ((LocalPlayer_inCross > 0 && LocalPlayer_inCross <= 64) && (Trigger_EntityBase != NULL) && (Trigger_EntityTeam != LocalPlayer_Team) && (!Trigger_EntityDormant))
            {
                std::cout << "x:" << HeadPos.x << " y:" << HeadPos.y << "z:" << HeadPos.z <<std::endl;
                Mem->Write<Vec3>(ClientState + hazedumper::signatures::dwClientState_ViewAngles, NewAngles);
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceAttack, 5);
                Sleep(10);
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceAttack, 4);
            }
        }
        Sleep(10);
    }
}*/

bool BhopToggled = false;

void Bhop()
{
    while (true) {
        if (!BhopToggled) {
            Sleep(100);
        }

        else if (BhopToggled && GetAsyncKeyState(VK_SPACE))
        {
            DWORD LocalPlayer = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
            int iFlags = Mem->Read<DWORD>(LocalPlayer + hazedumper::netvars::m_fFlags);
            int dwForceJumpCurrent = Mem->Read<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceJump);
            int dwForceJumpSet = (iFlags == 257) ? 5 : 4;
            if (dwForceJumpCurrent != dwForceJumpSet) {
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceJump, dwForceJumpSet);
            }
        }

        Sleep(10);
    }
}

bool AutostrafeToggled = false;
Vec3 ViewAnglesPrev{0,0,0};

void Autostrafe()
{
    while (true) {

        if (!AutostrafeToggled) {
            Sleep(100);
        }

        else if (AutostrafeToggled && GetAsyncKeyState(VK_SPACE))
        {
            DWORD ClientState = Mem->Read<DWORD>(Mem->EngineDLLBase + hazedumper::signatures::dwClientState);
            Vec3 ViewAnglesCur = Mem->Read<Vec3>(ClientState + hazedumper::signatures::dwClientState_ViewAngles);

            if (ViewAnglesCur.y > ViewAnglesPrev.y) {
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceLeft, 6);
            }

            else if (ViewAnglesCur.y < ViewAnglesPrev.y) {
                Mem->Write<int>(Mem->ClientDLLBase + hazedumper::signatures::dwForceRight, 6);
            }

            ViewAnglesPrev = ViewAnglesCur;
        }

        Sleep(10);

    }
}

int GetEntity (int id)
{
    int Entity = Mem->Read<int>(Mem->ClientDLLBase + hazedumper::signatures::dwEntityList + ((id) * 0x10));
    return Entity;
}

bool GlowToggled = false;
bool GlowColorMode = false;
GlowColor TColor = {1.f,0.63137254902f,0.f,1.f};
GlowColor CtColor = {0.f,0.63137254902f,1.f,1.f};
GlowSettings CurrentGlowSettings = {true,false,false};

void Glow() 
{
    while (true) {
        if (!GlowToggled) {
            Sleep(100);
        }

        else if (GlowToggled) {
            for (int i = 1; i < Mem->Read<int>(Mem->ClientDLLBase + hazedumper::signatures::dwGlowObjectManager + 0xC); i++){

                if (!GlowToggled) break;
                
                DWORD LocalPlayer = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
                int LocalPlayerTeam = Mem->Read<int>(LocalPlayer + hazedumper::netvars::m_iTeamNum);
                int EntityTeam = Mem->Read<int>(GetEntity(i) + hazedumper::netvars::m_iTeamNum);
                bool EntityDormant = Mem->Read<bool>(GetEntity(i) + hazedumper::signatures::m_bDormant);
                int GlowIndex =  Mem->Read<int>(GetEntity(i) + hazedumper::netvars::m_iGlowIndex);
                DWORD GlowManager = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwGlowObjectManager);
                if(!EntityDormant && !GlowColorMode) {
                    switch (EntityTeam)
                    {
                    case 2:
                        Mem->Write<GlowColor>(GlowManager + ((GlowIndex * 0x38) + 0x4), TColor);
                        Mem->Write<GlowSettings>(GlowManager + ((GlowIndex * 0x38) + 0x24), CurrentGlowSettings);
                        break;
                    case 3:
                        Mem->Write<GlowColor>(GlowManager + ((GlowIndex * 0x38) + 0x4), CtColor);
                        Mem->Write<GlowSettings>(GlowManager + ((GlowIndex * 0x38) + 0x24), CurrentGlowSettings);
                        break;
                    }
                } else if (!EntityDormant && GlowColorMode && (EntityTeam == 2 || EntityTeam == 3)) {
                    switch (EntityTeam == LocalPlayerTeam)
                    {
                    case 0:
                        Mem->Write<GlowColor>(GlowManager + ((GlowIndex * 0x38) + 0x4), TColor);
                        Mem->Write<GlowSettings>(GlowManager + ((GlowIndex * 0x38) + 0x24), CurrentGlowSettings);
                        break;
                    case 1:
                        Mem->Write<GlowColor>(GlowManager + ((GlowIndex * 0x38) + 0x4), CtColor);
                        Mem->Write<GlowSettings>(GlowManager + ((GlowIndex * 0x38) + 0x24), CurrentGlowSettings);
                        break;
                    }
                }
            }
        }
        Sleep(2);
    }
}


bool RadarToggled = false;

void Radar()
{
    while (true) {
        if (!RadarToggled) {
            Sleep(100);
        }

        else if (RadarToggled)
        {
            for (int i = 0; i < Mem->Read<int>(Mem->ClientDLLBase + hazedumper::signatures::dwGlowObjectManager + 0xC); i++){
                if (!RadarToggled) break;
                Mem->Write<bool>(GetEntity(i) + hazedumper::netvars::m_bSpotted, true);
            }
        }

        Sleep(50);
    }
}

bool NoflashToggled = false;

void Noflash()
{
    while (true) {
        if (!NoflashToggled) {
            Sleep(100);
        }

        else if (NoflashToggled)
        {
            DWORD LocalPlayer = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
            float FlashAlpha = Mem->Read<float>(LocalPlayer + hazedumper::netvars::m_flFlashMaxAlpha);
            float FlashDuration = Mem->Read<float>(LocalPlayer + hazedumper::netvars::m_flFlashDuration);

            if (FlashAlpha != 0.f || FlashDuration != 0.f)
            {
                Mem->Write<float>(LocalPlayer + hazedumper::netvars::m_flFlashMaxAlpha, 0.f);
                Mem->Write<float>(LocalPlayer + hazedumper::netvars::m_flFlashDuration, 0.f);
            }
        }

        Sleep(10);
    }
}

bool RCSToggled = false;
bool RCSReset = false;
Vec3 OriginAngles = {0,0,0};

void RCS()
{
    while (true) {
        if (!RCSToggled) {
            Sleep(100);
        }

        else if (RCSToggled)
        {
            DWORD LocalPlayer = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
            int ShotsFired = Mem->Read<int>(LocalPlayer + hazedumper::netvars::m_iShotsFired);

            if (ShotsFired >= 1)
            {
                Vec3 VPunch = Mem->Read<Vec3>(LocalPlayer + hazedumper::netvars::m_aimPunchAngle);
                DWORD ClientState = Mem->Read<DWORD>(Mem->EngineDLLBase + hazedumper::signatures::dwClientState);
                Vec3 CurrentAngles = Mem->Read<Vec3>(ClientState + hazedumper::signatures::dwClientState_ViewAngles);
                Vec3 NewAngles = CurrentAngles;
                NewAngles.x = ((CurrentAngles.x + OriginAngles.x) - (VPunch.x * 2.f) );
                NewAngles.y = ((CurrentAngles.y + OriginAngles.y) - (VPunch.y * 2.f) );

                OriginAngles.x = VPunch.x * 2;
                OriginAngles.y = VPunch.y * 2;

                NewAngles.Normalize();
                if (ShotsFired >= 2)
                {
                    Mem->Write<Vec3>(ClientState + hazedumper::signatures::dwClientState_ViewAngles, NewAngles);
                }

            }
        }

        Sleep(25);
    }
}

bool SkinsToggled = false;
bool ForceUpdate = false;

void SkinChanger()
{
    while (true) {
        if (!SkinsToggled) {
            Sleep(100);
        }

        else if (SkinsToggled)
        {
            DWORD LocalPlayer = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwLocalPlayer);
            DWORD ClientState = Mem->Read<DWORD>(Mem->EngineDLLBase + hazedumper::signatures::dwClientState);

            for (int i = 0; i <= 5; i++){
                int WeaponEnt = Mem->Read<int>(LocalPlayer + hazedumper::netvars::m_hMyWeapons + i * 0x04) & 0xFFF;
                DWORD WeaponBase = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwEntityList + (WeaponEnt - 1) * 0x10);
                int WeaponId = Mem->Read<short>(WeaponBase + hazedumper::netvars::m_iItemDefinitionIndex);
                int AccountId = Mem->Read<int>(WeaponBase + hazedumper::netvars::m_OriginalOwnerXuidLow);
                int ItemIdHigh = Mem->Read<int>(WeaponBase + hazedumper::netvars::m_iItemIDHigh);
                int CurrentKit = Mem->Read<int>(WeaponBase + hazedumper::netvars::m_nFallbackPaintKit);
                //std::cout << WeaponId << std::endl;
                if (Skins.find(WeaponId) != Skins.end()) {
                    SkinObject Skin = Skins[WeaponId];

                    Mem->Write<int>(WeaponBase + hazedumper::netvars::m_OriginalOwnerXuidHigh, 0);
                    Mem->Write<int>(WeaponBase + hazedumper::netvars::m_OriginalOwnerXuidLow, 0);
                    Mem->Write<int>(WeaponBase + hazedumper::netvars::m_nFallbackPaintKit, Skin.kit);
                    Mem->Write<int>(WeaponBase + hazedumper::netvars::m_nFallbackSeed, Skin.seed);
                    Mem->Write<float>(WeaponBase + hazedumper::netvars::m_flFallbackWear, Skin.wear);
                    Mem->Write<int>(WeaponBase + hazedumper::netvars::m_iAccountID, AccountId);

                    if (Skin.kit != CurrentKit) {
                        ForceUpdate = true;
                    }

                    if (Skin.stattrak != 0) {
                        Mem->Write<int>(WeaponBase + hazedumper::netvars::m_nFallbackStatTrak, Skin.stattrak);
                    }

                    if (ItemIdHigh != -1) {
                        Mem->Write<int>(WeaponBase + hazedumper::netvars::m_iItemIDHigh, -1);
                    }

                }
            }
            
            if (ForceUpdate) {
                Mem->Write<int>(ClientState + hazedumper::signatures::clientstate_delta_ticks, -1);
                ForceUpdate = false;
                Sleep(25);
            }
        }

        Sleep(10);
    }
}

Napi::Value SetClanTag(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();
  if (!args[0].IsString()) {
    Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string tagArg(args[0].As<Napi::String>().Utf8Value());
  const char* tag = tagArg.c_str();
  const char* name = "fuck";
  HANDLE ProcessHandle = Mem->GetProcHandle();
  DWORD address = Mem->EngineDLLBase + hazedumper::signatures::dwSetClanTag;

	unsigned char Shellcode[] =
		"\x51"                   
		"\x52"                  
		"\xB9\x00\x00\x00\x00"   
		"\xBA\x00\x00\x00\x00"   
		"\xE8\x00\x00\x00\x00"  
		"\x83\x04\x24\x0A"      
		"\x68\x00\x00\x00\x00"  
		"\xC3"          
		"\x5A"              
		"\x59"               
		"\xC3"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" 
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	unsigned int SHELLCODE_SIZE = sizeof(Shellcode) - 0x21;
	unsigned int TAG_SIZE = (strlen(tag) > 15) ? 15 : strlen(tag);
	unsigned int NAME_SIZE = (strlen(name) > 15) ? 15 : strlen(name);
	unsigned int DATA_SIZE = TAG_SIZE + NAME_SIZE + 2;

	LPVOID ShellCodeAddress = VirtualAllocEx(ProcessHandle,
		0,
		SHELLCODE_SIZE + DATA_SIZE,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
		);

	DWORD dwTagAddress = (DWORD)ShellCodeAddress + SHELLCODE_SIZE;
	DWORD dwNameAddress = (DWORD)ShellCodeAddress + SHELLCODE_SIZE + TAG_SIZE + 1;
	DWORD dwSetClanAddress = address; 

	memcpy(Shellcode + 0x3, &dwTagAddress, sizeof(DWORD));
	memcpy(Shellcode + 0x8, &dwNameAddress, sizeof(DWORD));
	memcpy(Shellcode + 0x16, &dwSetClanAddress, sizeof(DWORD));
	memcpy(Shellcode + SHELLCODE_SIZE, tag, TAG_SIZE);
	memcpy(Shellcode + SHELLCODE_SIZE + TAG_SIZE + 1, name, NAME_SIZE);

	WriteProcessMemory(ProcessHandle, ShellCodeAddress, Shellcode, SHELLCODE_SIZE + DATA_SIZE, 0);

	HANDLE hThread = CreateRemoteThread(ProcessHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)ShellCodeAddress, NULL, NULL, NULL);
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(ProcessHandle, ShellCodeAddress, 0, MEM_RELEASE);

  return env.Null();
}

DWORD ClientCmdPtr = 0;
void updateClientCmd()
{
	DWORD start = ClientCmdPtr;
    int i = 0;
    start = Mem->GrabSig(Mem->EngineDLLBase, Mem->EngineDLLSize, (PBYTE)"\x55\x8B\xEC\xA1\x00\x00\x00\x00\x33\xC9\x8B\x55\x08", "xxxx????xxxxx");
	ClientCmdPtr = start;
}

void command(const char* command)
{
	LPVOID addr = (LPVOID)ClientCmdPtr;
	LPVOID vCommand = (LPVOID)VirtualAllocEx(Mem->GetProcHandle(), NULL, strlen(command) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(Mem->GetProcHandle(), vCommand, command, strlen(command), NULL);
	HANDLE hThread = CreateRemoteThread(Mem->GetProcHandle(), NULL, NULL, (LPTHREAD_START_ROUTINE)addr, vCommand, NULL, NULL);
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(Mem->GetProcHandle(), vCommand, NULL, MEM_RELEASE);
	CloseHandle(hThread);
}

int GetCvar (std::string cvar) {
    DWORD pICvar = Mem->Read<DWORD>(Mem->VstdlibDLLBase + hazedumper::signatures::interface_engine_cvar);
    if (pICvar != 0) {
        DWORD Shortcut = Mem->Read<DWORD>(pICvar + 52);
        int HashMapEntry = Mem->Read<int>(Shortcut);
        while (HashMapEntry != 0){
            int pConVar = Mem->Read<int>(HashMapEntry + 4);
            DWORD pConVarNamePtr = Mem->Read<int>(pConVar + 12);
            std::string pConVarName = Mem->ReadString(pConVarNamePtr);
            if (pConVarName.substr(0, pConVarName.size()-1) == cvar) {
                return Mem->Read<int>(HashMapEntry + 4);
            }
            HashMapEntry = Mem->Read<DWORD>(HashMapEntry + 0x4);
        }
    }
}


Napi::Value SetName(const Napi::CallbackInfo& args)
{
    Napi::Env env = args.Env();
    int pConVar = GetCvar("name");
    std::cout << Mem->Read<int>(pConVar + 0x14) << std::endl;
    Mem->Write<int>(pConVar + 0x44 + 0xC, Mem->Read<int>(pConVar + 0x14) + (1 << 30));
    Mem->Write<int>(pConVar + 0x44 + 0xC, 0);
    Mem->Write<char*>(Mem->Read<int>(pConVar + 0x24), (char*)"\n\xAD\xAD\xAD");
    Mem->Write<int>(pConVar + 0x28, strlen((char*)"\n\xAD\xAD\xAD"));

    std::string cmdArg("name");
    cmdArg.append(" \"");
    cmdArg.append(args[0].As<Napi::String>().Utf8Value());
    cmdArg.append("\"\0");
    const char* cmd = cmdArg.c_str();
    //command("name \"\x0A\xAD\xAD\xAD\"");
    command(cmd);

    return env.Null();
}

void SetCvarInt (std::string cvar, int value) {
    int pConVar = GetCvar(cvar);
    Mem->Write<int>(pConVar + 0x14, 8);
    return;
}

Napi::Value ToggleTrigger(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    TriggerToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, TriggerToggled);

	return v;
}

Napi::Value ToggleBhop(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    BhopToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, BhopToggled);

	return v;
}

Napi::Value ToggleAutostrafe(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    AutostrafeToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, AutostrafeToggled);

	return v;
}

Napi::Value ToggleGlow(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    GlowToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, GlowToggled);

	return v;
}

Napi::Value ToggleRadar(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    RadarToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, RadarToggled);

	return v;
}

Napi::Value ToggleNoflash(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    NoflashToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, NoflashToggled);

	return v;
}

Napi::Value ToggleRCS(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    RCSToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, RCSToggled);

	return v;
}

Napi::Value ToggleSkins(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    SkinsToggled = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, SkinsToggled);

	return v;
}

Napi::Value ToggleGlowColorMode(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsBoolean()) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    GlowColorMode = args[0].As<Napi::Boolean>();

    Napi::Boolean v = Napi::Boolean::New(env, GlowColorMode);

	return v;
}

Napi::Value SetTColor(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsNumber() || !args[1].IsNumber() || !args[2].IsNumber() ) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    TColor = {
        args[0].As<Napi::Number>().FloatValue() / 255.f,
        args[1].As<Napi::Number>().FloatValue() / 255.f,
        args[2].As<Napi::Number>().FloatValue() / 255.f,
        1.f
    };

	return env.Null();
}

Napi::Value SetCtColor(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
  	if (!args[0].IsNumber() || !args[1].IsNumber() || !args[2].IsNumber() ) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    CtColor = {
        args[0].As<Napi::Number>().FloatValue() / 255.f,
        args[1].As<Napi::Number>().FloatValue() / 255.f,
        args[2].As<Napi::Number>().FloatValue() / 255.f,
        1.f
    };

	return env.Null();
}

Napi::Value SetCvar(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();

    std::string cmdArg(args[0].As<Napi::String>().Utf8Value());
    cmdArg.append(" ");
    cmdArg.append(std::to_string(args[1].As<Napi::Number>().FloatValue()));
    cmdArg.append("\0");
    const char* cmd = cmdArg.c_str();

    SetCvarInt(args[0].As<Napi::String>().Utf8Value(), args[1].As<Napi::Number>().Int32Value());
    Sleep(1);
    command(cmd);

  	return env.Null();
}


Napi::Value SetSkin(const Napi::CallbackInfo& args) {
    Napi::Env env = args.Env();

    if (!args[0].IsNumber() || !args[1].IsNumber() || !args[2].IsNumber() || !args[3].IsNumber() || !args[4].IsNumber() ) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    SkinObject Skin;

    int id = args[0].As<Napi::Number>().Int32Value();
    Skin.kit = args[1].As<Napi::Number>().Int32Value();
    Skin.seed = args[2].As<Napi::Number>().Int32Value();
    Skin.wear = args[3].As<Napi::Number>().FloatValue();
    Skin.stattrak = args[4].As<Napi::Number>().Int32Value();

    Skins[id] = Skin;

    
    Napi::Boolean bitchbool = Napi::Boolean::New(env, (Skins.find(id) == Skins.end()) ? false : true);
    return bitchbool;
}

Napi::Value SetTriggerBind(const Napi::CallbackInfo& args) {
    Napi::Env env = args.Env();

    if (!args[0].IsNumber() ) {
        Napi::Error::New(env, "Invalid Arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    TriggerBind = args[0].As<Napi::Number>().Int32Value();

    
    Napi::Number v = Napi::Number::New(env, TriggerBind);
    return v;
}

Napi::Value InitCheat(const Napi::CallbackInfo& args) {
    Napi::Env env = args.Env();
	Mem = new Memory();
    updateClientCmd();
	
    std::thread (SkinChanger).detach();
	std::thread (Autostrafe).detach();
    std::thread (Noflash).detach();
    std::thread (Trigger).detach();
    std::thread (Radar).detach();
    std::thread (Bhop).detach();
    std::thread (Glow).detach();
    //std::thread (Aim).detach();
    std::thread (RCS).detach();
	
    return env.Null();
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  	exports.Set(Napi::String::New(env, "initialize"), Napi::Function::New(env, InitCheat));

	exports.Set(Napi::String::New(env, "toggleTrigger"), Napi::Function::New(env, ToggleTrigger));
    exports.Set(Napi::String::New(env, "toggleBhop"), Napi::Function::New(env, ToggleBhop));
    exports.Set(Napi::String::New(env, "toggleAutostrafe"), Napi::Function::New(env, ToggleAutostrafe));
    exports.Set(Napi::String::New(env, "toggleGlow"), Napi::Function::New(env, ToggleGlow));
    exports.Set(Napi::String::New(env, "toggleGlowColorMode"), Napi::Function::New(env, ToggleGlowColorMode));
    exports.Set(Napi::String::New(env, "toggleRadar"), Napi::Function::New(env, ToggleRadar));
    exports.Set(Napi::String::New(env, "toggleNoflash"), Napi::Function::New(env, ToggleNoflash));
    exports.Set(Napi::String::New(env, "toggleRCS"), Napi::Function::New(env, ToggleRCS));
    exports.Set(Napi::String::New(env, "toggleSkins"), Napi::Function::New(env, ToggleSkins));

    exports.Set(Napi::String::New(env, "setTColor"), Napi::Function::New(env, SetTColor));
    exports.Set(Napi::String::New(env, "setCtColor"), Napi::Function::New(env, SetCtColor));
    exports.Set(Napi::String::New(env, "setClanTag"), Napi::Function::New(env, SetClanTag));
    exports.Set(Napi::String::New(env, "setSkin"), Napi::Function::New(env, SetSkin));
    exports.Set(Napi::String::New(env, "setCvar"), Napi::Function::New(env, SetCvar));
    exports.Set(Napi::String::New(env, "setName"), Napi::Function::New(env, SetName));
    exports.Set(Napi::String::New(env, "setTriggerBind"), Napi::Function::New(env, SetTriggerBind));

  	return exports;
}

NODE_API_MODULE(lizzyjs, init)