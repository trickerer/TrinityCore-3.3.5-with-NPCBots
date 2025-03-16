#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"

#define GOSSIP_HELLO_TP1  "Send me to the MGA Voters Area!"
#define GOSSIP_HELLO_TP3  "Nevermind - Bye!"
#define GOSSIP_HELLO_TP2  "IMPORT DETECTED!!! - Teleport me to the voters area."
#define GOSSIP_HELLO_TP4  "Send Me Home!"

#define GOSSIP_HELLO_TPNO  "You have not voted in the last 12 hours on this account, if you wish to use me please go and vote and I will send you to a Very Very nice location."

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


enum Spells
{
    SPELL_TRICK_OR_TREATED      = 24755,
    SPELL_TREAT                 = 24715
};

enum Npc
{
    NPC_GOSSIP_MENU = 9733,
    NPC_GOSSIP_MENU_EVENT = 342,
};

class npc_voters_zone : public CreatureScript
{
public:
    npc_voters_zone() : CreatureScript("npc_voters_zone") { }

    struct npc_voters_zoneAI : public ScriptedAI
    {
        npc_voters_zoneAI(Creature* creature) : ScriptedAI(creature) { }
		
        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

        bool OnGossipHello(Player* player) override
        {
            me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            InitGossipMenuFor(player, NPC_GOSSIP_MENU);
			
			QueryResult result;
            result = WorldDatabase.PQuery("SELECT * FROM `guild_transfer` WHERE `name1` = '%s' AND `duplicate` = '0' AND `status` = '0'", std::string(player->GetName()).c_str());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            }			
			
            time_t tseconds;
            tseconds = time(NULL);
            uint32 ttcheck;
            ttcheck = (tseconds - 43200); // 12 hours
			QueryResult result2;
			if (player->IsGameMaster())
            {
                result2 = WorldDatabase.PQuery("SELECT * FROM `vote_tp` WHERE `guid` > 0");
            }
            else
            {
                result2 = WorldDatabase.PQuery("SELECT * FROM `vote_tp` WHERE `guid`='%d' AND `time` >'%d' LIMIT 1", player->GetSession()->GetAccountId(), ttcheck);
            }
            
            //result2 = WorldDatabase.PQuery("SELECT * FROM `vote_tp` WHERE `guid`='%d' AND `time` >'%d' LIMIT 1", player->GetSession()->GetAccountId(), ttcheck);
           
            if (result2)
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
				AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1003);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
            }
            else if (player->IsGameMaster())
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1003);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TP3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_HELLO_TPNO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
				me->Yell("For me to talk to you, you must complete a simple task for me.  Go to the website and vote, do this and I will happy to talk to you more.", LANG_UNIVERSAL);
            }
			

            if (IsHolidayActive(HOLIDAY_HALLOWS_END) && !player->HasAura(SPELL_TRICK_OR_TREATED))
                AddGossipItemFor(player, NPC_GOSSIP_MENU_EVENT, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (me->IsQuestGiver())
                player->PrepareQuestMenu(me->GetGUID());

            if (me->IsVendor())
                AddGossipItemFor(player, NPC_GOSSIP_MENU, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

            if (me->IsInnkeeper())
                AddGossipItemFor(player, NPC_GOSSIP_MENU, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INN);

            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
            if (action == GOSSIP_ACTION_INFO_DEF + 1 && IsHolidayActive(HOLIDAY_HALLOWS_END) && !player->HasAura(SPELL_TRICK_OR_TREATED))
            {
                player->CastSpell(player, SPELL_TRICK_OR_TREATED, true);

                if (urand(0, 1))
                    player->CastSpell(player, SPELL_TREAT, true);
                else
                {
                    uint32 trickspell = 0;
                    switch (urand(0, 13))
                    {
                        case 0: trickspell = 24753; break; // cannot cast, random 30sec
                        case 1: trickspell = 24713; break; // lepper gnome costume
                        case 2: trickspell = 24735; break; // male ghost costume
                        case 3: trickspell = 24736; break; // female ghostcostume
                        case 4: trickspell = 24710; break; // male ninja costume
                        case 5: trickspell = 24711; break; // female ninja costume
                        case 6: trickspell = 24708; break; // male pirate costume
                        case 7: trickspell = 24709; break; // female pirate costume
                        case 8: trickspell = 24723; break; // skeleton costume
                        case 9: trickspell = 24753; break; // Trick
                        case 10: trickspell = 24924; break; // Hallow's End Candy
                        case 11: trickspell = 24925; break; // Hallow's End Candy
                        case 12: trickspell = 24926; break; // Hallow's End Candy
                        case 13: trickspell = 24927; break; // Hallow's End Candy
                    }
                    player->CastSpell(player, trickspell, true);
                }
                CloseGossipMenuFor(player);
                return true;
            }
			if (action == GOSSIP_ACTION_INFO_DEF + 1000)
			{
				//_Creature->Say("TELEPORT", LANG_UNIVERSAL);
				CloseGossipMenuFor(player);
				if (player->GetClass() == CLASS_DEATH_KNIGHT && player->GetLevel() < 80)
					player->SetPhaseMask(PHASEMASK_NORMAL, false);
				player->TeleportTo(571, 5817.41f, 601.99f, 570.55f, 3.13f);        // dala sewer arena (visual copy)
				player->SetPvP(false);
				return true;
			}
			if (action == GOSSIP_ACTION_INFO_DEF + 1001)
			{
				me->Say("Farewell "+player->GetName(), LANG_UNIVERSAL);
				//me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
				CloseGossipMenuFor(player);
				return true;
			}
			if (action == GOSSIP_ACTION_INFO_DEF + 1002)
			{
				CloseGossipMenuFor(player);
				me->Say("Farewell "+player->GetName(), LANG_UNIVERSAL);
				//me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
				return true;
			}
			if (action == GOSSIP_ACTION_INFO_DEF + 1003)
			{
				CloseGossipMenuFor(player);
				player->CastSpell(player, 8690, true); //https://www.wowhead.com/wotlk/spell=8690/hearthstone
				player->SetPvP(false);
				return true;
			}

            CloseGossipMenuFor(player);

            switch (action)
            {
                case GOSSIP_ACTION_TRADE: player->GetSession()->SendListInventory(me->GetGUID()); break;
                case GOSSIP_ACTION_INN: player->SetBindPoint(me->GetGUID()); break;
            }
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_voters_zoneAI(creature);
    }
};

void AddSC_npc_voters_zone()
{
    new npc_voters_zone();
}
