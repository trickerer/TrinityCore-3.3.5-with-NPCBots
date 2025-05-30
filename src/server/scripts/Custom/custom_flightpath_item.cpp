#include "ScriptMgr.h"
#include "Player.h"
#include "DBCStores.h"
#include "TaxiPathGraph.h"

class item_flight_master_whistle : public ItemScript
{
public:
    item_flight_master_whistle() : ItemScript("item_flight_master_whistle") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        uint32 count = 0;

        for (uint32 i = 0; i < sTaxiPathNodesByPath.size(); ++i)
        {
            const TaxiPathNodeList& path = sTaxiPathNodesByPath[i];
            if (!path.empty())
            {
                for (const TaxiPathNode& node : path)
                {
                    if (!player->HasTaxiPath(node.mapid, node.id))
                    {
                        player->SetTaxiPath(node.mapid, node.id);
                        count++;
                    }
                }
            }
        }

        player->GetSession()->SendAreaTriggerMessage("You have learned all flight paths!");
        ChatHandler(player->GetSession()).PSendSysMessage("Learned %u flight paths.", count);
        return true;
    }
};

void AddSC_item_flight_master_whistle()
{
    new item_flight_master_whistle();
}
