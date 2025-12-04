OldaleTown_House2_MapScripts::
	.byte 0

OldaleTown_House2_EventScript_Woman::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_MACHO_BRACE_OLDALE, OldaleTown_House2_EventScript_Woman_ReceivedItem
	msgbox OldaleTown_House2_Text_WomanGift, MSGBOX_DEFAULT
	giveitem ITEM_MACHO_BRACE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_MACHO_BRACE_OLDALE
	release
	end

OldaleTown_House2_EventScript_Woman_ReceivedItem::
	msgbox OldaleTown_House2_Text_WomanReminder, MSGBOX_DEFAULT
	release
	end

OldaleTown_House2_EventScript_Man::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_WIMPY_BRACE_OLDALE, OldaleTown_House2_EventScript_Man_ReceivedItem
	msgbox OldaleTown_House2_Text_ManGift, MSGBOX_DEFAULT
	giveitem ITEM_WIMPY_BRACE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_WIMPY_BRACE_OLDALE
	release
	end

OldaleTown_House2_EventScript_Man_ReceivedItem::
	msgbox OldaleTown_House2_Text_ManReminder, MSGBOX_DEFAULT
	release
	end

OldaleTown_House2_Text_WomanGift:
	.string "When Pokémon battle, they eventually\n"
	.string "level up and become stronger.\p"
	.string "But levels aren't everything. Effort\n"
	.string "matters too. Here, take this.\p"
	.string "A Pokémon wearing it will be slowed\n"
	.string "down, but they'll put in twice the\l"
	.string "effort!$"

OldaleTown_House2_Text_WomanReminder:
	.string "Training hard? The Macho Brace makes\n"
	.string "every battle count double!$"

OldaleTown_House2_Text_ManGift:
	.string "If the Pokémon with you become\n"
	.string "stronger, you'll be able to go farther\l"
	.string "away from here...\p"
	.string "I've been training to leave, but this\n"
	.string "item seems defective. Here, you take it.$"

OldaleTown_House2_Text_ManReminder:
	.string "After I stopped using that item, my\n"
	.string "Pokémon started to grow!\p"
	.string "Maybe relying on items wasn't the\n"
	.string "answer after all...$"
