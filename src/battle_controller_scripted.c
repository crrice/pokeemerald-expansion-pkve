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

// Forward declarations
static void ScriptedHandleLoadMonSprite(u32 battler);
static void ScriptedHandleSwitchInAnim(u32 battler);
static void ScriptedHandleDrawTrainerPic(u32 battler);
static void ScriptedHandleTrainerSlide(u32 battler);
static void ScriptedHandleTrainerSlideBack(u32 battler);
static void ScriptedHandleMoveAnimation(u32 battler);
static void ScriptedHandlePrintString(u32 battler);
static void ScriptedHandleChooseAction(u32 battler);
static void ScriptedHandleChooseMove(u32 battler);
static void ScriptedHandleChoosePokemon(u32 battler);
static void ScriptedHandleHealthBarUpdate(u32 battler);
static void ScriptedHandleIntroTrainerBallThrow(u32 battler);
static void ScriptedHandleDrawPartyStatusSummary(u32 battler);
static void ScriptedHandleBattleAnimation(u32 battler);
static void ScriptedHandleEndLinkBattle(u32 battler);

static void ScriptedBufferRunCommand(u32 battler);
static void ScriptedBufferExecCompleted(u32 battler);

// Current scripted battle data
EWRAM_DATA static const struct ScriptedBattle *sCurrentScriptedBattle = NULL;
EWRAM_DATA static u8 sScriptIndex[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA static MainCallback sCallback2_AfterScriptedBattle = NULL;
EWRAM_DATA static struct Pokemon *sSavedPlayerParty = NULL;
EWRAM_DATA static struct Pokemon *sSavedOpponentParty = NULL;
EWRAM_DATA static u8 sSavedPlayerPartyCount = 0;
EWRAM_DATA static u8 sSavedEnemyPartyCount = 0;

// Command table - similar to player partner but with scripted handlers
static void (*const sScriptedBufferCommands[CONTROLLER_CMDS_COUNT])(u32 battler) =
{
    [CONTROLLER_GETMONDATA]               = BtlController_HandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_Empty,
    [CONTROLLER_SETMONDATA]               = BtlController_HandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = BtlController_HandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = ScriptedHandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = ScriptedHandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = BtlController_HandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = ScriptedHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = ScriptedHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = ScriptedHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = BtlController_HandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = BtlController_Empty,
    [CONTROLLER_SUCCESSBALLTHROWANIM]     = BtlController_Empty,
    [CONTROLLER_BALLTHROWANIM]            = BtlController_Empty,
    [CONTROLLER_PAUSE]                    = BtlController_Empty,
    [CONTROLLER_MOVEANIMATION]            = ScriptedHandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = ScriptedHandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = BtlController_Empty,
    [CONTROLLER_CHOOSEACTION]             = ScriptedHandleChooseAction,
    [CONTROLLER_YESNOBOX]                 = BtlController_Empty,
    [CONTROLLER_CHOOSEMOVE]               = ScriptedHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = BtlController_Empty,
    [CONTROLLER_CHOOSEPOKEMON]            = ScriptedHandleChoosePokemon,
    [CONTROLLER_23]                       = BtlController_Empty,
    [CONTROLLER_HEALTHBARUPDATE]          = ScriptedHandleHealthBarUpdate,
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
    [CONTROLLER_INTROTRAINERBALLTHROW]    = ScriptedHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = ScriptedHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = BtlController_HandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = BtlController_Empty,
    [CONTROLLER_SPRITEINVISIBILITY]       = BtlController_HandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = ScriptedHandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = BtlController_Empty,
    [CONTROLLER_RESETACTIONMOVESELECTION] = BtlController_Empty,
    [CONTROLLER_ENDLINKBATTLE]            = ScriptedHandleEndLinkBattle,
    [CONTROLLER_DEBUGMENU]                = BtlController_Empty,
    [CONTROLLER_TERMINATOR_NOP]           = BtlController_TerminatorNop
};

void SetControllerToScripted(u32 battler)
{
    gBattlerControllerEndFuncs[battler] = ScriptedBufferExecCompleted;
    gBattlerControllerFuncs[battler] = ScriptedBufferRunCommand;
}

static void ScriptedBufferRunCommand(u32 battler)
{
    if (gBattleControllerExecFlags & (1u << battler))
    {
        if (gBattleResources->bufferA[battler][0] < ARRAY_COUNT(sScriptedBufferCommands))
            sScriptedBufferCommands[gBattleResources->bufferA[battler][0]](battler);
        else
            ScriptedBufferExecCompleted(battler);
    }
}

static void ScriptedBufferExecCompleted(u32 battler)
{
    gBattlerControllerFuncs[battler] = ScriptedBufferRunCommand;
    gBattleControllerExecFlags &= ~(1u << battler);
}

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

// Callback helpers
static void WaitForMonAnimAfterLoad(u32 battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].animEnded && gSprites[gBattlerSpriteIds[battler]].x2 == 0)
        ScriptedBufferExecCompleted(battler);
}

static void SwitchIn_WaitAndEnd(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive
        && gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy)
    {
        ScriptedBufferExecCompleted(battler);
    }
}

static void SwitchIn_ShowSubstitute(u32 battler)
{
    if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy)
    {
        CopyBattleSpriteInvisibility(battler);
        if (gBattleSpritesDataPtr->battlerData[battler].behindSubstitute)
            InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_MON_TO_SUBSTITUTE);

        gBattlerControllerFuncs[battler] = SwitchIn_WaitAndEnd;
    }
}

static void SwitchIn_ShowHealthbox(u32 battler)
{
    struct Pokemon *party = (GetBattlerSide(battler) == B_SIDE_PLAYER) ? gPlayerParty : gEnemyParty;

    if (gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;

        FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
        FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);

        CreateTask(Task_PlayerController_RestoreBgmAfterCry, 10);
        HandleLowHpMusicChange(&party[gBattlerPartyIndexes[battler]], battler);
        StartSpriteAnim(&gSprites[gBattlerSpriteIds[battler]], 0);
        UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], &party[gBattlerPartyIndexes[battler]], HEALTHBOX_ALL);
        StartHealthboxSlideIn(battler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);

        gBattlerControllerFuncs[battler] = SwitchIn_ShowSubstitute;
    }
}

static void SwitchIn_TryShinyAnim(u32 battler)
{
    struct Pokemon *party = (GetBattlerSide(battler) == B_SIDE_PLAYER) ? gPlayerParty : gEnemyParty;

    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
    {
        TryShinyAnimation(battler, &party[gBattlerPartyIndexes[battler]]);
    }

    if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
     && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
    {
        DestroySprite(&gSprites[gBattleControllerData[battler]]);
        gBattlerControllerFuncs[battler] = SwitchIn_ShowHealthbox;
    }
}

// Intro sequence helpers
static void Intro_DelayAndEnd(u32 battler)
{
    if (--gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay == (u8)-1)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 0;
        ScriptedBufferExecCompleted(battler);
    }
}

static void Intro_WaitForHealthbox(u32 battler)
{
    bool32 finished = FALSE;

    if (!IsDoubleBattle() || (IsDoubleBattle() && (gBattleTypeFlags & BATTLE_TYPE_MULTI)))
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy)
            finished = TRUE;
    }
    else
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
            && gSprites[gHealthboxSpriteIds[BATTLE_PARTNER(battler)]].callback == SpriteCallbackDummy)
        {
            finished = TRUE;
        }
    }

    if (IsCryPlayingOrClearCrySongs())
        finished = FALSE;

    if (finished)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 3;
        gBattlerControllerFuncs[battler] = Intro_DelayAndEnd;
    }
}

static void Scripted_ShowIntroHealthbox(u32 battler)
{
    struct Pokemon *party = (GetBattlerSide(battler) == B_SIDE_PLAYER) ? gPlayerParty : gEnemyParty;

    if (!gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive
        && gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
        && gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy
        && ++gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay != 1)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 0;

        TryShinyAnimation(battler, &party[gBattlerPartyIndexes[battler]]);

        if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
        {
            DestroySprite(&gSprites[gBattleControllerData[BATTLE_PARTNER(battler)]]);
            UpdateHealthboxAttribute(gHealthboxSpriteIds[BATTLE_PARTNER(battler)], &party[gBattlerPartyIndexes[BATTLE_PARTNER(battler)]], HEALTHBOX_ALL);
            StartHealthboxSlideIn(BATTLE_PARTNER(battler));
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[BATTLE_PARTNER(battler)]);
        }

        DestroySprite(&gSprites[gBattleControllerData[battler]]);
        UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], &party[gBattlerPartyIndexes[battler]], HEALTHBOX_ALL);
        StartHealthboxSlideIn(battler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);

        gBattleSpritesDataPtr->animationData->introAnimActive = FALSE;

        gBattlerControllerFuncs[battler] = Intro_WaitForHealthbox;
    }
}

// Handler implementations
static void ScriptedHandleLoadMonSprite(u32 battler)
{
    BtlController_HandleLoadMonSprite(battler, WaitForMonAnimAfterLoad);
}

static void ScriptedHandleSwitchInAnim(u32 battler)
{
    bool32 isPlayerSide = (GetBattlerSide(battler) == B_SIDE_PLAYER);
    BtlController_HandleSwitchInAnim(battler, isPlayerSide, SwitchIn_TryShinyAnim);
}

static void ScriptedHandleDrawTrainerPic(u32 battler)
{
    s16 xPos;
    u32 trainerPicId;
    bool32 isFrontPic;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        // Player side (left, back sprite) - the challenger
        trainerPicId = sCurrentScriptedBattle->playerBackPic;
        xPos = 80;
        isFrontPic = FALSE;
        BtlController_HandleDrawTrainerPic(battler, trainerPicId, isFrontPic, xPos,
            (8 - gTrainerBacksprites[trainerPicId].coordinates.size) * 4 + 80, -1);
    }
    else
    {
        // Opponent side (right, front sprite)
        trainerPicId = sCurrentScriptedBattle->opponentTrainerPic;
        xPos = 176;
        isFrontPic = TRUE;
        BtlController_HandleDrawTrainerPic(battler, trainerPicId, isFrontPic, xPos, 40, -1);
    }
}

static void ScriptedHandleTrainerSlide(u32 battler)
{
    u32 trainerPicId;

    // Use announcer pic if set, otherwise fall back to opponent pic
    if (sCurrentScriptedBattle != NULL && sCurrentScriptedBattle->announcerTrainerPic != 0)
    {
        trainerPicId = sCurrentScriptedBattle->announcerTrainerPic;
    }
    else if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        // Player side slides in with back pic
        trainerPicId = (sCurrentScriptedBattle != NULL) ? sCurrentScriptedBattle->playerBackPic : TRAINER_BACK_PIC_BRENDAN;
    }
    else
    {
        // Opponent side slides in with front pic
        trainerPicId = (sCurrentScriptedBattle != NULL) ? sCurrentScriptedBattle->opponentTrainerPic : TRAINER_PIC_HIKER;
    }

    BtlController_HandleTrainerSlide(battler, trainerPicId);
}

static void ScriptedHandleTrainerSlideBack(u32 battler)
{
    BtlController_HandleTrainerSlideBack(battler, 35, FALSE);
}

static void ScriptedHandleMoveAnimation(u32 battler)
{
    BtlController_HandleMoveAnimation(battler, FALSE);
}

static void ScriptedHandlePrintString(u32 battler)
{
    BtlController_HandlePrintString(battler, FALSE, FALSE);
}

// Core scripted action handlers
static void ScriptedHandleChooseAction(u32 battler)
{
    // Peek at the action type without consuming - ChooseMove/ChoosePokemon will consume
    const struct ScriptedBattleAction *action = PeekCurrentScriptedAction(battler);

    if (action == NULL)
    {
        // No more actions, just use move
        BtlController_EmitTwoReturnValues(battler, BUFFER_B, B_ACTION_USE_MOVE, 0);
    }
    else
    {
        BtlController_EmitTwoReturnValues(battler, BUFFER_B, action->actionType, 0);
    }

    ScriptedBufferExecCompleted(battler);
}

static void ScriptedHandleChooseMove(u32 battler)
{
    const struct ScriptedBattleAction *action = GetNextScriptedAction(battler);

    if (action == NULL)
    {
        // No script action, use move 0 on opponent
        u8 target = (GetBattlerSide(battler) == B_SIDE_PLAYER) ? 1 : 0;
        BtlController_EmitTwoReturnValues(battler, BUFFER_B, B_ACTION_EXEC_SCRIPT, 0 | (target << 8));
    }
    else
    {
        BtlController_EmitTwoReturnValues(battler, BUFFER_B, B_ACTION_EXEC_SCRIPT, action->moveSlotOrPartyIndex | (action->target << 8));
    }

    ScriptedBufferExecCompleted(battler);
}

static void ScriptedHandleChoosePokemon(u32 battler)
{
    const struct ScriptedBattleAction *action = GetNextScriptedAction(battler);
    s32 chosenMonId;

    if (action != NULL && action->actionType == SCRIPTED_ACTION_SWITCH)
    {
        chosenMonId = action->moveSlotOrPartyIndex;
    }
    else
    {
        // Find first alive mon that isn't currently out
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
    BtlController_EmitChosenMonReturnValue(battler, BUFFER_B, chosenMonId, NULL);
    ScriptedBufferExecCompleted(battler);
}

static void ScriptedHandleHealthBarUpdate(u32 battler)
{
    BtlController_HandleHealthBarUpdate(battler, FALSE);
}

static void ScriptedHandleIntroTrainerBallThrow(u32 battler)
{
    const u32 *trainerPal;
    u32 trainerPicId;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        trainerPicId = sCurrentScriptedBattle->playerBackPic;
        trainerPal = gTrainerBacksprites[trainerPicId].palette.data;
    }
    else
    {
        trainerPicId = sCurrentScriptedBattle->opponentTrainerPic;
        trainerPal = gTrainerSprites[trainerPicId].palette.data;
    }

    BtlController_HandleIntroTrainerBallThrow(battler, 0xD6F9, trainerPal, 24, Scripted_ShowIntroHealthbox);
}

static void ScriptedHandleDrawPartyStatusSummary(u32 battler)
{
    u32 side = GetBattlerSide(battler);
    BtlController_HandleDrawPartyStatusSummary(battler, side, TRUE);
}

static void ScriptedHandleBattleAnimation(u32 battler)
{
    BtlController_HandleBattleAnimation(battler, FALSE, FALSE);
}

static void ScriptedHandleEndLinkBattle(u32 battler)
{
    gBattleOutcome = gBattleResources->bufferA[battler][1];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(3);
    ScriptedBufferExecCompleted(battler);
    gBattlerControllerFuncs[battler] = SetBattleEndCallbacks;
}

// Scripted battle initialization
void ScriptedBattle_Init(const struct ScriptedBattle *battle)
{
    s32 i;

    sCurrentScriptedBattle = battle;

    // Reset script indices
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        sScriptIndex[i] = 0;
}

// Save current parties before scripted battle (allocates memory)
static void ScriptedBattle_SaveParties(void)
{
    s32 i;

    // Allocate memory for saved parties
    sSavedPlayerParty = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);
    sSavedOpponentParty = Alloc(sizeof(struct Pokemon) * PARTY_SIZE);

    if (sSavedPlayerParty != NULL && sSavedOpponentParty != NULL)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            sSavedPlayerParty[i] = gPlayerParty[i];
            sSavedOpponentParty[i] = gEnemyParty[i];
        }
    }
    sSavedPlayerPartyCount = gPlayerPartyCount;
    sSavedEnemyPartyCount = gEnemyPartyCount;
}

// Restore parties after scripted battle (frees memory)
static void ScriptedBattle_RestoreParties(void)
{
    s32 i;

    if (sSavedPlayerParty != NULL && sSavedOpponentParty != NULL)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            gPlayerParty[i] = sSavedPlayerParty[i];
            gEnemyParty[i] = sSavedOpponentParty[i];
        }
    }
    gPlayerPartyCount = sSavedPlayerPartyCount;
    gEnemyPartyCount = sSavedEnemyPartyCount;

    // Free allocated memory
    TRY_FREE_AND_SET_NULL(sSavedPlayerParty);
    TRY_FREE_AND_SET_NULL(sSavedOpponentParty);
}

// Build a party from ScriptedPokemon definitions
// Returns the party count
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
        partyCount = i + 1;  // Track highest slot used

        // Set HP
        if (src->currentHP == 0)
        {
            // Fainted
            u16 zero = 0;
            SetMonData(&party[i], MON_DATA_HP, &zero);
        }
        else if (src->currentHP != 0xFFFF)
        {
            // Specific HP value
            SetMonData(&party[i], MON_DATA_HP, &src->currentHP);
        }
        // else leave at max HP

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

        // Set nature (using hidden nature / mint system)
        SetMonData(&party[i], MON_DATA_HIDDEN_NATURE, &src->nature);

        // Set friendship (affects Return/Frustration power)
        SetMonData(&party[i], MON_DATA_FRIENDSHIP, &src->friendship);

        // Recalculate stats after setting IVs/EVs/nature
        CalculateMonStats(&party[i]);

        // Re-apply HP after stat recalc (since max HP may have changed)
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
    ScriptedBattle_RestoreParties();
    sCurrentScriptedBattle = NULL;
    SetMainCallback2(sCallback2_AfterScriptedBattle);
}

// Task to start battle after transition
// data[0] = state, data[1] = transition ID
static void Task_StartScriptedBattle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0:
        // Start transition if one is specified
        if (data[1] != 0)
        {
            BattleTransition_Start(data[1]);
            data[0] = 1;
        }
        else
        {
            // No transition, go straight to battle
            data[0] = 2;
        }
        break;
    case 1:
        // Wait for transition to complete
        if (IsBattleTransitionDone())
            data[0] = 2;
        break;
    case 2:
        // Start battle
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

    // Save current parties and party counts
    ScriptedBattle_SaveParties();

    // Initialize scripted battle state
    ScriptedBattle_Init(battle);

    // Build parties from script and set party counts
    gPlayerPartyCount = ScriptedBattle_BuildParty(gPlayerParty, battle->playerParty);
    gEnemyPartyCount = ScriptedBattle_BuildParty(gEnemyParty, battle->opponentParty);

    // Set up trainer battle parameter so trainer names display correctly
    TRAINER_BATTLE_PARAM.opponentA = battle->opponentTrainerId;

    // Set up battle flags:
    // - RECORDED: doesn't affect save data (no money loss, no Pokemon changes)
    // - SCRIPTED: disables EXP gain and forces "Set" battle style (no switch prompts)
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_SCRIPTED | BATTLE_TYPE_RECORDED | battle->battleFlags;

    // Set RNG seed for deterministic playback
    // Because we use BATTLE_TYPE_RECORDED, the battle system will restore gRngValue from
    // gRecordedBattleRngSeed during init. So we set that seed here.
    if (battle->rngSeed != 0)
        gRecordedBattleRngSeed = LocalRandomSeed(battle->rngSeed);

    // Create task to handle transition and start battle
    taskId = CreateTask(Task_StartScriptedBattle, 1);
    gTasks[taskId].data[0] = 0;  // State
    gTasks[taskId].data[1] = battle->transitionId;  // Transition ID

    // Play battle music and set callback
    PlayMapChosenOrBattleBGM(FALSE);
    SetMainCallback2(CB2_ScriptedBattleWait);
}

// Get custom intro text for battle_message.c (returns NULL to use default)
const u8 *GetScriptedBattleIntroText(void)
{
    if (sCurrentScriptedBattle == NULL)
        return NULL;
    return sCurrentScriptedBattle->introText;
}

// Get announcer message for a given slide type (for trainer_slide.c integration)
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
