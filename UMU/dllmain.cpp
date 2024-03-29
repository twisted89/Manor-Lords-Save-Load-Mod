// UMU 2022 (An Unreal Engine 4 Third Party Content Creation Unlocker)
// Created By GHFear @ Illusory Software


// INCLUDES ------------------------>

#include "stdafx.h"
#include <stdint.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "Global.h"
#include "MinHook/MinHook.h"
#include "DXGIProxy/proxy.h"
#include "UMUClasses/SigScanner/SigScanners/SigScanner_LooseFileLoader.h"
#include "UMUClasses/FunctionHooks/LooseFileLoadingHooks.h"

#include <shellapi.h>

//------------------------ END OF INCLUDES


// Setup for the dxgi Proxy
EXTERN_C IMAGE_DOS_HEADER __ImageBase; // We might need this some day. Don't remove.
HMODULE ourModule = 0; // Declare our "extern HMODULE ourModule" from proxy.cpp inside dllmain, so we can pass hModule to Proxy_Attach();


//Function Defs -------------------->
DWORD WINAPI RevertToStock(LPVOID lpParam);
void Hooks();
void Initiate_Hooks();

//------------------------ END OF Function Defs



//Set our important Global Vars ------>
char* exe_base = nullptr;

//------------------------ END OF Global Vars

//Start routine
BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		Proxy_Attach();

		Initiate_Hooks();

	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
		Proxy_Detach();

	return TRUE;
}


//Show an error message and start the game without hooking.
DWORD WINAPI RevertToStock(LPVOID lpParam)
{
	
	const int result = MessageBox(NULL, L"Warning! \n\nThe game could be using SteamStub DRM (Use Steamless and / or Goldberg Emulator to get around this) \nor The mod unlocker does not support this title or doesn't support this engine version yet! \n\nClick YES to join our discord so we can quickly add support for your game! (Contact GHFear) \n\nClick NO if you want to play the game vanilla style! \n\nClick CANCEL to Quit the game!", L"Team Illusory", MB_YESNOCANCEL | MB_SYSTEMMODAL);

	switch (result)
	{
	case IDYES:
		ShellExecute(0, 0, L"https://discord.com/invite/Mt3qzgN", 0, 0, SW_SHOW);
		break;
	case IDNO:
		break;
	case IDCANCEL:
		abort();
		break;
	}
	
	return 0;

}

void CreateDebugConsole(const wchar_t* lPConsoleTitle)
{

	HANDLE lStdHandle = 0;
	int hConHandle = 0;
	AllocConsole();
	lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
	SetConsoleTitle(lPConsoleTitle);
	SetConsoleTextAttribute(lStdHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_RED);

	// Redirect CRT standard input, output and error handles to the console window.
	FILE* pNewStdout = nullptr;
	FILE* pNewStderr = nullptr;
	FILE* pNewStdin = nullptr;

	::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
	::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
	::freopen_s(&pNewStdin, "CONIN$", "r", stdin);

	// Clear the error state for all of the C++ standard streams. Attempting to accessing the streams before they refer
	// to a valid target causes the stream to enter an error state. Clearing the error state will fix this problem,
	// which seems to occur in newer version of Visual Studio even when the console has not been read from or written
	// to yet.
	std::cout.clear();
	std::cerr.clear();
	std::cin.clear();

	std::wcout.clear();
	std::wcerr.clear();
	std::wcin.clear();
}


void Hooks()
{
	SigScanner_LooseFileLoader sigScanner_LooseFileLoader; //Create sigscanner object for the loose file loader
	MH_STATUS status = MH_OK;

	//PUBLIC LOOSE FILE HOOKS

	const size_t addr_FindFileInPakFiles = sigScanner_LooseFileLoader.Get_addr_Pakfile__Find(); // address of FindFileInPakFiles
	char* FindFileInPakFiles = exe_base + addr_FindFileInPakFiles;
	char* Patch1Addr = exe_base + 0xFBB1A0;
	char* Patch2Addr = exe_base + 0xFEBFA0;
	char* Patch3Addr = exe_base + 0x1098960;
	char* Patch4Addr = exe_base + 0x1063ff0;
	char* Patch5Addr = exe_base + 0x10F7461;

	// FPakPlatformFile::IsNonPakFilenameAllowed(FString const &)
	const size_t addr_IsNonPakFilenameAllowed = sigScanner_LooseFileLoader.Get_IsNonPakFileNameAllowedAddr(); // address of IsNonPakFilenameAllowed;
	char* IsNonPakFilenameAllowed = exe_base + addr_IsNonPakFilenameAllowed; // exe_base + addr_IsNonPakFilenameAllowed;

	//Create a status profile for whatever is implemented in the sig scanning profile so we can select the proper function hook for what we want to do.
	LooseFileLoadingHooks::LOOSEFILE_STATUS LooseFileStatus = LooseFileLoadingHooks::CheckLooseFileStatus(
		addr_FindFileInPakFiles, 
		addr_IsNonPakFilenameAllowed, 
		sigScanner_LooseFileLoader.p_Find_File_In_PakFile_Version,
		sigScanner_LooseFileLoader.p_bUses_IsNonPakFileNameAllowed
	); //Get LOOSEFILE STATUS

	if (MH_CreateHook(reinterpret_cast<LPVOID>(Patch1Addr), 
		reinterpret_cast<LPVOID>(&LooseFileLoadingHooks::Patch1_hook), 
		reinterpret_cast<LPVOID*>(&LooseFileLoadingHooks::Patch1_orig)) == MH_OK)
	{
		status = MH_EnableHook((void*)Patch1Addr);
		if (status != MH_OK)
			printf("Patch1 Hook failed: %s\n", MH_StatusToString(status));
	}

	if (MH_CreateHook(reinterpret_cast<LPVOID>(Patch2Addr),
		reinterpret_cast<LPVOID>(&LooseFileLoadingHooks::Patch2_hook),
		reinterpret_cast<LPVOID*>(&LooseFileLoadingHooks::Patch2_orig)) == MH_OK)
	{
		status = MH_EnableHook((void*)Patch2Addr);
		if (status != MH_OK)
			printf("Patch2 Hook failed: %s\n", MH_StatusToString(status));
	}

	if (MH_CreateHook(reinterpret_cast<LPVOID>(Patch3Addr),
		reinterpret_cast<LPVOID>(&LooseFileLoadingHooks::Patch3_hook),
		reinterpret_cast<LPVOID*>(&LooseFileLoadingHooks::Patch3_orig)) == MH_OK)
	{
		status = MH_EnableHook((void*)Patch3Addr);
		if (status != MH_OK)
			printf("Patch3 Hook failed: %s\n", MH_StatusToString(status));
	}

	if (MH_CreateHook(reinterpret_cast<LPVOID>(Patch4Addr),
		reinterpret_cast<LPVOID>(&LooseFileLoadingHooks::Patch4_hook),
		reinterpret_cast<LPVOID*>(&LooseFileLoadingHooks::Patch4_orig)) == MH_OK)
	{
		status = MH_EnableHook((void*)Patch4Addr);
		if (status != MH_OK)
			printf("Patch4 Hook failed: %s\n", MH_StatusToString(status));
	}

	lpPatch5Return = Patch5Addr + 0xED;
	if (MH_CreateHook(reinterpret_cast<LPVOID>(Patch5Addr),
		reinterpret_cast<LPVOID>(&hkPatch5),
		&lpPatch5ROrig) == MH_OK)
	{
		status = MH_EnableHook((void*)Patch5Addr);
		if (status != MH_OK)
			printf("Patch5 Hook failed: %s\n", MH_StatusToString(status));
	}

	switch (LooseFileStatus) //DO WHATEVER THE STATUS TELLS US TO DO
	{
	case LooseFileLoadingHooks::LOOSEFILE_VER1_USESISALLOWEDTRUE:
		if (MH_CreateHook((void*)FindFileInPakFiles, 
			LooseFileLoadingHooks::FindFileInPakFiles_Hook, 
			(LPVOID*)&LooseFileLoadingHooks::FindFileInPakFiles_orig) == MH_OK
			)// Create our PakFile__Find_hook with minhook
		{
			status = MH_EnableHook((void*)FindFileInPakFiles); // Enable our PakFile__Find_hook
		}

		if (MH_CreateHook((void*)IsNonPakFilenameAllowed, 
			LooseFileLoadingHooks::IsNonPakFilenameAllowed_Hook, 
			(LPVOID*)&LooseFileLoadingHooks::PakFile__IsNonPakFilenameAllowed_orig) == MH_OK
			)// Create our IsNonPakFilenameAllowed_hook with minhook
		{
			status = MH_EnableHook((void*)IsNonPakFilenameAllowed); // Enable our IsNonPakFilenameAllowed_hook
		}
		break;
	case LooseFileLoadingHooks::LOOSEFILE_VER2_USESISALLOWEDTRUE:
		if (MH_CreateHook((void*)FindFileInPakFiles, 
			LooseFileLoadingHooks::FindFileInPakFiles_2_Hook, 
			(LPVOID*)&LooseFileLoadingHooks::FindFileInPakFiles_2_orig) == MH_OK
			)// Create our PakFile__Find_2_hook with minhook
		{
			status = MH_EnableHook((void*)FindFileInPakFiles); // Enable our PakFile__Find_2_hook
		}


		if (MH_CreateHook((void*)IsNonPakFilenameAllowed, 
			LooseFileLoadingHooks::IsNonPakFilenameAllowed_Hook, 
			(LPVOID*)&LooseFileLoadingHooks::PakFile__IsNonPakFilenameAllowed_orig) == MH_OK
			)// Create our IsNonPakFilenameAllowed_hook with minhook
		{
			status = MH_EnableHook((void*)IsNonPakFilenameAllowed); // Enable our IsNonPakFilenameAllowed_hook
		}
		break;
	case LooseFileLoadingHooks::LOOSEFILE_VER1_USESISALLOWEDFALSE:
		if (MH_CreateHook(
			(void*)FindFileInPakFiles, 
			LooseFileLoadingHooks::FindFileInPakFiles_Hook, 
			(LPVOID*)&LooseFileLoadingHooks::FindFileInPakFiles_orig) == MH_OK
			)// Create our PakFile__Find_hook with minhook
		{
			status = MH_EnableHook((void*)FindFileInPakFiles); // Enable our PakFile__Find_hook
		}
		break;
	case LooseFileLoadingHooks::LOOSEFILE_VER2_USESISALLOWEDFALSE:
		if (MH_CreateHook((void*)FindFileInPakFiles, 
			LooseFileLoadingHooks::FindFileInPakFiles_2_Hook, 
			(LPVOID*)&LooseFileLoadingHooks::FindFileInPakFiles_2_orig) == MH_OK)// Create our PakFile__Find_2_hook with minhook
		{
			status = MH_EnableHook((void*)FindFileInPakFiles); // Enable our PakFile__Find_2_hook
		}
		break;
	case LooseFileLoadingHooks::LOOSEFILE_UNKNOWN:
		CreateThread(NULL, 0, &RevertToStock, NULL, 0, NULL); // Show a revert to stock message and start the game without hooking anything.
		break;
	default:
		CreateThread(NULL, 0, &RevertToStock, NULL, 0, NULL); // Show a revert to stock message and start the game without hooking anything.
		break;
	}

}


void Initiate_Hooks()
{
	exe_base = (char*)GetModuleHandleA(NULL); //Get exe base

	if (!exe_base)
	{
		return;
	}

	//CreateDebugConsole(L"Manor Lords Debug Console");
	//MessageBox(NULL, L"Loaded", L"Test", MB_OK);

	if (MH_Initialize() != MH_OK)
		printf("MH Init failed\n");
	Hooks(); //Run HookPakFile function
}


