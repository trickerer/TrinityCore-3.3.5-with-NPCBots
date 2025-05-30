#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Pet.h"
#include "Chat.h"
#include "DBCStores.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <sstream>
#include <string>



enum SpellsAndItemIDsAndCost
{
    MAINPROFF = 3,  // token cost
    SECPROFF = 2,   // token cost

    ITEMCOSTID = 18154,  //MGA Mini Token

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

#define MAINPROFFT "3"
#define SECPROFFT "2"

#define GOSSIP_HELLO_OP1  "First Aid"
#define GOSSIP_HELLO_OP2  "Blacksmith"
#define GOSSIP_HELLO_OP3  "Leather Worker"
#define GOSSIP_HELLO_BYE  "Goodbye."





#define GOSSIP_HELLO_NOFREE "You already have max main professions."






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


class npc_proff_master : public CreatureScript
{
public:
    npc_proff_master() : CreatureScript("npc_proff_master") { }

    struct npc_proff_masterAI : public ScriptedAI
    {
        npc_proff_masterAI(Creature* creature) : ScriptedAI(creature) {}

        void WhisperTo(Player* player, char const* message)
        {
            me->Whisper(message, LANG_UNIVERSAL, player);
        }

		

        bool OnGossipHello(Player* player) override
        {
            uint32 freeProfs = player->GetFreePrimaryProfessionPoints();
            std::string proffcount = std::to_string(freeProfs);
            player->SetFreePrimaryProfessions(freeProfs);

            WorldSession* session = player->GetSession();
            // CHEDCK IF FREE SKILL SLOT
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "Main Professions", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            if (freeProfs < 1)
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, GOSSIP_HELLO_NOFREE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            }
            else
            {
                // AD TOO MAIN PROFFS
                if (freeProfs == 2)
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "You can learn " + proffcount + " main professions", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
                if (freeProfs == 1)
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, "You can learn " + proffcount + " main profession", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);

                
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


            }
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "-", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "Secondary Professions", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9999);

            // 2nd PROFFS
            if (!player->HasSkill(FirstAidSkill))
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Learn First Aid", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000);  //1stAid
            else
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "<Unlearn First Aid>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3000);  //1stAid

            if (!player->HasSkill(CookingSkill))
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Learn Cooking", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);  //cooking
            else
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "<Unlearn Cooking>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3001);  //cooking

            if (!player->HasSkill(FishingSkill))
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Learn Fishing", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);  //fishing
            else
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "<Unlearn Fishing>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3002);  //fishing

            AddGossipItemFor(player, GOSSIP_ICON_TALK, GOSSIP_HELLO_BYE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);

            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            WorldSession* session = player->GetSession();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            uint32 freep = player->GetFreePrimaryProfessionPoints();
            //std::string proffcount = std::to_string(freep);
            ClearGossipMenuFor(player);

            if (action == GOSSIP_ACTION_INFO_DEF + 1) // INFO TEST
            {
                // use this one to say gossip
                //me->Say(proffcount, LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                return false;
            }
           
			if (action == GOSSIP_ACTION_INFO_DEF + 1000) // first aid
			{
                //CHECK FOR TOKEN
                if (player->HasItemCount(ITEMCOSTID, SECPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, SECPROFF, true);
                    player->SetSkill(FirstAidSkill, 1, 450, 450);
                    player->CastSpell(player, FirstAidSPell);
                    HandleLearnSkillRecipesHelper(player, FirstAidSkill);
                    me->Say("Your First Aid skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                    
                }
                else
                {
                    me->Say("You are missing "+SECPROFF" MGA Mini Tokens, " + player->GetName() +" Go and get 2 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;
               
			}

            if (action == GOSSIP_ACTION_INFO_DEF + 1001) // cooking
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(ITEMCOSTID, SECPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, SECPROFF, true);
                    player->SetSkill(CookingSkill, 1, 450, 450);
                    player->CastSpell(player, CookingSpell);
                    HandleLearnSkillRecipesHelper(player, CookingSkill);
                    me->Say("Your Cooking skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                    
                }
                else
                {
                    me->Say("You are missing "+SECPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 2 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 1002) // fishing
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(ITEMCOSTID, SECPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, SECPROFF, true);
                    player->SetSkill(FishingSkill, 1, 450, 450);
                    player->CastSpell(player, FishingSpell);
                    HandleLearnSkillRecipesHelper(player, FishingSkill);
                    me->Say("Your Fishing skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                    
                }
                else
                {
                    me->Say("You are missing "+SECPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 2 and come back and talk to me", LANG_UNIVERSAL);
                    
                }
                CloseGossipMenuFor(player);
                return false;

            }



            // MAIN PROFFS

            if (action == GOSSIP_ACTION_INFO_DEF + 2001) // blacksmith
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, BlackSmithSPell);
                    player->SetSkill(BlackSmithSkill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, BlackSmithSkill);
                    me->Say("Your Blacksmithing is now at max " + player->GetName(), LANG_UNIVERSAL);   
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2002) // leather working
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, LeatherWorkingSpell);
                    player->SetSkill(LeatherWorkingSKill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, LeatherWorkingSKill);
                    me->Say("Your leather working is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2003) // ALCH
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, AlchemySPell);
                    player->SetSkill(AlchemySkill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, AlchemySkill);
                    me->Say("Your Alchemy skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2004) // HERB
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, HerbalismSpell);
                    player->SetSkill(HerbalismSkill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, HerbalismSkill);
                    me->Say("Your Herbalism skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2005) // mining
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, MiningSpell);
                    player->SetSkill(MiningSKill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, MiningSKill);
                    me->Say("Your Mining skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2006) // tailor
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, TailoringSpell);
                    player->SetSkill(TailoringSKill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, TailoringSKill);
                    me->Say("Your Tailoring skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2007) // eng
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, EngSPell);
                    player->SetSkill(EngSkill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, EngSkill);
                    me->Say("Your Engineering skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2008) // enchanter
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, EnchanterSpell);
                    player->SetSkill(EnchanterSkill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, EnchanterSkill);
                    me->Say("Your Enchanting skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2009) // skinner
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, SkinningSPell);
                    player->SetSkill(SKinningSkill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, SKinningSkill);
                    me->Say("Your Skinning skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2010) // jewel
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, JewelSpell);
                    player->SetSkill(JewelSKill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, JewelSKill);
                    player->SetSkill(MiningSKill, 0, 0, 0);  // WHY DID WE LEARN MINING???????
                    me->Say("Your Jewelcrafting skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                    
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }

            if (action == GOSSIP_ACTION_INFO_DEF + 2011) // scribe
            {
                //CHECK FOR TOKEN
                if (player->HasItemCount(21140, MAINPROFF))
                {
                    //LEARN PROFF
                    player->DestroyItemCount(ITEMCOSTID, MAINPROFF, true);
                    player->CastSpell(player, InscriptSpell);
                    player->SetSkill(InscriptSKill, 1, 450, 450);
                    HandleLearnSkillRecipesHelper(player, InscriptSKill);
                    me->Say("Your Jewelcrafting skill is now at max " + player->GetName(), LANG_UNIVERSAL);
                }
                else
                {
                    me->Say("You are missing "+MAINPROFF" MGA Mini Tokens, " + player->GetName() + " Go and get 3 and come back and talk to me", LANG_UNIVERSAL);
                }
                CloseGossipMenuFor(player);
                return false;

            }
            

            if (action == GOSSIP_ACTION_INFO_DEF + 2000)  // bye
            {
                me->Say("Farwell "+player->GetName(), LANG_UNIVERSAL);
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 3000) // remove 1st aid
            {
                player->SetSkill(FirstAidSkill, 0, 0, 0);
                me->Say("First Aid Has Been Removed, " + player->GetName(), LANG_UNIVERSAL);
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 3001) // remove cooking
            {
                player->SetSkill(CookingSkill, 0, 0, 0);
                me->Say("Cooking Has Been Removed, " + player->GetName(), LANG_UNIVERSAL);
            }
            if (action == GOSSIP_ACTION_INFO_DEF + 3002) //remove fishing
            {
                player->SetSkill(FishingSkill, 0, 0, 0);
                me->Say("Cooking Has Been Removed, " + player->GetName(), LANG_UNIVERSAL);
            }
            CloseGossipMenuFor(player);
            return false;




            if (action == GOSSIP_ACTION_INFO_DEF + 9999) //close
            {
                CloseGossipMenuFor(player);
                return false;
            }
            return true;

        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_proff_masterAI(creature);
    }
};

void AddSC_npc_proff_master()
{
    new npc_proff_master();
}
