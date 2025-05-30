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

            // Skip invalid or cross-faction taxi nodes
            if ((player->GetTeam() == ALLIANCE && !(node->Flags & TAXI_NODE_FLAG_ALLIANCE)) ||
                (player->GetTeam() == HORDE && !(node->Flags & TAXI_NODE_FLAG_HORDE)))
                continue;

            if (!player->m_taxi.IsTaximaskNodeKnown(i))
            {
                player->m_taxi.SetTaximaskNode(i);
                ++count;
            }
        }

        player->GetSession()->SendAreaTriggerMessage("You have learned %u flight paths.", count);
        ChatHandler(player->GetSession()).PSendSysMessage("Learned %u flight paths.", count);
        return true;
    }
};

void AddSC_item_learn_flightpaths()
{
    new item_learn_flightpaths();
}
