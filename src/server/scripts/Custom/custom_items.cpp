#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "WorldSession.h"
#include "Map.h"
#include "Log.h"

class item_aaron_summon : public ItemScript
{
public:
    item_aaron_summon() : ItemScript("item_aaron_summon") { }

    std::unordered_map<uint64, uint32> lastUsedTime;

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        uint64 guid = player->GetGUID();
        TC_LOG_INFO("player.hooks", "Item used by player GUID: %llu", guid);

        // Implement item-specific logic here
        // Example: Check if the player can use the item
        if (player->GetLevel() < 10)
        {
            player->GetSession()->SendNotification("You must be level 10 to use this item.");
            return false;
        }

        // If the item can be used, do something (e.g., summon a creature)
        uint32 creatureId = 500612; // Replace with your custom creature ID
        float x, y, z;
        player->GetPosition(x, y, z);

        Position pos(x + 2, y + 2, z, player->GetOrientation());
        Creature* summon = player->GetMap()->SummonCreature(creatureId, pos, nullptr, 2 * MINUTE * IN_MILLISECONDS, player);

        if (summon)
        {
            player->Say("Aaron has been summoned!", LANG_UNIVERSAL);
        }

        return true;  // Item use was successful
    }
};

void AddSC_item_aaron_summon()
{
    new item_aaron_summon();
}