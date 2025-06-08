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

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        uint32 count = 0;

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

            //if (player->m_taxi.IsTaximaskNodeKnown(node->ID))
            //    continue;

            player->m_taxi.SetTaximaskNode(node->ID);
            ++count;
        }

        if (count > 0)
        {
            player->DestroyItemCount(461146, 1, true);
            
            WorldPacket data(SMSG_NEW_TAXI_PATH, 8 + 4 * TAXI_MASK_SIZE);
            data << player->GetPackGUID();
            for (uint8 i = 0; i < TAXI_MASK_SIZE; ++i)
                data << uint32(player->GetTaxiMask()[i]);
            player->SendDirectMessage(&data);
            
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
