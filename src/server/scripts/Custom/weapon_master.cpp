#include "ScriptMgr.h"
#include "Creature.h"
#include "Player.h"
#include "Chat.h"
#include "GossipDef.h"
#include "ScriptPCH.h"

class npc_weapon_master : public CreatureScript
{
public:
    npc_weapon_master() : CreatureScript("npc_weapon_master") {}

    struct npc_weapon_masterAI : public ScriptedAI
    {
        npc_weapon_masterAI(Creature* creature) : ScriptedAI(creature) {}

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
                    SKILL_TWO_HANDED_SWORDS,
                    SKILL_TWO_HANDED_MACES,
                    SKILL_TWO_HANDED_AXES
                };

                for (uint32 skillId : skills)
                {
                    if (!player->HasSkill(skillId))
                        player->LearnSkill(skillId, 1, player->GetMaxSkillValueForLevel(player->GetLevel()));
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
