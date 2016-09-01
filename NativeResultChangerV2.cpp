#pragma once
#include "StdInc.h"
#include <scrEngine.h>
#include "natives.h"

/*Native Result changer by ghost30812. A HUGE THX TO SNOOX,HE HELPED A LOT !*/

#include <Windows.h>
#include "MinHook.h"
typedef int Hash;
typedef int Ped;
typedef int Player;
//#pragma comment(lib, "libMinHook.x64.lib")
using namespace rage;


//very early concept code lol /try with minhook now
template <typename T>
bool hookNativev2(UINT64 hash, LPVOID hookFunction, T** trampoline)
{
	if (*reinterpret_cast<LPVOID*>(trampoline) != NULL)
		return true;
	//scrEngine hello;
	auto originalFunction = scrEngine::GetNativeHandler(hash);

	if (originalFunction != 0) {
		MH_STATUS createHookStatus = MH_CreateHook(originalFunction, hookFunction, reinterpret_cast<LPVOID*>(trampoline));
		if (
			((createHookStatus == MH_OK) || (createHookStatus == MH_ERROR_ALREADY_CREATED))
			&& (MH_EnableHook(originalFunction) == MH_OK)
			)
		{
			//Log::Debug("Hooked 0x%#p", hash);
			return true;
		}
	}

	return false;
}

scrEngine::NativeHandler ORIG_IS_DLC_PRESENT = NULL;
void* __cdecl MY_IS_DLC_PRESENTv2(NativeContext *cxt)
{
	Hash DlcHash = cxt->GetArgument<Hash>(0);

	if (DlcHash == 2532323046) { // DEV
								 // game requested dev dlc -> return true;
		cxt->SetResult(0, true);
	}
	else
	{
		ORIG_IS_DLC_PRESENT(cxt);
	}

	return cxt;
}

scrEngine::NativeHandler ORIG_CLEAR_PED_TASKS_IMMEDIATELY = NULL;
void *__cdecl MY_CLEAR_PED_TASKS_IMMEDIATELY(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1))
	{
		ORIG_CLEAR_PED_TASKS_IMMEDIATELY(cxt);
	}
	else
	{
	//	Log::Debug("Prevented CLEAR_PED_TASKS_IMMEDIATELY");
	}

	return cxt;
}

scrEngine::NativeHandler ORIG_CLEAR_PED_TASKS = NULL;
void *__cdecl MY_CLEAR_PED_TASKS(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1))
	{
		ORIG_CLEAR_PED_TASKS(cxt);
	}
	else
	{
	//	Log::Debug("Prevented CLEAR_PED_TASKS");
	}

	return cxt;
}

scrEngine::NativeHandler ORIG_CLEAR_PED_SECONDARY_TASK = NULL;
void *__cdecl MY_CLEAR_PED_SECONDARY_TASK(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1))
	{
		ORIG_CLEAR_PED_SECONDARY_TASK(cxt);
	}
	else
	{
		//Log::Debug("Prevented CLEAR_PED_SECONDARY_TASK");
	}

	return cxt;
}

scrEngine::NativeHandler ORIG_CLONE_PED = NULL;
void *__cdecl MY_CLONE_PED(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1)) // seems to be actually asc order
	{
		ORIG_CLONE_PED(cxt);
	}
	else
	{
		//Log::Debug("Prevented CLONE_PED");
	}

	return cxt;
}

scrEngine::NativeHandler ORIG_NETWORK_SESSION_KICK_PLAYER = NULL;
void *__cdecl MY_NETWORK_SESSION_KICK_PLAYER(NativeContext *cxt)
{
	if (cxt->GetArgument<Player>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1)) {
		ORIG_NETWORK_SESSION_KICK_PLAYER(cxt);
	}
	else
	{
		//Log::Debug("Prevented NETWORK_SESSION_KICK_PLAYER");
	}

	return cxt;
}

scrEngine::NativeHandler ORIG_ADD_OWNED_EXPLOSION = NULL;
void *__cdecl MY_ADD_OWNED_EXPLOSION(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1))
	{
		ORIG_ADD_OWNED_EXPLOSION(cxt);
	}
	else
	{
		//Log::Debug("Prevented ADD_OWNED_EXPLOSION");
	}

	return cxt;
}

scrEngine::NativeHandler CATCHED_1 = NULL;
void *__cdecl MY_ADD_OWNED_EXPLOSION(NativeContext *cxt)
{
	if (cxt->GetArgument<Ped>(0) != NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>(-1))
	{
		CATCHED_1(cxt);
	}
	else
	{
		//Log::Debug("Prevented ADD_OWNED_EXPLOSION");
	}

	return cxt;
}

//NETWORK::NETWORK_IS_MULTIPLAYER_DISABLED

bool AttemptHookNativesv2() {
	static DWORD64 dwThreadCollectionPtr = 0;
/*
	if (!dwThreadCollectionPtr) {
		// scan for GTA Thread Pool
		dwThreadCollectionPtr = Pattern::Scan(g_MainModuleInfo, "48 8B 05 ? ? ? ? 8B CA 4C 8B 0C C8 45 39 51 08");
	}

	if (!dwThreadCollectionPtr
		|| !Pattern::Scan(g_MainModuleInfo, "76 61 49 8B 7A 40 48 8D 0D") // scan for Native Registration Table
		) {
		// too early. GetNativeHandler would log a fatal error and exit the process
		return false;
	}*/
	if (!NativeInvoke::Invoke<NETWORK_IS_IN_SESSION, bool>()) //custom loading screens could cause some issues ... thats why we enfroce in lovely cpp the showing
	{
	return false;
	}
	return true
		//actual hashes missing
		&& hookNativev2(IS_DLC_PRESENT, &MY_IS_DLC_PRESENTv2, &ORIG_IS_DLC_PRESENT)
	/*	&& hookNativev2(0xC1A624FF5CF18419, &MY_NETWORK_SESSION_KICK_PLAYER, &ORIG_NETWORK_SESSION_KICK_PLAYER)
		&& hookNativev2(0x1E2B48EE3EC55DCF, &MY_CLEAR_PED_TASKS_IMMEDIATELY, &ORIG_CLEAR_PED_TASKS_IMMEDIATELY)
		&& hookNativev2(0x27CC98B7C879C320, &MY_CLEAR_PED_TASKS, &ORIG_CLEAR_PED_TASKS)
		&& hookNativev2(0x4191220706130B86, &MY_CLEAR_PED_SECONDARY_TASK, &ORIG_CLEAR_PED_SECONDARY_TASK)
		&& hookNativev2(0x237D19F4F17C2B85, &MY_CLONE_PED, &ORIG_CLONE_PED)
		&& hookNativev2(0x0AEB0F5A0526E37F, &MY_ADD_OWNED_EXPLOSION, &ORIG_ADD_OWNED_EXPLOSION)*/
		// add more hooks here
		;
	
}

DWORD WINAPI lpHookNatives2(LPVOID lpParam) {
	//Log::Debug("Initializing Hooks");

	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		//Log::Fatal("Failed to initialize MinHook");
	}

	while (!AttemptHookNativesv2()) {
		Sleep(100);
	}

	//Log::Debug("Finished hooking");

	return 0;
}

void SpawnHookNativesv2() {
	CreateThread(0, 0, lpHookNatives2, 0, 0, 0);
}