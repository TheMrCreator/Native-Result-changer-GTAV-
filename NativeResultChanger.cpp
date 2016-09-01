#pragma once
#include <StdInc.h>
#include <scrEngine.h>
#include "natives.h"
#include "NativeUnhook.h"
#include "matrixpattern.h"
#include "Hooking.h"
#define MAX_HOOKS 1000
/*Native Result changer by ghost30812. A HUGE THX TO SNOOX,HE HELPED A LOT !*/
static bool researchmode = true;
	typedef struct _HOOK_INFO
	{
		ULONG_PTR Function;
		ULONG_PTR Hook;
		ULONG_PTR OrigBytes;
	} HOOK_INFO, *PHOOK_INFO;

	HOOK_INFO HookInfo[MAX_HOOKS];
	UINT NumberOfHooks = 0;
	bool hookFailed = false;
	BYTE *pOrigBytesBuffer = NULL;



	HOOK_INFO *GetHookInfoFromFunction(ULONG_PTR OriginalFunction)
	{
		if (NumberOfHooks == 0)
			return NULL;

		for (UINT x = 0; x < NumberOfHooks; x++)
		{
			if (HookInfo[x].Function == OriginalFunction)
				return &HookInfo[x];
		}

		return NULL;
	}

	void WriteJump(void *pAddress, ULONG_PTR JumpTo)
	{
		// be extra safe and overwrite memory with a single memcpy
		BYTE *pJumpInstructionBuffer = (BYTE *)malloc(14);
		BYTE *pCur = pJumpInstructionBuffer;

		*pCur = 0xff;		// jmp [rip+addr]
		*(++pCur) = 0x25;
		*((DWORD *) ++pCur) = 0; // addr = 0
		pCur += sizeof(DWORD);
		*((ULONG_PTR *)pCur) = JumpTo;

		DWORD dwOldProtect = 0;
		VirtualProtect(pAddress, 14, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy(pAddress, pJumpInstructionBuffer, 14);

		DWORD dwBuf = 0;
		VirtualProtect(pAddress, 14, dwOldProtect, &dwBuf);

		free(pJumpInstructionBuffer);
	}

	void *BackupOrigBytes(ULONG_PTR originalFunction)
	{
		if (pOrigBytesBuffer == NULL) {
			pOrigBytesBuffer = (BYTE *)malloc(MAX_HOOKS * 14);
		}

		VOID *pOrigBytes = (VOID *)&pOrigBytesBuffer[NumberOfHooks * 14];

		memcpy(&pOrigBytesBuffer[NumberOfHooks * 14], (void *)originalFunction, 14);

		return pOrigBytes;
	}

	bool hookNative(UINT64 hash, ULONG_PTR hookFunction = NULL)
	{
		auto originalFunction = rage::scrEngine::GetNativeHandler(hash);

		if (originalFunction == 0) {
			hookFailed = true;
			return false;
		}

		HOOK_INFO *hinfo = GetHookInfoFromFunction((ULONG_PTR)originalFunction);

		if (hinfo) {
			WriteJump(originalFunction, hinfo->Hook);
		}
		else
		{
			if (NumberOfHooks == (MAX_HOOKS - 1))
				return false;

			VOID *pOrigBytes = BackupOrigBytes((ULONG_PTR)originalFunction);

			HookInfo[NumberOfHooks].Function = (ULONG_PTR)originalFunction;
			HookInfo[NumberOfHooks].OrigBytes = (ULONG_PTR)pOrigBytes;
			HookInfo[NumberOfHooks].Hook = hookFunction;

			NumberOfHooks++;

			WriteJump(originalFunction, hookFunction);
		}

		return true;
	}

	void unhookNative(UINT64 hash) {
		auto originalFunction = rage::scrEngine::GetNativeHandler(hash);

		HOOK_INFO *hinfo = GetHookInfoFromFunction((ULONG_PTR)originalFunction);

		DWORD dwOldProtect = 0;

		VirtualProtect(originalFunction, 14, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy((void *)hinfo->Function, (void *)hinfo->OrigBytes, 14);

		DWORD dwBuf = 0;
		VirtualProtect(originalFunction, 14, dwOldProtect, &dwBuf);
	}

	void* __cdecl MY_IS_DLC_PRESENT(NativeContext *cxt)
	{
		int DlcHash = cxt->GetArgument<int>(0);

		if (DlcHash == 2532323046) { // DEV
									 // game game requested dev dlc -> return true;
			cxt->SetResult(0, true);
			return cxt;
		}

		// restore original function
		unhookNative(IS_DLC_PRESENT);

		// get result of original function
		BOOL result = NativeInvoke::Invoke<IS_DLC_PRESENT, bool>(cxt->GetArgument<int>(0));
		//BOOL result = DLC2::IS_DLC_PRESENT(cxt->GetArgument<int>(0));
		cxt->SetResult(0, result);

		// hook us up again
		hookNative(IS_DLC_PRESENT);
		return cxt;
	}


	void* __cdecl MY_NETWORK_ARE_ROS_AVAILABLE(NativeContext *cxt)
	{
	
		
		unhookNative(NETWORK_ARE_ROS_AVAILABLE);

		// get result of original function
		BOOL result = NativeInvoke::Invoke<NETWORK_ARE_ROS_AVAILABLE, bool>(cxt->GetArgument<int>(0));
		//force result to be true 
		cxt->SetResult(0, result);

		// hook us up again
		hookNative(NETWORK_ARE_ROS_AVAILABLE);
		return cxt;

	}

	void* __cdecl GetPlayerName(NativeContext *cxt) //well patches like it should but has to be done on strtup
	{
		int player = cxt->GetArgument<int>(0);

		if (player == NativeInvoke::Invoke<PLAYER_ID, int>(0)) {
			
			cxt->SetResult(0, "0x542");
			return cxt;
		}
		unhookNative(PLAYER_ID);
		char *result = NativeInvoke::Invoke<GET_PLAYER_NAME, char*>(player);
	//	char *result = PLAYER::GET_PLAYER_NAME(player);
		cxt->SetResult(0, result);

		hookNative(PLAYER_ID);
		return cxt;
	}

	void* __cdecl MY_IS_DURANGO_VERSION(NativeContext *cxt) //very likely crash
	{
		unhookNative(IS_DURANGO_VERSION);
		BOOL result = NativeInvoke::Invoke<IS_DURANGO_VERSION, bool>(cxt->GetArgument<int>(0));
		cxt->SetResult(0, result);
		hookNative(IS_DURANGO_VERSION);
		return cxt;

	}

	void* __cdecl MY_IS_PC_VERSION(NativeContext *cxt)
	{
		unhookNative(IS_PC_VERSION);
		cxt->SetResult(0, false);
		hookNative(IS_PC_VERSION);
		return cxt;

	}
	void hook_customnatives()
	{
		hookNative(IS_DLC_PRESENT, (ULONG_PTR)&MY_IS_DLC_PRESENT); //well why not on keypress bitch
		hookNative(NETWORK_ARE_ROS_AVAILABLE, (ULONG_PTR)&MY_NETWORK_ARE_ROS_AVAILABLE); //we are fucking online,no discussion

	}

	/*template <typename DD>
	void* __cdecl MY_TEMPLATE_FUNCTION(NativeContext *hook, uint64_t nativehash,DD returnval)
	{
		unhookNative(nativehash);
		cxt->SetResult(0, returnval);
		hookNative(nativehash);
		return cxt;
	}

	template <typename Dx>
	void CUSTOMHOOK(uint64_t mynewhash,Dx returnval)
	{
		hookNative(mynewhash, (ULONG_PTR)&MY_TEMPLATE_FUNCTION(,mynewhash,returnval)); //well why not on keypress bitch
	}*/

	static bool ucount = false;
	bool AttemptHookNatives() {
	//	static DWORD64 dwThreadCollectionPtr = 0;
	//		MODULEINFO me;
		/*	if (!dwThreadCollectionPtr) {
				// scan for GTA Thread Pool

				dwThreadCollectionPtr = Pattern::Scan(me,"48 8B 05 ? ? ? ? 8B CA 4C 8B 0C C8 45 39 51 08");
			}

			if (!dwThreadCollectionPtr
				|| !Pattern::Scan(me, "76 61 49 8B 7A 40 48 8D 0D") // scan for Native Registration Table
				) {
				// too early. GetNativeHandler would log a fatal error and exit the process
				return false;
			//	hook::pattern("74 17 48 8B C8 E8 ? ? ? ? 48 8D 0D").count(1).get(0).get<char>(13);
			}*/


		//bool present = hook::pattern("74 17 48 8B C8 E8 ? ? ? ? 48 8D 0D").count(1).get(0).get<bool>(13);
	/*	if (!hook::pattern("76 61 49 8B 7A 40 48 8D 0D").count(1).get(0).get<char>(9))
		{
			return false;
		}*/
		if (!NativeInvoke::Invoke<NETWORK_IS_IN_SESSION, bool>()) //custom loading screens could cause some issues ... thats why we enfroce in lovely cpp the showing
		{
				return false;
		}
	/*		if (!ucount)
			{
				goto loc; //prevent the hook from illegal calls
			}
		loc: return false;
		}
		if (NativeInvoke::Invoke<IS_LOADING_PROMPT_BEING_DISPLAYED, bool>()) //since the memory isnt loaded again we can disable the watchdog
		{
			ucount = true;
			MessageBoxA(NULL, "2patch", "", MB_OK);
		}*/
		//ucount += 1;
		hookFailed = false;

	//	hookNative(PLAYER_ID, (ULONG_PTR)&GetPlayerName); //we now know its to late called
		hookNative(IS_DLC_PRESENT, (ULONG_PTR)&MY_IS_DLC_PRESENT); //well why not on keypress bitch
		hookNative(NETWORK_ARE_ROS_AVAILABLE, (ULONG_PTR)&MY_NETWORK_ARE_ROS_AVAILABLE); //we are fucking online,no discussion
		if (researchmode)
		{
			//hook the crazy shit here lol
		//	hookNative(IS_PC_VERSION, (ULONG_PTR)&MY_IS_PC_VERSION);

		//	hookNative(IS_DURANGO_VERSION, (ULONG_PTR)&MY_IS_DURANGO_VERSION);

		}
		// add more hooks here
		MessageBoxA(NULL, "SUP", "", MB_OK);
		return !hookFailed;
	}

	DWORD WINAPI lpHookNatives(LPVOID lpParam) {
		while (!AttemptHookNatives()) {
			Sleep(100);
		}

		return 0;
	}
	
	void SpawnHookNatives() {
	//	CreateThread(0, 0, lpHookNatives, 0, 0, 0);
	}
