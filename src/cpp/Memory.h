#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include "Offsets.hpp"

class Memory
{
private:
	DWORD dwPID;
	HANDLE Handle;

public:
	MODULEENTRY32 ClientDLL,
		VstdlibDLL,
		EngineDLL;
	DWORD ClientDLLBase,
		ClientDLLSize,
		VstdlibDLLBase,
		VstdlibDLLSize,
		EngineDLLBase,
		EngineDLLSize;

	bool AttachProcess(wchar_t* ProcessName)
	{
		HANDLE hPID = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		do {
            //MessageBoxA(NULL, procEntry.szExeFile, "FUCK", MB_ICONSTOP | MB_OK);
			if (!wcscmp((wchar_t*)procEntry.szExeFile, ProcessName))
			{
				this->dwPID = procEntry.th32ProcessID;
				CloseHandle(hPID);
				this->Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->dwPID);
				return true;
			}
        }
		while (Process32Next(hPID, &procEntry));

		CloseHandle(hPID);
		return false;
	}

	DWORD GetModule(char* ModuleName)
	{
		HMODULE hModules[1024];
        unsigned int i;
        DWORD cbNeeded; 
		MODULEENTRY32 mEntry;
		mEntry.dwSize = sizeof(mEntry);

		if( EnumProcessModulesEx(Handle, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_32BIT) )
        {
            for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
            {
                TCHAR szModName[MAX_PATH];


                if ( GetModuleBaseName( Handle, hModules[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                {
                    //MessageBoxA(NULL, szModName, "FUCK", MB_ICONSTOP | MB_OK);
                    if (!strcmp(szModName, ModuleName))
                    {
                        return (DWORD)hModules[i];
                    }
                }
        }
    }
		return (DWORD)0x0;
	}

	DWORD GetModuleSize(char* ModuleName)
	{
		HMODULE hModules[1024];
        unsigned int i;
        DWORD cbNeeded; 
		MODULEENTRY32 mEntry;
		mEntry.dwSize = sizeof(mEntry);

		if( EnumProcessModulesEx(Handle, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_32BIT) )
        {
            for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
            {
                TCHAR szModName[MAX_PATH];


                if ( GetModuleBaseName( Handle, hModules[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                {
                    //MessageBoxA(NULL, szModName, "FUCK", MB_ICONSTOP | MB_OK);
                    if (!strcmp(szModName, ModuleName))
                    {
						MODULEINFO mInfo;
						DWORD cb;
						GetModuleInformation(Handle, hModules[i], &mInfo, cb);
                        return mInfo.SizeOfImage;
                    }
                }
        }
    }
		return (DWORD)0x0;
	}


	bool DataCompare(BYTE* data, BYTE* sign, char* mask)
	{
		for (; *mask; mask++, sign++, data++)
		{
			if (*mask == 'x' && *data != *sign)
			{
				return false;
			}
		}
		return true;
	}

	DWORD GrabSig(DWORD base, DWORD size, BYTE* sign, char* mask)
	{
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		DWORD offset = 0;
		while (offset < size)
		{
			VirtualQueryEx(Handle, (LPCVOID)(base + offset), &mbi, sizeof(MEMORY_BASIC_INFORMATION));
			if (mbi.State != MEM_FREE)
			{
				BYTE* buffer = new BYTE[mbi.RegionSize];
				ReadProcessMemory(Handle, mbi.BaseAddress, buffer, mbi.RegionSize, NULL);
				for (int i = 0; i < mbi.RegionSize; i++)
				{
					if (DataCompare(buffer + i, sign, mask))
					{
						delete[] buffer;
						return (DWORD)mbi.BaseAddress + i;
					}
				}

				delete[] buffer;
			}
			offset += mbi.RegionSize;
		}
		return 0;
	}

	template<class c>
	c Read(DWORD dwAddress)
	{
		c val;
		ReadProcessMemory(Handle, (LPVOID)dwAddress, &val, sizeof(c), NULL);
		return val;
	}

	char ReadChar(DWORD dwAddress) {
		char value;
		ReadProcessMemory(Handle, (LPVOID)dwAddress, &value, sizeof(char), NULL);
		return value;
	}

	std::string ReadString(DWORD dwAddress) {
		std::vector<char> chars;
		int offset = 0x0;
		while (true) {
			char c = ReadChar(dwAddress + offset);
			chars.push_back(c);

			// break at 1 million chars
			if (offset == (sizeof(char) * 1000000)) {
				chars.clear();
				break;
			}

			// break at terminator (end of string)
			if (c == '\0') {
				break;
			}

			// go to next char
			offset += sizeof(char);
		}

		std::string s(chars.begin(), chars.end());
		return s;
	}

	template<class c>
	BOOL Write(DWORD dwAddress, c ValueToWrite)
	{
		return WriteProcessMemory(Handle, (LPVOID)dwAddress, &ValueToWrite, sizeof(c), NULL);
	}

	DWORD GetProcID() { return this->dwPID; }
	HANDLE GetProcHandle() { return this->Handle; }

	Memory()
	{
		this->Handle = NULL;
		this->dwPID = NULL;
		try {
			if (!AttachProcess((wchar_t*)"csgo.exe")) throw 1;
			//this->ClientDLL = GetModule((wchar_t*)"client.dll");
			//this->EngineDLL = GetModule((wchar_t*)"engine.dll");
			this->ClientDLLBase = GetModule("client.dll");
			this->EngineDLLBase = GetModule("engine.dll");
			this->VstdlibDLLBase = GetModule("vstdlib.dll");
			if (this->ClientDLLBase == 0x0) throw 2;
			if (this->EngineDLLBase == 0x0) throw 3;
			if (this->VstdlibDLLBase == 0x0) throw 4;
			this->ClientDLLSize = GetModuleSize("client.dll");
			this->EngineDLLSize = GetModuleSize("engine.dll");
			this->VstdlibDLLSize = GetModuleSize("vstdlib.dll");
		}
		catch (int iEx) {
			switch (iEx)
			{
			case 1: std::cout << "CS:GO must be running"; exit(0); break;
			case 2: std::cout << "Couldn't find Client.dll"; exit(0); break;
			case 3: std::cout << "Couldn't find Engine.dll"; exit(0); break;
			case 4: std::cout << "Couldn't find vstdlib.dll"; exit(0); break;
			default: throw;
			}
		}
		catch (...) {
			MessageBoxA(NULL, "Unhandled exception thrown", "FUCK", MB_ICONSTOP | MB_OK);
			exit(0);
		}
	}

	~Memory()
	{
		CloseHandle(this->Handle);
	}

};