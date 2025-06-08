#include "ScriptMgr.h"
#include "Player.h"
#include "DBCStores.h"
#include "WorldSession.h"
#include "World.h"
#include "Chat.h"

class item_learn_flightpaths : public ItemScript
{
public:
    item_learn_flightpaths() : ItemScript("item_learn_flightpaths") {}

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const&)
    {
        uint32 count = 0;

        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
                continue;

            // Check faction: MountCreatureID[0] = Alliance, [1] = Horde
            if (player->GetTeam() == ALLIANCE && node->MountCreatureID[0] == 0)
                continue;
            if (player->GetTeam() == HORDE && node->MountCreatureID[1] == 0)
                continue;

            // Skip if player already knows this node
            if (player->m_taxi.IsTaximaskNodeKnown(node->ID))
                continue;

            // Mark as known in server mask & update client
            if (player->m_taxi.SetTaximaskNode(node->ID))
            {
                player->GetSession()->SendDiscoverNewTaxiNode(node->ID);
                ++count;
            }
        }

        if (count > 0)
        {
            // Destroy 1 of your item (change ID if needed)
            player->DestroyItemCount(461146, 1, true);

            player->SetTaxiCheater(true);
            player->m_taxi.SaveTaxiNodes();
            player->SaveToDB();

            player->GetSession()->SendAreaTriggerMessage("You have learned %u flight paths.", count);
            ChatHandler(player->GetSession()).PSendSysMessage("Learned %u flight paths.", count);
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
