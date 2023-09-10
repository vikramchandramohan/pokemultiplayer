#include "global.h"
#include "gflib.h"
#include "data.h"
#include "m4a.h"
#include "task.h"
#include "util.h"
#include "pokeball.h"
#include "random.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_interface.h"
#include "battle_tower.h"
#include "battle_gfx_sfx_util.h"
#include "battle_ai_script_commands.h"
#include "battle_ai_switch_items.h"
#include "trainer_tower.h"
#include "constants/battle_anim.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/sound.h"
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

static void OpponentHandleGetMonData(void);
static void OpponentHandleGetRawMonData(void);
static void OpponentHandleSetMonData(void);
static void OpponentHandleSetRawMonData(void);
static void OpponentHandleLoadMonSprite(void);
static void OpponentHandleSwitchInAnim(void);
static void OpponentHandleReturnMonToBall(void);
static void OpponentHandleDrawTrainerPic(void);
static void OpponentHandleTrainerSlide(void);
static void OpponentHandleTrainerSlideBack(void);
static void OpponentHandleFaintAnimation(void);
static void OpponentHandlePaletteFade(void);
static void OpponentHandleSuccessBallThrowAnim(void);
static void OpponentHandleBallThrowAnim(void);
static void OpponentHandlePause(void);
static void OpponentHandleMoveAnimation(void);
static void OpponentHandlePrintString(void);
static void OpponentHandlePrintSelectionString(void);
static void OpponentHandleChooseAction(void);
static void OpponentHandleUnknownYesNoBox(void);
static void OpponentHandleChooseMove(void);
static void OpponentHandleChooseItem(void);
static void OpponentHandleChoosePokemon(void);
static void OpponentHandleCmd23(void);
static void OpponentHandleHealthBarUpdate(void);
static void OpponentHandleExpUpdate(void);
static void OpponentHandleStatusIconUpdate(void);
static void OpponentHandleStatusAnimation(void);
static void OpponentHandleStatusXor(void);
static void OpponentHandleDataTransfer(void);
static void OpponentHandleDMA3Transfer(void);
static void OpponentHandlePlayBGM(void);
static void OpponentHandleCmd32(void);
static void OpponentHandleTwoReturnValues(void);
static void OpponentHandleChosenMonReturnValue(void);
static void OpponentHandleOneReturnValue(void);
static void OpponentHandleOneReturnValue_Duplicate(void);
static void OpponentHandleCmd37(void);
static void OpponentHandleCmd38(void);
static void OpponentHandleCmd39(void);
static void OpponentHandleCmd40(void);
static void OpponentHandleHitAnimation(void);
static void OpponentHandleCmd42(void);
static void OpponentHandlePlaySE(void);
static void OpponentHandlePlayFanfare(void);
static void OpponentHandleFaintingCry(void);
static void OpponentHandleIntroSlide(void);
static void OpponentHandleIntroTrainerBallThrow(void);
static void OpponentHandleDrawPartyStatusSummary(void);
static void OpponentHandleHidePartyStatusSummary(void);
static void OpponentHandleEndBounceEffect(void);
static void OpponentHandleSpriteInvisibility(void);
static void OpponentHandleBattleAnimation(void);
static void OpponentHandleLinkStandbyMsg(void);
static void OpponentHandleResetActionMoveSelection(void);
static void OpponentHandleCmd55(void);
static void OpponentCmdEnd(void);

// Keep the same functions as battle_controller_opponent for things like animations

static void (*const sOpponentBufferCommands[CONTROLLER_CMDS_COUNT])(void) =
{
    [CONTROLLER_GETMONDATA]               = OpponentHandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = OpponentHandleGetRawMonData,
    [CONTROLLER_SETMONDATA]               = OpponentHandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = OpponentHandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = OpponentHandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = OpponentHandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = OpponentHandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = OpponentHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = OpponentHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = OpponentHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = OpponentHandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = OpponentHandlePaletteFade,
    [CONTROLLER_SUCCESSBALLTHROWANIM]     = OpponentHandleSuccessBallThrowAnim,
    [CONTROLLER_BALLTHROWANIM]            = OpponentHandleBallThrowAnim,
    [CONTROLLER_PAUSE]                    = OpponentHandlePause,
    [CONTROLLER_MOVEANIMATION]            = OpponentHandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = OpponentHandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = OpponentHandlePrintSelectionString,
    [CONTROLLER_CHOOSEACTION]             = OpponentHandleChooseAction,
    [CONTROLLER_UNKNOWNYESNOBOX]          = OpponentHandleUnknownYesNoBox,
    [CONTROLLER_CHOOSEMOVE]               = OpponentHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = OpponentHandleChooseItem,
    [CONTROLLER_CHOOSEPOKEMON]            = OpponentHandleChoosePokemon,
    [CONTROLLER_23]                       = OpponentHandleCmd23,
    [CONTROLLER_HEALTHBARUPDATE]          = OpponentHandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = OpponentHandleExpUpdate,
    [CONTROLLER_STATUSICONUPDATE]         = OpponentHandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = OpponentHandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = OpponentHandleStatusXor,
    [CONTROLLER_DATATRANSFER]             = OpponentHandleDataTransfer,
    [CONTROLLER_DMA3TRANSFER]             = OpponentHandleDMA3Transfer,
    [CONTROLLER_PLAYBGM]                  = OpponentHandlePlayBGM,
    [CONTROLLER_32]                       = OpponentHandleCmd32,
    [CONTROLLER_TWORETURNVALUES]          = OpponentHandleTwoReturnValues,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = OpponentHandleChosenMonReturnValue,
    [CONTROLLER_ONERETURNVALUE]           = OpponentHandleOneReturnValue,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = OpponentHandleOneReturnValue_Duplicate,
    [CONTROLLER_CLEARUNKVAR]              = OpponentHandleCmd37,
    [CONTROLLER_SETUNKVAR]                = OpponentHandleCmd38,
    [CONTROLLER_CLEARUNKFLAG]             = OpponentHandleCmd39,
    [CONTROLLER_TOGGLEUNKFLAG]            = OpponentHandleCmd40,
    [CONTROLLER_HITANIMATION]             = OpponentHandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = OpponentHandleCmd42,
    [CONTROLLER_PLAYSE]                   = OpponentHandlePlaySE,
    [CONTROLLER_PLAYFANFARE]              = OpponentHandlePlayFanfare,
    [CONTROLLER_FAINTINGCRY]              = OpponentHandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = OpponentHandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = OpponentHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = OpponentHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = OpponentHandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = OpponentHandleEndBounceEffect,
    [CONTROLLER_SPRITEINVISIBILITY]       = OpponentHandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = OpponentHandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = OpponentHandleLinkStandbyMsg,
    [CONTROLLER_RESETACTIONMOVESELECTION] = OpponentHandleResetActionMoveSelection,
    [CONTROLLER_ENDLINKBATTLE]            = OpponentHandleCmd55,
    [CONTROLLER_TERMINATOR_NOP]           = OpponentCmdEnd
};

// unknown unused data
static const u8 sUnused[] = { 0xB0, 0xB0, 0xC8, 0x98, 0x28, 0x28, 0x28, 0x20 };

void SendInfoToWebpage(void)
{
    int port = 80;
    char *host = "";
    
}

static void OpponentDummy(void)
{
}

void SetControllerToMultiplayer(void)
{
    gBattlerControllerFuncs[gActiveBattler] = GetActionFromOtherPlayer;
}

static void GetActionFromOtherPlayer(void)
{
    // Send info to webpage
    
    // Receive info from webpage
    if (gBattleControllerExecFlags & gBitTable[gActiveBattler])
    {
        if (gBattleBufferA[gActiveBattler][0] < NELEMS(sOpponentBufferCommands))
            sOpponentBufferCommands[gBattleBufferA[gActiveBattler][0]]();
        else
            OpponentBufferExecCompleted();
    }
}

static void CompleteOnBattlerSpriteCallbackDummy(void)
{
    if (gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
        OpponentBufferExecCompleted();
}

