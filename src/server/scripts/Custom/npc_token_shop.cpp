#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Pet.h"
#include "Chat.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "DBCStores.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"
#include "AchievementMgr.h"
#include "botmgr.h"
#include "botdatamgr.h"
#include <sstream>
#include <string>

#define tokentext8 "Bye Bye "
#define tokentext9 "Wooahh the Power!!"
#define tokentext10 "You need one MGA Token, go find one!"
#define tokentext11 "Here you go 500 gold."
#define tokentext12 "Silly you're missing a MGA Token, get me one or 2 if you want to swap it for some gold."
#define tokentext13 "Here you go 1250 gold."
#define tokentext14 "Here You Go, Thanks"
#define tokentext15 "You Can't Afford This."
#define tokentext16 "Exchange Complete!"
#define tokentext17 "Exchange Complete!"
#define tokentext18 "Exchange Complete!"

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


class npc_token_shop : public CreatureScript
{
public:
    npc_token_shop() : CreatureScript("npc_token_shop") { }

    struct npc_token_shopAI : public ScriptedAI
    {
        npc_token_shopAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		
		bool UpdateReNameCharData(Player* player, int16 status)
        {
            WorldDatabase.PExecute("UPDATE `char_rename` SET `status`='{}' WHERE `charid`='{}' AND `status` = '0'", status, player->GetSession()->GetGUIDLow());
            return true;
        }

        bool OnGossipHello(Player* player) override
        {
            //me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Level Rewards", GOSSIP_SENDER_MAIN, 1000);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Gold Rewards", GOSSIP_SENDER_MAIN, 1300);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            //AddGossipItemFor(player,  GOSSIP_ICON_INTERACT_1, "Armor Rewards"    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1400);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Reputation Rewards", GOSSIP_SENDER_MAIN, 1500);
            //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Mount Rewards", GOSSIP_SENDER_MAIN, 1600);
			
			AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye", GOSSIP_SENDER_MAIN, 2);
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
            case 1: // Main menu recall
            {
                //me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Level Rewards", GOSSIP_SENDER_MAIN, 1000);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Gold Rewards", GOSSIP_SENDER_MAIN, 1300);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                //AddGossipItemFor(player,  GOSSIP_ICON_INTERACT_1, "Armor Rewards"    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1400);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Reputation Rewards", GOSSIP_SENDER_MAIN, 1500);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Mount Rewards", GOSSIP_SENDER_MAIN, 1600);
				
				AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye", GOSSIP_SENDER_MAIN, 2);

                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 2: // Bye
            {
                CloseGossipMenuFor(player);
                me->Say(tokentext8+player->GetName(), LANG_UNIVERSAL);
                me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            }break;
            case 1000: // Level Rewards
            {

                if (lvl < 60)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "5 Levels at the cost of 1 MGA Token", GOSSIP_SENDER_MAIN, 1001);
                }
                else if (lvl >= 60 && lvl <= 79)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "2 Level at the cost of 1 MGA Token", GOSSIP_SENDER_MAIN, 1004);
                }
                else if (lvl >= 80)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "You are too high level to boost yourself max is level 77", GOSSIP_SENDER_MAIN, 1);
                }
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "[Back]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1001: // 60, 2L for 1 Token
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 1))
                {
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    player->GiveLevel(lvl + 5);
                    player->InitTalentForLevel();
                    player->InitStatsForLevel();
                    player->InitGlyphsForLevel(); // scribe only???
                    player->InitTaxiNodesForLevel();
                    player->UpdateSkillsForLevel();
                    player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL);
                    if (player->HaveBot())
                    {
                        player->GetBotMgr()->SetBotsShouldUpdateStats();
                    }
                    if (Pet* pet = player->GetPet())
                        pet->SynchronizeLevelWithOwner();
                    me->Say(tokentext9, LANG_UNIVERSAL);
                    player->DestroyItemCount(21140, 1, true);
                    player->SaveToDB();

                }
                else
                {
                    me->Say(tokentext10, LANG_UNIVERSAL);
                }
            }break;
            case 1004: // 70, 
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 1))
                {
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    player->GiveLevel(lvl + 2);
                    player->InitTalentForLevel();
                    player->InitStatsForLevel();
                    player->InitGlyphsForLevel(); // scribe only???
                    player->InitTaxiNodesForLevel();
                    player->UpdateSkillsForLevel();
                    player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL);
                    if (player->HaveBot())
                    {
                        player->GetBotMgr()->SetBotsShouldUpdateStats();
                    }
                    if (Pet* pet = player->GetPet())
                        pet->SynchronizeLevelWithOwner();
                }
                else
                {
                    me->Say(tokentext10, LANG_UNIVERSAL);
                }
            }break;
            case 1300: // Gief Gawld Menu
            {
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "5000 Gold at the cost of 1 MGA Token", GOSSIP_SENDER_MAIN, 1301);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "12500 Gold at the cost of 2 MGA Tokens", GOSSIP_SENDER_MAIN, 1302);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                //AddGossipItemFor(player,  GOSSIP_ICON_VENDOR, "2500 Gold at the cost of 4 MGA Tokens"    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1303);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "[Back]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1301:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 1))
                {
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    player->SetMoney(player->GetMoney() + 50000000);
                    me->Say(tokentext11, LANG_UNIVERSAL);
                    player->DestroyItemCount(21140, 1, true);
                    player->SaveToDB();
                }
                else
                {
                    me->Say(tokentext12, LANG_UNIVERSAL);
                }
            }break;
            case 1302:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 2))
                {
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    player->SetMoney(player->GetMoney() + 125000000);
                    me->Say(tokentext13, LANG_UNIVERSAL);
                    player->DestroyItemCount(21140, 2, true);
                    player->SaveToDB();
                }
                else
                {
                    me->Say(tokentext12, LANG_UNIVERSAL);
                }
            }break;
            case 1303:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 4))
                {
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    player->SetMoney(player->GetMoney() + 25000000);
                    me->Say(tokentext13, LANG_UNIVERSAL);
                    player->DestroyItemCount(21140, 4, true);

                    player->SaveToDB();
                }
                else
                {
                    me->Say(tokentext12, LANG_UNIVERSAL);
                }
            }
            break;
            case 1500:
            {
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Aldor Reputation Tokens", GOSSIP_SENDER_MAIN, 1501);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Scryers Reputation Tokens", GOSSIP_SENDER_MAIN, 1502);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Honor Hold Reputation Tokens", GOSSIP_SENDER_MAIN, 1550);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Thrallmar Reputation Tokens", GOSSIP_SENDER_MAIN, 1551);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "[Back]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1501: // aldor
            {
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "1 Mark of Sargeras at the cost of 1 Mini Token", GOSSIP_SENDER_MAIN, 1510);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "10 Marks of Sargeras at the cost of 10 Mini Tokens", GOSSIP_SENDER_MAIN, 1511);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "1 Fel Armament at the cost of 4 Mini Tokens", GOSSIP_SENDER_MAIN, 1512);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "10 Fel Armament at the cost of 40 Mini Tokens", GOSSIP_SENDER_MAIN, 1513);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Back]", GOSSIP_SENDER_MAIN, 1500);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Main Menu]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1502: // scryers
            {
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "1 Sunfury Signet at the cost of 1 Mini Token", GOSSIP_SENDER_MAIN, 1514);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "10 Sunfury Signets at the cost of 10 Mini Tokens", GOSSIP_SENDER_MAIN, 1515);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "1 Arcane Tome at the cost of 4 Mini Tokens", GOSSIP_SENDER_MAIN, 1516);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "10 Arcane Tomes at the cost of 40 Mini Tokens", GOSSIP_SENDER_MAIN, 1517);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Back]", GOSSIP_SENDER_MAIN, 1500);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Main Menu]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }break;
            case 1510:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 1))
                {
                    uint32 itemId = 30809;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 1, true);
                        me->Say(tokentext14, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1511:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 10))
                {
                    uint32 itemId = 30809;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 10);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 10, true);
                        me->Say(tokentext14, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1512:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 4))
                {
                    uint32 itemId = 29740;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 4, true);
                        me->Say(tokentext14, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1513:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 40))
                {
                    uint32 itemId = 29740;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 10);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 40, true);
                        me->Say(tokentext14, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1514:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 1))
                {
                    uint32 itemId = 30810;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 1, true);
                        me->Say(tokentext16, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1515:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 10))
                {
                    uint32 itemId = 30810;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 10);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 10, true);
                        me->Say(tokentext16, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1516:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 4))
                {
                    uint32 itemId = 29739;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 4, true);
                        me->Say(tokentext16, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1517:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(18154, 40))
                {
                    uint32 itemId = 29739;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 10);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(18154, 40, true);
                        me->Say(tokentext16, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1550:
            {
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "5 Marks of Honor Hold at the cost of 1 MGA Token", GOSSIP_SENDER_MAIN, 1552);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Back]", GOSSIP_SENDER_MAIN, 1500);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Main Menu]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1551:
            {
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "5 Marks of Thrallmar at the cost of 1 MGA Token", GOSSIP_SENDER_MAIN, 1553);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Back]", GOSSIP_SENDER_MAIN, 1500);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Main Menu]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1552:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 1))
                {
                    uint32 itemId = 24579;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 5);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(21140, 1, true);
                        me->Say(tokentext17, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }break;
            case 1553:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 1))
                {
                    uint32 itemId = 24581;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 5);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(21140, 1, true);
                        me->Say(tokentext18, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }
            break;
            case 1600:
            {
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Peep's Whistle for 5 Tokens", GOSSIP_SENDER_MAIN, 1601);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "X-53 Touring Rocket for 5 Tokens", GOSSIP_SENDER_MAIN, 1602);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "--", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "[Back]", GOSSIP_SENDER_MAIN, 1);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
				SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

                return true;
            }break;
            case 1601:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 5))
                {
                    uint32 itemId = 25596;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(21140, 5, true);
                        me->Say(tokentext18, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
                }
            }
            break;
            case 1602:
            {
                CloseGossipMenuFor(player);
                if (player->HasItemCount(21140, 5))
                {
                    uint32 itemId = 54860;
                    ItemPosCountVec dest;

                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemId, 1, true);
                        player->DestroyItemCount(21140, 5, true);
                        me->Say(tokentext18, LANG_UNIVERSAL);
                    }
                    else
                    {
                        player->SendEquipError(msg, NULL, NULL);
                    }
                }
                else
                {
                    me->Yell(tokentext15, LANG_UNIVERSAL);
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
        return new npc_token_shopAI(creature);
    }
};

void AddSC_npc_token_shop()
{
    new npc_token_shop();
}
