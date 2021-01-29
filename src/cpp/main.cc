#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <napi.h>

#include "Memory.h"
#include "Offsets.hpp"

Memory* Mem;

struct Vec3 {
	float x, y, z;

	Vec3 operator+(Vec3 d) {
		return { x + d.x, y + d.y, z + d.z };
	}
	Vec3 operator-(Vec3 d) {
		return { x - d.x, y - d.y, z - d.z };
	}
	Vec3 operator*(float d) {
		return { x * d, y * d, z * d };
	}

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

struct GlowColor {
	float r, g, b, a;
};

struct GlowSettings {
    bool renderWhenOccluded, renderWhenUnoccluded, fullBloom;
};

bool TriggerToggled = false;

void Trigger()
{
    while (true)
    {

        if (!TriggerToggled || !GetAsyncKeyState(VK_XBUTTON2)) {
            Sleep(100);
        }

        else if (TriggerToggled && GetAsyncKeyState(VK_XBUTTON2)){
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
                
                int EntityTeam = Mem->Read<int>(GetEntity(i) + hazedumper::netvars::m_iTeamNum);
                bool EntityDormant = Mem->Read<bool>(GetEntity(i) + hazedumper::signatures::m_bDormant);
                int GlowIndex =  Mem->Read<int>(GetEntity(i) + hazedumper::netvars::m_iGlowIndex);
                DWORD GlowManager = Mem->Read<DWORD>(Mem->ClientDLLBase + hazedumper::signatures::dwGlowObjectManager);
                if(!EntityDormant) {
                    switch (EntityTeam)
                    {
                    case 3:
                        Mem->Write<GlowColor>(GlowManager + ((GlowIndex * 0x38) + 0x4), CtColor);
                        Mem->Write<GlowSettings>(GlowManager + ((GlowIndex * 0x38) + 0x24), CurrentGlowSettings);
                        break;
                    case 2:
                        Mem->Write<GlowColor>(GlowManager + ((GlowIndex * 0x38) + 0x4), TColor);
                        Mem->Write<GlowSettings>(GlowManager + ((GlowIndex * 0x38) + 0x24), CurrentGlowSettings);
                        break;
                    }
                }
            }
        }
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
  const char* name = "";
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
	DWORD start;
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

Napi::Value InitCheat(const Napi::CallbackInfo& args) {
    Napi::Env env = args.Env();
	Mem = new Memory();
	
	std::thread (Autostrafe).detach();
    std::thread (Trigger).detach();
    std::thread (Bhop).detach();
    std::thread (Glow).detach();
	
    return env.Null();
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

Napi::Value Thingy(const Napi::CallbackInfo& args) {
	Napi::Env env = args.Env();
    updateClientCmd();

    //std::string cmdArg(args[0].As<Napi::String>().Utf8Value());
    //cmdArg.append("\0");
    //const char* cmd = cmdArg.c_str();

    
    //command(cmd);

    Napi::Number num = Napi::Number::New(env, ClientCmdPtr);

  	return num;
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  	exports.Set(Napi::String::New(env, "initialize"), Napi::Function::New(env, InitCheat));

	exports.Set(Napi::String::New(env, "toggleTrigger"), Napi::Function::New(env, ToggleTrigger));
    exports.Set(Napi::String::New(env, "toggleBhop"), Napi::Function::New(env, ToggleBhop));
    exports.Set(Napi::String::New(env, "toggleAutostrafe"), Napi::Function::New(env, ToggleAutostrafe));
    exports.Set(Napi::String::New(env, "toggleGlow"), Napi::Function::New(env, ToggleGlow));

    exports.Set(Napi::String::New(env, "setTColor"), Napi::Function::New(env, SetTColor));
    exports.Set(Napi::String::New(env, "setCtColor"), Napi::Function::New(env, SetCtColor));
    exports.Set(Napi::String::New(env, "setClanTag"), Napi::Function::New(env, SetClanTag));

    exports.Set(Napi::String::New(env, "thingy"), Napi::Function::New(env, Thingy));

  	return exports;
}

NODE_API_MODULE(lizzyjs, init)