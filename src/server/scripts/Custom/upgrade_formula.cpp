#include "ScriptMgr.h"
#include "ScriptPCH.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ObjectMgr.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "GossipDef.h"
#include "WorldSession.h"
#include "ItemTemplate.h"
#include "Chat.h"
#include "Item.h"
#include "Log.h"

struct UpgradeData
{
    uint32 entry;
    uint32 upgraded;
    uint32 requiredItem[6];
    uint32 requiredItemCount[6];
    float chance;
    uint64 reqMG;
    uint64 reqGold;
};

enum UpgradeGemIds
{
    GEM_25 = 999992,
    GEM_50 = 999993,
    GEM_75 = 999994,
    GEM_100 = 999995,
};

static std::unordered_map<uint32, UpgradeData> Upgrades;

static std::string GetItemLink(uint32 entry, WorldSession* player)
{
    const ItemTemplate* temp = sObjectMgr->GetItemTemplate(entry);
    LocaleConstant loc_idx = player->GetSessionDbLocaleIndex();
    std::string name = temp->Name1;
    if (ItemLocale const* il = sObjectMgr->GetItemLocale(entry))
        ObjectMgr::GetLocaleString(il->Name, loc_idx, name);
    std::ostringstream oss;
    oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
        "|Hitem:" << entry << ":0:0:0:0:0:0:0:0:0|h[" << name << "]|h|r";

    return oss.str();
}

static uint32 getDisplayId(uint32 ent)
{
    uint32 id = 0;
    if (const ItemTemplate* proto = sObjectMgr->GetItemTemplate(ent))
        id = proto->DisplayInfoID;
    return id;
}

class item_upgrader_loader : public WorldScript
{
public:
    item_upgrader_loader() : WorldScript("item_upgrader_loader") {}
    void OnStartup()
    {
        TC_LOG_INFO("server.loading", "Loading Item Upgrades");
        uint32 count = 0;
        Upgrades.clear();  //                              0		1      2	3	4	   5	6	7	8	9	  10	11	12	  13	14	  15		
        QueryResult result = WorldDatabase.Query("SELECT entry, upgradedE, ri1, ri2, ri3, ri4, ri5, ri6, qi1, qi2, qi3, qi4, qi5, qi6, chance, ureqmg, ureqgold FROM item_template WHERE upgradedE > 0");
        if (result)
            do
            {
                Field* data = result->Fetch();
                UpgradeData& upgrade = Upgrades[data[0].GetUInt32()];
                upgrade.entry = data[0].GetUInt32();
                upgrade.upgraded = data[1].GetUInt32();

                for (int i = 0; i < 6; i++)
                {

                    upgrade.requiredItem[i] = data[2 + i].GetUInt32();

                    upgrade.requiredItemCount[i] = data[8 + i].GetUInt32();
                }

                upgrade.chance = data[14].GetFloat();
                upgrade.reqMG = data[15].GetUInt64();
                upgrade.reqGold = data[16].GetUInt64();
                ++count;
            } while (result->NextRow());

            TC_LOG_INFO("server.loading", ">> %u Item Upgrades loaded.", count);

    }
    void OnConfigLoad(bool reload)
    {
        if (reload)
            OnStartup();
    }
};
class item_upgrader : public ItemScript
{
public:
    item_upgrader() : ItemScript("item_upgrader") { }

    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();


        std::ostringstream details;

        uint32 entry = item->GetEntry();
        uint32 upgradedEntry = Upgrades[entry].upgraded;
        float chance = Upgrades[entry].chance;
        if (chance != 100)
        {
            if (Item* gemz = player->GetItemByPos(INVENTORY_SLOT_BAG_0, 23))
            {
                switch (gemz->GetEntry())
                {
                case GEM_25:
                    chance  = 25;
                    break;
                case GEM_50:
                    chance = 50;
                    break;
                case GEM_75:
                    chance = 75;
                    break;
                case GEM_100:
                    chance = 100;
                    break;
                default:
                    ChatHandler(player->GetSession()).PSendSysMessage("No chance gem was found!");
                    break;
                }
            }
        }


        for (uint8 i = 0; i < 6; i++)
        {
            if (sObjectMgr->GetItemTemplate(Upgrades[entry].requiredItem[i]))
            {
                details << "|cffFF0000+|r  |cff00A40ARequire|r |cffFF0000: " << GetItemLink(Upgrades[entry].requiredItem[i], player->GetSession());
                if (uint32 count = Upgrades[entry].requiredItemCount[i])
                    if (count > 1)
                        details << "|cffFF0000 x" << count;
                details << "\n";
            }
        }
        details << "\n\n";
        if (uint32 mg = Upgrades[entry].reqMG)
            details << "|cffFF0000+|r |cff00A40ARequire|r |cffFF0000: " << mg << " |cff00A40AMg\n";
        if (uint32 gold = Upgrades[entry].reqGold)
            details << "|cffFF0000+|r |cff00A40ARequire|r |cffFF0000: " << gold << " |cff00A40AGold\n";
        details << "|cffFF0000+|r |cff00A40AProbablity|r |cffFF0000: " << chance << "% \n\n";

        std::string questTitle = "Upgrade Formula";

        std::string questDetails = details.str().c_str();
        std::string questObjectives = "If you have Upgrade Gem.please put it in your first slot of your backpag.\n|cffEA3AFF[Tips] Click |cff00A40A* Accept *|r |cffEA3AFFwill begin to upgrade!";
        std::string questAreaDescription = "";

        WorldPacket data(SMSG_QUESTGIVER_QUEST_DETAILS, 100);   // guess size
        data << uint64(item->GetGUID());
        data << uint64(0);
        data << uint32(420);//Quest->GetID
        data << questTitle;
        data << questDetails;
        data << questObjectives;
        data << uint8(0);                  // CGQuestInfo::m_autoLaunched
        data << uint32(0); // 3.3.3 questFlags
        data << uint32(0);
        data << uint8(0);


        data << uint32(1);


        data << uint32(upgradedEntry);
        data << uint32(1);
        data << uint32(getDisplayId(upgradedEntry));

        data << uint32(0);
        data << uint32(0);
        data << uint32(0);


        // rewarded honor points. Multiply with 10 to satisfy client
        data << uint32(0);
        data << float(0.0f);                                    // unk, honor multiplier?
        data << uint32(0);
        data << int32(0);                // cast spell
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);                                      // unk

        for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
            data << uint32(0);

        for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
            data << int32(0);

        for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
            data << int32(0);

        data << uint32(QUEST_EMOTE_COUNT);
        for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
        {
            data << uint32(0);
            data << uint32(0);       // DetailsEmoteDelay (in ms)
        }
        player->GetSession()->SendPacket(&data);
    }

    bool OnQuestAccept(Player* player, Item* item, Quest const* quest)
    {
        uint32 entry = item->GetEntry();//item entry
        float chance = Upgrades[entry].chance;
        if (chance > 100)
            chance = 100;
        int gem = 0;
        uint32 chanceMorris = 0;
        if(chance != 100)
            if (Item* gemz = player->GetItemByPos(INVENTORY_SLOT_BAG_0, 23))
            {
                switch (gemz->GetEntry())
                {
                case GEM_25:
                case GEM_50:
                case GEM_75:
                case GEM_100:
                    gem = gemz->GetEntry();
                    break;
                }
            }
        switch (gem)
        {
        case GEM_25:
            chanceMorris = 4;
            break;
        case GEM_50:
            chanceMorris = 3;
            break;
        case GEM_75:
            chanceMorris = 2;
            break;
        case GEM_100:
            chanceMorris = 1;
            break;
        }
        if ((player->HasItemCount(Upgrades[entry].entry, 1)) && ((Upgrades[entry].requiredItem[0] == 0 || Upgrades[entry].requiredItemCount[0] == 0) || (player->HasItemCount(Upgrades[entry].requiredItem[0], Upgrades[entry].requiredItemCount[0]))) && ((Upgrades[entry].requiredItem[1] == 0 || Upgrades[entry].requiredItemCount[1] == 0) || (player->HasItemCount(Upgrades[entry].requiredItem[1], Upgrades[entry].requiredItemCount[1]))) && ((Upgrades[entry].requiredItem[2] == 0 || Upgrades[entry].requiredItemCount[2] == 0) || (player->HasItemCount(Upgrades[entry].requiredItem[2], Upgrades[entry].requiredItemCount[2]))) && ((Upgrades[entry].requiredItem[3] == 0 || Upgrades[entry].requiredItemCount[3] == 0) || (player->HasItemCount(Upgrades[entry].requiredItem[3], Upgrades[entry].requiredItemCount[3]))) && ((Upgrades[entry].requiredItem[4] == 0 || Upgrades[entry].requiredItemCount[4] == 0) || (player->HasItemCount(Upgrades[entry].requiredItem[4], Upgrades[entry].requiredItemCount[4]))) && ((Upgrades[entry].requiredItem[5] == 0 || Upgrades[entry].requiredItemCount[5] == 0) || (player->HasItemCount(Upgrades[entry].requiredItem[5], Upgrades[entry].requiredItemCount[5]))))
        {
            if (player->GetCustomCurrency(MG) >= Upgrades[entry].reqMG)
            {
                if (gem)
                {
                    player->DestroyItemCount(gem, 1, true);
                    std::ostringstream ss;
                    ss << "Used Chance Gem: " << GetItemLink(gem, player->GetSession());
                    ChatHandler(player->GetSession()).PSendSysMessage(ss.str().c_str());
                }
                player->RemoveCustomCurrency(MG, Upgrades[entry].reqMG, true);
                for (int i = 0; i < 6; i++)
                    player->DestroyItemCount(Upgrades[entry].requiredItem[i], Upgrades[entry].requiredItemCount[i], true);
                if (gem) //if there is upgrade gem, use chancemorris...
                {
					if (chanceMorris)
						if (urand(1, chanceMorris))
                             
                    {

                        player->DestroyItemCount(Upgrades[entry].entry, 1, true);
                        if (player->AddItem(Upgrades[entry].upgraded, 1))

                            ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00Success!");
                        else
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00Success!");
                            player->SendItemRetrievalMail(Upgrades[entry].upgraded, 1);
                            ChatHandler(player->GetSession()).PSendSysMessage("Item has been sent to your mailbox.");
                        }
                    }
                    else
                    {
                        ChatHandler(player->GetSession()).PSendSysMessage("|cFFDF0101Failed!");
                    }
                }
				if (!gem)//if theres no upgrade gem, go with normal chance
					if (urand(chance, 100))
                      {

                        player->DestroyItemCount(Upgrades[entry].entry, 1, true);
                        if (player->AddItem(Upgrades[entry].upgraded, 1))

                            ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00Success!");
                        else
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00Success!");
                            player->SendItemRetrievalMail(Upgrades[entry].upgraded, 1);
                            ChatHandler(player->GetSession()).PSendSysMessage("Item has been sent to your mailbox.");
                        }
                    }
                    else
                    {
                        ChatHandler(player->GetSession()).PSendSysMessage("|cFFDF0101Failed!");
                    }
            }
            else

                ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000Not Enough MG!|r");
        }
        else
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000You don't have the required materials!|r");
        return true;
    }
};

void AddSC_itemscripts()
{
    new item_upgrader();
    new item_upgrader_loader();
}