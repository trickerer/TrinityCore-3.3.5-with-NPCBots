/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GameTime.h"
#include "ScriptMgr.h"
#include "Util.h"
#include "World.h"
#include "Item.h"

namespace
{
    bool IsXPBoostActive()
    {
        time_t time = GameTime::GetGameTime();
        tm localTm = TimeBreakdown(time);
        uint32 weekdayMaskBoosted = sWorld->getIntConfig(CONFIG_XP_BOOST_DAYMASK);
        uint32 weekdayMask = (1 << localTm.tm_wday);
        bool currentDayBoosted = (weekdayMask & weekdayMaskBoosted) != 0;
        return currentDayBoosted;
    }
}

class xp_boost_PlayerScript : public PlayerScript
{
public:
    xp_boost_PlayerScript() : PlayerScript("xp_boost_PlayerScript") { }

    void OnGiveXP(Player* player, uint32& amount, Unit* /*unit*/) override
    {
        if (IsXPBoostActive())
        {
            
            if ((player->HasItemCount(461144, 1)) && (!player->HasItemCount(461141, 1)) && (!player->HasItemCount(461142, 1)))
            {
                amount *= 2;
            }
            else if ((player->HasItemCount(461141, 1)) && (!player->HasItemCount(461142, 1)) && (!player->HasItemCount(461144, 1)))  
            {
                amount *= 3;
            }
            else if ((player->HasItemCount(461142, 1)) && (!player->HasItemCount(461144, 1)) && (!player->HasItemCount(461141, 1))) 
            {
                amount *= 10;
            }
            else
            {
                amount *= sWorld->getRate(RATE_XP_BOOST);
            }
        }
    }
};

class xp_boost_ItemRestrict : public PlayerScript
{
public:
    xp_boost_ItemRestrict() : PlayerScript("xp_boost_ItemRestrict") { }

    bool CanEquipItem(Player* player, uint8 /*slot*/, uint16 /*entry*/, Item* newItem, bool /*swap*/, Item* /*oldItem*/) override
    {
        return CanCarryOnlyOneXPItem(player, newItem->GetEntry());
    }

    bool CanTakeItem(Player* player, Item* item) //override
    {
        return CanCarryOnlyOneXPItem(player, item->GetEntry());
    }

    void OnLogin(Player* player) //override
    {
        RemoveExtraXPItems(player);
    }

private:
    std::set<uint32> xpItemIds = { 461144, 461141, 461142 };

    bool CanCarryOnlyOneXPItem(Player* player, uint32 newItemId)
    {
        if (xpItemIds.find(newItemId) == xpItemIds.end())
            return true; // not an XP item, allow

        for (uint32 id : xpItemIds)
        {
            if (id == newItemId)
                continue;

            if (player->HasItemCount(id, 1, true))
            {
                // Player already has a different XP item
                if (Player* p = player)
                    ChatHandler(p->GetSession()).SendSysMessage("You can only carry one XP booster item at a time.");
                return false;
            }
        }

        return true;
    }

    void RemoveExtraXPItems(Player* player)
    {
        std::vector<uint32> found;

        for (uint32 id : xpItemIds)
        {
            if (player->HasItemCount(id, 1, true))
                found.push_back(id);
        }

        if (found.size() > 1)
        {
            // Keep the first, remove the rest
            for (size_t i = 1; i < found.size(); ++i)
                player->DestroyItemCount(found[i], player->GetItemCount(found[i], true), true);

            ChatHandler(player->GetSession()).SendSysMessage("Only one XP booster item is allowed. Extras were removed.");
        }
    }
};

void AddSC_xp_boost()
{
    new xp_boost_PlayerScript();
    new xp_boost_ItemRestrict();
}
