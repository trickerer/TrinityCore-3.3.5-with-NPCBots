#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "ScriptPCH.h"
#include "WorldSession.h"
#include <cstring>
#include <Cache/CharacterCache.h>
#include <DataStores/DBCStores.cpp>

// Gossip Texts
#define GOSSIP_ITEM_LEVEL_UP_60 "Make me level 60! UwU"
#define GOSSIP_ITEM_LEVEL_UP_70 "Make me level 70! UwU"
#define GOSSIP_ITEM_LEVEL_UP_80 "Make me level 80! UwU"

// MenuOptionId 1-1 with above
enum LEVELUP_NPC_MENU_ACTIONS
{
    GOSSIP_ITEM_LEVEL_UP_60_OPTION = 1,
    GOSSIP_ITEM_LEVEL_UP_70_OPTION = 2,
    GOSSIP_ITEM_LEVEL_UP_80_OPTION = 3
};

enum LEVELUP_PROFESSION_STEPS {
    MAX_LEVEL_AT_60 = 300,
    MAX_LEVEL_AT_70 = 375,
    MAX_LEVEL_AT_80 = 450
};

class LevelUpNPCGossip : public CreatureScript
{
public:
    LevelUpNPCGossip() : CreatureScript("npc_levelup") {}

    struct LevelUpNPCGossipAI : public ScriptedAI
    {
        LevelUpNPCGossipAI(Creature* creature) : ScriptedAI(creature) { }

        bool OnGossipHello(Player* player) override
        {
            uint8 pLevel = player->GetLevel();

            if (pLevel < 60)
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_LEVEL_UP_60, GOSSIP_ACTION_INFO_DEF, GOSSIP_ITEM_LEVEL_UP_60_OPTION);
            if (pLevel < 70)
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_LEVEL_UP_70, GOSSIP_ACTION_INFO_DEF, GOSSIP_ITEM_LEVEL_UP_70_OPTION);
            if (pLevel < 80)
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_LEVEL_UP_80, GOSSIP_ACTION_INFO_DEF, GOSSIP_ITEM_LEVEL_UP_80_OPTION);

            SendGossipMenuFor(player, 12498, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
            switch (action)
            {
            case GOSSIP_ITEM_LEVEL_UP_60_OPTION: { LevelUp(player, 60); } break;
            case GOSSIP_ITEM_LEVEL_UP_70_OPTION: { LevelUp(player, 70); } break;
            case GOSSIP_ITEM_LEVEL_UP_80_OPTION: { LevelUp(player, 80); } break;
            default: {
                CloseGossipMenuFor(player);
                return true;
            }
            }
            CloseGossipMenuFor(player);
            player->UpdateWeaponsSkillsToMaxSkillsForLevel();
            player->UpdateSkillsForLevel();
            ObjectAccessor::SaveAllPlayers();//Save
            return true;
        }
    };

    static bool LevelUp(Player* player, int16 newlevel)
    {
        if (newlevel < 1)
            newlevel = 1;

        if (newlevel > static_cast<int16>(STRONG_MAX_LEVEL))
            newlevel = static_cast<int16>(STRONG_MAX_LEVEL);
        {
            player->GiveLevel(static_cast<uint8>(newlevel));
            player->InitTalentForLevel();
            player->SetXP(0);
            LevelUpProfessions(player, newlevel);
        }

        return true;
    }

    static bool LevelUpProfessions(Player* player, uint16 newPlayerLevel)
    {
        for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(i);
            if (!skillInfo)
                continue;

            if ((skillInfo->CategoryID == SKILL_CATEGORY_PROFESSION || skillInfo->CategoryID == SKILL_CATEGORY_SECONDARY)
                && player->GetSkillValue(skillInfo->ID) > 0)
            {
                uint16 maxForLevel = GetMaxProfessionLevel(newPlayerLevel);
                player->SetSkill(skillInfo->ID, player->GetSkillStep(skillInfo->ID), maxForLevel, maxForLevel);
                uint32 classmask = player->GetClassMask();
            }
        }
        return true;
    }

    static uint16 GetMaxProfessionLevel(uint16 level)
    {
        switch (level)
        {
        case 60: return MAX_LEVEL_AT_60;
        case 70: return MAX_LEVEL_AT_70;
        case 80: return MAX_LEVEL_AT_80;
        }

        return 0;
    }


    CreatureAI* GetAI(Creature* creature) const override
    {
        return new LevelUpNPCGossipAI(creature);
    }
};

void AddSC_LevelUpNPC() {
    new LevelUpNPCGossip();
};
