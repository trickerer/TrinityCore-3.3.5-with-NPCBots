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

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
    {
        uint64 guid = player->GetGUID();
        uint32 now = time(nullptr);
        
        TC_LOG_INFO("player.hooks", "ITEM CLICKED");
        // 1800 = 30 minutes
        if (lastUsedTime.count(guid) && now - lastUsedTime[guid] < 1800)
        {
            uint32 remaining = 1800 - (now - lastUsedTime[guid]);
            player->GetSession()->SendNotification("You must wait %u more seconds to use this item again.", remaining);
            return false;
        }

        lastUsedTime[guid] = now;

        uint32 creatureId = 500612; // Replace with your custom creature
        float x, y, z;
        player->GetPosition(x, y, z);

        // Position object
        Position pos(x + 2, y + 2, z, player->GetOrientation());

        Creature* summon = player->GetMap()->SummonCreature(
            creatureId,           // Creature ID
            pos,                  // Position object
            nullptr,              // No SummonPropertiesEntry
            2 * MINUTE * IN_MILLISECONDS,  // Timed despawn duration in milliseconds
            player,               // Player as the summoner (WorldObject pointer)
            0,                    // No spell ID
            0,                    // No vehicle ID
            ObjectGuid::Empty     // Default empty ObjectGuid for privateObjectOwner
        );
        
        if (summon)
            player->Say("Aaron has been summoned!", LANG_UNIVERSAL);

        return true;
    }
};

void AddSC_item_aaron_summon()
{
    new item_aaron_summon();
}
