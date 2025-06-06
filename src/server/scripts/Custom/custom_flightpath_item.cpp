#include "ScriptMgr.h"
#include "Player.h"
#include "DBCStores.h"
#include "WorldSession.h"
#include "World.h"
#include "Chat.h"
#include "TaxiNodes.h"
#include "TaxiPathGraph.h"

class item_learn_flightpaths : public ItemScript
{
public:
    item_learn_flightpaths() : ItemScript("item_learn_flightpaths") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        uint32 count = 0;

        // Iterate over TaxiNodesStore by ID list instead of row index
        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
                continue;

            // Faction filtering
            if (player->GetTeam() == ALLIANCE && node->MountCreatureID[0] == 0)
                continue;
            if (player->GetTeam() == HORDE && node->MountCreatureID[1] == 0)
                continue;

            // Already known?
            if (player->m_taxi.IsTaximaskNodeKnown(node->ID))
                continue;

            // Learn node
            player->m_taxi.SetTaximaskNode(node->ID);
            player->SendDiscoverNewTaxiNode(node->ID); // Notify client
            ++count;
        }

        if (count > 0)
        {
            player->DestroyItemCount(item->GetEntry(), 1, true);
            player->GetSession()->SendAreaTriggerMessage("You have learned %u flight paths.", count);
            ChatHandler(player->GetSession()).PSendSysMessage("Learned %u flight paths.", count);
            player->SaveToDB();
        }
        else
        {
            player->GetSession()->SendAreaTriggerMessage("You already know all available flight paths.");
        }

        return true;
    }
};

void AddSC_item_learn_flightpaths()
{
    new item_learn_flightpaths();
}
