@ Local IDs for object events
.set LOCALID_PHANTUMP, LOCALID_OLDALE_RUINS_PHANTUMP

OldaleRuins_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, OldaleRuins_OnLoad
	.byte 0

OldaleRuins_OnLoad:
	callnative InitBerryThiefTreeState
	@ Ensure Phantump is hidden by default
	setflag FLAG_TEMP_2
	end

@ Berry Thief Tree interaction script
OldaleRuins_EventScript_BerryThiefTree::
	lockall
	@ Check if quest is complete - use normal berry tree behavior
	goto_if_quest_complete QUEST_BERRY_THIEF, OldaleRuins_EventScript_BerryThiefTree_Normal
	@ Check if quest is in reward state - use normal berry tree behavior
	goto_if_quest_reward QUEST_BERRY_THIEF, OldaleRuins_EventScript_BerryThiefTree_Normal
	@ Check if quest is active
	goto_if_quest_active QUEST_BERRY_THIEF, OldaleRuins_EventScript_BerryThiefTree_QuestActive
	@ Quest not started - tree is stuck at flowering
	msgbox OldaleRuins_Text_BerriesNotReady, MSGBOX_DEFAULT
	releaseall
	end

OldaleRuins_EventScript_BerryThiefTree_QuestActive:
	@ Check time of day (gettimeofday puts 0-3 in VAR_0x8000)
	gettimeofday
	goto_if_ne VAR_0x8000, TIME_NIGHT, OldaleRuins_EventScript_BerryThiefTree_Day
	@ It's night - offer to wait by the tree
	msgbox OldaleRuins_Text_WaitByTree, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleRuins_EventScript_BerryThiefTree_DontWait
	closemessage
	@ Fade to black, reposition player and hide follower
	fadescreen FADE_TO_BLACK
	hidefollower 0
	setobjectxy OBJ_EVENT_ID_PLAYER, 8, 12
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceUp
	@ Spawn Phantump (hidden by FLAG_TEMP_2, so clear it)
	clearflag FLAG_TEMP_2
	addobject LOCALID_PHANTUMP
	@ Hold black screen for 1 second (60 frames)
	delay 60
	fadescreen FADE_FROM_BLACK
	@ Small pause before Phantump moves
	delay 20
	@ Phantump hops in place (startled awake / emerging)
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpEmerge
	waitmovement 0
	@ Phantump walks south to behind the berry tree
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpToTree
	waitmovement 0
	@ Pause, then question mark (noticing something)
	delay 30
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpNotice
	waitmovement 0
	@ Peek left side of tree first
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpPeekLeft
	waitmovement 0
	@ Then peek right side - spots the player!
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpPeekRight
	waitmovement 0
	@ Comedic pause before it realizes...
	delay 40
	@ Startled! Exclamation mark
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpStartled
	waitmovement 0
	@ Startled hops
	applymovement LOCALID_PHANTUMP, OldaleRuins_Movement_PhantumpEmerge
	waitmovement 0
	@ Text before battle
	msgbox OldaleRuins_Text_StartledAttack, MSGBOX_DEFAULT
	closemessage
	@ Start battle with Harvest ability Phantump
	createmon 1, 0, SPECIES_PHANTUMP, 5, item=ITEM_SITRUS_BERRY, abilityNum=2
	dowildbattle
	@ Remove Phantump sprite after battle
	removeobject LOCALID_PHANTUMP
	setflag FLAG_TEMP_2
	@ Check battle result
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, OldaleRuins_EventScript_BerryThiefTree_Defeated
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, OldaleRuins_EventScript_BerryThiefTree_Captured
	@ Player lost or fled - nothing happens
	releaseall
	end

OldaleRuins_EventScript_BerryThiefTree_Defeated:
	@ Player defeated the Phantump - it flees
	questmenu QUEST_MENU_SET_REWARD, QUEST_BERRY_THIEF
	callnative InitBerryThiefTreeState
	msgbox OldaleRuins_Text_PhantumpFled, MSGBOX_DEFAULT
	releaseall
	end

OldaleRuins_EventScript_BerryThiefTree_Captured:
	@ Player caught the Phantump
	questmenu QUEST_MENU_SET_REWARD, QUEST_BERRY_THIEF
	callnative InitBerryThiefTreeState
	msgbox OldaleRuins_Text_PhantumpCaught, MSGBOX_DEFAULT
	releaseall
	end

OldaleRuins_EventScript_BerryThiefTree_Day:
	@ Daytime while quest active - berries not ready
	msgbox OldaleRuins_Text_BerriesNotReady, MSGBOX_DEFAULT
	releaseall
	end

OldaleRuins_EventScript_BerryThiefTree_DontWait:
	@ Player chose not to wait
	releaseall
	end

OldaleRuins_EventScript_BerryThiefTree_Normal:
	@ Normal berry tree behavior - call the standard script
	releaseall
	goto BerryTreeScript

@ Text strings
OldaleRuins_Text_BerriesNotReady:
	.string "The berries aren't ripe yet...$"

OldaleRuins_Text_WaitByTree:
	.string "Wait by the tree?$"

OldaleRuins_Text_StartledAttack:
	.string "The startled POKéMON attacks!$"

OldaleRuins_Text_PhantumpFled:
	.string "The Phantump fled into the forest...\p"
	.string "The berries on the tree look ready\n"
	.string "to harvest now!$"

OldaleRuins_Text_PhantumpCaught:
	.string "You caught the Phantump!\p"
	.string "The berries on the tree look ready\n"
	.string "to harvest now!$"

@ Movement data for Phantump cutscene
OldaleRuins_Movement_PhantumpEmerge:
	jump_in_place_down
	delay_16
	jump_in_place_down
	delay_16
	delay_16
	step_end

OldaleRuins_Movement_PhantumpToTree:
	walk_slow_down
	walk_slow_down
	walk_slow_down
	step_end

OldaleRuins_Movement_PhantumpNotice:
	emote_question_mark
	delay_16
	delay_16
	delay_16
	delay_16
	step_end

OldaleRuins_Movement_PhantumpPeekLeft:
	walk_slow_left
	face_down
	delay_16
	delay_16
	step_end

OldaleRuins_Movement_PhantumpPeekRight:
	walk_slow_right
	walk_slow_right
	walk_slow_down
	walk_slow_down
	face_left
	delay_16
	step_end

OldaleRuins_Movement_PhantumpStartled:
	emote_exclamation_mark
	delay_16
	delay_16
	delay_16
	delay_16
	step_end
