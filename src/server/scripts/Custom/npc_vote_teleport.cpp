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

#define nemhtext11   -1700239 // tell player to log out and back in
#define nemhtext21   -1700240 // tell player they not entilied
#define nemhtext31   -1700241 // no mga token

#define GOSSIP_HELLO_TP1  "Location 1"
#define GOSSIP_HELLO_TP2  "Location 2"
#define GOSSIP_HELLO_TP3  "Location 3"
#define GOSSIP_HELLO_TP4  "Location 4"
#define GOSSIP_HELLO_TP5  "Location 5"
#define GOSSIP_HELLO_TP6  "Location 6"
#define GOSSIP_HELLO_TP7  "Location 7"
#define GOSSIP_HELLO_TP8  "Location 8"
#define GOSSIP_HELLO_TP9  "Location 9"
#define GOSSIP_HELLO_TP10  "Location 10"

//HORDE
#define GOSSIP_HELLO_HTP1  "Orgrimmar"
#define GOSSIP_HELLO_HTP2  "Undercity"
#define GOSSIP_HELLO_HTP3  "Thunder Bluff"
#define GOSSIP_HELLO_HTP4  "Silvermoon City"
// START LOCS
#define GOSSIP_HELLO_HSTP1  "Orc Starting Zone"
#define GOSSIP_HELLO_HSTP2  "Undead Starting Zone"
#define GOSSIP_HELLO_HSTP3  "Tauren Starting Zone"
#define GOSSIP_HELLO_HSTP4  "Blood Elf Starting Zone"
#define GOSSIP_HELLO_HSTP5  "Goblin Starting Zone"
#define GOSSIP_HELLO_HSTP6  "Worgen Elf Starting Zone"
#define GOSSIP_HELLO_HSTP7  "Troll Starting Zone"

// ALLY
#define GOSSIP_HELLO_ATP1  "Stormwind"
#define GOSSIP_HELLO_ATP2  "Ironforge"
#define GOSSIP_HELLO_ATP3  "Darnassus"
#define GOSSIP_HELLO_ATP4  "Exodar"

//GM
#define GOSSIP_HELLO_GM1  "-----------"

// START LOCS
#define GOSSIP_HELLO_ASTP1  "Human Starting Zone"
#define GOSSIP_HELLO_ASTP2  "Gnome Starting Zone"
#define GOSSIP_HELLO_ASTP3  "Night Elf Starting Zone"
#define GOSSIP_HELLO_ASTP4  "Draenei Starting Zone"
#define GOSSIP_HELLO_ASTP5  "Dwarf Starting Zone"

#define GOSSIP_HELLO_L1  "Shattrath City"
#define GOSSIP_HELLO_L2  "City of Dalaran"
#define GOSSIP_HELLO_L3  "Stairway to Heaven"
#define GOSSIP_HELLO_L4  "Send Me Home!"
#define GOSSIP_HELLO_L5  "Gadgetzan"
#define GOSSIP_HELLO_L6  "Bootybay"

#define GOSSIP_HELLO_TPNO  "You have not voted in the last 12 hours, if you wish to use me please go and vote"
#define GOSSIP_HELLO_TPN01  "You used me within the last 5 mins please wait a little longer."

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


class npc_vote_teleport : public CreatureScript
{
public:
    npc_vote_teleport() : CreatureScript("npc_vote_teleport") { }

    struct npc_vote_teleportAI : public ScriptedAI
    {
        npc_vote_teleportAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		
        bool UpdateVoteTPData(Player* player, uint32 newwtime)
        {
            WorldDatabase.PExecute("UPDATE `vote_tp` SET `tptime`='{}' WHERE `guid`='{}'", newwtime, player->GetSession()->GetAccountId());
            return true;
        }

        bool OnGossipHello(Player* player) override
        {
            me->Say("Hail " + player->GetName(), LANG_UNIVERSAL);
            me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            WorldSession* session = player->GetSession();
            QueryResult result;
            time_t tseconds;
            tseconds = time(NULL);
            uint32 ttcheck;
            uint32 fiveminabusecheck;
            uint32 lasttptime;
            
            ttcheck = (tseconds - 43200); // 12 hours

            if (player->IsGameMaster())
            { 
                result = WorldDatabase.PQuery("SELECT * FROM `vote_tp`  LIMIT 1");
            }
            else
            {
                result = WorldDatabase.PQuery("SELECT * FROM `vote_tp` WHERE `guid`='{}' AND `time` >'{}' LIMIT 1", player->GetSession()->GetAccountId(), ttcheck);
            }
            fiveminabusecheck = (tseconds - 300); // 5mins
            
            //result = WorldDatabase.PQuery("SELECT * FROM `vote_tp` WHERE `guid`='{}' AND `time` >'{}' LIMIT 1", player->GetSession()->GetAccountId(), ttcheck);
            //sLog->outError("SELECT * FROM `vote_tp` WHERE `guid`='{}' AND `time` >'{}' LIMIT 1", player->GetSession()->GetAccountId(),ttcheck);          

            if (result)
            {

                Field* fields = result->Fetch();

                lasttptime = fields[2].GetUInt32();
                if (lasttptime < 1)
                    lasttptime = (tseconds - 300); // current time - 5 mins was no entry in DB
				
				if (player->IsGameMaster())
				{
					fiveminabusecheck = tseconds;
					lasttptime = tseconds;
				}
				
				if (fiveminabusecheck < lasttptime)
				{
					AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_TPN01, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1050);
					player->PlayDirectSound(11965);
					AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye!", GOSSIP_SENDER_MAIN, 2);
					//sLog->outError("MGA: FAILED:: time from db = {} time we checking = {}", lasttptime,fiveminabusecheck);
				}
                else
                {
                    //sLog->outError("MGA: WORKED:: time from db = {} time we checking = {}", lasttptime,fiveminabusecheck);
                    if (player->GetTeamId() == TEAM_HORDE || player->IsGameMaster())
                    {
                        // HORDE LOCATIONS
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HTP1, GOSSIP_SENDER_MAIN, 1011);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HTP2, GOSSIP_SENDER_MAIN, 1012);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HTP3, GOSSIP_SENDER_MAIN, 1013);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HTP4, GOSSIP_SENDER_MAIN, 1014);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP1, GOSSIP_SENDER_MAIN, 1022); // orc starting zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP2, GOSSIP_SENDER_MAIN, 1023); // undead start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP3, GOSSIP_SENDER_MAIN, 1024); // tauren start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP4, GOSSIP_SENDER_MAIN, 1025); // bloodelf start zone
                        //AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP5, GOSSIP_SENDER_MAIN, 10125); // goblin start zone
                        //AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP6, GOSSIP_SENDER_MAIN, 10126); // worgen start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_HSTP7, GOSSIP_SENDER_MAIN, 1031); // troll starting zone
                    }

                    if (player->IsGameMaster())
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_GM1, GOSSIP_SENDER_MAIN, 9999);
                    }

                    if (player->GetTeamId() == TEAM_ALLIANCE || player->IsGameMaster())
                    {
                        // ALLY LOCATIONS
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ATP1, GOSSIP_SENDER_MAIN, 1016);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ATP2, GOSSIP_SENDER_MAIN, 1017);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ATP3, GOSSIP_SENDER_MAIN, 1018);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ATP4, GOSSIP_SENDER_MAIN, 1019);
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ASTP1, GOSSIP_SENDER_MAIN, 1026); // human start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ASTP2, GOSSIP_SENDER_MAIN, 1027); // gnome start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ASTP3, GOSSIP_SENDER_MAIN, 1028); // night elf start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ASTP4, GOSSIP_SENDER_MAIN, 1029); // draenei start zone
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_ASTP5, GOSSIP_SENDER_MAIN, 1030); // dwarf start zone
                    }
					
					if (player->GetLevel() > 40 || player->IsGameMaster())
					{
						// Gadg
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_L5, GOSSIP_SENDER_MAIN, 5001);
					}
					if (player->GetLevel() > 40 || player->IsGameMaster())
					{
						// bootybay
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_L6, GOSSIP_SENDER_MAIN, 5002);
					}

                    if (player->GetLevel() > 58 || player->IsGameMaster())
                    {
                        // SHAT CITY
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_L1, GOSSIP_SENDER_MAIN, 1020);
                    }
                    if (player->GetLevel() > 68 || player->IsGameMaster())
                    {
                        // DALARAN
                        AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_L2, GOSSIP_SENDER_MAIN, 1021);
                    }
                    if (player->GetQuestStatus(3861999) == QUEST_STATUS_INCOMPLETE || player->IsGameMaster())
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "To Stormwind Vault!", GOSSIP_SENDER_MAIN, 3000);
                    }
					AddGossipItemFor(player, GOSSIP_ICON_DOT, "Send Me Home!", GOSSIP_SENDER_MAIN, 1);
					AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye!", GOSSIP_SENDER_MAIN, 2);
                }

            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_TPNO, GOSSIP_SENDER_MAIN, 1050);
				player->PlayDirectSound(11965);
				AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye!", GOSSIP_SENDER_MAIN, 2);
            }
			me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
			WorldSession* session = player->GetSession();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
			CloseGossipMenuFor(player);
            time_t ttseconds;
            ttseconds = time(NULL);
            switch (action)
            {
			case 1:
            {
                CloseGossipMenuFor(player);
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, 0.0f);
				//player->CastSpell(player, 8690, true); //https://www.wowhead.com/wotlk/spell=8690/hearthstone
				player->SetPvP(false);
				return true;;
            }
			break;	
            case 2:
            {
                CloseGossipMenuFor(player);
                me->Say("Farewell "+player->GetName(), LANG_UNIVERSAL);
                me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            }
			break;
            case 1001:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1002:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1003:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1004:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1005:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1006:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1007:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1008:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1009:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;
            case 1010:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(578, 987.53f, 1065.11f, 359.98f, 0);    
                player->SetPvP(false);
                return false;
            }
            break;

            case 1011:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, 1632.38f, -4440.45f, 15.71f, 2.77f); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1012:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, 1630.80f, 240.08f, -43.10f, 3.09f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1013:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, -1210.98f, 40.00f, 132.13f, 2.43f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1014:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(530, 9532.57f, -7239.16f, 16.40f, 4.82f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1016:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, -8867.23f, 673.80f, 97.90f, 5.20f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1017:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, -4850.82f, -868.45f, 501.91f, 4.94f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1018:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, 9951.97f, 2280.06f, 1341.39f, 1.60f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1019:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(530, -3962.08f, -11657.74f, -138.81f, -138.81f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1020:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(530, -1838.16f, 5301.79f, -12.42f, 0);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1021:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(571, 5804.14f, 624.77f, 647.76f, 0); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1022:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, -603.57f, -4198.80f, 41.09f, 4.74f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1023:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, 1663.47f, 1662.78f, 141.85f, 6.10f); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1024:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, -2883.02f, -229.16f, 55.16f, 4.05f); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1025:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(530, 10348.84f, -6375.89f, 36.92f, 1.80f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1026:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, -8911.54f, -141.84f, 82.21f, 2.07f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1027:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, -6230.41f, 330.22f, 383.10f, 6.17f);
                player->SetPvP(false);
                return false;
            }
            break;
            case 1028:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, 10317.47f, 821.03f, 1326.37f, 0.74f); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1029:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(530, -3962.26f, -13930.66f, 100.56f, 2.09f); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1030:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                
                player->TeleportTo(0, -6230.41f, 330.22f, 383.10f, 6.17f); 
                player->SetPvP(false);
                return false;
            }
            break;
            case 1031:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(1, -603.57f, -4198.80f, 41.09f, 4.74f);
                player->SetPvP(false);
                return false;
            }
            break;

            // BYE BYE	
            case 1050:
            {
                CloseGossipMenuFor(player);
            }
            break;

            case 1125:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(648, -8242.04f, 1361.12f, 104.67f, 1.80f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 1126:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(638, -1436.46f, 1419.81f, 35.55f, 1.80f);  
                player->SetPvP(false);
                return false;
            }
            break;
            case 3000:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                player->TeleportTo(0, -8633.600586f, 594.303101f, 95.690689f, 1.487484f);
                player->SetPvP(false);
                return false;
            }
			break;
			case 5001:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);
                
                player->TeleportTo(1, -7177.14f, -3785.34f, 8.09f, 5.37f); 
                player->SetPvP(false);
                return false;
            }
			break;
			case 5002:
            {
                CloseGossipMenuFor(player);
                UpdateVoteTPData(player, ttseconds);

                player->TeleportTo(0, -14446.542969f, 15.206598f, 5.212281f, 5.37f); 
                player->SetPvP(false);
                return false;
            }
            break;
            }

            CloseGossipMenuFor(player);
			
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_vote_teleportAI(creature);
    }
};

void AddSC_npc_vote_teleport()
{
    new npc_vote_teleport();
}
