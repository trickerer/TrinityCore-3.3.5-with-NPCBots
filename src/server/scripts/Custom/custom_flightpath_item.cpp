#include "ScriptMgr.h"
#include "Player.h"
#include "DBCStores.h"
#include "WorldSession.h"
#include "World.h"
#include "Chat.h"
#include "DatabaseEnv.h"

#define CHAR_UPD_TAXI_MASK "UPDATE characters SET taximask = ? WHERE guid = ?"

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
            
            uint64 taxiMask = uint64(player->m_taxi.m_taximask[0]) | (uint64(player->m_taxi.m_taximask[1]) << 32);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_TAXI_MASK);
            stmt->setUInt64(0, taxiMask);
            stmt->setUInt64(1, player->GetGUID().GetRawValue());
            CharacterDatabase.Execute(stmt);
    
            //player->SaveToDB();

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
