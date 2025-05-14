#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "WorldSession.h"
#include "Map.h"
#include "Log.h"

class item_aaron_summon : public ItemScript
{
public:
    item_aaron_summon() : ItemScript("item_aaron_summon") 
    {
        TC_LOG_INFO("player.hooks", "Loaded item_aaron_summon script.");
    }

    bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
    {
        TC_LOG_INFO("player.hooks", "item_aaron_summon OnUse triggered!");
        player->Say("Item worked!", LANG_UNIVERSAL);
        return true;
    }
};

void AddSC_item_aaron_summon()
{
    new item_aaron_summon();
}