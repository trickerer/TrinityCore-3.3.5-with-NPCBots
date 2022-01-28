#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "ScriptPCH.h"
#include <cstring>

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

                if(pLevel < 60)
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
                    case GOSSIP_ITEM_LEVEL_UP_60_OPTION: { player->SetLevel(60); } break;
                    case GOSSIP_ITEM_LEVEL_UP_70_OPTION: { player->SetLevel(70); } break;
                    case GOSSIP_ITEM_LEVEL_UP_80_OPTION: { player->SetLevel(80); } break;
                }
                CloseGossipMenuFor(player);
                return true;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new LevelUpNPCGossipAI(creature);
        }
};

void AddSC_LevelUpNPC() {
    new LevelUpNPCGossip();
};
