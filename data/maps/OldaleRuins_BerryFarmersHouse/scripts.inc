OldaleRuins_BerryFarmersHouse_MapScripts::
	.byte 0

@ Old Man NPC script
OldaleRuins_BerryFarmersHouse_EventScript_OldMan::
	lock
	faceplayer
	@ Check quest states in order of priority
	goto_if_quest_complete QUEST_BERRY_THIEF, OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Complete
	goto_if_quest_reward QUEST_BERRY_THIEF, OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Reward
	goto_if_quest_active QUEST_BERRY_THIEF, OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Active
	@ Quest not started - offer quest
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro1, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro2, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro3, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Decline
	@ Player accepted
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Accept, MSGBOX_DEFAULT
	startquest QUEST_BERRY_THIEF
	release
	end

OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Decline:
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Decline, MSGBOX_DEFAULT
	release
	end

OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Active:
	@ Quest in progress - remind player
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Reminder1, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Reminder2, MSGBOX_DEFAULT
	release
	end

OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Reward:
	@ Player caught the thief - thank them and complete quest
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Reward1, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Reward2, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Reward3, MSGBOX_DEFAULT
	giveitem ITEM_ORAN_BERRY, 3
	completequest QUEST_BERRY_THIEF
	release
	end

OldaleRuins_BerryFarmersHouse_EventScript_OldMan_Complete:
	@ Quest already done - friendly chat
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Complete1, MSGBOX_DEFAULT
	msgbox OldaleRuins_BerryFarmersHouse_Text_OldMan_Complete2, MSGBOX_DEFAULT
	release
	end

@ Text strings - Quest intro
OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro1:
	.string "Oh, hello there young trainer!$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro2:
	.string "I've been having trouble lately...\p"
	.string "Something keeps eating the berries\n"
	.string "from my WATMEL tree before I can harvest!\p"
	.string "WATMEL BERRIES are rare around here...\n"
	.string "I'm one of the few who can grow them.$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro3:
	.string "I've tried staying up to catch\n"
	.string "the thief, but I always doze off...$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Intro4:
	.string "Would you help an old man out\n"
	.string "and investigate for me?$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Accept:
	.string "Thank you!\p"
	.string "The thief only seems to come out\n"
	.string "at night.$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Decline:
	.string "Oh... well, come back if you\n"
	.string "change your mind.$"

@ Text strings - Quest active
OldaleRuins_BerryFarmersHouse_Text_OldMan_Reminder1:
	.string "Any luck catching that thief?$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Reminder2:
	.string "Remember, it only seems to\n"
	.string "appear at night...$"

@ Text strings - Quest reward
OldaleRuins_BerryFarmersHouse_Text_OldMan_Reward1:
	.string "You caught the thief?!$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Reward2:
	.string "A PHANTUMP? Well I'll be...\p"
	.string "It must have been hungry,\n"
	.string "poor little thing.$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Reward3:
	.string "The WATMEL tree should be back to\n"
	.string "normal now. Help yourself to some!\p"
	.string "And here, take some ORAN BERRIES too -\n"
	.string "they're not as rare, but just as useful!$"

@ Text strings - Quest complete
OldaleRuins_BerryFarmersHouse_Text_OldMan_Complete1:
	.string "Thanks again for helping me\n"
	.string "with that PHANTUMP!$"

OldaleRuins_BerryFarmersHouse_Text_OldMan_Complete2:
	.string "My WATMEL BERRIES have been growing\n"
	.string "wonderfully ever since!$"
