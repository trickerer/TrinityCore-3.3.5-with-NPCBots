#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "WorldSession.h"
#include "Log.h"
#include "SpellMgr.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectAccessor.h"
#include "GameObject.h"
#include "ObjectGuid.h"


// AARON
class item_aaron_summon : public ItemScript
{
public:
    item_aaron_summon() : ItemScript("item_aaron_summon") { }

    std::unordered_map<uint64, uint32> lastUsedTime;

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
    {
        uint64 guid = player->GetGUID();

        // Implement cooldown logic to prevent abuse (30 minutes cooldown)
        uint32 now = time(nullptr);
        if (lastUsedTime.count(guid) && now - lastUsedTime[guid] < 1800) // 1800 seconds = 30 minutes
        {
            uint32 remaining = 1800 - (now - lastUsedTime[guid]);
            uint32 remainingMinutes = remaining / 60;
            uint32 remainingSeconds = remaining % 60;
            
            player->GetSession()->SendNotification("You must wait %u minute(s) and %u second(s) to use this item again.", remainingMinutes, remainingSeconds);
            return false;  // Do not continue if item is on cooldown
        }

        lastUsedTime[guid] = now; // Update last used time

        // If the item can be used, proceed to summon a creature (Aaron)
        uint32 creatureId = 500612; // Replace with your custom creature ID (Aaron)
        float x, y, z;
        player->GetPosition(x, y, z); // Get player's current position

        // Calculate position to spawn the creature directly in front of the player (facing direction)
        float orientation = player->GetOrientation() + M_PI; // Reverse the orientation (to be in front)
        float distance = -2.0f; // 2 yards in front of the player

        // Position calculation
        float spawnX = x + distance * std::cos(orientation);
        float spawnY = y + distance * std::sin(orientation);
        Position pos(spawnX, spawnY, z, orientation);

        // Summon the creature
        Creature* summon = player->GetMap()->SummonCreature(
            creatureId,              // Creature ID
            pos,                     // Position to spawn at
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
            player->Yell("Aaron has been summoned! You have 2 minutes!", LANG_UNIVERSAL);
        }

        return true;  // Item use was successful
    }
};

// MAILBOX
class item_temp_mailbox : public ItemScript
{
public:
    item_temp_mailbox() : ItemScript("item_temp_mailbox") { }
    
    std::unordered_map<uint64, uint32> lastUsedTime2;

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
    {
        uint64 guid = player->GetGUID();
        // Implement cooldown logic to prevent abuse (30 minutes cooldown)
        uint32 now = time(nullptr);
        if (lastUsedTime2.count(guid) && now - lastUsedTime2[guid] < 1800) // 1800 seconds = 30 minutes
        {
            uint32 remaining = 1800 - (now - lastUsedTime2[guid]);
            uint32 remainingMinutes = remaining / 60;
            uint32 remainingSeconds = remaining % 60;
            
            player->GetSession()->SendNotification("You must wait %u minute(s) and %u second(s) to use this item again.", remainingMinutes, remainingSeconds);
            return false;  // Do not continue if item is on cooldown
        }
        
        lastUsedTime2[guid] = now; // Update last used time
        
        float x, y, z;
        player->GetPosition(x, y, z);

        float orientation = player->GetOrientation();
        float distance = 2.0f;

        float spawnX = x + distance * std::cos(orientation);
        float spawnY = y + distance * std::sin(orientation);

        uint32 mailboxId = 144113;

        QuaternionData rotation; // default zero rotation

        Seconds respawnTime(5 * MINUTE); // 5 minutes lifetime

        // Summon the mailbox using correct parameters
        GameObject* go = player->SummonGameObject(mailboxId, spawnX, spawnY, z + 0.5f, orientation, rotation, respawnTime);

        if (go)
        {
            player->Yell("Mailbox summoned!", LANG_UNIVERSAL);
        }
        else
        {
            player->Yell("Failed to summon mailbox!", LANG_UNIVERSAL);
        }

        return true;
    }
};

// Register scripts
void AddSC_item_aaron_summon()
{
    new item_aaron_summon(); // Register the Aaron summon item script
}

void AddSC_item_temp_mailbox()
{
    new item_temp_mailbox(); // Register the temporary mailbox item script
}
