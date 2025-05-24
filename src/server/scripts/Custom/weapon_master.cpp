#include "ScriptMgr.h"
#include "Creature.h"
#include "Player.h"
#include "Chat.h"
#include "GossipDef.h"
#include "ScriptPCH.h"

class npc_weapon_trainer : public CreatureScript
{
public:
    npc_weapon_trainer() : CreatureScript("npc_weapon_trainer") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, "Train me in all weapon skills I can use.", GOSSIP_SENDER_MAIN, 1);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        if (sender == GOSSIP_SENDER_MAIN && action == 1)
        {
            static const uint32 weaponSkills[] =
            {
                SKILL_AXES,
                SKILL_BOWS,
                SKILL_CROSSBOWS,
                SKILL_DAGGERS,
                SKILL_FIST_WEAPONS,
                SKILL_GUNS,
                SKILL_MACES,
                SKILL_POLEARMS,
                SKILL_STAVES,
                SKILL_SWORDS,
                SKILL_THROWN,
                SKILL_WANDS,
                SKILL_UNARMED,
                SKILL_TWO_HANDED_SWORDS,
                SKILL_TWO_HANDED_MACES,
                SKILL_TWO_HANDED_AXES
            };

            uint32 trainedCount = 0;

            for (uint32 skillId : weaponSkills)
            {
                if (player->CanUseSkill(skillId) && !player->HasSkill(skillId))
                {
                    player->LearnSkill(skillId, 1, player->GetMaxSkillValueForLevel(player->getLevel(), skillId));
                    trainedCount++;
                }
            }

            ChatHandler(player->GetSession()).PSendSysMessage("You have been trained in %u weapon skill(s).", trainedCount);
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        return false;
    }
};

void AddSC_npc_weapon_trainer()
{
    new npc_weapon_trainer();
}
