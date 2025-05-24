#include "ScriptMgr.h"
#include "Creature.h"
#include "Player.h"
#include "Chat.h"
#include "GossipDef.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SharedDefines.h"
#include "ScriptPCH.h"

class npc_weapon_master : public CreatureScript
{
public:
    npc_weapon_master() : CreatureScript("npc_weapon_master") {}

    struct npc_weapon_masterAI : public ScriptedAI
    {
        npc_weapon_masterAI(Creature* creature) : ScriptedAI(creature) {}

        bool CanUseWeaponSkill(Player* player, uint32 skillId)
        {
            switch (player->GetClass())
            {
                case CLASS_WARRIOR:
                    return (skillId == SKILL_SWORDS || skillId == SKILL_AXES || skillId == SKILL_MACES || skillId == SKILL_POLEARMS || skillId == SKILL_STAVES || skillId == SKILL_CROSSBOWS || skillId == SKILL_GUNS || skillId == SKILL_THROWN || skillId == SKILL_UNARMED);
                case CLASS_PALADIN:
                    return (skillId == SKILL_SWORDS || skillId == SKILL_MACES || skillId == SKILL_POLEARMS || skillId == SKILL_STAVES || skillId == SKILL_UNARMED || skillId == SKILL_POLEARMS);
                case CLASS_HUNTER:
                    return (skillId == SKILL_BOWS || skillId == SKILL_CROSSBOWS || skillId == SKILL_GUNS || skillId == SKILL_THROWN || skillId == SKILL_SWORDS || skillId == SKILL_AXES || skillId == SKILL_MACES || skillId == SKILL_UNARMED);
                case CLASS_ROGUE:
                    return (skillId == SKILL_SWORDS || skillId == SKILL_DAGGERS || skillId == SKILL_AXES || skillId == SKILL_FIST_WEAPONS || skillId == SKILL_THROWN || skillId == SKILL_UNARMED);
                case CLASS_DEATH_KNIGHT:
                    return (skillId == SKILL_SWORDS || skillId == SKILL_AXES || skillId == SKILL_MACES || skillId == SKILL_UNARMED || skillId == SKILL_POLEARMS);
                case CLASS_SHAMAN:
                    return (skillId == SKILL_AXES || skillId == SKILL_MACES || skillId == SKILL_STAVES || skillId == SKILL_UNARMED);
                case CLASS_MAGE:
                    return (skillId == SKILL_STAVES || skillId == SKILL_WANDS || skillId == SKILL_UNARMED || skillId == SKILL_SWORDS);
                case CLASS_PRIEST:
                    return (skillId == SKILL_STAVES || skillId == SKILL_DAGGERS || skillId == SKILL_WANDS || skillId == SKILL_MACES || skillId == SKILL_UNARMED);
                case CLASS_WARLOCK:
                    return (skillId == SKILL_STAVES || skillId == SKILL_DAGGERS || skillId == SKILL_WANDS || skillId == SKILL_UNARMED);
                case CLASS_DRUID:
                    return (skillId == SKILL_FIST_WEAPONS || skillId == SKILL_STAVES || skillId == SKILL_UNARMED);

                default:
                    return false;
            }
        }

        bool OnGossipHello(Player* player) override
        {
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Train me in all weapon skills I can use.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                static const uint32 skills[] =
                {
                    SKILL_SWORDS,
                    SKILL_AXES,
                    SKILL_MACES,
                    SKILL_POLEARMS,
                    SKILL_STAVES,
                    SKILL_DAGGERS,
                    SKILL_FIST_WEAPONS,
                    SKILL_BOWS,
                    SKILL_GUNS,
                    SKILL_CROSSBOWS,
                    SKILL_THROWN,
                    SKILL_WANDS,
                    SKILL_UNARMED
                };

                uint32 maxSkill = player->GetMaxSkillValueForLevel(player);

                for (uint32 skillId : skills)
                {
                    if (CanUseWeaponSkill(player, skillId))
                        player->SetSkill(skillId, 1, maxSkill, maxSkill);
                }

                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2)
            {
                CloseGossipMenuFor(player);
                return true;
            }

            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_weapon_masterAI(creature);
    }
};

void AddSC_npc_weapon_master()
{
    new npc_weapon_master();
}
