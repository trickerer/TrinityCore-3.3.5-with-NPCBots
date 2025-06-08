#include "ScriptMgr.h"
#include "Player.h"
#include "DBCStores.h"
#include "WorldSession.h"
#include "World.h"
#include "Chat.h"
#include "DatabaseEnv.h"

class item_learn_flightpaths : public ItemScript
{
public:
    item_learn_flightpaths() : ItemScript("item_learn_flightpaths") {}

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const&)
    {
        uint32 count = 0;

        // Build taxi mask manually from known nodes
        uint64 taxiMask = 0;

        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
                continue;

            // Check faction
            if (player->GetTeam() == ALLIANCE && node->MountCreatureID[0] == 0)
                continue;
            if (player->GetTeam() == HORDE && node->MountCreatureID[1] == 0)
                continue;

            // Skip if player already knows node
            if (player->m_taxi.IsTaximaskNodeKnown(node->ID))
                continue;

            // Mark as known and notify client
            if (player->m_taxi.SetTaximaskNode(node->ID))
            {
                player->GetSession()->SendDiscoverNewTaxiNode(node->ID);
                ++count;
            }
        }

        // Now build the taxiMask from known nodes again (bitmask)
        // We must do this because we can't access private m_taximask directly

        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            if (player->m_taxi.IsTaximaskNodeKnown(i))
            {
                if (i < 199)
                    taxiMask |= (uint64(1) << i);
                else
                {
                    // If you have more than 64 nodes, you need a second mask, or handle differently
                    // For now, only first 64 taxi nodes handled
                }
            }
        }

        if (count > 0)
        {
            player->DestroyItemCount(461146, 1, true);
            player->SetTaxiCheater(true);

            std::string query = StringFormat("UPDATE characters SET taximask = '%llu' WHERE guid = '%u'", taxiMask, player->GetGUID().GetCounter());
            CharacterDatabase.Execute(query.c_str());

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
