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

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        TC_LOG_INFO("player.hooks", "AARON ITEM USED BY: %s", player->GetName().c_str());
        player->Say("IT WORKED!!", LANG_UNIVERSAL);
        return true;
    }
};

void AddSC_item_aaron_summon()
{
    new item_aaron_summon();
}