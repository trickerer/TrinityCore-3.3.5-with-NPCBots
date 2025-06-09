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

#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

void mySleep(int sleepMs)
{
#ifdef LINUX
    usleep(sleepMs);
#endif
#ifdef WINDOWS
    Sleep(sleepMs);
#endif
}

#define nemhtext11   800013 // tell player to log out and back in
#define nemhtext21   800014 // tell player they not entilied
#define nemhtext31   800012 // no mga token

#define GOSSIP_HELLO_NEMH3  "Insta80 Me (costs 1 MGA Token)"
#define GOSSIP_HELLO_NEMH4  "You are not in the Insta80 system please visit the vote shop the web site to enable Insta80 for this character. (near bottom of list ont he shop)"
#define GOSSIP_HELLO_NEMH5  "Goodbye."



enum SpellsAndItemIDsAndCost
{
    MAINPROFF = 3,  // token cost
    SECPROFF = 2,   // token cost

    ITEMCOSTID = 461143,  //MGA Instant Level 80 Voucher

    FirstAidSkill = 129,
    FirstAidSPell = 65292,

    CookingSkill = 185,
    CookingSpell = 65291,

    FishingSkill = 356,
    FishingSpell = 65293,

    BlackSmithSkill = 164,
    BlackSmithSPell = 51298,

    LeatherWorkingSKill = 165,
    LeatherWorkingSpell = 51301,

    AlchemySkill = 171,
    AlchemySPell = 65281,

    HerbalismSkill = 182,
    HerbalismSpell = 65288,

    MiningSKill = 186,
    MiningSpell = 65289,

    TailoringSKill = 197,
    TailoringSpell = 65283,

    EngSkill = 202,
    EngSPell = 61464,

    EnchanterSkill = 333,
    EnchanterSpell = 51312,

    SKinningSkill = 393,
    SkinningSPell = 50307,

    JewelSKill = 755,
    JewelSpell = 65289,

    InscriptSKill = 773,
    InscriptSpell = 65287,
};

static void HandleLearnSkillRecipesHelper(Player* player, uint32 skillId)
{
    uint32 classmask = player->GetClassMask();

    std::vector<SkillLineAbilityEntry const*> const* skillLineAbilities = GetSkillLineAbilitiesBySkill(skillId);
    if (!skillLineAbilities)
        return;

    for (SkillLineAbilityEntry const* skillLine : *skillLineAbilities)
    {
        // not high rank
        if (skillLine->SupercededBySpell)
            continue;

        // skip racial skills
        if (skillLine->RaceMask != 0)
            continue;

        // skip wrong class skills
        if (skillLine->ClassMask && (skillLine->ClassMask & classmask) == 0)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->Spell);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
            continue;

        player->LearnSpell(skillLine->Spell, false);
    }
}



static bool HandleLearnMySpellsCommand(Player* player)
{
    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(player->GetClass());
    if (!classEntry)
        return true;

    std::vector<Trainer::Trainer const*> const& trainers = sObjectMgr->GetClassTrainers(player->GetClass());

    bool hadNew;
    do
    {
        hadNew = false;
        for (Trainer::Trainer const* trainer : trainers)
        {
            if (!trainer->IsTrainerValidForPlayer(player))
                continue;
            for (Trainer::Spell const& trainerSpell : trainer->GetSpells())
            {
                if (!trainer->CanTeachSpell(player, &trainerSpell))
                    continue;

                if (trainerSpell.IsCastable())
                    player->CastSpell(player, trainerSpell.SpellId, true);
                else
                    player->LearnSpell(trainerSpell.SpellId, false);

                hadNew = true;
            }
        }
    } while (hadNew);

    //player->SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
    return true;
}

static bool HandleMaxSkillCommand(Player* player)
{
    player->UpdateWeaponsSkillsToMaxSkillsForLevel();
    return true;
}

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


class npc_insta_80 : public CreatureScript
{
public:
    npc_insta_80() : CreatureScript("npc_insta_80") { }

    struct npc_insta_80AI : public ScriptedAI
    {
        npc_insta_80AI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		
		bool DeleteInsta80CharData(Player* player)
        {
            WorldDatabase.PExecute("DELETE FROM `char_Insta80` WHERE `acct_id` = '{}'", player->GetSession()->GetAccountId());
			WorldDatabase.PExecute("INSERT INTO `char_Insta80_done` (`char_id`, `acct_id`) VALUES ({}, {})", player->GetSession()->GetGUIDLow(), player->GetSession()->GetAccountId());
			//WorldDatabase.PExecute("UPDATE `char_Insta80` SET `status`='{}' WHERE `char_id`='{}'", status, player->GetSession()->GetGUIDLow());
            return true;
        }
		bool UpdateInsta80CharData(Player* player, int16 status)
        {
            WorldDatabase.PExecute("UPDATE `char_Insta80` SET `status`='{}' WHERE `char_id`='{}'", status, player->GetSession()->GetGUIDLow());
            return true;
        }
		bool InsertInsta80CharData(Player* player, int16 acctid, int16 status)
        {
            WorldDatabase.PExecute("INSERT INTO `char_Insta80` (`char_id`, `acct_id`, `status`) VALUES ({}, {}, {})", player->GetSession()->GetGUIDLow(), acctid, status);
            return true;
        }

        bool OnGossipHello(Player* player) override
        {

            //InitGossipMenuFor(player, NPC_GOSSIP_MENU);
            WorldSession* session = player->GetSession();
/*
            if (!player->IsGameMaster())
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "TEST MODE GMS ONLY", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "LOL Bye.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }
*/
			QueryResult result;

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `acct_id`='{}' AND `status` = 9 LIMIT 1", player->GetSession()->GetAccountId());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                if (player->IsGameMaster())
                {
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "REMOVE ME FROM SYSTEM-GM ONLY OPTION", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9950);
                }
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "YOU HAVE ALL READY MADE A LEVEL 80", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "LOL Bye.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            if (!player->HasItemCount(ITEMCOSTID, 1))
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "You dont have a instant level voucher!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9997);
				player->PlayDirectSound(11965);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "How do i get one?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9997);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Bye.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' LIMIT 1", player->GetSession()->GetGUIDLow());
            if (!result)
            {
                if (player->IsGameMaster())
                {
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "IF YOU CAN SEE THIS YOU MUST .GM OFF!!!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "Ok.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                    player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                    SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, ". I want to make > " + player->GetName() + " < a level 80.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                //AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, ". You can only make ONE level 80.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9998);
                //AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Please Visit the Weapon Master First!.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9100);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);

                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Nevermind, Bye.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 0 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "I want to make > " + player->GetName() + " < a level 80.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);
                //AddGossipItemFor(player, GOSSIP_ICON_TALK, "You can only make ONE level 80.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9998);

                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 8 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "PvE & PvP Gear! + Bonus", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9000);
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 7 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Bags!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8000);
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 6 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Learn ALL my spell.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7000);
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 5 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Level 80", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6000);
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }


            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 4 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Honor Points", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5000);
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }

            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 3 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Give me some Gold!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4000);
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }


            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 2 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {
                uint32 freeProfs = player->GetFreePrimaryProfessionPoints();
                if (freeProfs < 1)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "YOU HAVE TOO MANY PROFFESSIONS!!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Skip Second Profession", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9996);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "I Will Finish Laster.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                    player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                    SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Pick Second Proffesion", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
                if (!player->HasSkill(BlackSmithSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Blacksmithing", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3001); //BS
                if (!player->HasSkill(LeatherWorkingSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Leather Worker", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3002);  //LW
                if (!player->HasSkill(AlchemySkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Alchemist", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3003);  //ALC
                if (!player->HasSkill(HerbalismSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Herbalist", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3004);  //HERB
                if (!player->HasSkill(MiningSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Miner", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3005);  //MINING
                if (!player->HasSkill(TailoringSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Tailor", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3006);  //Tailor
                if (!player->HasSkill(EngSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Engineer", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3007);  //Eng
                if (!player->HasSkill(EnchanterSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Enchanter", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3008);  //Enchanter
                if (!player->HasSkill(SKinningSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Skinner", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3009);  //Skinner
                if (!player->HasSkill(JewelSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Jeweler", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3010);  //Jewel
                if (!player->HasSkill(InscriptSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Inscription", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3011);  //scribe

                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }




			// FIRST PROFF
            result = WorldDatabase.PQuery("SELECT * FROM `char_Insta80` WHERE `char_id`='{}' AND `status` = 1 LIMIT 1", player->GetSession()->GetGUIDLow());
            if (result)
            {

                uint32 freeProfs = player->GetFreePrimaryProfessionPoints();
                if (freeProfs < 1)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "YOU HAVE TOO MANY PROFFESSIONS!!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Skip First Profession", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9995);
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "---------------------------------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "I Will Finsh Later", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                    player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                    SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Pick First Proffesion", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				if (!player->HasSkill(BlackSmithSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Blacksmithing", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2001); //BS
                if (!player->HasSkill(LeatherWorkingSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Leather Worker", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2002);  //LW
                if (!player->HasSkill(AlchemySkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Alchemist", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2003);  //ALC
                if (!player->HasSkill(HerbalismSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Herbalist", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2004);  //HERB
                if (!player->HasSkill(MiningSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Miner", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2005);  //MINING
                if (!player->HasSkill(TailoringSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Tailor", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2006);  //Tailor
                if (!player->HasSkill(EngSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Engineer", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2007);  //Eng
                if (!player->HasSkill(EnchanterSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Enchanter", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2008);  //Enchanter
                if (!player->HasSkill(SKinningSkill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Skinner", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2009);  //Skinner
                if (!player->HasSkill(JewelSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Jeweler", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2010);  //Jewel
                if (!player->HasSkill(InscriptSKill))
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Inscription", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2011);  //scribe

                AddGossipItemFor(player, GOSSIP_ICON_DOT, "I Will Finsh Later.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
                player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
                return true;
            }
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "Use Have Used Me on This Account!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            WorldSession* session = player->GetSession();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);

            if (action == GOSSIP_ACTION_INFO_DEF + 2000)
            {
                me->Say("Start Level 80 " + player->GetName(), LANG_UNIVERSAL);
                UpdateInsta80CharData(player, 1);
                CloseGossipMenuFor(player);
                return true;

            }
           
			if (action == GOSSIP_ACTION_INFO_DEF + 1000)
			{
                me->Say("Start Level 80 "+player->GetName(), LANG_UNIVERSAL);
				UpdateInsta80CharData(player, 1);
				CloseGossipMenuFor(player);
				return true;

			}
			if (action == GOSSIP_ACTION_INFO_DEF + 1001)
			{
                UpdateInsta80CharData(player, 1);
                me->Say(player->GetName()+" You have been added to the instant Level 80 system, talk to me again.", LANG_UNIVERSAL);
                InsertInsta80CharData(player,  player->GetSession()->GetAccountId(), 1);
				CloseGossipMenuFor(player);
				return true;
			}
            if (action == GOSSIP_ACTION_INFO_DEF + 1002)
            {
                me->Say("Farewell "+player->GetName(), LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
				me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                return true;
            }
			
			
            // MAIN PROFFS

            if (action == GOSSIP_ACTION_INFO_DEF + 2001) // blacksmith
            {

                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, BlackSmithSPell);
                player->SetSkill(BlackSmithSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, BlackSmithSkill);
                me->Say("Your Blacksmithing is now at max " + player->GetName()+" , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2002) // leather working
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, LeatherWorkingSpell);
                player->SetSkill(LeatherWorkingSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, LeatherWorkingSKill);
                me->Say("Your leather working is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2003) // ALCH
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, AlchemySPell);
                player->SetSkill(AlchemySkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, AlchemySkill);
                me->Say("Your Alchemy skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2004) // HERB
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, HerbalismSpell);
                player->SetSkill(HerbalismSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, HerbalismSkill);
                me->Say("Your Herbalism skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2005) // mining
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, MiningSpell);
                player->SetSkill(MiningSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, MiningSKill);
                me->Say("Your Mining skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2006) // tailor
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, TailoringSpell);
                player->SetSkill(TailoringSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, TailoringSKill);
                me->Say("Your Tailoring skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2007) // eng
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, EngSPell);
                player->SetSkill(EngSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, EngSkill);
                me->Say("Your Engineering skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2008) // enchanter
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, EnchanterSpell);
                player->SetSkill(EnchanterSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, EnchanterSkill);
                me->Say("Your Enchanting skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2009) // skinner
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, SkinningSPell);
                player->SetSkill(SKinningSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, SKinningSkill);
                me->Say("Your Skinning skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2010) // jewel
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, JewelSpell);
                player->SetSkill(JewelSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, JewelSKill);
                player->SetSkill(MiningSKill, 0, 0, 0);  // WHY DID WE LEARN MINING???????
                me->Say("Your Jewelcrafting skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2011) // scribe
            {
                UpdateInsta80CharData(player, 2);
                player->CastSpell(player, InscriptSpell);
                player->SetSkill(InscriptSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, InscriptSKill);
                me->Say("Your Jewelcrafting skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }



            // SECOND PROFFS

            if (action == GOSSIP_ACTION_INFO_DEF + 3001) // blacksmith
            {

                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, BlackSmithSPell);
                player->SetSkill(BlackSmithSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, BlackSmithSkill);
                me->Say("Your Blacksmithing is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3002) // leather working
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, LeatherWorkingSpell);
                player->SetSkill(LeatherWorkingSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, LeatherWorkingSKill);
                me->Say("Your leather working is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3003) // ALCH
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, AlchemySPell);
                player->SetSkill(AlchemySkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, AlchemySkill);
                me->Say("Your Alchemy skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3004) // HERB
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, HerbalismSpell);
                player->SetSkill(HerbalismSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, HerbalismSkill);
                me->Say("Your Herbalism skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3005) // mining
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, MiningSpell);
                player->SetSkill(MiningSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, MiningSKill);
                me->Say("Your Mining skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3006) // tailor
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, TailoringSpell);
                player->SetSkill(TailoringSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, TailoringSKill);
                me->Say("Your Tailoring skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3007) // eng
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, EngSPell);
                player->SetSkill(EngSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, EngSkill);
                me->Say("Your Engineering skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3008) // enchanter
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, EnchanterSpell);
                player->SetSkill(EnchanterSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, EnchanterSkill);
                me->Say("Your Enchanting skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3009) // skinner
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, SkinningSPell);
                player->SetSkill(SKinningSkill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, SKinningSkill);
                me->Say("Your Skinning skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3010) // jewel
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, JewelSpell);
                player->SetSkill(JewelSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, JewelSKill);
                player->SetSkill(MiningSKill, 0, 0, 0);  // WHY DID WE LEARN MINING???????
                me->Say("Your Jewelcrafting skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 3011) // scribe
            {
                UpdateInsta80CharData(player, 3);
                player->CastSpell(player, InscriptSpell);
                player->SetSkill(InscriptSKill, 1, 350, 350);
                HandleLearnSkillRecipesHelper(player, InscriptSKill);
                me->Say("Your scribe skill is now at max " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;

            }
			


            if (action == GOSSIP_ACTION_INFO_DEF + 4000)
            {
                UpdateInsta80CharData(player, 4);
                player->ModifyMoney(50000000);
                me->Say("Here you go 5K gold! " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }


            if (action == GOSSIP_ACTION_INFO_DEF + 5000)
            {
                UpdateInsta80CharData(player, 5);
                player->ModifyHonorPoints(175000);
                me->Say("You now have 175000 honor points " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 6000)
            {
                UpdateInsta80CharData(player, 6);
                player->SetLevel(80);
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
                me->Say("You now level 80 " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 7000)
            {
                UpdateInsta80CharData(player, 7); 
                HandleLearnMySpellsCommand(player);
                me->Say("You have learned all spells. Make sure you set your tallents and you may need to visit your class trainer! " + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 8000)
            {
                int bag_id = 23162;
                for (int i = 0; i < 4; ++i)
                {
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, bag_id, 1);
                    if (msg == EQUIP_ERR_OK)
                    {

                        Item* item = player->StoreNewItem(dest, bag_id, 1, true);
                        player->SendNewItem(item, 1, true, false);
                    }
                    else
                    {
                        me->Yell("OMG!! " + player->GetName() + " You Had no space, some items they have been lost to the ether!!!!", LANG_UNIVERSAL);
                        mySleep(5);
                    }
                }
                UpdateInsta80CharData(player, 8);
                me->Say("4x Mega Bags!" + player->GetName() + " , talk to me again to continue.", LANG_UNIVERSAL);
                mySleep(1000);
                me->Yell("EQUIP YOUR BAGS NOW!!!!!", LANG_UNIVERSAL);
                
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 9000)
            {
                HandleMaxSkillCommand(player);
                uint32 bonusitem = 47241; // Emblem of Triumph
                for (int i = 1; i <= 50; i++)
                {
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, bonusitem, 1);
                    if (msg == EQUIP_ERR_OK)
                    {

                        Item* item = player->StoreNewItem(dest, bonusitem, 1, true);
                        player->SendNewItem(item, 1, true, false);
                        mySleep(5);
                    }
                }
                /*static const unsigned short Tier0_5List[] =  // Correspond line numbers with uint8 class in enum Classes
                {
                40783, 40801, 40819, 40840, 40859, 0, 0, 0, // Warrior 1
                40782, 40802, 40821, 40842, 40861, 0, 0, 0, // Paladin 2
                41085, 41141, 41155, 41203, 41215, 0, 0, 0, // Hunter 3
                41648, 41653, 41670, 41681, 41765, 0, 0, 0, // Rogue 4
                41913, 41919, 41925, 41931, 41938, 0, 0, 0, // Priest 5
                0, 0, 0, 0, 0, 0, 0, 0, // DK 6 - no set available
                40989, 41005, 41017, 41031, 41042, 0, 0, 0, // Shaman 7
                41944, 41950, 41957, 41963, 41969, 0, 0, 0, // Mage 8
                41991, 42001, 42003, 42009, 42015, 0, 0, 0, // Warlock 9
                0, 0, 0, 0, 0, 0, 0, 0, // Unused 10
                41279, 41291, 41302, 41314, 41325, 0, 0, 0, // Druid 11
                };*/ //HATEFULL!!!!
                /*static const unsigned short Tier0_5List[] =  // Correspond line numbers with uint8 class in enum Classes
                {
                40786, 40804, 40823, 40844, 40862, 0, 0, 0, // Warrior 1
                40785, 40805, 40825, 40846, 40864, 0, 0, 0, // Paladin 2
                41086, 41142, 41156, 41204, 41216, 0, 0, 0, // Hunter 3
                41649, 41654, 41671, 41682, 41766, 0, 0, 0, // Rogue 4
                41914, 41920, 41926, 41933, 41939, 0, 0, 0, // Priest 5
                40863, 40784, 40806, 40824, 40845, 0, 0, 0, // DK 6 - no set available
                40990, 41000, 41012, 41026, 41037, 0, 0, 0, // Shaman 7
                41945, 41951, 41958, 41964, 41970, 0, 0, 0, // Mage 8
                41992, 41997, 42004, 42010, 42016, 0, 0, 0, // Warlock 9
                0, 0, 0, 0, 0, 0, 0, 0, // Unused 10
                41280, 41292, 41303, 41315, 41326, 0, 0, 0, // Druid 11
                }; // DEADLY SET*/
                static const unsigned short Tier0_5List[] =  // Correspond line numbers with uint8 class in enum Classes
                {
                    40847, 40789, 40807, 40826, 40866, 0, 0, 0, // Warrior 1 / done
                    40933, 40907, 40927, 40939, 40963, 0, 0, 0, // Paladin 2 / done				
                    41087, 41143, 41157, 41205, 41217, 0, 0, 0, // Hunter 3	/ done			
                    41650, 41655, 41672, 41683, 41767, 0, 0, 0, // Rogue 4	/ done			
                    41915, 41921, 41927, 41934, 41940, 0, 0, 0, // Priest 5 / done				
                    40787, 40809, 40827, 40848, 40868, 0, 0, 0, // DK 6 - / done		
                    41019, 40993, 41007, 41033, 41044, 0, 0, 0, // Shaman 7 / done				
                    41971, 41946, 41953, 41959, 41965, 0, 0, 0, // Mage 8 / done
                    42017, 41993, 41998, 42005, 42011, 0, 0, 0, // Warlock 9 / done
                    0, 0, 0, 0, 0, 0, 0, 0, // Unused 10
                    41281, 41293, 41304, 41316, 41327, 0, 0, 0, // Druid 11 / done
                }; // Furious SET*/
                int8 pclass = player->GetClass();
                int loop = ((pclass * 8) - 8);
                //for (unsigned long i = loop; i < (loop + 8); i++)
                for (unsigned long i = loop; i < (loop + 8lu); i++)
                {
                    uint32 itemid = Tier0_5List[i];
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemid, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemid, true);
                        player->SendNewItem(item, 1, true, false);
                        mySleep(5);
                    }

                }
                static const unsigned short Tier0_6List[] =  // Correspond line numbers with uint8 class in enum Classes
                {
                    30120, 30118, 30119, 30121, 30122, 0, 0, 0, // Warrior 1
                    30129, 30130, 30132, 30133, 30131, 0, 0, 0, // Paladin 2
                    30139, 30140, 30141, 30142, 30143, 0, 0, 0, // Hunter 3
                    30144, 30145, 30146, 30148, 30149, 0, 0, 0, // Rogue 4
                    30160, 30161, 30162, 30159, 30163, 0, 0, 0, // Priest 5
                    40559, 40565, 40563, 40567, 40568, 0, 0, 0, // DK 6 - no set available // updated as of 2023
                    30169, 30170, 30171, 30172, 30173, 0, 0, 0, // Shaman 7
                    30206, 30205, 30207, 30210, 30196, 0, 0, 0, // Mage 8
                    30211, 30212, 30213, 30215, 30214, 0, 0, 0, // Warlock 9
                    0, 0, 0, 0, 0, 0, 0, 0, // Unused 10
                    30231, 30232, 30233, 30234, 30235, 0, 0, 0 // Druid 11
                }; // Armor Sets 70+
                int8 pclass2 = player->GetClass();
                int loop2 = ((pclass2 * 8) - 8);
                //for (unsigned long i = loop; i < (loop + 8); i++)
                for (unsigned long i = loop2; i < (loop2 + 8lu); i++)
                {
                    uint32 itemid = Tier0_6List[i];
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemid, 1);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, itemid, true);
                        player->SendNewItem(item, 1, true, false);
                        mySleep(5);
                    }

                }

                player->DestroyItemCount(ITEMCOSTID, 1, true);
                UpdateInsta80CharData(player, 9);
                CloseGossipMenuFor(player);
                player->SaveToDB();
                mySleep(5);
				DeleteInsta80CharData(player);
                //player->SetSkill(162, 1, 350, 350);  //unarmed
                //player->SetSkill(95, 1, 350, 350);  //Def
                if (pclass2 == 4) // rouge
                {
                    player->SetSkill(633, 1, 400, 400);  //lockoicking
                }
                mySleep(5);
                me->Yell("All Done! " + player->GetName() + " Welcome to the family", LANG_UNIVERSAL);
                mySleep(5);
                if (!player->IsGameMaster())
                {
                    if (player->GetTeamId() == TEAM_HORDE)
                        sWorld->SendWorldText(30000, "Horde", player->GetName().c_str());
                    else
                        sWorld->SendWorldText(30000, "Alliance", player->GetName().c_str());
                }
                return true;
            }
			
            if (action == GOSSIP_ACTION_INFO_DEF + 9995)
            {
                UpdateInsta80CharData(player, 2);
                me->Say("Skipped First Profession, " + player->GetName(), LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 9996)
            {
                UpdateInsta80CharData(player, 3);
                me->Say("Skipped Second Profession, " + player->GetName(), LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 9997)
            {
                me->Say(player->GetName()+" via the website, the vote shop. If you want another online shop mgawow.online/shop", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }


            if (action == GOSSIP_ACTION_INFO_DEF + 9998)
            {
                me->Say("You can only do this once, so pick right!!! " + player->GetName(), LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }
			
			if (action == GOSSIP_ACTION_INFO_DEF + 9999)
            {
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 9100)
            {
                me->Say("He's stood next to me! " + player->GetName() + ". Go learn all your weapon types for free!", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }

            if (action == GOSSIP_ACTION_INFO_DEF + 9950)
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 461143, 1);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, 461143, true);
                    player->SendNewItem(item, 1, true, false);
                }
                WorldDatabase.PExecute("DELETE FROM `char_Insta80` WHERE `acct_id` = '{}'", player->GetSession()->GetAccountId());
                me->Say("REMOVED YOU FROM SYSTEM", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return true;
            }
            
            CloseGossipMenuFor(player);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_insta_80AI(creature);
    }
};

void AddSC_npc_insta_80()
{
    new npc_insta_80();
}
