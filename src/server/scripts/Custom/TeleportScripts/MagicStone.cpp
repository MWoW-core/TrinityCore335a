#include "Player.h"
#include "DatabaseWorker.h"
#include "WorldDatabase.h"
#include "Field.h"
#include "QueryResult.h"
#include "Chat.h"
#include "RBAC.h"
#include "Item.h"
#include "DatabaseEnv.h"
#include "Spell.h"
#include "Logger.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "WorldSession.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "Pet.h"
#include "DBCStores.h"
#include "SpellMgr.h"

enum MenuType
{
	MainMenu = 2,
	SubMenu = 3,
	SecondSubMenu = 4,
	ThirdSubMenu = 5,
	Teleport = 6,
	TokenSevices = 7,
	SpecialServices = 8,
	CharInformation = 9,
	FullTalents = 10,
	TitansGrip = 11,
	GoBack = 16,
	VipZone = 17,
	DualWield = 18,
	HomeBind = 19,
	Mailbox = 30,
	Bank = 31,
	ChangeName = 32,
	ChangeRace = 33,
	ChangeFaction = 34,

};

struct MagicStoneStruct
{
	uint32 menuId;
	uint32 menuType;
	uint32 menuParent;
	std::string menuIcon;
	std::string menuName;
	uint32 mapId;
	float pos_x;
	float pos_y;
	float pos_z;
	float orientation;
};

using StoneMap_t = std::unordered_map<uint32, MagicStoneStruct>;

std::unordered_map<uint32, StoneMap_t> MagicStoneMap;

void LoadMagicStone()
{
	MagicStoneMap.clear();
	QueryResult result = WorldDatabase.Query("SELECT menu_id, menu_type, menu_parent, menu_icon, menu_name, map, position_x, position_y, position_z, orientation FROM z_magic_stone");
	if (result)
	{
		do
		{
			Field* fields = result->Fetch();
			MagicStoneStruct mst;

			mst.menuId = fields[0].GetUInt32();
			mst.menuType = fields[1].GetUInt32();
			mst.menuParent = fields[2].GetUInt32();
			mst.menuIcon = fields[3].GetString();
			mst.menuName = fields[4].GetString();
			mst.mapId = fields[5].GetUInt32();
			mst.pos_x = fields[6].GetFloat();
			mst.pos_y = fields[7].GetFloat();
			mst.pos_z = fields[8].GetFloat();
			mst.orientation = fields[9].GetFloat();

			MagicStoneMap[mst.menuParent][mst.menuId] = mst;

		} while (result->NextRow());
	}
}

class magic_stone_loader : public WorldScript
{
public:
	magic_stone_loader() : WorldScript("magic_stone_loader") { }

	void OnStartup()
	{
		LoadMagicStone();
	}
};

class magic_stone_reload_command : public CommandScript
{
public:
	magic_stone_reload_command() : CommandScript("magic_stone_reload_command") { }

	std::vector<ChatCommand> GetCommands() const
	{
		static std::vector<ChatCommand> commandTable =
		{
			{ "reload_magic_stone", rbac::RBAC_PERM_COMMAND_RELOAD, true, &HandleReloadMagicStoneCommand, "" },
		};
		return commandTable;
	}

	static bool HandleReloadMagicStoneCommand(ChatHandler * handler, const char * args)
	{
		handler->SendSysMessage("Reloading Magic Stone System");
		LoadMagicStone();
		handler->SendSysMessage("Magic Stone System Reloaded");
		return true;
	}
};

class magic_stone : public ItemScript
{
public:
	magic_stone() : ItemScript("magic_stone") {}

	bool LearnFullTalents(Player* player)
	{
		player->ResetTalents();
		uint32 classMask = player->getClassMask();
		Pet* pet = player->GetPet();

		for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
		{
			TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
			if (!talentInfo)
				continue;

			TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
			if (!talentTabInfo)
				continue;

			if ((classMask & talentTabInfo->ClassMask) == 0)
				continue;

			// search highest talent rank
			uint32 spellId = 0;
			for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
			{
				if (talentInfo->RankID[rank] != 0)
				{
					spellId = talentInfo->RankID[rank];
					break;
				}
			}

			if (!spellId)                                        // ??? none spells in talent
				continue;

			SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
			if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
				continue;

			// learn highest rank of talent and learn all non-talent spell ranks (recursive by tree)
			player->LearnSpellHighestRank(spellId);
			player->AddTalent(spellId, player->GetActiveSpec(), true);
		}

		if (!pet)
			return false;

		CreatureTemplate const* creatureInfo = pet->GetCreatureTemplate();
		if (!creatureInfo)
			return false;

		CreatureFamilyEntry const* petFamily = sCreatureFamilyStore.LookupEntry(creatureInfo->family);

		if (petFamily->petTalentType < 0)
			return false;

		for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
		{
			TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
			if (!talentInfo)
				continue;

			TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
			if (!talentTabInfo)
				continue;

			// prevent learn talent for different family (cheating)
			if (((1 << petFamily->petTalentType) & talentTabInfo->petTalentMask) == 0)
				continue;

			// search highest talent rank
			uint32 spellId = 0;

			for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
			{
				if (talentInfo->RankID[rank] != 0)
				{
					spellId = talentInfo->RankID[rank];
					break;
				}
			}

			if (!spellId)                                        // ??? none spells in talent
				continue;

			SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
			if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
				continue;

			// learn highest rank of talent and learn all non-talent spell ranks (recursive by tree)
			pet->learnSpellHighRank(spellId);
		}

		pet->SetFreeTalentPoints(0);
		player->SetFreeTalentPoints(0);
		return true;
	}

	bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
	{
		if (player->HasAura(15007))
		{
			player->RemoveAura(15007);
		}

		if (player->IsInCombat())
		{
			player->GetSession()->SendNotification("You are in combat!");
			CloseGossipMenuFor(player);
			return false;
		}

		if (player->GetAura(8326) && !player->IsAlive())
		{
			player->ResurrectPlayer(1.0f);
			player->SpawnCorpseBones();
			player->SaveToDB();
			player->RemoveAurasDueToSpell(45322);
			return true;
		}

		player->PlayerTalkClass->ClearMenus();
		AddGossipItemFor(player, 0, "|TInterface/ICONS/achievement_character_human_male:30:30:-18:0|t|rCharacter Information", 1, 8);
		AddGossipItemFor(player, 0, "|TInterface/ICONS/inv_misc_coin_02:30:30:-18:0|t|rSpecial Services", 1, 9);
		for (const auto& elem : MagicStoneMap[2])
		{
			const auto& info = elem.second;
			AddGossipItemFor(player, 0, info.menuIcon + info.menuName, info.menuParent, info.menuId);
		}
		SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
		return true;
	}

	void OnGossipSelect(Player * player, Item * item, uint32 parentId, uint32 menuId)
	{
		const auto& menu = MagicStoneMap[parentId][menuId];
		player->PlayerTalkClass->ClearMenus();

		switch (menu.menuType)
		{
		case MenuType::SubMenu:
		{
			for (const auto& menu : MagicStoneMap[menu.menuId])
			{
				const auto& info = menu.second;
				AddGossipItemFor(player, 0, info.menuIcon + info.menuName, info.menuParent, info.menuId);
			}
			AddGossipItemFor(player, 0, "Main Menu", 1, 10);
			SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
		}break;

		case MenuType::SecondSubMenu:
		{
			for (const auto& menu : MagicStoneMap[menu.menuId])
			{
				const auto& info = menu.second;
				AddGossipItemFor(player, 0, info.menuIcon + info.menuName, info.menuParent, info.menuId);
			}
			AddGossipItemFor(player, 0, "Main Menu", 1, 10);
			SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
		}break;

		case MenuType::ThirdSubMenu:
		{
			for (const auto& menu : MagicStoneMap[menu.menuId])
			{
				const auto& info = menu.second;
				AddGossipItemFor(player, 0, info.menuIcon + info.menuName, info.menuParent, info.menuId);
			}
			AddGossipItemFor(player, 0, "Main Menu", 1, 10);
			SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
		}break;

		case MenuType::Teleport:
		{
			SpellCastTargets targets;
			player->TeleportTo(menu.mapId, menu.pos_x, menu.pos_y, menu.pos_z, menu.orientation);
			OnUse(player, item, targets);
		}
		break;

		case MenuType::SpecialServices:
			AddGossipItemFor(player, 0, "Full Talents", 1, 4);
			AddGossipItemFor(player, 0, "Titans Grip", 1, 5);
			AddGossipItemFor(player, 0, "VIP Zone", 1, 11);
			AddGossipItemFor(player, 0, "Dual Wield", 1, 12);
			AddGossipItemFor(player, 0, "VIP Binding", 1, 13);
			AddGossipItemFor(player, 0, "Main Menu", 1, 10);
			SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
			break;

		case MenuType::FullTalents:
			/*if (player->GetSession()->GetSecurity() < SEC_MODERATOR)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You are not VIP");
				return;
			}*/
			if (player->GetCustomCurrency(DP) < 25)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough DP to make this purchase! Required : 25DP");
				return;
			}
			LoginDatabase.PQuery("UPDATE account SET dp=dp-25 WHERE id=%u", player->GetSession()->GetAccountId());
			ChatHandler(player->GetSession()).PSendSysMessage("You have purchased Full-Talents!");
			LearnFullTalents(player);
			break;

		case MenuType::TitansGrip:
			/*if (player->GetSession()->GetSecurity() < SEC_MODERATOR)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You are not VIP");
				return;
			}*/
			if (player->GetCustomCurrency(DP) < 50)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough DP to make this purchase! Required : 50DP");
				return;
			}
			LoginDatabase.PQuery("UPDATE account SET dp=dp-50 WHERE id=%u", player->GetSession()->GetAccountId());
			ChatHandler(player->GetSession()).PSendSysMessage("You have learnt Titan's Grip!");
			player->LearnSpell(46917, false);
			break;
		case MenuType::DualWield:
		{
			/*if (player->GetSession()->GetSecurity() < SEC_MODERATOR)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You are not VIP");
				return;
			}*/
			ChatHandler(player->GetSession()).PSendSysMessage("You have learnt Dual-Wield!");
			player->LearnSpell(674, false);
		}
		break;

		case MenuType::VipZone:
			/*if (player->GetSession()->GetSecurity() < SEC_MODERATOR)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You are not VIP");
				return;
			}*/
			{
				SpellCastTargets targets;
				player->TeleportTo(menu.mapId, menu.pos_x, menu.pos_y, menu.pos_z, menu.orientation);
				OnUse(player, item, targets);
			}
			break;
			break;

		case MenuType::HomeBind:
		{
			/*if (player->GetSession()->GetSecurity() < SEC_MODERATOR)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You are not VIP");
				return;
			}*/
			CloseGossipMenuFor(player);
			WorldLocation loc = player->GetWorldLocation();
			player->SetHomebind(loc, player->GetAreaId());
			ChatHandler(player->GetSession()).PSendSysMessage("Your new home has been set.");
		}
		break;

		case MenuType::CharInformation:
			AddGossipItemFor(player, 0, "You have " || player->GetCustomCurrency(DP) + " DP", 1, 10);
			AddGossipItemFor(player, 0, "You have " || player->GetCustomCurrency(VP) + " VP", 1, 10);
			AddGossipItemFor(player, 0, "You have " || player->GetCustomCurrency(MG) + " MG", 1, 10);
			AddGossipItemFor(player, 0, "Mailbox", 1, 30);
			AddGossipItemFor(player, 0, "Bank", 1, 31);
			AddGossipItemFor(player, 0, "Main Menu", 1, 16); // De her burde virke, men. Den her det rigtige id
			SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
			break;

		case MenuType::Mailbox:
			CloseGossipMenuFor(player);
			player->GetSession()->SendShowMailBox(player->GetGUID());
			break;

		case MenuType::Bank:
			CloseGossipMenuFor(player);
			player->GetSession()->SendShowBank(player->GetGUID());
			break;

		case MenuType::ChangeFaction:
			if (!player->HasItemCount(ChangeFaction, 1))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have the required token!");
				return;
			}
			player->DestroyItemCount(ChangeFaction, 1, true);
			player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
			ChatHandler(player->GetSession()).PSendSysMessage("Please relog for the system to take effect!");
			break;

		case MenuType::ChangeRace:
			if (!player->HasItemCount(ChangeRace, 1))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have the required token!");
				return;
			}
			player->DestroyItemCount(ChangeRace, 1, true);
			player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
			ChatHandler(player->GetSession()).PSendSysMessage("Please relog for the system to take effect!");
			break;

		case MenuType::ChangeName:
			if (!player->HasItemCount(ChangeName, 1))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have the required token!");
				return;
			}
			player->DestroyItemCount(ChangeName, 1, true);
			player->SetAtLoginFlag(AT_LOGIN_RENAME);
			ChatHandler(player->GetSession()).PSendSysMessage("Please relog for the system to take effect!");
			break;

		case MenuType::GoBack:
		{
			SpellCastTargets targets;
			OnUse(player, item, targets);
		}
		break;

		}

	}

};

// Add to script loader
void AddSC_magic_stone()
{
	new magic_stone();
	new magic_stone_reload_command();
	new magic_stone_loader();
}
