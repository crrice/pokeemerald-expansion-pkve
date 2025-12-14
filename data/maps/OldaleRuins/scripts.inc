OldaleRuins_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, OldaleRuins_OnLoad
	.byte 0

OldaleRuins_OnLoad:
	callnative InitBerryThiefTreeState
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
	@ It's night - Phantump encounter!
	msgbox OldaleRuins_Text_SomethingRustling, MSGBOX_DEFAULT
	closemessage
	@ Create Phantump with Harvest ability (slot 2), random everything else
	createmon 1, 0, SPECIES_PHANTUMP, 5, abilityNum=2
	dowildbattle
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

OldaleRuins_EventScript_BerryThiefTree_Normal:
	@ Normal berry tree behavior - call the standard script
	releaseall
	goto BerryTreeScript

@ Text strings
OldaleRuins_Text_BerriesNotReady:
	.string "The berries aren't ripe yet...$"

OldaleRuins_Text_SomethingRustling:
	.string "Something is rustling in the tree!$"

OldaleRuins_Text_PhantumpFled:
	.string "The Phantump fled into the forest...\p"
	.string "The berries on the tree look ready\n"
	.string "to harvest now!$"

OldaleRuins_Text_PhantumpCaught:
	.string "You caught the Phantump!\p"
	.string "The berries on the tree look ready\n"
	.string "to harvest now!$"
