#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "Unit.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "World.h"
#include <sstream>
#include <string>

enum ItemIds
{
    itemId = 461141,  // 3x Rates
    itemId2 = 461142, // 10x Rates
    itemId3 = 21140, // MGA TOKEN
};

/*
5 ways to notify player
session->SendNotification("MESSAGE");
session->SendAreaTriggerMessage("MESSAGE");
WhisperTo(player, "MESSAGE");
me->Say("MESSAGE", LANG_UNIVERSAL);
me->Yell("MESSAGE", LANG_UNIVERSAL);

EMOTE
me->HandleEmoteCommand(EMOTE_STATE_TALK);
*/


class npc_dungeon_master : public CreatureScript
{
public:
    npc_dungeon_master() : CreatureScript("npc_dungeon_master") { }

    struct npc_dungeon_masterAI : public ScriptedAI
    {
        npc_dungeon_masterAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		

        bool OnGossipHello(Player* player) override
        {
            WorldSession* session = player->GetSession();
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Stormwind Vault 5 Man Dungeon COMING SOON!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
            
            

            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            WorldSession* session = player->GetSession();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
           
            if (action == GOSSIP_ACTION_INFO_DEF + 1000)
            {
                CloseGossipMenuFor(player);
                player->TeleportTo(0, -8633.600586f, 594.303101f, 95.690689f, 1.487484f);
                player->SetPvP(false);
                return false;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2000)
            {
                me->Say(player->GetName() + " Bye.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }
            CloseGossipMenuFor(player);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_dungeon_masterAI(creature);
    }
};

void AddSC_npc_dungeon_master()
{
    new npc_dungeon_master();
}
