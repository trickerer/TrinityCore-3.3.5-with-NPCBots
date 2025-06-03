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

#define tokentext1 "Thank you, here you go. " // Swapped 1 MGA for mini
#define tokentext2 "You can't afford that!!" // Cant afford
#define tokentext3 -1700002
#define tokentext4 -1700003
#define tokentext5 "Mini Tokens can be earned a few ways, you will find clues in the game or ask in world channel in game or Discord...." // How to Get tokens..
#define tokentext6 "UNKNOWN!" //??
#define tokentext7 "Farewell! " // Bye
#define tokentext8 "Exchange Complete." //??
#define tokentext9 "You own this, dont be silly.." //??

#define GOSSIP_HELLO_TSWAP1  "Create 1 MGA Token (Cost 50 Mini Tokens)"
#define GOSSIP_HELLO_TSWAP2  "Exchange 3 WSG Marks for 50 Honor"
#define GOSSIP_HELLO_TSWAP3  "Exchange 2 AB Marks for 50 Honor"
#define GOSSIP_HELLO_TSWAP4  "Exchange 1 AV Mark for 50 Honor" 
#define GOSSIP_HELLO_TSWAP5  "Exchange 1 EOTS Mark for 50 Honor"
#define GOSSIP_HELLO_TSWAP11  "Exchange 10 WG Mark for 50 Honor"
#define GOSSIP_HELLO_TSWAP6  "1 MGA Super VIP Emblem at the cost of 100 Emblem of MGA"
#define GOSSIP_HELLO_TSWAP7  "Obtain 10 Sunmotes at the cost of 1 Badge of Justice"
#define GOSSIP_HELLO_TSWAP8  "How do I earn Mini Tokens?"
#define GOSSIP_HELLO_TSWAP10  "Exchange 5K honor for 1 Mini Token"
#define GOSSIP_HELLO_TSWAP12  "Exchange 75K honor for 100 Arena Points."
#define GOSSIP_HELLO_TSWAP13  "Summon Aaron item. (cost 10 mini tokens)"
#define GOSSIP_HELLO_TSWAP14  "Summon Mailbox item. (cost 10 mini tokens)"
#define GOSSIP_HELLO_TSWAP15  "Summon Guild Vault item. (cost 10 mini tokens)"
#define GOSSIP_HELLO_TSWAP16  "Exchange 5 MGA Tokens For VIP Item"
#define GOSSIP_HELLO_TSWAP17  "Exchange 450K gold For VIP Item"
#define GOSSIP_HELLO_TSWAP18  "I see you are a VIP, Learn All FLight Paths!"
#define GOSSIP_HELLO_TSWAP9  "Farewell!"
#define GOSSIP_HELLO_NOTVIP  "I Can only make MGA Super VIP Emblem's for VIPs!"

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


class npc_token_exchange : public CreatureScript
{
public:
    npc_token_exchange() : CreatureScript("npc_token_exchange") { }

    struct npc_token_exchangeAI : public ScriptedAI
    {
        npc_token_exchangeAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

        bool OnGossipHello(Player* player) override
        {
            me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP1, GOSSIP_SENDER_MAIN, 1001);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP2, GOSSIP_SENDER_MAIN, 1002);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP3, GOSSIP_SENDER_MAIN, 1003);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP4, GOSSIP_SENDER_MAIN, 1004);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            //AddGossipItemFor(player, GOSSIP_ICON_VENDOR, GOSSIP_HELLO_TSWAP8, GOSSIP_SENDER_MAIN, 1005);
			
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP10, GOSSIP_SENDER_MAIN, 1010);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP11, GOSSIP_SENDER_MAIN, 1011);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP12, GOSSIP_SENDER_MAIN, 1012);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP13, GOSSIP_SENDER_MAIN, 1013);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP14, GOSSIP_SENDER_MAIN, 1014);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP15, GOSSIP_SENDER_MAIN, 1015);
            
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP16, GOSSIP_SENDER_MAIN, 1016);
            
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP17, GOSSIP_SENDER_MAIN, 1017);
            
			if (player->HasItemCount(461145, 1))
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP18, GOSSIP_SENDER_MAIN, 1018);
                
			//AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GOSSIP_HELLO_TSWAP5, GOSSIP_SENDER_MAIN, 1005);

		
			
			AddGossipItemFor(player, GOSSIP_ICON_TALK, GOSSIP_HELLO_TSWAP9, GOSSIP_SENDER_MAIN, 1009);
			
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            WorldSession* session = player->GetSession();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
			
			uint8 lvl = player->GetLevel();
           
            switch (action)
            {
            case 1001: // Give item if dont alrdy have it
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 50))
                {
                    uint32 itemId = 21140;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->SendNewItem(item, 1, true, false);
                        player->DestroyItemCount(18154, 50, true);
                        me->Say(tokentext1, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, nullptr, nullptr);
                    }
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1002: // close and cast spell on player and insuly
                CloseGossipMenuFor(player);
                if (player->HasItemCount(20558, 3))
                {
                    player->DestroyItemCount(20558, 3, true);
                    player->SetHonorPoints(player->GetHonorPoints() + 50);
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1003:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(20559, 2))
                {
                    player->DestroyItemCount(20559, 2, true);
                    player->SetHonorPoints(player->GetHonorPoints() + 50);
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1004:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(37711, 100))
                {
                    uint32 itemId = 22484;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->SendNewItem(item, 1, true, false);
                        player->DestroyItemCount(37711, 100, true);
                    }
                    else
                    {
                        player->SendEquipError(msg, nullptr, nullptr);
                    }
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1005:
                CloseGossipMenuFor(player);
                me->Yell(tokentext5, LANG_UNIVERSAL);
                break;
            case 1006:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(29434, 1))
                {
                    uint32 itemId = 34664;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, false, 10);
                        player->SendNewItem(item, 10, true, false);
                        player->DestroyItemCount(29434, 1, true);
                        me->Say(tokentext6, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, nullptr, nullptr);
                    }
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1007:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(20560, 1))
                {
                    player->DestroyItemCount(20560, 1, true);
                    player->SetHonorPoints(player->GetHonorPoints() + 50);
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1008:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(29024, 1))
                {
                    player->DestroyItemCount(29024, 1, true);
                    player->SetHonorPoints(player->GetHonorPoints() + 50);
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1009:
                CloseGossipMenuFor(player);
                me->Yell(tokentext7+player->GetName(), LANG_UNIVERSAL);
                me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                break;
            case 1010:
                CloseGossipMenuFor(player);
                if (player->GetHonorPoints() > 4999)
                {
                    uint32 itemId = 18154;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->SendNewItem(item, 1, true, false);
                        uint32 newHonor = player->GetHonorPoints() - 5000;
                        if (newHonor < 0) newHonor = 0;
                        player->SetHonorPoints(newHonor);
                    };
                }
                break;
            case 1011:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(43589, 10))
                {
                    player->DestroyItemCount(43589, 10, true);
                    player->SetHonorPoints(player->GetHonorPoints() + 50);
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1012:
                CloseGossipMenuFor(player);
                if (player->GetHonorPoints() > 74999)
                {
                    uint32 newHonor = player->GetHonorPoints() - 75000;
                    if (newHonor < 0) newHonor = 0;
                    player->SetHonorPoints(newHonor);
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1013:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 10))
                {
                    if (player->HasItemCount(90001, 1))
                    {
                        me->Yell(tokentext9, LANG_UNIVERSAL); // Already has reward
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        me->PlayDirectSound(11466);
                        return true;
                    }
                    else
                    {
                        uint32 itemId = 90001;
                        ItemPosCountVec dest;

                        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                        if (msg == EQUIP_ERR_OK)
                        {
                            Item* item = player->StoreNewItem(dest, itemId, 1, true);
                            player->SendNewItem(item, 1, true, false);
                            player->DestroyItemCount(18154, 10, true);
                            me->Say(tokentext8, LANG_UNIVERSAL); // Success
                        }
                        else
                        {
                            player->SendEquipError(msg, nullptr, nullptr); // Better to use nullptr
                        }
                    }
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL); // Not enough items
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
            case 1014:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 10))
                {
                    if (player->HasItemCount(90002, 1))
                    {
                        me->Yell(tokentext9, LANG_UNIVERSAL); // Already has reward
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        me->PlayDirectSound(11466);
                        return true;
                    }
                    else
                    {
                        uint32 itemId = 90002;
                        ItemPosCountVec dest;

                        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                        if (msg == EQUIP_ERR_OK)
                        {
                            Item* item = player->StoreNewItem(dest, itemId, 1, true);
                            player->SendNewItem(item, 1, true, false);
                            player->DestroyItemCount(18154, 10, true);
                            me->Say(tokentext8, LANG_UNIVERSAL); // Success
                        }
                        else
                        {
                            player->SendEquipError(msg, nullptr, nullptr); // Better to use nullptr
                        }
                    }
                    
                }
                break;
                case 1015:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 10))
                {
                    if (player->HasItemCount(90003, 1))
                    {
                        me->Yell(tokentext9, LANG_UNIVERSAL); // Already has reward
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        me->PlayDirectSound(11466);
                        return true;
                    }
                    else
                    {
                        uint32 itemId = 90003;
                        ItemPosCountVec dest;

                        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                        if (msg == EQUIP_ERR_OK)
                        {
                            Item* item = player->StoreNewItem(dest, itemId, 1, true);
                            player->SendNewItem(item, 1, true, false);
                            player->DestroyItemCount(18154, 10, true);
                            me->Say(tokentext8, LANG_UNIVERSAL); // Success
                        }
                        else
                        {
                            player->SendEquipError(msg, nullptr, nullptr); // Better to use nullptr
                        }
                    }
                    
                }
                break;
                case 1016:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 5))
                {
                    if (player->HasItemCount(461145, 1))
                    {
                        me->Yell(tokentext9, LANG_UNIVERSAL); // Already has reward
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        me->PlayDirectSound(11466);
                        return true;
                    }
                    else
                    {

                        uint32 itemId = 461145;
                        ItemPosCountVec dest;

                        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                        if (msg == EQUIP_ERR_OK)
                        {
                            Item* item = player->StoreNewItem(dest, itemId, 1, true);
                            player->SendNewItem(item, 1, true, false);
                            player->DestroyItemCount(21140, 5, true);
                            me->Say(tokentext8, LANG_UNIVERSAL); // Success
                        }
                        else
                        {
                            player->SendEquipError(msg, nullptr, nullptr);
                        }
                    }
                }
                break;
                case 1017:
                CloseGossipMenuFor(player);
                if (player->GetMoney() >= 4500000000)
                {
                    if (player->HasItemCount(461145, 1))
                    {
                        me->Yell(tokentext9, LANG_UNIVERSAL); // Already has reward
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        me->PlayDirectSound(11466);
                        return true;
                    }
                    else
                    {
                        uint32 itemId = 461145;
                        ItemPosCountVec dest;

                        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                        if (msg == EQUIP_ERR_OK)
                        {
                            Item* item = player->StoreNewItem(dest, itemId, 1, true);
                            player->SendNewItem(item, 1, true, false);
                            player->ModifyMoney(-4500000000);
                            me->Say(tokentext8, LANG_UNIVERSAL); // Success
                        }
                        else
                        {
                            player->SendEquipError(msg, nullptr, nullptr); // Better to use nullptr
                        }
                    }
                }
                else
                {
                    me->Yell(tokentext2, LANG_UNIVERSAL); // Not enough items
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    me->PlayDirectSound(11466);
                }
                break;
                case 1018:
                CloseGossipMenuFor(player);
                if (player->HasItemCount(461145, 1))
                {
                    uint32 itemId = 461146;
                    ItemPosCountVec dest;
                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->SendNewItem(item, 1, true, false);
                        me->Say(tokentext8, LANG_UNIVERSAL); // Success
                    }
                }
                break;
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
        return new npc_token_exchangeAI(creature);
    }
};

void AddSC_npc_token_exchange()
{
    new npc_token_exchange();
}
