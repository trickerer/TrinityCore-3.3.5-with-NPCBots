#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "Unit.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include <sstream>
#include <string>

#define nemhtext11   800013 // tell player to log out and back in
#define nemhtext21   800014 // tell player they not entilied
#define nemhtext31   800012 // no mga token

#define GOSSIP_HELLO_NEMH3  "Rename Me"
#define GOSSIP_HELLO_NEMH4  "You are not in the rename system please visit the vote shop the web site to enable rename for this character. (near bottom of list ont he shop)"
#define GOSSIP_HELLO_NEMH5  "Goodbye."

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


class npc_renamer : public CreatureScript
{
public:
    npc_renamer() : CreatureScript("npc_renamer") { }

    struct npc_renamerAI : public ScriptedAI
    {
        npc_renamerAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		
		bool UpdateReNameCharData(Player* player, int16 status)
        {
            uint32 guidLow = player->GetGUID().GetCounter();
			std::string guidStr = std::to_string(guidLow);
			WorldDatabase.PExecute("UPDATE `char_rename` SET `status`='{}' WHERE `charid`='{}' AND `status` = '0'", status, guidStr.c_str());
            return true;
        }

        bool OnGossipHello(Player* player) override
        {
            //InitGossipMenuFor(player, NPC_GOSSIP_MENU);
			
            WorldSession* session = player->GetSession();
			
			uint32 guidLow = player->GetGUID().GetCounter();
			std::string guidStr = std::to_string(guidLow);
			//WhisperTo(player, guidStr.c_str());
			
			QueryResult result;
            result = WorldDatabase.PQuery("SELECT * FROM `char_rename` WHERE `charid`='{}' AND `status`='0' LIMIT 1", guidStr.c_str());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_NEMH3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_NEMH4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
				player->PlayDirectSound(11965);
            }
			AddGossipItemFor(player, GOSSIP_ICON_TALK, GOSSIP_HELLO_NEMH5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1003);
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
				UpdateReNameCharData(player, 1);
				player->SetAtLoginFlag(AT_LOGIN_RENAME);
				me->Say(player->GetName()+" Logout and back in you will be prompted to change your name.", LANG_UNIVERSAL);
				session->SendNotification("Logout and back in you will be prompted to change your name.");
				return true;
				
               /* if (player->HasItemCount(21140, 1))
                {
                    UpdateReNameCharData(player, 1);
                    player->DestroyItemCount(21140, 1, true);
                    player->SetAtLoginFlag(AT_LOGIN_RENAME);
                    //WhisperTo(player, "Logout and back in!");
                    me->Say(player->GetName()+" Logout and back in you will be prompted to change your name.", LANG_UNIVERSAL);
					session->SendNotification("Logout and back in you will be prompted to change your name.");
                    return true;
                }
                else
                {
                    // no mga token
                    me->Say("You need 1 MGA token!", LANG_UNIVERSAL);
                    //WhisperTo(player, "You need 1 MGA Token!");
                    session->SendNotification("You need 1 MGA Token!");
                    return true;
                }
				*/
			}
			if (action == GOSSIP_ACTION_INFO_DEF + 1002)
			{
                //me->HandleEmoteCommand(EMOTE_STATE_TALK);
                me->Say("To rename your char, goto players area on website, my info & click rename icon for this char", LANG_UNIVERSAL);
                //WhisperTo(player, "To rename your char, goto players area on website, my info & click rename icon for this char");
				CloseGossipMenuFor(player);
				return true;
			}
            if (action == GOSSIP_ACTION_INFO_DEF + 1003)
            {
                me->Say("Farewell "+player->GetName(), LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
				me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                return true;
            }
            
            CloseGossipMenuFor(player);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_renamerAI(creature);
    }
};

void AddSC_npc_renamer()
{
    new npc_renamer();
}
