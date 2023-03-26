#pragma once
#include <sstream>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include "../../stdafx.h"

namespace LooseFileLoadingHooks

{

	const wchar_t* GameStart = L"../../../"; // This is at the start of every game path


	// LOOSEFILE Error Codes.
	typedef enum LOOSEFILE_STATUS
	{
		LOOSEFILE_VER1_USESISALLOWEDTRUE = 0,

		LOOSEFILE_VER2_USESISALLOWEDTRUE = 1,

		LOOSEFILE_VER1_USESISALLOWEDFALSE,

		LOOSEFILE_VER2_USESISALLOWEDFALSE,

		LOOSEFILE_UNKNOWN

	}
	LOOSEFILE_STATUS;


	LOOSEFILE_STATUS CheckLooseFileStatus(
		const size_t param_addr_FindFileInPakFiles, 
		const size_t param_addr_IsNonPakFilenameAllowed, 
		int param_version, 
		bool p_bUses_IsNonPakFileNameAllowed
	)	
	{
		LOOSEFILE_STATUS status;

		if (param_addr_FindFileInPakFiles != -1 && param_addr_IsNonPakFilenameAllowed != -1 && param_version == 1 && p_bUses_IsNonPakFileNameAllowed == true)
		{
			status = LOOSEFILE_VER1_USESISALLOWEDTRUE;
		}
		else if (param_addr_FindFileInPakFiles != -1 && param_addr_IsNonPakFilenameAllowed != -1 && param_version == 2 && p_bUses_IsNonPakFileNameAllowed == true)
		{
			status = LOOSEFILE_VER2_USESISALLOWEDTRUE;
		}
		else if (param_addr_IsNonPakFilenameAllowed == -1 && param_addr_FindFileInPakFiles != -1 && param_version == 1 && p_bUses_IsNonPakFileNameAllowed == false)
		{
			status = LOOSEFILE_VER1_USESISALLOWEDFALSE;
		}
		else if (param_addr_IsNonPakFilenameAllowed == -1 && param_addr_FindFileInPakFiles != -1 && param_version == 2 && p_bUses_IsNonPakFileNameAllowed == false)
		{
			status = LOOSEFILE_VER2_USESISALLOWEDFALSE;
		}
		else
		{
			status = LOOSEFILE_UNKNOWN;
		}

		return status;
	}


	//Check if file exists at path
	bool DoesFileExist(LPCWSTR path)
	{
		DWORD dwAttrib = GetFileAttributesW(path);
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}


	// FindFileInPakFiles
	typedef void* (*FindFileInPakFiles_ptr)(void* PakFile, void* Filename, void* FileEntry);
	FindFileInPakFiles_ptr FindFileInPakFiles_orig;

	// PakFile::Find hook: this will check for any loose file with the same filename, and if a loose file is found will return false (ie: saying that the .pak doesn't contain it)
	// FindFileInPakFiles Version 1
	void* __fastcall FindFileInPakFiles_Hook(void* PakFile, void* Filename, void* FileEntry)
	{

		const TCHAR* fname = *(TCHAR**)Filename;

		if (Filename && wcsstr(fname, GameStart) && DoesFileExist(fname)) 
		{


			return 0; // The file exists outside the PAK and, return false here so the game believes that it doesn't exists in the PAK.
		}

		return FindFileInPakFiles_orig(PakFile, Filename, FileEntry);
	}

	typedef void* (*Patch1_Def)(intptr_t p1, intptr_t p2);
	Patch1_Def Patch1_orig = nullptr;

	void* __fastcall Patch1_hook(intptr_t p1, intptr_t p2)
	{
		if (*reinterpret_cast<uint64_t*>(p1 + 0x3D0) == 0)
			return nullptr;

		return Patch1_orig(p1, p2);
	}

	typedef void* (*Patch2_Def)(intptr_t p1, intptr_t p2, intptr_t p3);
	Patch2_Def Patch2_orig = nullptr;

	void* __fastcall Patch2_hook(intptr_t p1, intptr_t p2, intptr_t p3)
	{
		if (*reinterpret_cast<uint64_t*>(p1 + 0x7A) == 0)
			return nullptr;

		return Patch2_orig(p1, p2, p3);
	}

	typedef void* (*Patch3_Def)(intptr_t p1, int p2, int p3);
	Patch3_Def Patch3_orig = nullptr;

	void* __fastcall Patch3_hook(intptr_t p1, int p2, int p3)
	{
		auto v12 = *(intptr_t*)(p1 + 2112) + 144 * p2;
		if (*(int*)v12 == p3 && *(int*)(v12 + 72) > 0)
			return nullptr;
		auto v13 = *(intptr_t**)(v12 + 128);
		for (auto i = &v13[*(int*)(v12 + 136)]; v13 != i; ++v13)
		{
			if (v13 == 0 || *v13 == 0)
				return nullptr;
		}

		return Patch3_orig(p1, p2, p3);
	}

	typedef void* (*Patch4_Def)(intptr_t p1);
	Patch4_Def Patch4_orig = nullptr;

	void* __fastcall Patch4_hook(intptr_t p1)
	{
		if (!p1)
			return nullptr;

		return Patch4_orig(p1);
	}

	typedef void* (*Patch5_Def)(intptr_t p1);
	Patch5_Def Patch5_orig = nullptr;

	void* __fastcall Patch5_hook(intptr_t p1)
	{
		if (!p1)
			return nullptr;

		return Patch5_orig(p1);
	}


	// FindFileInPakFiles 2
	typedef void* (*FindFileInPakFiles_2_ptr)(void* PakFile, void* Filename, void** FileEntry);
	FindFileInPakFiles_2_ptr FindFileInPakFiles_2_orig;

	// PakFile::Find hook: this will check for any loose file with the same filename, and if a loose file is found will return false (ie: saying that the .pak doesn't contain it)
	// FindFileInPakFiles Version 2
	void* __fastcall FindFileInPakFiles_2_Hook(void* PakFile, TCHAR* Filename, void** FileEntry)
	{

		if (FileEntry)
		{
			*FileEntry = nullptr;
		}
			

		if (Filename && wcsstr(Filename, GameStart) && DoesFileExist(Filename))
		{
			return 0; // The file exists outside the PAK and, return false here so the game believes that it doesn't exists in the PAK.
		}
			

		return FindFileInPakFiles_2_orig(PakFile, Filename, FileEntry);
	}





	// ISNONPAKFILENAMEALLOWED
	typedef __int64* (*PakFile__IsNonPakFilenameAllowed_ptr)(void* thisptr, void* InFilename);
	PakFile__IsNonPakFilenameAllowed_ptr PakFile__IsNonPakFilenameAllowed_orig;

	// This allows us to enter the if statement in IFileHandle* FPakPlatformFile::OpenRead where we can then pass the Filename of our loose file so it loads from loose files only if they are available.
	__int64 __fastcall IsNonPakFilenameAllowed_Hook(void* thisptr, void* InFilename)
	{
		// 4.0 ---> 4.11 doesn't have access to this function and doesn't need it.
		// 4.12 ---> 4.27 Has access to this function and does need it to return 1 to load files outside of a pak.
		return 1;
	}


}

