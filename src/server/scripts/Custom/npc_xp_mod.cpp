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
    itemId = 461141,  // 1x Rates
    itemId2 = 461142, // XP BOOSTER item
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


class npc_xp_mod : public CreatureScript
{
public:
    npc_xp_mod() : CreatureScript("npc_xp_mod") { }

    struct npc_xp_modAI : public ScriptedAI
    {
        npc_xp_modAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		

        bool OnGossipHello(Player* player) override
        {
            WorldSession* session = player->GetSession();
            if (player->HasItemCount(itemId, 1) || player->HasItemCount(itemId2, 1))
            
            {
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Set XP To Normal - Deletes Item!! * WARNING *  BANK IT????", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            }

            if (!player->HasItemCount(itemId, 1) && !player->HasItemCount(itemId2, 1))
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Set XP Rates to 1x", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            }

            if (!player->HasItemCount(itemId2, 1) && !player->HasItemCount(itemId, 1))
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "BOOST XP TO 10x", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            }

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
                if (player->HasItemCount(itemId, 1) || player->HasItemCount(itemId2, 1))
                {
                    player->DestroyItemCount(itemId, 1, true);
                    player->DestroyItemCount(itemId2, 1, true);
                    me->Say("XP Modification Item Removed! XP Rates Server Normal!", LANG_UNIVERSAL);
                    return true;
                }
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 1001)
			{
                CloseGossipMenuFor(player);
                if (player->HasItemCount(itemId, 1))
                {
                    me->Say("You have the XP deduction item", LANG_UNIVERSAL);
                    return true;
                }
                ItemPosCountVec dest;
                InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, itemId, 1, true);
                    player->SendNewItem(item, 1, true, false);
                    me->Say("Here you go, carry this item and XP is set to 1x. Bank the item then you're done.", LANG_UNIVERSAL);
                }

			}

            if (action == GOSSIP_ACTION_INFO_DEF + 1002)
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(itemId2, 1))
                {
                    me->Say("You have the XP Booster item", LANG_UNIVERSAL);
                    return true;
                }

                if (!player->HasItemCount(itemId3, 1))
                {
                    me->Say("To boost XP to 10x, I need a MGA Token as payment! Let me know when you have one.", LANG_UNIVERSAL);
                    return true;
                }
                player->DestroyItemCount(itemId3, 1, true);

                ItemPosCountVec dest;
                InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId2, 1);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, itemId2, 1, true);
                    player->SendNewItem(item, 1, true, false);
                    me->Yell("Here you go, carry this item and XP is boosted to 10x. Bank the item then you're done.", LANG_UNIVERSAL);
                }

            }
            if (action == GOSSIP_ACTION_INFO_DEF + 2000)
            {
                me->Say(player->GetName() + " Bye.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 9999)
            {
                CloseGossipMenuFor(player);
                return true;
            }

            
            CloseGossipMenuFor(player);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_xp_modAI(creature);
    }
};

void AddSC_npc_xp_mod()
{
    new npc_xp_mod();
}
