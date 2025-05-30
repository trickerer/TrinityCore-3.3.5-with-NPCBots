#include "ScriptMgr.h"
#include "Player.h"
#include "DBCStores.h"
#include "WorldSession.h"
#include "Chat.h"

class item_learn_flightpaths : public ItemScript
{
public:
    item_learn_flightpaths() : ItemScript("item_learn_flightpaths") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        uint32 count = 0;

        // Loop through all taxi nodes
        for (uint32 i = 0; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
                continue;

            // Faction filtering using MountCreatureID
            // Alliance typically has MountCreatureID[0], Horde has MountCreatureID[1]
            if (player->GetTeam() == ALLIANCE && node->MountCreatureID[0] == 0)
                continue;
            if (player->GetTeam() == HORDE && node->MountCreatureID[1] == 0)
                continue;

            if (!player->m_taxi.IsTaximaskNodeKnown(i))
            {
                player->m_taxi.SetTaximaskNode(i);
                ++count;
            }
        }
        //player->Yell("DID IT RUN!", LANG_UNIVERSAL);
        player->DestroyItemCount(item->GetEntry(), 1, true);
        player->GetSession()->SendAreaTriggerMessage("You have learned %u flight paths.", count);
        ChatHandler(player->GetSession()).PSendSysMessage("Learned %u flight paths.", count);
        return true;
    }
};

void AddSC_item_learn_flightpaths()
{
    new item_learn_flightpaths();
}
