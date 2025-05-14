#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "WorldSession.h"
#include "Log.h"
#include "SpellMgr.h"

class item_aaron_summon : public ItemScript
{
public:
    item_aaron_summon() : ItemScript("item_aaron_summon") { }

    std::unordered_map<uint64, uint32> lastUsedTime;

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
    {
        uint64 guid = player->GetGUID();
        TC_LOG_INFO("player.hooks", "Item used by player GUID: %llu", guid);

        // Check if the player is level 10 or higher before allowing item use
        if (player->GetLevel() < 10)
        {
            player->GetSession()->SendNotification("You must be level 10 to use this item.");
            return false;
        }

        // Implement cooldown logic to prevent abuse (30 minutes cooldown)
        uint32 now = time(nullptr);
        if (lastUsedTime.count(guid) && now - lastUsedTime[guid] < 1800) // 1800 seconds = 30 minutes
        {
            uint32 remaining = 1800 - (now - lastUsedTime[guid]);
            player->GetSession()->SendNotification("You must wait %u more seconds to use this item again.", remaining);
            return false;
        }

        lastUsedTime[guid] = now; // Update last used time

        // If the item can be used, proceed to summon a creature
        uint32 creatureId = 500612; // Replace with your custom creature ID
        float x, y, z;
        player->GetPosition(x, y, z); // Get player's current position

        // Define position offset for summoned creature (2 yards away from the player)
        Position pos(x + 2, y + 2, z, player->GetOrientation());
        
        // Summon the creature
        Creature* summon = player->GetMap()->SummonCreature(
            creatureId,              // Creature ID
            pos,                     // Position to summon at
            nullptr,                 // No custom summon properties
            2 * MINUTE * IN_MILLISECONDS, // Creature will despawn in 2 minutes
            player,                  // Player who summoned it
            0,                       // No spell ID for summoning
            0,                       // No vehicle ID
            ObjectGuid::Empty        // Default ObjectGuid
        );

        if (summon)
        {
            // If the creature is successfully summoned, announce it
            player->Say("Aaron has been summoned!", LANG_UNIVERSAL);
        }

        return true;  // Item use was successful
    }
};

// Function to add the script to the server
void AddSC_item_aaron_summon()
{
    new item_aaron_summon(); // Register the item script
}