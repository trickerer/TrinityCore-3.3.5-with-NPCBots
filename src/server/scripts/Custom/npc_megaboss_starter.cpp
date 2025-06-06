#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Chat.h"
#include "ChannelMgr.h"
#include "Channel.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "World.h"
#include <sstream>
#include <string>

using namespace std::chrono;

enum NPCs
{
   NPC_BOSS_5MAN					= 500935,
   NPC_BOSS_10MAN					= 500928,
   NPC_BOSS_25MAN					= 500930,
};

#define _QUERY1_ "UPDATE `mga_event_data` SET `active` = '1' WHERE `id` ='1'"
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


class npc_megaboss_starter : public CreatureScript
{
public:
    npc_megaboss_starter() : CreatureScript("npc_megaboss_starter") { }

    struct npc_megaboss_starterAI : public ScriptedAI
    {
        npc_megaboss_starterAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		

        bool OnGossipHello(Player* player) override
        {
            WorldSession* session = player->GetSession();
            QueryResult result;
            result = WorldDatabase.PQuery("SELECT * FROM `mga_event_data` WHERE `id` = '1' AND `active` = '0'");
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Start 5 Man Mode", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Start 10 Man Mode", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Start 20 Man Mode", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "YOU HAVE STARTED!!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
            }
            

            AddGossipItemFor(player, GOSSIP_ICON_DOT, "Send Me Home!", GOSSIP_SENDER_MAIN, 8000);
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
                WorldDatabase.PExecute(_QUERY1_);
                me->SummonCreature(NPC_BOSS_5MAN, -9676.397461, -6.144296, -20.832001, 2.609818, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, milliseconds(60000));
                if (ChannelMgr* channelMgr = ChannelMgr::forTeam(TEAM_NEUTRAL))
                {
                    if (Channel* channel = channelMgr->GetChannel(0, "world", player, false, nullptr))
                    {
                        channel->Say(ObjectGuid::Empty, "MGA Mega Boss event has started! Prepare yourselves!", LANG_UNIVERSAL);
                    }
                }
                return true;
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 1001)
            {
                CloseGossipMenuFor(player);
                WorldDatabase.PExecute(_QUERY1_);
                me->SummonCreature(NPC_BOSS_10MAN, -9676.397461, -6.144296, -20.832001, 2.609818, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, milliseconds(60000));
                ChannelMgr* cMgr = ChannelMgr::forTeam(TEAM_NEUTRAL);
                Player* player = nullptr; // no player context needed, but if you want one, pass a GM or dummy player
                Channel* channel = cMgr->GetChannel(0, "world", player, false, nullptr);
                if (channel)
                    channel->Say(player->GetGUID(), "MGA Mega Boss event has started! Prepare yourselves!", LANG_UNIVERSAL);
                return true;
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 1002)
            {
                CloseGossipMenuFor(player);
                WorldDatabase.PExecute(_QUERY1_);
                me->SummonCreature(NPC_BOSS_25MAN, -9676.397461, -6.144296, -20.832001, 2.609818, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, milliseconds(60000));
                ChannelMgr* cMgr = ChannelMgr::forTeam(TEAM_NEUTRAL);
                Player* player = nullptr; // no player context needed, but if you want one, pass a GM or dummy player
                Channel* channel = cMgr->GetChannel(0, "world", player, false, nullptr);
                if (channel)
                    channel->Say(player->GetGUID(), "MGA Mega Boss event has started! Prepare yourselves!", LANG_UNIVERSAL);
                return true;
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 8000)
            {
                CloseGossipMenuFor(player);
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, 0.0f);
				//player->CastSpell(player, 8690, true); //https://www.wowhead.com/wotlk/spell=8690/hearthstone
				player->SetPvP(false);
				return true;
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 9000)
            {
                CloseGossipMenuFor(player);
                me->Say("Farewell "+player->GetName(), LANG_UNIVERSAL);
                me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                return true;
            }
            CloseGossipMenuFor(player);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_megaboss_starterAI(creature);
    }
};

void AddSC_npc_megaboss_starter()
{
    new npc_megaboss_starter();
}
