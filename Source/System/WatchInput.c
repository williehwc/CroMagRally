// WATCH INPUT
// (C) 2022 Iliyas Jorio
// (C) 2022 Willie Chang
// This file is part of Cro-Mag Rally. https://github.com/williehwc/CroMagRally

#ifdef WATCH
#include "game.h"

/***************/
/* CONSTANTS   */
/***************/

enum
{
	KEYSTATE_ACTIVE_BIT		= 0b001,
	KEYSTATE_CHANGE_BIT		= 0b010,
	KEYSTATE_IGNORE_BIT		= 0b100,

	KEYSTATE_OFF			= 0b000,
	KEYSTATE_PRESSED		= KEYSTATE_ACTIVE_BIT | KEYSTATE_CHANGE_BIT,
	KEYSTATE_HELD			= KEYSTATE_ACTIVE_BIT,
	KEYSTATE_UP				= KEYSTATE_OFF | KEYSTATE_CHANGE_BIT,
	KEYSTATE_IGNOREHELD		= KEYSTATE_OFF | KEYSTATE_IGNORE_BIT,
};

#define kJoystickDeadZone				(33 * 32767 / 100)
#define kJoystickDeadZone_UI			(66 * 32767 / 100)
#define kJoystickDeadZoneFrac			(kJoystickDeadZone / 32767.0f)
#define kJoystickDeadZoneFracSquared	(kJoystickDeadZoneFrac * kJoystickDeadZoneFrac)

/**********************/
/*     PROTOTYPES     */
/**********************/

typedef uint8_t KeyState;

Boolean				gUserPrefersGamepad = false;

extern Byte gInputFlags;
extern bool gInputDecel;
extern bool gInputBrake;
extern float gInputSteer;

#pragma mark -

/**********************/
/* PRIVATE FUNCTIONS  */
/**********************/

bool readAndClearInputFlag(Byte inputFlag) {
    if (gInputFlags & inputFlag) {
        gInputFlags ^= inputFlag;
        return true;
    } else {
        return false;
    }
}

#pragma mark -

/**********************/
/* PUBLIC FUNCTIONS   */
/**********************/

void DoSDLMaintenance(void)
{
}

#pragma mark -

Boolean GetKeyState(uint16_t sdlScancode)
{
    return false;
}

Boolean GetNewKeyState(uint16_t sdlScancode)
{
    return false;
}

#pragma mark -

Boolean GetClickState(int mouseButton)
{
    return false;
}

Boolean GetNewClickState(int mouseButton)
{
    return false;
}

#pragma mark -

Boolean GetNeedState(int needID, int playerID)
{
    return GetNewNeedStateAnyP(needID);
}

Boolean GetNeedStateAnyP(int needID)
{
    return GetNewNeedStateAnyP(needID);
}

Boolean GetNewNeedState(int needID, int playerID)
{
    return GetNewNeedStateAnyP(needID);
}

Boolean GetNewNeedStateAnyP(int needID)
{
    Byte inputFlag;
    switch (needID) {
        case kControlBit_Forward:
            if (gTrackNum == TRACK_NUM_ATLANTIS) {
                return gInputBrake;
            } else {
                return !gInputDecel && !gInputBrake;
            }
        case kControlBit_Backward:
            if (gTrackNum == TRACK_NUM_ATLANTIS) {
                return gInputDecel;
            } else {
                return gInputBrake;
            }
        case kNeed_ThrowForward:
            inputFlag = INPUT_FLAG_FWRD;
            break;
        case kNeed_ThrowBackward:
            inputFlag = INPUT_FLAG_BWRD;
            break;
        case kNeed_UILeft:
            inputFlag = INPUT_FLAG_UP;
            break;
        case kNeed_UIRight:
            inputFlag = INPUT_FLAG_DN;
            break;
        case kNeed_UIUp:
            inputFlag = INPUT_FLAG_UP;
            break;
        case kNeed_UIDown:
            inputFlag = INPUT_FLAG_DN;
            break;
        case kNeed_UIConfirm:
            inputFlag = INPUT_FLAG_OK;
            break;
        case kNeed_UIBack:
            inputFlag = INPUT_FLAG_ESC;
            break;
        case kNeed_UIPause:
            inputFlag = INPUT_FLAG_PAUSE;
            break;
        default:
            return false;
    }
    return readAndClearInputFlag(inputFlag);
}

Boolean UserWantsOut(void)
{
	return GetNewNeedStateAnyP(kNeed_UIConfirm)
		|| GetNewNeedStateAnyP(kNeed_UIBack)
		|| GetNewNeedStateAnyP(kNeed_UIPause)
//		|| GetNewClickState(SDL_BUTTON_LEFT)
        ;
}

Boolean IsCheatKeyComboDown(void)
{
    return false;
}

#pragma mark -

OGLVector2D GetAnalogSteering(int playerID)
{
    OGLVector2D steer = {gInputSteer * 150, 0};
    return steer;
}

#pragma mark -

/****************************** SDL JOYSTICK FUNCTIONS ********************************/

int GetNumControllers(void)
{
    return 0;
}

static int FindFreeControllerSlot()
{
    return -1;
}

static int GetControllerSlotFromJoystick(int joystickIndex)
{
    return -1;
}

void Rumble(float strength, uint32_t ms)
{
	#if 0	// TODO: Rumble for specific player
	if (NULL == gSDLController || !gGamePrefs.gamepadRumble)
		return;

#if !(SDL_VERSION_ATLEAST(2,0,9))
	#warning Rumble support requires SDL 2.0.9 or later
#else
	SDL_GameControllerRumble(gSDLController, (Uint16)(strength * 65535), (Uint16)(strength * 65535), ms);
#endif
	#endif
}

static void CloseController(int controllerSlot)
{
}

static void MoveController(int oldSlot, int newSlot)
{
}

static void CompactControllerSlots(void)
{
}

static void TryFillUpVacantControllerSlots(void)
{
}

void LockPlayerControllerMapping(void)
{
}

void UnlockPlayerControllerMapping(void)
{
}

const char* GetPlayerName(int whichPlayer)
{
	static char playerName[64];

	snprintf(playerName, sizeof(playerName),
			"%s %d", Localize(STR_PLAYER), whichPlayer+1);

	return playerName;
}

const char* GetPlayerNameWithInputDeviceHint(int whichPlayer)
{
    return "\0";
}

#pragma mark -

void ResetDefaultKeyboardBindings(void)
{
	for (int i = 0; i < NUM_CONTROL_NEEDS; i++)
	{
		memcpy(gGamePrefs.bindings[i].key, kDefaultInputBindings[i].key, sizeof(gGamePrefs.bindings[i].key));
	}
}

void ResetDefaultGamepadBindings(void)
{
	for (int i = 0; i < NUM_CONTROL_NEEDS; i++)
	{
		memcpy(gGamePrefs.bindings[i].pad, kDefaultInputBindings[i].pad, sizeof(gGamePrefs.bindings[i].pad));
	}
}

void ResetDefaultMouseBindings(void)
{
	for (int i = 0; i < NUM_CONTROL_NEEDS; i++)
	{
		gGamePrefs.bindings[i].mouseButton = kDefaultInputBindings[i].mouseButton;
	}
}
#endif
