#include <windows.h>
#include <winuser.h>
#include <tchar.h>
#include <stdio.h>
#pragma pack(1)

//begin patch code

//performance fix
BYTE pattern1[] = {0x83,0xec,0x18,0x53,0x8b,0x1d,0xa4,0x11,0xe4,0x00};
BYTE pattern1_replacement[] = {0xc3};
BYTE pattern2[] = {0x55,0x8b,0xec,0x83,0xe4,0xf8,0x83,0xec,0x08,0x53,0x8b,0x5e,0x0c,0x57,0x8b,0x7e,0x08,0x8d,0x44,0x24,0x08,0x50,0xff,0x15,0xa4,0x11,0xe4,0x00};
BYTE pattern2_replacement[] = {0xc3};
BYTE pattern3[] = {0xd9,0xee,0x8b,0x35,0xa4,0x11,0xe4,0x00};
BYTE pattern3_replacement[] = {0xeb,0x79};
//auto update disabler
BYTE pattern4[] = {0xb8,0x10,0x00,0x00,0x00,0x39,0x46,0x3c,0x72,0x05};
BYTE pattern4_replacement[] = {0x31,0xc0,0xc3};

DWORD FindPattern(DWORD start, DWORD end, BYTE* pattern, unsigned int patternLength) {
	unsigned int matchLength = 0;
	for (DWORD ptr = start; ptr <= end - patternLength; ptr++) {
		while (((BYTE*)ptr)[matchLength] == pattern[matchLength]) {
			matchLength++;
			if (matchLength == patternLength) return ptr;
		}
		matchLength = 0;
	}
	return NULL;
}

void ReplaceCode(DWORD target, BYTE* replacement, unsigned int replacementLength) {
	DWORD originalProtection;
	VirtualProtect((void*)target, replacementLength, PAGE_EXECUTE_READWRITE, &originalProtection);
	for (unsigned int i = 0; i < replacementLength; i++) {
		((BYTE*)target)[i] = replacement[i];
	}
	VirtualProtect((void*)target, replacementLength, originalProtection, NULL);
}

void PatchGame() {
	DWORD target1 = NULL, target2 = NULL, target3 = NULL, target4 = NULL;
	//find patterns in memory
	for (int count = 0; count < 10; count++) {
		target1 = FindPattern(0x401000, 0xe40e0c, pattern1, sizeof(pattern1));
		target2 = FindPattern(0x401000, 0xe40e0c, pattern2, sizeof(pattern2));
		target3 = FindPattern(0x401000, 0xe40e0c, pattern3, sizeof(pattern3));
		target4 = FindPattern(0x401000, 0xe40e0c, pattern4, sizeof(pattern4));
		if (target1 && target2 && target3 && target4) {
			break; //all patterns found, stop looking for them
		}
		else {
			Sleep(300); //one or more patterns were not found, try again in 300ms, max 10 tries
		}
	}
	if (target1 && target2 && target3 && target4) {
		//all patterns found, apply patch
		ReplaceCode(target1, pattern1_replacement, sizeof(pattern1_replacement));
		ReplaceCode(target2, pattern2_replacement, sizeof(pattern2_replacement));
		ReplaceCode(target3, pattern3_replacement, sizeof(pattern3_replacement));
		ReplaceCode(target4, pattern4_replacement, sizeof(pattern4_replacement));
	}
	else {
		//one or more patterns were not found, show error
		MessageBox(0, "Patching failed! Make sure you're using the GOG version of the game.\n\nThe game will continue, but no patches have been applied.", "WH:MOC Performance Fix - Error", MB_OK | MB_ICONWARNING);
	}
}

//patch code ends here

//begin ASI loader

void LoadASImods() {
	WIN32_FIND_DATA fd;
	char currfile[FILENAME_MAX];
	HANDLE asiFile = FindFirstFile(".\\asi\\*.asi", &fd);
	if (asiFile == INVALID_HANDLE_VALUE) return;
	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			strcpy_s(currfile, ".\\asi\\");
			strcat_s(currfile, fd.cFileName);
			if (!LoadLibrary(currfile)) {
				char errmsg[128 + FILENAME_MAX];
				strcpy_s(errmsg, "ASI mod failed to load: ");
				strcat_s(errmsg, currfile);
				strcat_s(errmsg, "\n\nThe game will continue without this mod");
				MessageBox(0, errmsg, "WH:MOC ASI Loader - Error", MB_OK | MB_ICONWARNING);
			}
		}
	} while (FindNextFile(asiFile, &fd));
	FindClose(asiFile);
}

//end ASI loader, the next code is just for passing through bink calls to the original bink dll that we replaced

HINSTANCE hL = NULL;
FARPROC p[72] = { 0 }; //pointers to original bink calls

void LoadOriginalBink() {
	HINSTANCE hL = LoadLibrary(".\\binkw23.dll");
	if (!hL) {
		MessageBox(0, "Failed to load binkw23.dll\n\nYou did not apply the fix correctly, the game will crash.", "WH:MOC Performance Fix - Error", MB_OK | MB_ICONWARNING);
		return;
	}
	p[0] = GetProcAddress(hL, "_BinkStartAsyncThread@8");
	p[1] = GetProcAddress(hL, "_BinkBufferOpen@16");
	p[2] = GetProcAddress(hL, "_BinkWait@4");
	p[3] = GetProcAddress(hL, "_BinkBufferSetOffset@12");
	p[4] = GetProcAddress(hL, "_BinkRegisterFrameBuffers@8");
	p[5] = GetProcAddress(hL, "_BinkSetIO@4");
	p[6] = GetProcAddress(hL, "_BinkShouldSkip@4");
	p[7] = GetProcAddress(hL, "_BinkClose@4");
	p[8] = GetProcAddress(hL, "_BinkGetRealtime@12");
	p[9] = GetProcAddress(hL, "_BinkSetError@4");
	//p[10] = GetProcAddress(hL, "EntryPoint"); //no need to load this, LoadLibrary already called it
	p[11] = GetProcAddress(hL, "_BinkBufferGetError@0");
	p[12] = GetProcAddress(hL, "_BinkGetTrackData@8");
	p[13] = GetProcAddress(hL, "_BinkOpenWaveOut@4");
	p[14] = GetProcAddress(hL, "_BinkDoFrameAsync@12");
	p[15] = GetProcAddress(hL, "_BinkBufferSetDirectDraw@8");
	p[16] = GetProcAddress(hL, "_BinkGetKeyFrame@12");
	p[17] = GetProcAddress(hL, "_BinkSetMixBins@16");
	p[18] = GetProcAddress(hL, "_BinkSetVolume@12");
	p[19] = GetProcAddress(hL, "_BinkSetMemory@8");
	p[20] = GetProcAddress(hL, "_BinkBufferGetDescription@4");
	p[21] = GetProcAddress(hL, "_BinkCheckCursor@20");
	p[22] = GetProcAddress(hL, "_BinkDX8SurfaceType@4");
	p[23] = GetProcAddress(hL, "_RADTimerRead@0");
	p[24] = GetProcAddress(hL, "_BinkNextFrame@4");
	p[25] = GetProcAddress(hL, "_BinkGetRects@8");
	p[26] = GetProcAddress(hL, "_BinkCopyToBuffer@28");
	p[27] = GetProcAddress(hL, "_BinkBufferClear@8");
	p[28] = GetProcAddress(hL, "_BinkRestoreCursor@4");
	p[29] = GetProcAddress(hL, "_BinkPause@8");
	p[30] = GetProcAddress(hL, "_BinkDoFrameAsyncWait@8");
	p[31] = GetProcAddress(hL, "_BinkOpenTrack@8");
	p[32] = GetProcAddress(hL, "_BinkBufferBlit@12");
	p[33] = GetProcAddress(hL, "_BinkGoto@12");
	p[34] = GetProcAddress(hL, "_BinkControlBackgroundIO@8");
	p[35] = GetProcAddress(hL, "_BinkCloseTrack@4");
	p[36] = GetProcAddress(hL, "_BinkBufferSetResolution@12");
	p[37] = GetProcAddress(hL, "_BinkIsSoftwareCursor@8");
	p[38] = GetProcAddress(hL, "_BinkGetSummary@8");
	p[39] = GetProcAddress(hL, "_BinkGetFrameBuffersInfo@8");
	p[40] = GetProcAddress(hL, "_BinkControlPlatformFeatures@8");
	p[41] = GetProcAddress(hL, "_BinkBufferSetScale@12");
	p[42] = GetProcAddress(hL, "_BinkBufferCheckWinPos@12");
	p[43] = GetProcAddress(hL, "_BinkLogoAddress@0");
	p[44] = GetProcAddress(hL, "_BinkBufferClose@4");
	p[45] = GetProcAddress(hL, "_BinkOpen@8");
	p[46] = GetProcAddress(hL, "_BinkGetTrackMaxSize@8");
	p[47] = GetProcAddress(hL, "_BinkDoFrame@4");
	p[48] = GetProcAddress(hL, "_BinkRequestStopAsyncThread@4");
	p[49] = GetProcAddress(hL, "_BinkSetFrameRate@8");
	p[50] = GetProcAddress(hL, "_BinkCopyToBufferRect@44");
	p[51] = GetProcAddress(hL, "_BinkGetPalette@4");
	p[52] = GetProcAddress(hL, "_BinkSetSoundSystem@8");
	p[53] = GetProcAddress(hL, "_BinkGetError@0");
	p[54] = GetProcAddress(hL, "_BinkSetSoundTrack@8");
	p[55] = GetProcAddress(hL, "_BinkSetPan@12");
	p[56] = GetProcAddress(hL, "_BinkSetVideoOnOff@8");
	p[57] = GetProcAddress(hL, "_BinkSetSoundOnOff@8");
	p[58] = GetProcAddress(hL, "_BinkBufferSetHWND@8");
	p[59] = GetProcAddress(hL, "_BinkBufferUnlock@4");
	p[60] = GetProcAddress(hL, "_BinkGetTrackType@8");
	p[61] = GetProcAddress(hL, "_BinkBufferLock@4");
	p[62] = GetProcAddress(hL, "_BinkSetSimulate@4");
	p[63] = GetProcAddress(hL, "_BinkWaitStopAsyncThread@4");
	p[64] = GetProcAddress(hL, "_BinkOpenMiles@4");
	p[65] = GetProcAddress(hL, "_BinkService@4");
	p[66] = GetProcAddress(hL, "_BinkDDSurfaceType@4");
	p[67] = GetProcAddress(hL, "_BinkGetTrackID@8");
	p[68] = GetProcAddress(hL, "_BinkDX9SurfaceType@4");
	p[69] = GetProcAddress(hL, "_BinkOpenDirectSound@4");
	p[70] = GetProcAddress(hL, "_BinkSetMixBinVolumes@20");
	p[71] = GetProcAddress(hL, "_BinkSetIOSize@4");
	//make sure we loaded all the pointers correctly (except for p[10], we don't need that)
	for (int i = 0; i < 72; i++) {
		if (i == 10) continue;
		if (!p[i]) {
			MessageBox(0, "Wrong version of binkw23.dll! Make sure you're using the GOG version of the game.\n\nThe game will continue but it may not work properly", "WH:MOC Performance Fix - Error", MB_OK | MB_ICONWARNING);
			return;
		}
	}
}

DWORD WINAPI MainThread(LPVOID lpParam){
	LoadOriginalBink();
	PatchGame();
	LoadASImods();
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, MainThread, &dwThrdParam, 0, &dwThreadId);
	}
	if (reason == DLL_PROCESS_DETACH)
		FreeLibrary(hL);
	return TRUE;
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkStartAsyncThread(int a, int b)
{
__asm
{
	jmp p[0 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferOpen(int a, int b, int c, int d)
{
__asm
{
	jmp p[1 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkWait(int a)
{
__asm
{
	jmp p[2 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferSetOffset(int a, int b, int c)
{
__asm
{
	jmp p[3 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkRegisterFrameBuffers(int a, int b)
{
__asm
{
	jmp p[4 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetIO(int a)
{
__asm
{
	jmp p[5 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkShouldSkip(int a)
{
__asm
{
	jmp p[6 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkClose(int a)
{
__asm
{
	jmp p[7 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetRealtime(int a, int b, int c)
{
__asm
{
	jmp p[8 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetError(int a)
{
__asm
{
	jmp p[9 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferGetError()
{
__asm
{
	jmp p[11 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetTrackData(int a, int b)
{
__asm
{
	jmp p[12 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkOpenWaveOut(int a)
{
__asm
{
	jmp p[13 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkDoFrameAsync(int a, int b, int c)
{
__asm
{
	jmp p[14 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferSetDirectDraw(int a, int b)
{
__asm
{
	jmp p[15 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetKeyFrame(int a, int b, int c)
{
__asm
{
	jmp p[16 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetMixBins(int a, int b, int c, int d)
{
__asm
{
	jmp p[17 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetVolume(int a, int b, int c)
{
__asm
{
	jmp p[18 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetMemory(int a, int b)
{
__asm
{
	jmp p[19 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferGetDescription(int a)
{
__asm
{
	jmp p[20 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkCheckCursor(int a, int b, int c, int d, int e)
{
__asm
{
	jmp p[21 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkDX8SurfaceType(int a)
{
__asm
{
	jmp p[22 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall RADTimerRead()
{
__asm
{
	jmp p[23 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkNextFrame(int a)
{
__asm
{
	jmp p[24 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetRects(int a, int b)
{
__asm
{
	jmp p[25 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkCopyToBuffer(int a, int b, int c, int d, int e, int f, int g)
{
__asm
{
	jmp p[26 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferClear(int a, int b)
{
__asm
{
	jmp p[27 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkRestoreCursor(int a)
{
__asm
{
	jmp p[28 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkPause(int a, int b)
{
__asm
{
	jmp p[29 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkDoFrameAsyncWait(int a, int b)
{
__asm
{
	jmp p[30 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkOpenTrack(int a, int b)
{
__asm
{
	jmp p[31 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferBlit(int a, int b, int c)
{
__asm
{
	jmp p[32 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGoto(int a, int b, int c)
{
__asm
{
	jmp p[33 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkControlBackgroundIO(int a, int b)
{
__asm
{
	jmp p[34 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkCloseTrack(int a)
{
__asm
{
	jmp p[35 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferSetResolution(int a, int b, int c)
{
__asm
{
	jmp p[36 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkIsSoftwareCursor(int a, int b)
{
__asm
{
	jmp p[37 * 4];
}
}


extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetSummary(int a, int b)
{
__asm
{
	jmp p[38 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetFrameBuffersInfo(int a, int b)
{
__asm
{
	jmp p[39 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkControlPlatformFeatures(int a, int b)
{
__asm
{
	jmp p[40 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferSetScale(int a, int b, int c)
{
__asm
{
	jmp p[41 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferCheckWinPos(int a, int b, int c)
{
__asm
{
	jmp p[42 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkLogoAddress()
{
__asm
{
	jmp p[43 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferClose(int a)
{
__asm
{
	jmp p[44 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkOpen(int a, int b)
{
__asm
{
	jmp p[45 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetTrackMaxSize(int a, int b)
{
__asm
{
	jmp p[46 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkDoFrame(int a)
{
__asm
{
	jmp p[47 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkRequestStopAsyncThread(int a)
{
__asm
{
	jmp p[48 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetFrameRate(int a, int b)
{
__asm
{
	jmp p[49 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkCopyToBufferRect(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k)
{
__asm
{
	jmp p[50 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetPalette(int a)
{
__asm
{
	jmp p[51 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetSoundSystem(int a, int b)
{
__asm
{
	jmp p[52 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetError()
{
__asm
{
	jmp p[53 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetSoundTrack(int a, int b)
{
__asm
{
	jmp p[54 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetPan(int a, int b, int c)
{
__asm
{
	jmp p[55 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetVideoOnOff(int a, int b)
{
__asm
{
	jmp p[56 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetSoundOnOff(int a, int b)
{
__asm
{
	jmp p[57 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferSetHWND(int a, int b)
{
__asm
{
	jmp p[58 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferUnlock(int a)
{
__asm
{
	jmp p[59 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetTrackType(int a, int b)
{
__asm
{
	jmp p[60 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkBufferLock(int a)
{
__asm
{
	jmp p[61 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetSimulate(int a)
{
__asm
{
	jmp p[62 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkWaitStopAsyncThread(int a)
{
__asm
{
	jmp p[63 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkOpenMiles(int a)
{
__asm
{
	jmp p[64 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkService(int a)
{
__asm
{
	jmp p[65 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkDDSurfaceType(int a)
{
__asm
{
	jmp p[66 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkGetTrackID(int a, int b)
{
__asm
{
	jmp p[67 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkDX9SurfaceType(int a)
{
__asm
{
	jmp p[68 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkOpenDirectSound(int a)
{
__asm
{
	jmp p[69 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetMixBinVolumes(int a, int b, int c, int d, int e)
{
__asm
{
	jmp p[70 * 4];
}
}

extern "C" __declspec(dllexport) __declspec(naked) void __stdcall BinkSetIOSize(int a)
{
__asm
{
	jmp p[71 * 4];
}
}
