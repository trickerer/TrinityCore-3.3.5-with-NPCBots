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
            QueryResult result;
            if(player->GetLevel() >= 77)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Stormwind Vault 5 Man Dungeon COMING SOON!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Forgotten Scarlet Monastery 5 Man Dungeon COMING SOON!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Karazhan Crypts 5 Man Dungeon COMING SOON!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Stormwind Vault 5 Man Dungeon You Need Level 77+", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Forgotten Scarlet Monastery 5 Man Dungeon You Need Level 77+", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Karazhan Crypts 5 Man Dungeon You Need Level 77+", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
            }
            
            if(player->GetLevel() >= 80)
            {
                result = WorldDatabase.PQuery("SELECT * FROM `mga_event_data` WHERE `id` = '1' AND `active` = '0'");
                if (result)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "MGAWoW Mega Boss", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
                }
                else
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "MGAWoW Mega Boss IS ACTIVE!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
                }
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "MGAWoW Mega Boss You Need Level 80", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
            }

            AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
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
            
            if (action == GOSSIP_ACTION_INFO_DEF + 1001)
            {
                CloseGossipMenuFor(player);
                player->TeleportTo(44, 117.097931f, 11.423770f, 18.677391f, 4.618045f);
                player->SetPvP(false);
                return false;
            }
            
            if (action == GOSSIP_ACTION_INFO_DEF + 1002)
            {
                CloseGossipMenuFor(player);
                player->TeleportTo(0, -11086.000000f, -1802.150024f, 52.739799f, 1.664262f);
                player->SetPvP(false);
                return false;
            }
            
            if (action == GOSSIP_ACTION_INFO_DEF + 2000)
            {
                CloseGossipMenuFor(player);
                player->TeleportTo(1, -9954.21f, 128.26f, 0.38f, 1.664262f);
                player->SetPvP(false);
                return false;
            }
            
            if (action == GOSSIP_ACTION_INFO_DEF + 2001)
            {
                me->Say(player->GetName() + " The MEGA Boss Fight Has Started, try later!", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 9000)
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
