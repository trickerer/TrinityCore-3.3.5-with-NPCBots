#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "WorldSession.h"
#include "Map.h"

class item_roboticon_summon : public ItemScript
{
public:
    item_roboticon_summon() : ItemScript("item_roboticon_summon") { }

    std::unordered_map<uint64, uint32> lastUsedTime;

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
    {
        uint64 guid = player->GetGUID();
        uint32 now = time(nullptr);

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

        Creature* summon = player->GetMap()->SummonCreature(
        creatureId,
        x + 2, y + 2, z,
        player->GetOrientation(),
        TEMPSUMMON_TIMED_DESPAWN,
        2 * MINUTE * IN_MILLISECONDS,
        player
        );

        if (summon)
            player->Say("Aaron has been summoned!", LANG_UNIVERSAL);

        return true;
    }
};

void AddSC_item_roboticon_summon()
{
    new item_roboticon_summon();
}
