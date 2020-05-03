#include "ScriptMgr.h"
#include "Player.h"
#include "Group.h"
#include "Creature.h"
#include "Log.h"
#include "Chat.h"
#include "ObjectMgr.h"

class mg_system : public PlayerScript
{
public:
    mg_system() : PlayerScript("mg_system") {}

    void OnCreatureKill(Player* killerPlr, Creature* killedCre)
    {
        uint32 rewardMG = sObjectMgr->GetCreatureTemplate(killedCre->GetEntry())->rewardMG;
        if (rewardMG <= 0)
            return;

        std::ostringstream ss;
        
        if (Group* group = killerPlr->GetGroup())
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                if (Player* player = itr->GetSource())
                    if (player->IsAlive() && player->IsAtGroupRewardDistance(killedCre))
                    {
                        rewardMG /= group->GetMembersCount();
                        TC_LOG_INFO("server.player", "Player: %s kill creature: %s and recieved %u mg (was in a group)", player->GetName().c_str(), killedCre->GetName().c_str(), rewardMG);
                        player->AddCustomCurrency(MG, rewardMG, false);
                        ss << "By killing [" << killedCre->GetName().c_str() << "] you earned +" << rewardMG << " MG!";
                        ChatHandler(killerPlr->GetSession()).PSendSysMessage(ss.str().c_str());
                        return;
                    }

        if (killerPlr->IsAlive() && killerPlr->IsAtGroupRewardDistance(killedCre))
        {
            TC_LOG_INFO("server.player", "Player: %s kill creature: %s and recieved %u mg (Was alone)", killerPlr->GetName().c_str(), killedCre->GetName().c_str(), rewardMG);
            killerPlr->AddCustomCurrency(MG, rewardMG, false);
            ss << "By killing [" << killedCre->GetName().c_str() << "] you earned +" << rewardMG << " MG!";
            ChatHandler(killerPlr->GetSession()).PSendSysMessage(ss.str().c_str());
        }

    }
};


void AddSC_mg_system()
{
    new mg_system;
}
