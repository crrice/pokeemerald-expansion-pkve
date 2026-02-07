#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_interface.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "bg.h"
#include "data.h"
#include "link.h"
#include "main.h"
#include "m4a.h"
#include "overworld.h"
#include "palette.h"
#include "pokeball.h"
#include "pokemon.h"
#include "random.h"
#include "recorded_battle.h"
#include "reshow_battle_screen.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "util.h"
#include "window.h"
#include "malloc.h"
#include "scripted_battle.h"
#include "constants/battle_anim.h"
#include "constants/opponents.h"
#include "constants/songs.h"
#include "constants/trainers.h"

// Pokemon Verdant Emerald - Scripted Battle Controller
//
// This module provides two battle controllers for scripted/TV battles:
// - ScriptedPlayer: Controls the "player" side (position 0, back sprite)
// - ScriptedOpponent: Controls the "opponent" side (position 1, front sprite)
//
// Architecture follows the RecordedPlayer/RecordedOpponent pattern from upstream.
// Most commands use shared BtlController_Handle* functions directly.
// Custom handlers only for: action selection (script reading), trainer pics, and battle end.

// =============================================================================
// SCRIPTED BATTLE STATE
// =============================================================================

// Current scripted battle data
EWRAM_DATA static const struct ScriptedBattle *sCurrentScriptedBattle = NULL;
EWRAM_DATA static u8 sScriptIndex[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA static MainCallback sCallback2_AfterScriptedBattle = NULL;
// Static buffers for party save/restore - cannot use heap because
// CB2_InitBattle calls MoveSaveBlocks_ResetHeap() which invalidates heap allocations
EWRAM_DATA static struct Pokemon sSavedPlayerParty[PARTY_SIZE] = {0};
EWRAM_DATA static struct Pokemon sSavedOpponentParty[PARTY_SIZE] = {0};
EWRAM_DATA static u8 sSavedPlayerPartyCount = 0;
EWRAM_DATA static u8 sSavedEnemyPartyCount = 0;

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

// Scripted Player Controller
static void ScriptedPlayerBufferRunCommand(u32 battler);
static void ScriptedPlayerHandleDrawTrainerPic(u32 battler);
static void ScriptedPlayerHandleTrainerSlideBack(u32 battler);
static void ScriptedPlayerHandleChooseAction(u32 battler);
static void ScriptedPlayerHandleChooseMove(u32 battler);
static void ScriptedPlayerHandleChoosePokemon(u32 battler);
static void ScriptedPlayerHandleIntroTrainerBallThrow(u32 battler);
static void ScriptedPlayerHandleDrawPartyStatusSummary(u32 battler);
static void ScriptedPlayerHandleEndLinkBattle(u32 battler);

// Scripted Opponent Controller
static void ScriptedOpponentBufferRunCommand(u32 battler);
static void ScriptedOpponentHandleDrawTrainerPic(u32 battler);
static void ScriptedOpponentHandleTrainerSlide(u32 battler);
static void ScriptedOpponentHandleTrainerSlideBack(u32 battler);
static void ScriptedOpponentHandleChooseAction(u32 battler);
static void ScriptedOpponentHandleChooseMove(u32 battler);
static void ScriptedOpponentHandleChoosePokemon(u32 battler);
static void ScriptedOpponentHandleIntroTrainerBallThrow(u32 battler);
static void ScriptedOpponentHandleDrawPartyStatusSummary(u32 battler);
static void ScriptedOpponentHandleEndLinkBattle(u32 battler);


// =============================================================================
// SCRIPTED PLAYER CONTROLLER
// =============================================================================
// Mirrors RecordedPlayer controller - handles the "challenger" side (back sprite)

static void (*const sScriptedPlayerBufferCommands[CONTROLLER_CMDS_COUNT])(u32 battler) =
{
    [CONTROLLER_GETMONDATA]               = BtlController_HandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_Empty,
    [CONTROLLER_SETMONDATA]               = BtlController_HandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = BtlController_HandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = BtlController_HandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = BtlController_HandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = BtlController_HandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = ScriptedPlayerHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = BtlController_Empty,  // Player side doesn't slide in mid-battle
    [CONTROLLER_TRAINERSLIDEBACK]         = ScriptedPlayerHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = BtlController_HandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = BtlController_Empty,
    [CONTROLLER_SUCCESSBALLTHROWANIM]     = BtlController_Empty,
    [CONTROLLER_BALLTHROWANIM]            = PlayerHandleBallThrowAnim,
    [CONTROLLER_PAUSE]                    = BtlController_Empty,
    [CONTROLLER_MOVEANIMATION]            = BtlController_HandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = BtlController_HandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = BtlController_Empty,
    [CONTROLLER_CHOOSEACTION]             = ScriptedPlayerHandleChooseAction,
    [CONTROLLER_YESNOBOX]                 = BtlController_Empty,
    [CONTROLLER_CHOOSEMOVE]               = ScriptedPlayerHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = BtlController_Empty,  // Scripted battles don't use items
    [CONTROLLER_CHOOSEPOKEMON]            = ScriptedPlayerHandleChoosePokemon,
    [CONTROLLER_23]                       = BtlController_Empty,
    [CONTROLLER_HEALTHBARUPDATE]          = BtlController_HandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = BtlController_Empty,  // No EXP in scripted battles
    [CONTROLLER_STATUSICONUPDATE]         = BtlController_HandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = BtlController_HandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = BtlController_Empty,
    [CONTROLLER_DATATRANSFER]             = BtlController_Empty,
    [CONTROLLER_DMA3TRANSFER]             = BtlController_Empty,
    [CONTROLLER_PLAYBGM]                  = BtlController_Empty,
    [CONTROLLER_32]                       = BtlController_Empty,
    [CONTROLLER_TWORETURNVALUES]          = BtlController_Empty,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = BtlController_Empty,
    [CONTROLLER_ONERETURNVALUE]           = BtlController_Empty,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = BtlController_Empty,
    [CONTROLLER_HITANIMATION]             = BtlController_HandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = BtlController_Empty,
    [CONTROLLER_PLAYSE]                   = BtlController_HandlePlaySE,
    [CONTROLLER_PLAYFANFAREORBGM]         = BtlController_HandlePlayFanfareOrBGM,
    [CONTROLLER_FAINTINGCRY]              = BtlController_HandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = BtlController_HandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = ScriptedPlayerHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = ScriptedPlayerHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = BtlController_HandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = BtlController_Empty,
    [CONTROLLER_SPRITEINVISIBILITY]       = BtlController_HandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = BtlController_HandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = BtlController_Empty,
    [CONTROLLER_RESETACTIONMOVESELECTION] = BtlController_Empty,
    [CONTROLLER_ENDLINKBATTLE]            = ScriptedPlayerHandleEndLinkBattle,
    [CONTROLLER_DEBUGMENU]                = BtlController_Empty,
    [CONTROLLER_TERMINATOR_NOP]           = BtlController_TerminatorNop
};

void SetControllerToScriptedPlayer(u32 battler)
{
    gBattlerControllerEndFuncs[battler] = ScriptedPlayerBufferExecCompleted;
    gBattlerControllerFuncs[battler] = ScriptedPlayerBufferRunCommand;
}

static void ScriptedPlayerBufferRunCommand(u32 battler)
{
    // Safety check: gBattleResources may be freed during battle cleanup
    // while controller loop is still running on the same frame
    if (gBattleResources == NULL)
        return;

    if (IsBattleControllerActiveOnLocal(battler))
    {
        if (gBattleResources->bufferA[battler][0] < ARRAY_COUNT(sScriptedPlayerBufferCommands))
            sScriptedPlayerBufferCommands[gBattleResources->bufferA[battler][0]](battler);
        else
            BtlController_Complete(battler);
    }
}

void ScriptedPlayerBufferExecCompleted(u32 battler)
{
    gBattlerControllerFuncs[battler] = ScriptedPlayerBufferRunCommand;

    // Safety check: gBattleResources may be freed during battle cleanup
    if (gBattleResources == NULL)
        return;

    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        u8 playerId = GetMultiplayerId();
        PrepareBufferDataTransferLink(battler, B_COMM_CONTROLLER_IS_DONE, 4, &playerId);
        gBattleResources->bufferA[battler][0] = CONTROLLER_TERMINATOR_NOP;
    }
    else
    {
        MarkBattleControllerIdleOnLocal(battler);
    }
}

// =============================================================================
// SCRIPTED OPPONENT CONTROLLER
// =============================================================================
// Mirrors RecordedOpponent controller - handles the gym leader/trainer side (front sprite)

static void (*const sScriptedOpponentBufferCommands[CONTROLLER_CMDS_COUNT])(u32 battler) =
{
    [CONTROLLER_GETMONDATA]               = BtlController_HandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_Empty,
    [CONTROLLER_SETMONDATA]               = BtlController_HandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = BtlController_HandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = BtlController_HandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = BtlController_HandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = BtlController_HandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = ScriptedOpponentHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = ScriptedOpponentHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = ScriptedOpponentHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = BtlController_HandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = BtlController_Empty,
    [CONTROLLER_SUCCESSBALLTHROWANIM]     = BtlController_Empty,
    [CONTROLLER_BALLTHROWANIM]            = BtlController_Empty,
    [CONTROLLER_PAUSE]                    = BtlController_Empty,
    [CONTROLLER_MOVEANIMATION]            = BtlController_HandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = BtlController_HandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = BtlController_Empty,
    [CONTROLLER_CHOOSEACTION]             = ScriptedOpponentHandleChooseAction,
    [CONTROLLER_YESNOBOX]                 = BtlController_Empty,
    [CONTROLLER_CHOOSEMOVE]               = ScriptedOpponentHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = BtlController_Empty,
    [CONTROLLER_CHOOSEPOKEMON]            = ScriptedOpponentHandleChoosePokemon,
    [CONTROLLER_23]                       = BtlController_Empty,
    [CONTROLLER_HEALTHBARUPDATE]          = BtlController_HandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = BtlController_Empty,
    [CONTROLLER_STATUSICONUPDATE]         = BtlController_HandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = BtlController_HandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = BtlController_Empty,
    [CONTROLLER_DATATRANSFER]             = BtlController_Empty,
    [CONTROLLER_DMA3TRANSFER]             = BtlController_Empty,
    [CONTROLLER_PLAYBGM]                  = BtlController_Empty,
    [CONTROLLER_32]                       = BtlController_Empty,
    [CONTROLLER_TWORETURNVALUES]          = BtlController_Empty,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = BtlController_Empty,
    [CONTROLLER_ONERETURNVALUE]           = BtlController_Empty,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = BtlController_Empty,
    [CONTROLLER_HITANIMATION]             = BtlController_HandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = BtlController_Empty,
    [CONTROLLER_PLAYSE]                   = BtlController_HandlePlaySE,
    [CONTROLLER_PLAYFANFAREORBGM]         = BtlController_HandlePlayFanfareOrBGM,
    [CONTROLLER_FAINTINGCRY]              = BtlController_HandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = BtlController_HandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = ScriptedOpponentHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = ScriptedOpponentHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = BtlController_HandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = BtlController_Empty,
    [CONTROLLER_SPRITEINVISIBILITY]       = BtlController_HandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = BtlController_HandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = BtlController_Empty,
    [CONTROLLER_RESETACTIONMOVESELECTION] = BtlController_Empty,
    [CONTROLLER_ENDLINKBATTLE]            = ScriptedOpponentHandleEndLinkBattle,
    [CONTROLLER_DEBUGMENU]                = BtlController_Empty,
    [CONTROLLER_TERMINATOR_NOP]           = BtlController_TerminatorNop
};

void SetControllerToScriptedOpponent(u32 battler)
{
    gBattlerControllerEndFuncs[battler] = ScriptedOpponentBufferExecCompleted;
    gBattlerControllerFuncs[battler] = ScriptedOpponentBufferRunCommand;
}

static void ScriptedOpponentBufferRunCommand(u32 battler)
{
    // Safety check: gBattleResources may be freed during battle cleanup
    // while controller loop is still running on the same frame
    if (gBattleResources == NULL)
        return;

    if (IsBattleControllerActiveOnLocal(battler))
    {
        if (gBattleResources->bufferA[battler][0] < ARRAY_COUNT(sScriptedOpponentBufferCommands))
            sScriptedOpponentBufferCommands[gBattleResources->bufferA[battler][0]](battler);
        else
            BtlController_Complete(battler);
    }
}

void ScriptedOpponentBufferExecCompleted(u32 battler)
{
    gBattlerControllerFuncs[battler] = ScriptedOpponentBufferRunCommand;

    // Safety check: gBattleResources may be freed during battle cleanup
    if (gBattleResources == NULL)
        return;

    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        u8 playerId = GetMultiplayerId();
        PrepareBufferDataTransferLink(battler, B_COMM_CONTROLLER_IS_DONE, 4, &playerId);
        gBattleResources->bufferA[battler][0] = CONTROLLER_TERMINATOR_NOP;
    }
    else
    {
        MarkBattleControllerIdleOnLocal(battler);
    }
}

// =============================================================================
// SCRIPT ACTION HANDLING
// =============================================================================

// Get the script for a battler (player side = 0/2, opponent side = 1/3)
static const struct ScriptedBattleAction *GetScriptForBattler(u32 battler)
{
    if (sCurrentScriptedBattle == NULL)
        return NULL;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        return sCurrentScriptedBattle->playerScript;
    else
        return sCurrentScriptedBattle->opponentScript;
}

// Peek at current action without consuming it (for ChooseAction)
static const struct ScriptedBattleAction *PeekCurrentScriptedAction(u32 battler)
{
    const struct ScriptedBattleAction *script = GetScriptForBattler(battler);
    if (script == NULL)
        return NULL;

    const struct ScriptedBattleAction *action = &script[sScriptIndex[battler]];
    if (action->actionType == SCRIPTED_ACTION_END)
        return NULL;

    return action;
}

// Consume and return current action (for ChooseMove/ChoosePokemon)
const struct ScriptedBattleAction *GetNextScriptedAction(u32 battler)
{
    const struct ScriptedBattleAction *action = PeekCurrentScriptedAction(battler);
    if (action != NULL)
        sScriptIndex[battler]++;
    return action;
}

// =============================================================================
// SHARED SCRIPTED ACTION HANDLERS
// =============================================================================
// These are the same for both player and opponent sides - they just read from the script

static void HandleScriptedChooseAction(u32 battler)
{
    const struct ScriptedBattleAction *action = PeekCurrentScriptedAction(battler);

    if (action == NULL)
    {
        // No more actions, default to move
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, 0);
    }
    else if (action->actionType == SCRIPTED_ACTION_SWITCH)
    {
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
    }
    else
    {
        // SCRIPTED_ACTION_USE_MOVE
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, 0);
    }

    BtlController_Complete(battler);
}

static void HandleScriptedChooseMove(u32 battler)
{
    const struct ScriptedBattleAction *action = GetNextScriptedAction(battler);

    if (action == NULL)
    {
        // No script action, use move 0 on opponent
        u8 target = (GetBattlerSide(battler) == B_SIDE_PLAYER) ? 1 : 0;
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT,
                                          0 | (target << 8));
    }
    else
    {
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT,
                                          action->moveSlotOrPartyIndex | (action->target << 8));
    }

    BtlController_Complete(battler);
}

static void HandleScriptedChoosePokemon(u32 battler)
{
    // Peek first - only consume if it's actually a switch action
    const struct ScriptedBattleAction *action = PeekCurrentScriptedAction(battler);
    s32 chosenMonId;

    if (action != NULL && action->actionType == SCRIPTED_ACTION_SWITCH)
    {
        // Explicit switch action - consume it and use the specified Pokemon
        GetNextScriptedAction(battler);
        chosenMonId = action->moveSlotOrPartyIndex;
    }
    else
    {
        // Forced switch (due to fainting) - find first alive mon automatically
        // DO NOT consume the action - it's needed for the next turn's move selection
        struct Pokemon *party = (GetBattlerSide(battler) == B_SIDE_PLAYER) ? gPlayerParty : gEnemyParty;
        for (chosenMonId = 0; chosenMonId < PARTY_SIZE; chosenMonId++)
        {
            if (GetMonData(&party[chosenMonId], MON_DATA_HP) != 0
                && chosenMonId != gBattlerPartyIndexes[battler])
            {
                break;
            }
        }
        if (chosenMonId >= PARTY_SIZE)
            chosenMonId = 0;  // Fallback
    }

    gBattleStruct->monToSwitchIntoId[battler] = chosenMonId;
    BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, chosenMonId, NULL);
    BtlController_Complete(battler);
}

// =============================================================================
// SCRIPTED PLAYER COMMAND HANDLERS
// =============================================================================

static void ScriptedPlayerHandleDrawTrainerPic(u32 battler)
{
    u32 trainerPicId = sCurrentScriptedBattle->playerBackPic;
    BtlController_HandleDrawTrainerPic(battler, trainerPicId, FALSE,
                                       80, (8 - gTrainerBacksprites[trainerPicId].coordinates.size) * 4 + 80,
                                       -1);
}

static void ScriptedPlayerHandleTrainerSlideBack(u32 battler)
{
    BtlController_HandleTrainerSlideBack(battler, 35, FALSE);
}

static void ScriptedPlayerHandleChooseAction(u32 battler)
{
    HandleScriptedChooseAction(battler);
}

static void ScriptedPlayerHandleChooseMove(u32 battler)
{
    HandleScriptedChooseMove(battler);
}

static void ScriptedPlayerHandleChoosePokemon(u32 battler)
{
    HandleScriptedChoosePokemon(battler);
}

// Intro callback - wait for shiny animation and healthbox (player side)
static void ScriptedPlayer_Intro_WaitForShinyAnimAndHealthbox(u32 battler)
{
    bool32 healthboxAnimDone = FALSE;

    if (!IsDoubleBattle() || (IsDoubleBattle() && (gBattleTypeFlags & BATTLE_TYPE_MULTI)))
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy)
            healthboxAnimDone = TRUE;
    }
    else
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
            && gSprites[gHealthboxSpriteIds[BATTLE_PARTNER(battler)]].callback == SpriteCallbackDummy)
        {
            healthboxAnimDone = TRUE;
        }
    }

    if (healthboxAnimDone && gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim
        && gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].finishedShinyMonAnim)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].finishedShinyMonAnim = FALSE;
        FreeShinyStars();

        HandleLowHpMusicChange(GetBattlerMon(battler), battler);
        if (IsDoubleBattle())
            HandleLowHpMusicChange(GetBattlerMon(BATTLE_PARTNER(battler)), BATTLE_PARTNER(battler));

        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 3;
        gBattlerControllerFuncs[battler] = BtlController_Intro_DelayAndEnd;
    }
}

// Intro callback - try shiny animation and show healthbox (player side)
static void ScriptedPlayer_Intro_TryShinyAnimShowHealthbox(u32 battler)
{
    // Try shiny animation for this battler
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
        TryShinyAnimation(battler, GetBattlerMon(battler));

    // Try shiny animation for partner (in doubles)
    if (!gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
        TryShinyAnimation(BATTLE_PARTNER(battler), GetBattlerMon(BATTLE_PARTNER(battler)));

    // Once ball animations are done, show healthboxes
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted)
        {
            if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            {
                UpdateHealthboxAttribute(gHealthboxSpriteIds[BATTLE_PARTNER(battler)], GetBattlerMon(BATTLE_PARTNER(battler)), HEALTHBOX_ALL);
                StartHealthboxSlideIn(BATTLE_PARTNER(battler));
                SetHealthboxSpriteVisible(gHealthboxSpriteIds[BATTLE_PARTNER(battler)]);
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], GetBattlerMon(battler), HEALTHBOX_ALL);
            StartHealthboxSlideIn(battler);
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        }
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = TRUE;
    }

    // Once healthbox slide-in has started, wait for shiny and healthbox completion
    if (gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted
        && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
    {
        gBattlerControllerFuncs[battler] = ScriptedPlayer_Intro_WaitForShinyAnimAndHealthbox;
    }
}

static void ScriptedPlayerHandleIntroTrainerBallThrow(u32 battler)
{
    u32 trainerPicId = sCurrentScriptedBattle->playerBackPic;
    const u16 *trainerPal = gTrainerBacksprites[trainerPicId].palette.data;

    BtlController_HandleIntroTrainerBallThrow(battler, 0xD6F9, trainerPal, 24,
                                              ScriptedPlayer_Intro_TryShinyAnimShowHealthbox);
}

static void ScriptedPlayerHandleDrawPartyStatusSummary(u32 battler)
{
    BtlController_HandleDrawPartyStatusSummary(battler, B_SIDE_PLAYER, TRUE);
}

static void ScriptedPlayerHandleEndLinkBattle(u32 battler)
{
    gBattleOutcome = gBattleResources->bufferA[battler][1];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(3);
    BtlController_Complete(battler);
    gBattlerControllerFuncs[battler] = SetBattleEndCallbacks;
}

// =============================================================================
// SCRIPTED OPPONENT COMMAND HANDLERS
// =============================================================================

static void ScriptedOpponentHandleDrawTrainerPic(u32 battler)
{
    u32 trainerPicId = sCurrentScriptedBattle->opponentTrainerPic;
    BtlController_HandleDrawTrainerPic(battler, trainerPicId, TRUE, 176, 40, -1);
}

static void ScriptedOpponentHandleTrainerSlide(u32 battler)
{
    u32 trainerPicId;

    // Use announcer pic if set, otherwise use opponent pic
    if (sCurrentScriptedBattle != NULL && sCurrentScriptedBattle->announcerTrainerPic != 0)
        trainerPicId = sCurrentScriptedBattle->announcerTrainerPic;
    else
        trainerPicId = sCurrentScriptedBattle->opponentTrainerPic;

    BtlController_HandleTrainerSlide(battler, trainerPicId);
}

static void ScriptedOpponentHandleTrainerSlideBack(u32 battler)
{
    BtlController_HandleTrainerSlideBack(battler, 35, FALSE);
}

static void ScriptedOpponentHandleChooseAction(u32 battler)
{
    HandleScriptedChooseAction(battler);
}

static void ScriptedOpponentHandleChooseMove(u32 battler)
{
    HandleScriptedChooseMove(battler);
}

static void ScriptedOpponentHandleChoosePokemon(u32 battler)
{
    HandleScriptedChoosePokemon(battler);
}

// Intro callback - wait for shiny animation and healthbox (opponent side)
static void ScriptedOpponent_Intro_WaitForShinyAnimAndHealthbox(u32 battler)
{
    bool8 healthboxAnimDone = FALSE;

    if (!IsDoubleBattle() || (IsDoubleBattle() && (gBattleTypeFlags & BATTLE_TYPE_MULTI)))
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
         && gSprites[gBattlerSpriteIds[battler]].animEnded)
            healthboxAnimDone = TRUE;
    }
    else
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
         && gSprites[gHealthboxSpriteIds[BATTLE_PARTNER(battler)]].callback == SpriteCallbackDummy
         && gSprites[gBattlerSpriteIds[battler]].animEnded
         && gSprites[gBattlerSpriteIds[BATTLE_PARTNER(battler)]].animEnded)
            healthboxAnimDone = TRUE;
    }

    if (healthboxAnimDone)
    {
        if (GetBattlerPosition(battler) == B_POSITION_OPPONENT_LEFT)
        {
            if (!gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim)
                return;
            if (!gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].finishedShinyMonAnim)
                return;

            gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].finishedShinyMonAnim = FALSE;
            FreeShinyStars();
        }

        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 3;
        gBattlerControllerFuncs[battler] = BtlController_Intro_DelayAndEnd;
    }
}

// Intro callback - try shiny animation and show healthbox (opponent side)
static void ScriptedOpponent_Intro_TryShinyAnimShowHealthbox(u32 battler)
{
    bool32 bgmRestored = FALSE;
    bool32 battlerAnimsDone = FALSE;

    // Try shiny animation for this battler
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
     && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
        TryShinyAnimation(battler, GetBattlerMon(battler));

    // Try shiny animation for partner (in doubles)
    if (!gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].triedShinyMonAnim
     && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
        TryShinyAnimation(BATTLE_PARTNER(battler), GetBattlerMon(BATTLE_PARTNER(battler)));

    // Once ball animations are done, show healthboxes
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted)
        {
            if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            {
                UpdateHealthboxAttribute(gHealthboxSpriteIds[BATTLE_PARTNER(battler)], GetBattlerMon(BATTLE_PARTNER(battler)), HEALTHBOX_ALL);
                StartHealthboxSlideIn(BATTLE_PARTNER(battler));
                SetHealthboxSpriteVisible(gHealthboxSpriteIds[BATTLE_PARTNER(battler)]);
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], GetBattlerMon(battler), HEALTHBOX_ALL);
            StartHealthboxSlideIn(battler);
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        }
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = TRUE;
    }

    // Wait for cries to finish and restore BGM
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].waitForCry
        && gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].waitForCry
        && !IsCryPlayingOrClearCrySongs())
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored)
            m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored = TRUE;
        bgmRestored = TRUE;
    }

    // Check if battler animations are done
    if (!IsDoubleBattle())
    {
        if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy)
        {
            TrySetBattlerShadowSpriteCallback(battler);
            if (gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy)
                battlerAnimsDone = TRUE;
        }
    }
    else
    {
        if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattleControllerData[BATTLE_PARTNER(battler)]].callback == SpriteCallbackDummy)
        {
            TrySetBattlerShadowSpriteCallback(battler);
            TrySetBattlerShadowSpriteCallback(BATTLE_PARTNER(battler));
            if (gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy
                && gSprites[gBattlerSpriteIds[BATTLE_PARTNER(battler)]].callback == SpriteCallbackDummy)
            {
                battlerAnimsDone = TRUE;
            }
        }
    }

    // Once BGM is restored and animations are done, clean up and proceed
    if (bgmRestored && battlerAnimsDone)
    {
        if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            DestroySprite(&gSprites[gBattleControllerData[BATTLE_PARTNER(battler)]]);

        DestroySprite(&gSprites[gBattleControllerData[battler]]);

        gBattleSpritesDataPtr->animationData->introAnimActive = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = FALSE;

        gBattlerControllerFuncs[battler] = ScriptedOpponent_Intro_WaitForShinyAnimAndHealthbox;
    }
}

static void ScriptedOpponentHandleIntroTrainerBallThrow(u32 battler)
{
    // Opponent uses 0 for palette tag and NULL for palette (the shared handler handles opponent differently)
    BtlController_HandleIntroTrainerBallThrow(battler, 0, NULL, 0,
                                              ScriptedOpponent_Intro_TryShinyAnimShowHealthbox);
}

static void ScriptedOpponentHandleDrawPartyStatusSummary(u32 battler)
{
    BtlController_HandleDrawPartyStatusSummary(battler, B_SIDE_OPPONENT, TRUE);
}

static void ScriptedOpponentHandleEndLinkBattle(u32 battler)
{
    gBattleOutcome = gBattleResources->bufferA[battler][1];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(3);
    BtlController_Complete(battler);
    gBattlerControllerFuncs[battler] = SetBattleEndCallbacks;
}

// =============================================================================
// SCRIPTED BATTLE INFRASTRUCTURE
// =============================================================================
// Party management, initialization, and entry points

void ScriptedBattle_Init(const struct ScriptedBattle *battle)
{
    s32 i;

    sCurrentScriptedBattle = battle;

    // Reset script indices
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        sScriptIndex[i] = 0;
}

// Save current parties before scripted battle
static void ScriptedBattle_SaveParties(void)
{
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        sSavedPlayerParty[i] = gPlayerParty[i];
        sSavedOpponentParty[i] = gEnemyParty[i];
    }
    sSavedPlayerPartyCount = gPlayerPartyCount;
    sSavedEnemyPartyCount = gEnemyPartyCount;
}

// Restore parties after scripted battle
static void ScriptedBattle_RestoreParties(void)
{
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        gPlayerParty[i] = sSavedPlayerParty[i];
        gEnemyParty[i] = sSavedOpponentParty[i];
    }
    gPlayerPartyCount = sSavedPlayerPartyCount;
    gEnemyPartyCount = sSavedEnemyPartyCount;
}

// Build a party from ScriptedPokemon definitions
static u8 ScriptedBattle_BuildParty(struct Pokemon *party, const struct ScriptedPokemon *const *scriptedParty)
{
    s32 i, j;
    u8 partyCount = 0;

    // Zero this specific party array
    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&party[i]);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (scriptedParty[i] == NULL)
            continue;

        const struct ScriptedPokemon *src = scriptedParty[i];

        CreateMon(&party[i], src->species, src->level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
        partyCount = i + 1;

        // Set HP
        if (src->currentHP == 0)
        {
            u16 zero = 0;
            SetMonData(&party[i], MON_DATA_HP, &zero);
        }
        else if (src->currentHP != 0xFFFF)
        {
            SetMonData(&party[i], MON_DATA_HP, &src->currentHP);
        }

        // Set moves
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            if (src->moves[j] != MOVE_NONE)
                SetMonMoveSlot(&party[i], src->moves[j], j);
        }

        // Set status
        if (src->status != 0)
            SetMonData(&party[i], MON_DATA_STATUS, &src->status);

        // Set ability
        SetMonData(&party[i], MON_DATA_ABILITY_NUM, &src->abilityNum);

        // Set held item
        if (src->heldItem != ITEM_NONE)
            SetMonData(&party[i], MON_DATA_HELD_ITEM, &src->heldItem);

        // Set IVs
        SetMonData(&party[i], MON_DATA_HP_IV, &src->ivs[0]);
        SetMonData(&party[i], MON_DATA_ATK_IV, &src->ivs[1]);
        SetMonData(&party[i], MON_DATA_DEF_IV, &src->ivs[2]);
        SetMonData(&party[i], MON_DATA_SPEED_IV, &src->ivs[3]);
        SetMonData(&party[i], MON_DATA_SPATK_IV, &src->ivs[4]);
        SetMonData(&party[i], MON_DATA_SPDEF_IV, &src->ivs[5]);

        // Set EVs
        SetMonData(&party[i], MON_DATA_HP_EV, &src->evs[0]);
        SetMonData(&party[i], MON_DATA_ATK_EV, &src->evs[1]);
        SetMonData(&party[i], MON_DATA_DEF_EV, &src->evs[2]);
        SetMonData(&party[i], MON_DATA_SPEED_EV, &src->evs[3]);
        SetMonData(&party[i], MON_DATA_SPATK_EV, &src->evs[4]);
        SetMonData(&party[i], MON_DATA_SPDEF_EV, &src->evs[5]);

        // Set nature
        SetMonData(&party[i], MON_DATA_HIDDEN_NATURE, &src->nature);

        // Set friendship
        SetMonData(&party[i], MON_DATA_FRIENDSHIP, &src->friendship);

        // Recalculate stats after setting IVs/EVs/nature
        CalculateMonStats(&party[i]);

        // Re-apply HP after stat recalc
        if (src->currentHP == 0)
        {
            u16 zero = 0;
            SetMonData(&party[i], MON_DATA_HP, &zero);
        }
        else if (src->currentHP != 0xFFFF)
        {
            SetMonData(&party[i], MON_DATA_HP, &src->currentHP);
        }
    }

    return partyCount;
}

// Callback after scripted battle ends
static void CB2_AfterScriptedBattle(void)
{
    // Restore the player's actual party (this is all we need to do -
    // normal trainer battles don't reset gBattleOutcome/gBattleTypeFlags/etc.)
    ScriptedBattle_RestoreParties();
    sCurrentScriptedBattle = NULL;

    // Continue to the saved callback (e.g., return to overworld)
    SetMainCallback2(sCallback2_AfterScriptedBattle);
}

// Task to start battle after transition
static void Task_StartScriptedBattle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0:
        if (data[1] != 0)
        {
            BattleTransition_Start(data[1]);
            data[0] = 1;
        }
        else
        {
            data[0] = 2;
        }
        break;
    case 1:
        if (IsBattleTransitionDone())
            data[0] = 2;
        break;
    case 2:
        gMain.savedCallback = CB2_AfterScriptedBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
        break;
    }
}

// CB2 for waiting state
static void CB2_ScriptedBattleWait(void)
{
    AnimateSprites();
    BuildOamBuffer();
    RunTasks();
}

// Main entry point to start a scripted battle
void PlayScriptedBattle(const struct ScriptedBattle *battle, void (*callback)(void))
{
    u8 taskId;

    if (battle == NULL)
        return;

    // Save callback
    sCallback2_AfterScriptedBattle = callback;

    // Save current parties
    ScriptedBattle_SaveParties();

    // Initialize scripted battle state
    ScriptedBattle_Init(battle);

    // Build parties from script
    gPlayerPartyCount = ScriptedBattle_BuildParty(gPlayerParty, battle->playerParty);
    gEnemyPartyCount = ScriptedBattle_BuildParty(gEnemyParty, battle->opponentParty);

    // Set up trainer battle parameter for name display
    TRAINER_BATTLE_PARAM.opponentA = battle->opponentTrainerId;

    // Set up battle flags
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_SCRIPTED | BATTLE_TYPE_RECORDED | battle->battleFlags;

    // Set RNG seed for deterministic playback
    if (battle->rngSeed != 0)
        gRecordedBattleRngSeed = LocalRandomSeed(battle->rngSeed);

    // Create task to handle transition and start battle
    taskId = CreateTask(Task_StartScriptedBattle, 1);
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = battle->transitionId;

    // Play battle music and set callback
    PlayMapChosenOrBattleBGM(FALSE);
    SetMainCallback2(CB2_ScriptedBattleWait);
}

// Get custom intro text for battle_message.c
const u8 *GetScriptedBattleIntroText(void)
{
    if (sCurrentScriptedBattle == NULL)
        return NULL;
    return sCurrentScriptedBattle->introText;
}

// Get announcer message for trainer_slide.c integration
const u8 *GetScriptedBattleAnnouncerMsg(u32 slideId)
{
    if (sCurrentScriptedBattle == NULL)
        return NULL;

    switch (slideId)
    {
    case TRAINER_SLIDE_BEFORE_FIRST_TURN:
        return sCurrentScriptedBattle->announcer_BeforeFirstTurn;
    case TRAINER_SLIDE_LAST_SWITCHIN:
        return sCurrentScriptedBattle->announcer_LastSwitchIn;
    case TRAINER_SLIDE_BATTLE_WON:
        return sCurrentScriptedBattle->announcer_BattleEnd;
    default:
        return NULL;
    }
}

// Include scripted battle data definitions
#include "data/scripted_battles.h"

// Script-callable function to start the Norman TV battle
void Script_StartNormanTVBattle(struct ScriptContext *ctx)
{
    PlayScriptedBattle(&gScriptedBattle_NormanTV, CB2_ReturnToFieldContinueScript);
}
