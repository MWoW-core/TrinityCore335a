/*
* mond wow repack shit
* @Author - Artamedes
*/
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
static const uint32 LOC_OFFSET = 1200;

enum TeleStone
{
	MAIN_MENU = 200000,
	BuffPlayer = 400,

	WORLD_TELEPORT, //1
	KALIMDOR,//2
	EASTERN_KINGDOMS,//3
	NORTHREND,//4
	OUTLANDS,//5
	MAELSTROM,//6
	PANDARIA,//7
	DUNGEONS_AND_RAIDS, //8
	VANILLA_DUNGS_AND_RAIDS, //9
	TBC_DUNGS_AND_RAIDS,//10 
	LICH_KING_DUNGS_AND_RAIDS,//11
	CATACLYSM_DUNGS_AND_RAIDS,//12
	ARENAS,//14
	CUSTOM_INSTANCES,//15
	PLAYER_INFO,//16
	VIP_OPTIONS,//17
	VIP_FEATURES,//18
	DUNGEONS,//19
	RAIDS,//20
	WORLD_BOSSES,//21
	PLAYER_OPTIONS, //22
	ChangeName, //23
	ChangeRace, //24
	ChangeFaction, //25
	DualWield, //26
	TitanGrip, //27
	FullTalents, //28
	HomeBinding, //29
};

struct TeleportInfo
{
	uint32 mapid;
	float x, y, z, orientation;
};

static const TeleportInfo locations[] =
{
	//      x          y         z      o   map
	{ 530, -248.765f, 957.539f, 84.3573f, 4.60909f }, // Mall
																//Kalimdor 1 - 22
	{ 1, 1928.4f, -2166.0f, 94.0f, 0.2f }, //ashenvale 1
	{ 1, 2897.182f, -4833.0f, 128.5f, 1.7f }, //azshara 2
	{ 530, -3139.08f, -12548.6f, 38.029f, 4.2f }, //azurement isle 3
	{ 530, -1945.0f, -11873.70f, 49.4f, 6.04f }, //Bloodmyst Isle 4
	{ 1, 5767.250f, 298.50f, 20.60f, 6.0f }, //Darkshore 5 
	{ 1, 4102.25f, -1006.789978f, 272.717010f, 0.790048f }, //Felwood 6
	{ 1, 5534.0f, -3622.0f, 1567.0f, 2.0f }, //Mount Hyjal 7
	{ 1, 7654.3f, -2232.870f, 462.10f, 5.960f }, //Moonglade 8
	{ 1, 10111.3f, 1557.729980f, 1324.325806f, 4.042390f }, //Teldrassil  9
	{ 1, 6759.180f, -4419.630f, 763.2139890f, 4.434750f }, //Winterspring 10
	{ 1, 65.558f, -2713.2211f, 91.667f, 0.158f }, //The Barrens  11
	{ 1, -606.395f, 2211.750f, 92.981f, 0.809f }, //Desolace 12
	{ 1, 1007.780029f, -4446.220215f, 11.2022f, 0.20f }, //Durotar 13
	{ 1, -3646.879f, -3847.610f, 4.727f, 6.0868f }, //Dustwallow Marsh 14 
	{ 1, -2184.730f, -732.133484f, -13.645f, 0.487f }, //Mulgore 15 
	{ 1, 1570.920f, 1031.520f, 137.9590f, 3.330f }, //Stonetalon Mountains 16
	{ 1, -4840.42f, 1318.4f, 81.5f, 1.5f }, //Feralas 17
	{ 1, -7426.870f, 1005.309f, 1.135f, 2.960f }, //Silithus 18
	{ 1, -7931.200f, -3414.280029f, 80.736f, 0.665f }, //Tanaris 19
	{ 1, -5126.258f, -1982.943f, 87.866f, 4.168f }, //Thousand Needles 20
	{ 1, -10073.239f, -2697.993f, 9.767f, 2.90f }, //Uldum 21
	{ 1, -7943.20f, -2119.0900f, -218.3450f, 6.072f }, //Un'Goro Crater 22
													   //Eastern Kingdoms 23 - 40
	{ 0, -11182.50f, -3016.669922f, 7.424809f, 4.0004f },  //Blasted Lands 23 
	{ 0, -10438.79980f, -1932.750f, 104.616997f, 4.7740f },  //Deadwind Pass 24
	{ 0, -10904.0f, -364.240f, 39.5f, 3.04f },  //Duskwood 25
	{ 0, -9616.558f, -300.0f, 57.31f, 5.0f },  //Elwynn Forest 26
	{ 0, -8956.39f, -2304.44f, 132.460f, 1.588f },  //Redridge Mts 27 
	{ 0, -12634.480f, -378.139f, 18.886629f, 6.229370f },  //Stranglethorn Vale 28
	{ 0, -10132.4560f, -2471.5668f, 27.5627f, 3.9186f },  //Swamp of Sorrows 29
	{ 0, -10235.200f, 1222.469971f, 43.625f, 0.53f },  //Westfall 30
	{ 0, -6570.355957f, -3235.079102f, 244.407547f, 3.400f },  //Badlands 31
	{ 0, -7622.440f, -1222.197f, 247.272f, 5.169f },  //BlockRock Mountains 32
	{ 0, -8118.540f, -1633.83f, 132.9960f, 0.0890f },  //Burning Steppes 33
	{ 0, -5444.419f, -656.504f, 394.24f, 6.24f },  //Dun Morogh 34 
	{ 0, -5202.939941f, -2855.179932f, 335.901489f, 0.37f },  //Loch Modan 35
	{ 0, -7007.812988f, -1068.956421f, 241.932938f, 5.39600f },  //Searing Gorge 36 
	{ 0, -3242.810059f, -2469.0400f, 15.922f, 6.039f },  //wetlands 37
	{ 530, 9080.0f, -7193.2299f, 55.60f, 5.95f },  //Eversong Woods 38
	{ 530, 7360.9f, -6803.3f, 44.294201f, 5.836f },  //Ghostlands 39
	{ 530, 12947.40f, -6893.310f, 5.68f, 3.09f },  //Isle of Quel'Danas 40
												   //Outland 41 - 47
	{ 530, 3037.669922f, 5962.859863f, 130.774002f, 1.27530f }, //Blade's Edge Mountains 41
	{ 530, 417.249f, 3159.980f, -87.628f, 0.43f }, //Hellfire 42
	{ 530, -1145.949f, 8182.350f, 3.60f, 6.134f }, //Nagrand 43
	{ 530, 3830.229980f, 3426.50f, 88.614502f, 5.166770f }, //Netherstorm 44
	{ 530, -1955.109f, 4563.6088f, 12.54f, 4.45f }, //Tekkorar 45
	{ 530, -54.8620f, 5813.4399f, 20.60f, 0.08f }, //Zangarmarsh 46
	{ 530, -2746.632568f, 1556.131104f, 36.950989f, 3.7086f }, //Shadowmoon 47
															   //Northrend 48 - 56

	{ 571, 3256.570f, 5278.229980f, 40.80460f, 0.246f }, //borean 48
	{ 571, 4103.3599f, 263.477997f, 50.501900f, 3.093490f }, //dragonblight 49
	{ 571, 7253.640f, 1644.7800f, 433.67993f, 4.834f }, //icecrown 50
	{ 571, 5323.0f, 4942.0f, -133.558517f, 2.1700f }, //sholazar bsasin 51
	{ 571, 5614.674316f, -325.961578f, 154.875900f, 4.212954f }, //crystalsong 52
	{ 571, 5804.998535f, -3453.326172f, 372.397156f, 5.629090f }, // zuldrak 53
	{ 571, 4391.729980f, -3587.919922f, 238.531006f, 3.575260f }, //grizzly hills 54
	{ 571, 1902.1500f, -4883.910f, 171.3630f, 3.115370f }, // howling fjord 55
	{ 571, 7527.140f, -1260.890f, 919.0490f, 2.069600f }, // storm peaks 56

														  //cata 57-59

	{ 646, 915.004f, 503.842f, -49.230f, 0.196f }, //deepholm 57
	{ 0, -4336.856934f, -4162.810547f, 329.128f, 3.06f }, //twilgiht highalnds 58
	{ 861, 1037.712f, 335.423f, 40.963f, 1.2f }, //molten front
	{ 0, -13261.224f, 163.995f, 35.862f, 1.163f }, // GURUBASHI
};

class item_teleporter : public ItemScript
{
public:
	item_teleporter() : ItemScript("item_teleporter") { }

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

	void TeleportTo(Player* player, uint32 action)
	{
		uint32 idx = action - LOC_OFFSET;
		CloseGossipMenuFor(player);
		player->TeleportTo(locations[idx].mapid, locations[idx].x, locations[idx].y, locations[idx].z, locations[idx].orientation);
	}

	void AddMainMenuGossipItem(Player* _player)
	{
		AddGossipItemFor(_player, 0, "|cffFF0040| - Main Menu", GOSSIP_SENDER_MAIN, 0);
	}

	void OnGossipHello(Player* player, Item* item)
	{

		if (player->isDead())
			player->ResurrectPlayer(1.0f, true);

		AddGossipItemFor(player, 0, "|cffFA58F4| - Task Area", GOSSIP_SENDER_MAIN, LOC_OFFSET + 0); //start at 0
		AddGossipItemFor(player, 0, "|cffBF00FF| - Main Cities", GOSSIP_SENDER_MAIN, 2);
		AddGossipItemFor(player, 0, "|cffFF0040| - PvP Area", GOSSIP_SENDER_MAIN, 320001);
		AddGossipItemFor(player, 0, "|cff0040FF| - Custom Dungeons", GOSSIP_SENDER_MAIN, 310000);
		AddGossipItemFor(player, 0, "|cff3B0B39| - Special Dungeons", GOSSIP_SENDER_MAIN, 3100002);
		AddGossipItemFor(player, 0, "|cff4C0B5F| - Super Instances", GOSSIP_SENDER_MAIN, 1000000);
		AddGossipItemFor(player, 0, "|cff4C0B5F| - Dungeons & Raids", GOSSIP_SENDER_MAIN, 30);
		AddGossipItemFor(player, 0, "|cff0B610B| - World Boss", GOSSIP_SENDER_MAIN, 320000);
		// AddGossipItemFor(player, 0, "|cff0000ff| - Player Info", GOSSIP_SENDER_MAIN, 16);
		AddGossipItemFor(player, 0, "|cff0000ff| - Options", GOSSIP_SENDER_MAIN, 20000000);
		AddGossipItemFor(player, 0, "|cff4B00FF| - V.I.P Status", GOSSIP_SENDER_MAIN, 17); //start at 17

		player->PlayerTalkClass->SendGossipMenu(MAIN_MENU, item->GetGUID());
	}

	bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
	{
		if (player->isDead())
			player->ResurrectPlayer(1.0f, true);

		player->PlayerTalkClass->ClearMenus();
		if (!player->IsInCombat())
			OnGossipHello(player, item);
		else
			player->CombatStop(true);
		return true;
	}

	void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
	{
		player->PlayerTalkClass->ClearMenus();

		if (action >= LOC_OFFSET && action < LOC_OFFSET + sizeof(locations))
		{
			TeleportTo(player, action);
			return;
		}

		switch (action)
		{
		case 0:
			OnGossipHello(player, item);
			break;

		case 16: // player info
		{
			std::ostringstream ss;
			ss << "[|cFFFF0000" << player->GetName() << "|R]" << " Account ID: " << player->GetSession()->GetAccountId() << " VIP Level: " << int(player->GetSession()->GetVIPLevel());
			AddGossipItemFor(player, 0, ss.str().c_str(), GOSSIP_SENDER_MAIN, 7);
			ss.str("");
			ss.clear();
			ss << "MG: |cFFFF0000" << player->GetCustomCurrency(MG) << "|R";
			AddGossipItemFor(player, 10, ss.str().c_str(), GOSSIP_SENDER_MAIN, 7);
			ss.str("");
			ss.clear();
			ss << "VP: |cFFFF0000" << player->GetCustomCurrency(VP) << "|R";
			AddGossipItemFor(player, 10, ss.str().c_str(), GOSSIP_SENDER_MAIN, 7);
			ss.str("");
			ss.clear();
			ss << "DP: |cFFFF0000" << player->GetCustomCurrency(DP) << "|R";
			AddGossipItemFor(player, 10, ss.str().c_str(), GOSSIP_SENDER_MAIN, 7);
			ss.str("");
			ss.clear();
			ss << "Latency: |cFFFF0000" << player->GetSession()->GetLatency() << "|R ms";
			AddGossipItemFor(player, 10, ss.str().c_str(), GOSSIP_SENDER_MAIN, 7);
			ss.str("");
			ss.clear();

			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 20000000);
			player->PlayerTalkClass->SendGossipMenu(PLAYER_INFO, item->GetGUID());
		}
		break;

		case 320001: //pvp Arena
			AddGossipItemFor(player, 0, "|cFF9932CC| - Gurubashi arena", GOSSIP_SENDER_MAIN, 70000000);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(ARENAS, item->GetGUID());
			break;

		case 70000000: //Gurubashi arena
			player->TeleportTo(0, -13262.6f, 161.518f, 36.4557f, 1.09443f);
			break;

		case 2: //World Teleport
			AddGossipItemFor(player, 0, "|cFF9932CC| - Kalimdor", GOSSIP_SENDER_MAIN, 210000);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Eastern Kingdoms", GOSSIP_SENDER_MAIN, 220000);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Outland", GOSSIP_SENDER_MAIN, 230000);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Northrend", GOSSIP_SENDER_MAIN, 240000);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(WORLD_TELEPORT, item->GetGUID());
			break;

			break;
		case 3100002: //Special Dung
			AddGossipItemFor(player, 0, "|cFF9932CC| - Emerald Forest", GOSSIP_SENDER_MAIN, 310003);
			AddGossipItemFor(player, 0, "|cFF9932CC| - The Verdant Fields", GOSSIP_SENDER_MAIN, 310002);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Abyssal Sands", GOSSIP_SENDER_MAIN, 310005);
			AddGossipItemFor(player, 0, "|cFF9932CC| - The Desert Eagle", GOSSIP_SENDER_MAIN, 310006);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(CUSTOM_INSTANCES, item->GetGUID());
			break;

		case 310000: //Custom dung
			AddGossipItemFor(player, 0, "|cFF9932CC| - Shadowfang Keep [Tier 1]", GOSSIP_SENDER_MAIN, 9000001);
			AddGossipItemFor(player, 0, "|cFF9932CC| - The Deadmines", GOSSIP_SENDER_MAIN, 9000002);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Pools of Aggonar", GOSSIP_SENDER_MAIN, 9000003);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Thousand Needles", GOSSIP_SENDER_MAIN, 9000004);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Zul'Gurub", GOSSIP_SENDER_MAIN, 9000005);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Hyjal Jihad", GOSSIP_SENDER_MAIN, 9000006);
			AddGossipItemFor(player, 0, "|cFF9932CC| - The Frozen Island", GOSSIP_SENDER_MAIN, 9000007);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Onyxia's Lair", GOSSIP_SENDER_MAIN, 9000008);
			AddGossipItemFor(player, 0, "|cFF9932CC| - The Forge of Souls", GOSSIP_SENDER_MAIN, 9000009);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(CUSTOM_INSTANCES, item->GetGUID());
			break;

		case 9000001: // Shadowfang Keep [Tier 1]
			player->TeleportTo(33, -229.1349f, 2109.179f, 76.8998f, 1.2670f);
			break;

		case 9000002: //The Deadmines
			player->TeleportTo(36, -1670.48f, 467.406f, 1.70451f, 0.057373f);
			break;

		case 9000003: //Pools of Aggonar
			player->TeleportTo(543, -1029.34f, 2052.78f, 62.8145f, 6.02118f);
			break;

		case 9000004: //Thousand Needles
			player->TeleportTo(209, 2376.93f, 1585.09f, -62.0408f, 4.41077f);
			break;

		case 9000005: //Zul'Gurub
			player->TeleportTo(309, -11643.1f, -2360.08f, 5.73839f, 3.78048f);
			break;

		case 9000006: //Hyjal jihad
			player->TeleportTo(534, 4603.34f, -3915.06f, 944.172f, 1.09223f);
			break;

		case 9000007: //The Frozen island
			player->TeleportTo(599, 1317.69f, -323.907f, 488.236f, 0.948025f);
			break;

		case 9000008: //Onyxia' s Lair
			player->TeleportTo(249, 29.2204f, -70.286f, -7.50108f, 4.64004f);
			break;

		case 9000009: //The Forge of Souls
			player->TeleportTo(632, 4921.1f, 2179.44f, 638.734f, 2.00355f);
			break;

		case 1000000: //Super instance
			AddGossipItemFor(player, 0, "|cffFF0040| - Halls of Lightning", GOSSIP_SENDER_MAIN, 1000001);
			AddGossipItemFor(player, 0, "|cffFF0040| - The Obsidian Sanctum", GOSSIP_SENDER_MAIN, 1000002);
			AddGossipItemFor(player, 0, "|cffFF0040| - The Shattered Halls", GOSSIP_SENDER_MAIN, 1000003);
			AddGossipItemFor(player, 0, "|cffFF0040| - The Blood Furnace", GOSSIP_SENDER_MAIN, 1000004);
			AddGossipItemFor(player, 0, "|cffFF0040| - Dead Mans Well", GOSSIP_SENDER_MAIN, 1000005);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(CUSTOM_INSTANCES, item->GetGUID());
			break;

		case 1000001: /* halls of lighning */
			player->TeleportTo(602, 1331.3f, 254.842f, 53.398f, 4.70131f);
			break;

		case 1000002: /* The Obsidian Sanctum */
			player->TeleportTo(615, 4168.14f, 953.362f, 163.911f, 5.10836f);
			break;

		case 1000003: /* the Shattered Halls */
			player->TeleportTo(540, -15.2822f, 1.54341f, -13.195f, 6.25961f);
			break;

		case 1000004: /* The Blood Furnace */
			player->TeleportTo(542, -2.9642f, 7.02034f, -44.5292f, 4.84036f);
			break;

		case 1000005: /* Dead Mans Will */
			player->TeleportTo(0, -11215.3f, -1805.67f, -28.422f, 1.57742f);
			break;


		case 17: //V.I.P Status
			AddGossipItemFor(player, 0, "|cff00bd13| - V.I.P Mall |cFFFF0000| - [Disabled]", GOSSIP_SENDER_MAIN, 80000000);
			AddGossipItemFor(player, 0, "|cFF9932CC| - Bind Your Home [VIP1]", GOSSIP_SENDER_MAIN, 29);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Dual - Wield [VIP1]", GOSSIP_SENDER_MAIN, 26);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Titan's Grip [VIP2][35DP]", GOSSIP_SENDER_MAIN, 27);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Full Talents [VIP2][25DP]", GOSSIP_SENDER_MAIN, 28);
			AddMainMenuGossipItem(player);

			player->PlayerTalkClass->SendGossipMenu(DualWield, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(TitanGrip, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(FullTalents, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(HomeBinding, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(VIP_FEATURES, item->GetGUID());
			break;
		case 20000000: // player Options
			AddGossipItemFor(player, 0, "|cFF9932CC| - Bank", GOSSIP_SENDER_MAIN, 6);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Change Name", GOSSIP_SENDER_MAIN, 23);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Change Race", GOSSIP_SENDER_MAIN, 24);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Change Faction", GOSSIP_SENDER_MAIN, 25);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Buff Yourself", GOSSIP_SENDER_MAIN, 400);
			AddGossipItemFor(player, 0, "|cffBF00FF| - Player Info", GOSSIP_SENDER_MAIN, 16);
			AddMainMenuGossipItem(player);

			player->PlayerTalkClass->SendGossipMenu(ChangeName, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(ChangeRace, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(ChangeFaction, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(BuffPlayer, item->GetGUID());
			player->PlayerTalkClass->SendGossipMenu(PLAYER_INFO, item->GetGUID());
			break;


		case 310002://The Verdant Fields
			player->TeleportTo(169, -2842.033936f, -2606.086426f, 135.916718f, 1.505297f);
			break;

		case 310003: //Emerald Forest
			player->TeleportTo(169, 2905.32f, -3526.25f, 99.9136f, 2.1165f);
			break;

		case 310005: //Abyssal Sands
			player->TeleportTo(209, 1015.68f, 11.2978f, 31.1031f, 2.36565f);
			break;

		case 310006: //Desert Eagle
			player->TeleportTo(1, -8967.68f, 1550.15f, 20.0316f, 2.90594f);
			break;


		case 30: //Dungeons and Raids
			AddGossipItemFor(player, 0, "|cffFF0000| - Dungeons", GOSSIP_SENDER_MAIN, 31);
			AddGossipItemFor(player, 0, "|cffFF0000| - Raids", GOSSIP_SENDER_MAIN, 32);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(DUNGEONS_AND_RAIDS, item->GetGUID());
			break;

		case 881:
			player->TeleportTo(1, 4031.05f, -3054.48999f, 569.973999f, 1.938210f);
			break;

		case 882:
			player->TeleportTo(571, 2871.265381f, 66.218613f, 1.337f, 3.372304f);
			break;

		case 10:
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(PLAYER_OPTIONS, item->GetGUID());
			break;

		case 31://Dungeons
			AddGossipItemFor(player, 0, "|cffFF0000| - Vanilla", GOSSIP_SENDER_MAIN, 311);
			AddGossipItemFor(player, 0, "|cffFF0000| - The Burning Crusade", GOSSIP_SENDER_MAIN, 312);
			AddGossipItemFor(player, 0, "|cffFF0000| - Wrath of the Lich King", GOSSIP_SENDER_MAIN, 313);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 30);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(DUNGEONS, item->GetGUID());
			break;

		case 320000://Worldboss
			AddGossipItemFor(player, 0, "|cffFF0000| - Battle Angel", GOSSIP_SENDER_MAIN, 100000);
			AddGossipItemFor(player, 0, "|cffFF0000| - Lich King", GOSSIP_SENDER_MAIN, 100001);
			AddGossipItemFor(player, 0, "|cffFF0000| - Illidan", GOSSIP_SENDER_MAIN, 100002);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(WORLD_BOSSES, item->GetGUID());
			break;

		case 10000://Battle angel
			CloseGossipMenuFor(player);
			player->TeleportTo(0, 745.286f, -4016.07f, 93.4931f, 0.334191f);
			break;


		case 311:
			AddGossipItemFor(player, 0, "|cffFF0000| - Ragefire Chasm", 1, 99900);
			AddGossipItemFor(player, 0, "|cffFF0000| - Deadmines", 1, 99901);
			AddGossipItemFor(player, 0, "|cffFF0000| - Wailing Caverns", 1, 99902);
			AddGossipItemFor(player, 0, "|cffFF0000| - Shadowfang Keep", 1, 99903);
			AddGossipItemFor(player, 0, "|cffFF0000| - Blackfathom Deeps", 1, 99904);
			AddGossipItemFor(player, 0, "|cffFF0000| - Gnomeregan", 1, 99906);
			AddGossipItemFor(player, 0, "|cffFF0000| - Razorfen Kraul", 1, 99907);
			AddGossipItemFor(player, 0, "|cffFF0000| - Maraudon", 1, 99908);
			AddGossipItemFor(player, 0, "|cffFF0000| - Scarlet Monastery", 1, 99912);
			AddGossipItemFor(player, 0, "|cffFF0000| - Uldaman", 1, 99913);
			AddGossipItemFor(player, 0, "|cffFF0000| - Dire Maul", 1, 99914);
			AddGossipItemFor(player, 0, "|cffFF0000| - Scholomance", 1, 99915);
			AddGossipItemFor(player, 0, "|cffFF0000| - Razorfen Downs", 1, 99917);
			AddGossipItemFor(player, 0, "|cffFF0000| - Stratholme", 1, 99919);
			AddGossipItemFor(player, 0, "|cffFF0000| - Zul'Farrak", 1, 99920);
			AddGossipItemFor(player, 0, "|cffFF0000| - Blackrock Depths", 1, 99921);
			AddGossipItemFor(player, 0, "|cffFF0000| - Temple of Atal'Hakkar", 1, 99922);
			AddGossipItemFor(player, 0, "|cffFF0000| - Blackrock Spire", 1, 99924);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 31);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(DUNGEONS, item->GetGUID());
			break;

		case  99900:
			CloseGossipMenuFor(player);
			player->TeleportTo(389, 0.876914f, -23.6f, -20.458948f, 4.39f);
			break;
		case  99901:
			CloseGossipMenuFor(player);
			player->TeleportTo(36, -16.4f, -383.070f, 61.779999f, 2.526370f);
			break;

		case  99902:
			CloseGossipMenuFor(player);
			player->TeleportTo(43, -158.440994f, 131.600998f, -74.255203f, 5.846850f);
			break;

		case  99903:
			CloseGossipMenuFor(player);
			player->TeleportTo(33, -228.190994f, 2111.409912f, 76.890404f, 1.381951f);
			break;

		case  99904:
			CloseGossipMenuFor(player);
			player->TeleportTo(48, -150.233994f, 106.594002f, -39.77899f, 4.450590f);
			break;


		case  99906:
			CloseGossipMenuFor(player);
			player->TeleportTo(90, -340, 6.68f, -152.850f, 2.625f);
			break;

		case  99907:
			CloseGossipMenuFor(player);
			player->TeleportTo(47, 1943.0f, 1544.630f, 81.996666f, 1.38f);
			break;

		case  99908:
			CloseGossipMenuFor(player);
			player->TeleportTo(349, 752.909973f, -616.5300f, -33.110f, 1.370f);
			break;
		case  99912:
			CloseGossipMenuFor(player);
			player->TeleportTo(0, 2880.742f, -808.762f, 160.332993f, 5.323298f);
			break;
		case  99913:
			CloseGossipMenuFor(player);
			player->TeleportTo(70, -221.603928f, 63.427294f, -46.038364f, 3.117877f);
			break;
		case  99914:
			CloseGossipMenuFor(player);
			player->TeleportTo(429, -201.110f, -328.660f, -2.723914f, 5.22f);
			break;
		case  99915:
			CloseGossipMenuFor(player);
			player->TeleportTo(289, 200.4320f, 92.276497f, 128.442001f, 1.497f);
			break;

		case  99917:
			CloseGossipMenuFor(player);
			player->TeleportTo(129, 2593.680f, 1111.22998f, 50.951801f, 4.712390f);
			break;
		case  99919:
			CloseGossipMenuFor(player);
			player->TeleportTo(329, 3393.175293f, -3380.268066f, 142.717148f, 0.053661f);
			break;
		case  99920:
			CloseGossipMenuFor(player);
			player->TeleportTo(209, 1213.520f, 841.590f, 9.298f, 7.090f);
			break;
		case  99921:
			CloseGossipMenuFor(player);
			player->TeleportTo(230, 458.320f, 26.520f, -70.673f, 4.950f);
			break;
		case  99922:
			CloseGossipMenuFor(player);
			player->TeleportTo(109, -319.239990f, 99.90f, -131.850f, 3.711505f);
			break;
		case  99924:
			CloseGossipMenuFor(player);
			player->TeleportTo(0, -7524.189941f, -1230.130f, 285.7330f, 2.095f);
			break;

		case  312:
			AddGossipItemFor(player, 0, "|cffFF0000| - Hellfire Citadel", 1, 99925);
			AddGossipItemFor(player, 0, "|cffFF0000| - Coilfang Reservoir", 1, 99926);
			AddGossipItemFor(player, 0, "|cffFF0000| - Auchindoun", 1, 99927);
			AddGossipItemFor(player, 0, "|cffFF0000| - Caverns of Time", 1, 99928);
			AddGossipItemFor(player, 0, "|cffFF0000| - Tempest Keep", 1, 99929);
			AddGossipItemFor(player, 0, "|cffFF0000| - Magisters' Terrace", 1, 99930);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 31);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(DUNGEONS, item->GetGUID());
			break;
			//Teleports of outland instances.
		case  99925:
			player->TeleportTo(530, -335.308990f, 3126.635742f, -102.765564f, 2.194442f);
			break;
		case  99926:
			player->TeleportTo(530, 735.065979f, 6883.450195f, -66.291298f, 5.891720f);
			break;
		case  99927:
			player->TeleportTo(530, -3323.294922f, 4934.059570f, -100.308395f, 2.184640f);
			break;
		case  99928:
			player->TeleportTo(1, -8488.0f, -4212.0f, -215.0f, 3.978554f);
			break;
		case  99929:
			player->TeleportTo(530, 3098.744629f, 1507.355469f, 190.3f, 4.568055f);
			break;
		case  99930:
			player->TeleportTo(530, 12884.6f, -7317.7f, 65.502296f, 4.7990f);
			break;

		case  313://Northrend
			AddGossipItemFor(player, 0, "|cffFF0000| - Utgarde Keep", 1, 99931);
			AddGossipItemFor(player, 0, "|cffFF0000| - The Nexus", 1, 99932);
			AddGossipItemFor(player, 0, "|cffFF0000| - Ahn'kahet The Old Kingdom", 1, 99933);
			AddGossipItemFor(player, 0, "|cffFF0000| - Azjol-Nerub", 1, 99934);
			AddGossipItemFor(player, 0, "|cffFF0000| - Drak'Tharon Keep", 1, 99935);
			AddGossipItemFor(player, 0, "|cffFF0000| - The Violet Hold", 1, 99936);
			AddGossipItemFor(player, 0, "|cffFF0000| - Gundrak", 1, 99937);
			AddGossipItemFor(player, 0, "|cffFF0000| - Halls of Stone", 1, 99938);
			AddGossipItemFor(player, 0, "|cffFF0000| - Halls of Lightning", 1, 99939);
			AddGossipItemFor(player, 0, "|cffFF0000| - The Oculus", 1, 99940);
			AddGossipItemFor(player, 0, "|cffFF0000| - Culling of Stratholme", 1, 99941);
			AddGossipItemFor(player, 0, "|cffFF0000| - Utgarde Pinnacle", 1, 99942);
			AddGossipItemFor(player, 0, "|cffFF0000| - Trial of the Champion", 1, 99943);
			AddGossipItemFor(player, 0, "|cffFF0000| - Forge of Souls", 1, 99944);
			AddGossipItemFor(player, 0, "|cffFF0000| - Pit of Saron", 1, 99945);
			AddGossipItemFor(player, 0, "|cffFF0000| - Halls of Reflection", 1, 99946);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 31);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(DUNGEONS, item->GetGUID());
			break;
		case  99931:
			player->TeleportTo(574, 153.789f, -86.547997f, 12.551f, 0.304f);
			break;
		case  99932:
			player->TeleportTo(571, 3814.865967f, 6968.145508f, 99.356407f, 0.311923f);
			break;
		case  99933:
			player->TeleportTo(619, 342.075043f, -1104.613159f, 61.916801f, 0.572631f);
			break;
		case  99934:
			player->TeleportTo(601, 412.028564f, 786.141357f, 829.196777f, 5.978966f);
			break;
		case  99935:
			player->TeleportTo(600, -517.343018f, -487.976013f, 11.01f, 4.8310f);
			break;
		case  99936:
			player->TeleportTo(608, 1809.214355f, 803.929504f, 44.363998f, 6.282f);
			break;
		case  99937:
			player->TeleportTo(604, 1866.653198f, 628.712952f, 176.668945f, 3.097039f);
			break;
		case  99938:
			player->TeleportTo(599, 1153.239990f, 806.164001f, 195.936996f, 4.715f);
			break;
		case  99939:
			player->TeleportTo(602, 1331.469971f, 259.618988f, 53.397999f, 4.772000f);
			break;
		case  99940:
			player->TeleportTo(578, 1055.930054f, 986.849976f, 361.070007f, 5.745f);
			break;
		case  99941:
			player->TeleportTo(595, 1431.099976f, 556.919983f, 36.689999f, 5.1600f);
			break;
		case  99942:
			player->TeleportTo(574, 164.788025f, -83.097336f, 12.551000f, 0.304000f);
			break;
		case  99943:
			player->TeleportTo(650, 805.226990f, 618.038025f, 412.393005f, 3.1456f);
			break;
		case  99944:
			player->TeleportTo(632, 4922.859f, 2175.629883f, 638.734009f, 2.003550f);
			break;
		case  99945:
			player->TeleportTo(658, 437.883545f, 212.355774f, 528.708862f, 6.256460f);
			break;
		case  99946:
			player->TeleportTo(602, 1331.781738f, 254.395782f, 53.4f, 4.772f);
			break;


		case 32://Raids
			AddGossipItemFor(player, 0, "|cffFF0000| - Vanilla", GOSSIP_SENDER_MAIN, 321);
			AddGossipItemFor(player, 0, "|cffFF0000| - The Burning Crusade", GOSSIP_SENDER_MAIN, 322);
			AddGossipItemFor(player, 0, "|cffFF0000| - Wrath of the Lich King", GOSSIP_SENDER_MAIN, 323);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 30);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(RAIDS, item->GetGUID());
			break;
		case 321:
			AddGossipItemFor(player, 0, "|cffFF0000| - Zul'Gurub", 1, 99954);
			AddGossipItemFor(player, 0, "|cffFF0000| - Onyxia's Lair", 1, 99955);
			AddGossipItemFor(player, 0, "|cffFF0000| - Molten Core", 1, 99956);
			AddGossipItemFor(player, 0, "|cffFF0000| - Blackwing Lair", 1, 99957);
			AddGossipItemFor(player, 0, "|cffFF0000| - Ruins of Ahn'Qiraj", 1, 99958);
			AddGossipItemFor(player, 0, "|cffFF0000| - Temple of Ahn'Qiraj", 1, 99959);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 32);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(RAIDS, item->GetGUID());
			break;
		case 322:
			AddGossipItemFor(player, 0, "|cffFF0000| - Karazhan", 1, 99961);
			AddGossipItemFor(player, 0, "|cffFF0000| - Gruul's Lair", 1, 99962);
			AddGossipItemFor(player, 0, "|cffFF0000| - Magtheridon's Lair", 1, 99963);
			AddGossipItemFor(player, 0, "|cffFF0000| - Serpentshrine Cavern", 1, 99964);
			AddGossipItemFor(player, 0, "|cffFF0000| - Tempest Keep: The Eye", 1, 99965);
			AddGossipItemFor(player, 0, "|cffFF0000| - Battle for Mount Hyjal", 1, 99966);
			AddGossipItemFor(player, 0, "|cffFF0000| - Black Temple", 1, 99967);
			AddGossipItemFor(player, 0, "|cffFF0000| - Sunwell Plateau", 1, 99968);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 32);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(RAIDS, item->GetGUID());
			break;
		case 323:
			AddGossipItemFor(player, 0, "|cffFF0000| - Obsidian Sanctum", 1, 99969);
			AddGossipItemFor(player, 0, "|cffFF0000| - Vault of Archavon", 1, 99970);
			AddGossipItemFor(player, 0, "|cffFF0000| - Ulduar", 1, 99972);
			AddGossipItemFor(player, 0, "|cffFF0000| - Trial of the Crusader", 1, 99973);
			AddGossipItemFor(player, 0, "|cffFF0000| - Naxxramas", 1, 99960);
			AddGossipItemFor(player, 0, "|cffFF0000| - Ruby Sanctum", 1, 99975);
			AddGossipItemFor(player, 0, "|cffFF0000| - Icecrown Citadel", 1, 99976);
			AddGossipItemFor(player, 0, "|cff0000ff| - Return to Last Page", GOSSIP_SENDER_MAIN, 32);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(RAIDS, item->GetGUID());
			break;


		case 5: // Arenas
			AddGossipItemFor(player, 0, "Gurubashi Arena", GOSSIP_SENDER_MAIN, 51);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(ARENAS, item->GetGUID());
			break;

		case 51: // Gurubashi Arena #1
			player->TeleportTo(0, -13261.1f, 164.344f, 35.7824f, 1.09914f);
			break;

		case 210000: // Kalimdor
			AddGossipItemFor(player, 0, "|cffFF0000|Ashenvale|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 1);
			AddGossipItemFor(player, 0, "|cffFF0000|Azshara|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 2);
			AddGossipItemFor(player, 0, "|cffFF0000|Azuremyst Isle|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 3);
			AddGossipItemFor(player, 0, "|cffFF0000|Bloodmyst Isle|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 4);
			AddGossipItemFor(player, 0, "|cffFF0000|Darkshore|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 5);
			AddGossipItemFor(player, 0, "|cffFF0000|Felwood|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 6);
			AddGossipItemFor(player, 0, "|cffFF0000|Hyjal|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 7);
			AddGossipItemFor(player, 0, "|cffFF0000|Moonglade|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 8);
			AddGossipItemFor(player, 0, "|cffFF0000|Teldrassil|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 9);
			AddGossipItemFor(player, 0, "|cffFF0000|Winterspring|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 10);
			AddGossipItemFor(player, 0, "|cffFF0000|The Barrens|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 11);
			AddGossipItemFor(player, 0, "|cffFF0000|Desolace|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 12);
			AddGossipItemFor(player, 0, "|cffFF0000|Durotar|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 13);
			AddGossipItemFor(player, 0, "|cffFF0000|Dustwallow Marsh|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 14);
			AddGossipItemFor(player, 0, "|cffFF0000|Mulgore|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 15);
			AddGossipItemFor(player, 0, "|cffFF0000|Stonetalon Mountains|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 16);
			AddGossipItemFor(player, 0, "|cffFF0000|Feralas|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 17);
			AddGossipItemFor(player, 0, "|cffFF0000|Silithus|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 18);
			AddGossipItemFor(player, 0, "|cffFF0000|Tanaris|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 19);
			AddGossipItemFor(player, 0, "|cffFF0000|Thousand Needles|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 20);
			AddGossipItemFor(player, 0, "|cffFF0000|Uldum|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 21);
			AddGossipItemFor(player, 0, "|cffFF0000|Un'Goro Crater|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 22);
			AddGossipItemFor(player, 0, "|cff0000ff|Return to Last Page|r", GOSSIP_SENDER_MAIN, 2);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(KALIMDOR, item->GetGUID());
			break;
		case 220000: // Eastern Kingdoms
			AddGossipItemFor(player, 0, "|cffFF0000|Blasted Lands|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 23);
			AddGossipItemFor(player, 0, "|cffFF0000|Deadwind Pass|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 24);
			AddGossipItemFor(player, 0, "|cffFF0000|Duskwood|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 25);
			AddGossipItemFor(player, 0, "|cffFF0000|Elwynn Forest|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 26);
			AddGossipItemFor(player, 0, "|cffFF0000|Redridge Mountains|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 27);
			AddGossipItemFor(player, 0, "|cffFF0000|Stranglethorn Vale|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 28);
			AddGossipItemFor(player, 0, "|cffFF0000|Swamp of Sorrows|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 29);
			AddGossipItemFor(player, 0, "|cffFF0000|Westfall|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 30);
			AddGossipItemFor(player, 0, "|cffFF0000|Badlands|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 31);
			AddGossipItemFor(player, 0, "|cffFF0000|Blackrock Mountains|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 32);
			AddGossipItemFor(player, 0, "|cffFF0000|Burning Steppes|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 33);
			AddGossipItemFor(player, 0, "|cffFF0000|Dun Morogh|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 34);
			AddGossipItemFor(player, 0, "|cffFF0000|Loch Modan|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 35);
			AddGossipItemFor(player, 0, "|cffFF0000|Searing Gorge|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 36);
			AddGossipItemFor(player, 0, "|cffFF0000|Wetlands|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 37);
			AddGossipItemFor(player, 0, "|cffFF0000|Eversong Woods|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 38);
			AddGossipItemFor(player, 0, "|cffFF0000|Ghostlands|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 39);
			AddGossipItemFor(player, 0, "|cffFF0000|Isle of Quel'Danas|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 40);

			AddGossipItemFor(player, 0, "|cff0000ff|Return to Last Page|r", GOSSIP_SENDER_MAIN, 2);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(EASTERN_KINGDOMS, item->GetGUID());
			break;
		case 230000: // Outland

			AddGossipItemFor(player, 0, "|cffFF0000|Blade's Edge Mountains|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 41);
			AddGossipItemFor(player, 0, "|cffFF0000|Hellfire Peninsula|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 42);
			AddGossipItemFor(player, 0, "|cffFF0000|Nagrand|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 43);
			AddGossipItemFor(player, 0, "|cffFF0000|Netherstorm|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 44);
			AddGossipItemFor(player, 0, "|cffFF0000|Shadowmoon Valley|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 47);
			AddGossipItemFor(player, 0, "|cffFF0000|Terokkar Forest|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 45);
			AddGossipItemFor(player, 0, "|cffFF0000|Zangarmarsh|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 46);

			AddGossipItemFor(player, 0, "|cff0000ff|Return to Last Page|r", GOSSIP_SENDER_MAIN, 2);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(OUTLANDS, item->GetGUID());
			break;
		case 240000: // Northrend 48 - 56
			AddGossipItemFor(player, 0, "|cffFF0000|Borean Tundra|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 48);
			AddGossipItemFor(player, 0, "|cffFF0000|Dragonblight|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 49);
			AddGossipItemFor(player, 0, "|cffFF0000|Icecrown|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 50);
			AddGossipItemFor(player, 0, "|cffFF0000|Sholazar Basin|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 51);
			AddGossipItemFor(player, 0, "|cffFF0000|Crystalsong|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 52);
			AddGossipItemFor(player, 0, "|cffFF0000|Zul'Drak|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 53);
			AddGossipItemFor(player, 0, "|cffFF0000|Grizzly Hills|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 54);
			AddGossipItemFor(player, 0, "|cffFF0000|Howling Fjord|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 55);
			AddGossipItemFor(player, 0, "|cffFF0000|Storm Peaks|r", GOSSIP_SENDER_MAIN, LOC_OFFSET + 56);

			AddGossipItemFor(player, 0, "|cff0000ff|Return to Last Page|r", GOSSIP_SENDER_MAIN, 2);
			AddMainMenuGossipItem(player);
			player->PlayerTalkClass->SendGossipMenu(NORTHREND, item->GetGUID());
			break;

			/* VIP Stuff 300000 - 399999 */

			/* Disabled VIP Mall */

		case 80000000:
		{
			if (player->GetSession()->GetVIPLevel() < 1)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("VIP Mall is currently not available! Go to Task-Area instead!");
				player->TeleportTo(530, -248.765f, 957.539f, 84.3573f, 4.60909f);
				return;
			}
			ChatHandler(player->GetSession()).PSendSysMessage("We appreciate your Support to the server, but vip mall is currently disabled!" "\n" "Visit Task-Area instead!");
			player->TeleportTo(530, -248.765f, 957.539f, 84.3573f, 4.60909f);
		}
		break;

		/* Dual Wield - Any VIP */

		case 26:
		{
			if (player->GetSession()->GetVIPLevel() < 1)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need to be VIP 1+ To get Dual Wield");
				return;
			}
			ChatHandler(player->GetSession()).PSendSysMessage("You have learned Dual-Wield!");
			player->LearnSpell(674, false);
		}
		break;

		/* Titan's Grip 35 DP */

		case 27:
			if (player->GetSession()->GetVIPLevel() < 2)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need to be VIP 2+ To buy Titan's Grip");
				return;
			}
			if (player->GetCustomCurrency(DP) < 35)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough DP to make this purchase! Required : 35DP");
				return;
			}
			LoginDatabase.PQuery("UPDATE account SET dp=dp-35 WHERE id=%u", player->GetSession()->GetAccountId());
			ChatHandler(player->GetSession()).PSendSysMessage("You have learned Titan's Grip!");
			player->LearnSpell(46917, false);
			break;

			/* Full Talents 50DP */

		case 28:
			if (player->GetSession()->GetVIPLevel() < 2)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need to be VIP 2+ To buy Full Talents");
				return;
			}
			if (player->GetCustomCurrency(DP) < 25)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough DP to make this purchase! Required : 25DP");
				return;
			}
			LoginDatabase.PQuery("UPDATE account SET dp=dp-25 WHERE id=%u", player->GetSession()->GetAccountId());
			ChatHandler(player->GetSession()).PSendSysMessage("You have purchased Full-Talents!");
			LearnFullTalents(player);
			break;

		case 29:
		{
			if (player->GetSession()->GetVIPLevel() < 1)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need to be VIP 1+ To bind a Custom home!");
				return;
			}
			CloseGossipMenuFor(player);
			WorldLocation loc = player->GetWorldLocation();
			player->SetHomebind(loc, player->GetAreaId());
			ChatHandler(player->GetSession()).PSendSysMessage("You bound your Hearthstone here!");
		}
		break;

		/* Account Options 400000 - 499999 */

		/* Bank */

		case 6:
			player->GetSession()->SendShowBank(player->GetSession()->GetPlayer()->GetGUID());
			break;

			/* Change Name */

		case 23:
			if (!player->HasItemCount(200202, 1))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need a Name Change Token to Change Name!");
				return;
			}
			player->DestroyItemCount(200202, 1, true);
			player->SetAtLoginFlag(AT_LOGIN_RENAME);
			ChatHandler(player->GetSession()).PSendSysMessage("Success! Relog to take effect!");
			break;

			/* Change Race */

		case 24:
			if (!player->HasItemCount(200200, 1))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need a Race Change Token to Change Race!");
				return;
			}
			player->DestroyItemCount(200200, 1, true);
			player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
			ChatHandler(player->GetSession()).PSendSysMessage("Success! Relog to take effect!");
			break;

			/* Change Faction */

		case 25:
			if (!player->HasItemCount(200201, 1))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You need a Faction Change Token to Change Faction!");
				return;
			}
			player->DestroyItemCount(200201, 1, true);
			player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
			ChatHandler(player->GetSession()).PSendSysMessage("Success! Relog to take effect!");
			break;
		case 400:
		{
			uint32 auras[] = { 48162, 48074, 48170, 43223, 36880, 467, 48469 };
			if (player->InArena())
			{
				ChatHandler(player->GetSession()).PSendSysMessage("Don't try to cheat!");
				return;
			}
			if (player->GetAura(8362) && !player->IsAlive())
			{
				ChatHandler(player->GetSession()).PSendSysMessage("You Should Ressurrect First!");
			}
			player->RemoveAurasByType(SPELL_AURA_MOUNTED);
			for (int i = 0; i < 7; i++)
				player->AddAura(auras[i], player);
			ChatHandler(player->GetSession()).PSendSysMessage("You've been buffed!");
			return;
		}

		}
	}
};

void AddSC_item_teleporter()
{
	new item_teleporter();
}