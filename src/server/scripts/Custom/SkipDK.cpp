/* Skip Death Knight Module
   Original Module From Single Player Project Consolidated Skip Module
   Rewritten for TC 335 By Single Player Project Developer MDic
   Original Concept from conanhun513
*/

#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Common.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SharedDefines.h"
#include "World.h"
#include "WorldSession.h"

class Trinitycore_skip_deathknight_announce : public PlayerScript
{
public:
    Trinitycore_skip_deathknight_announce() : PlayerScript("Trinitycore_skip_deathknight_announce") { }

    void OnLogin(Player* Player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetBoolDefault("Skip.Deathknight.Starter.Announce.enable", true))
        {
            ChatHandler(Player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Trinitycore Skip Deathknight Starter |rmodule.");
        }
    }
};

class Trinitycore_skip_deathknight : public PlayerScript
{
public:
    Trinitycore_skip_deathknight() : PlayerScript("Trinitycore_skip_deathknight") { }

    void OnLogin(Player* player, bool firstLogin) override
    {
        int DKL = sConfigMgr->GetFloatDefault("Skip.Deathknight.Start.Level", 58);

        if (sConfigMgr->GetBoolDefault("Skip.Deathknight.Starter.Enable", true))
        {
            if (player->GetAreaId() == 4342)
            {
                if (!firstLogin)
                    return;
                player->SetLevel(DKL);
                player->LearnSpell(53428, false);//runeforging
                player->LearnSpell(53441, false);//runeforging
                player->LearnSpell(53344, false);//runeforging
                player->LearnSpell(62158, false);//runeforging
                player->LearnSpell(33391, false);//journeyman riding
                player->LearnSpell(54586, false);//runeforging credit
                player->LearnSpell(48778, false);//acherus deathcharger
                player->LearnSkillRewardedSpells(776, 375);//Runeforging
                player->LearnSkillRewardedSpells(960, 375);//Runeforging
                player->EquipNewItem(EQUIPMENT_SLOT_HEAD, 38661, true);//Greathelm of the Scourge Champion
                player->EquipNewItem(EQUIPMENT_SLOT_WRISTS, 38666, true);//Plated Saronite Bracers
                player->EquipNewItem(EQUIPMENT_SLOT_WAIST, 38668, true);//The Plaguebringer's Girdle
                player->EquipNewItem(EQUIPMENT_SLOT_HANDS, 38667, true);//Bloodbane's Gauntlets of Command
                player->EquipNewItem(EQUIPMENT_SLOT_CHEST, 38665, true);//Saronite War Plate
                player->EquipNewItem(EQUIPMENT_SLOT_LEGS, 38669, true);//Engraved Saronite Legplates
                player->EquipNewItem(EQUIPMENT_SLOT_SHOULDERS, 38663, true);// Blood-Soaked Saronite Plated Spaulders
                player->EquipNewItem(EQUIPMENT_SLOT_FEET, 38670, true);//Greaves of the Slaughter
                player->EquipNewItem(EQUIPMENT_SLOT_TRINKET1, 38675, true);//Signet of the Dark Brotherhood
                player->EquipNewItem(EQUIPMENT_SLOT_TRINKET2, 38674, true);//Soul Harvester's Charm
                player->EquipNewItem(EQUIPMENT_SLOT_FINGER1, 38671, true);//Valanar's Signet Ring
                player->EquipNewItem(EQUIPMENT_SLOT_FINGER2, 38672, true);// Keleseth's Signet Ring
                player->AddItem(39320, true);//Sky Darkener's Shroud of Blood
                player->AddItem(38664, true);//Sky Darkener's Shroud of the Unholy
                player->AddItem(39322, true);//Shroud of the North Wind
                player->AddItem(38632, true);//Greatsword of the Ebon Blade
                player->AddItem(6948, true);//Hearthstone
                player->AddItem(38707, true);//Runed Soulblade
                player->AddItem(40483, true);//Insignia of the Scourge

                if (player->GetQuestStatus(12657) == QUEST_STATUS_NONE)//The Might Of The Scourge
                {
                    player->AddQuest(sObjectMgr->GetQuestTemplate(12657), nullptr);
                    player->RewardQuest(sObjectMgr->GetQuestTemplate(12657), false, player);
                }
                if (player->GetQuestStatus(12801) == QUEST_STATUS_NONE)//The Light of Dawn
                {
                    player->AddQuest(sObjectMgr->GetQuestTemplate(12801), nullptr);
                    player->RewardQuest(sObjectMgr->GetQuestTemplate(12801), false, player);
                }
                if (player->GetTeam() == ALLIANCE && player->GetQuestStatus(13188) == QUEST_STATUS_NONE)//Where Kings Walk
                    player->AddQuest(sObjectMgr->GetQuestTemplate(13188), nullptr);
                else if (player->GetTeam() == HORDE && player->GetQuestStatus(13189) == QUEST_STATUS_NONE)//Saurfang's Blessing
                    player->AddQuest(sObjectMgr->GetQuestTemplate(13189), nullptr);

                if (sConfigMgr->GetBoolDefault("Skip.DeathKnight.EndLocation.Override.Enable", false))
                {
                    int mapid = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.MapID", 1);
                    float x = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.X", 16226.2);
                    float y = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.Y", 16257);
                    float z = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.Z", 13.2);
                    float o = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.O", 1.65);
                    player->TeleportTo(mapid, x, y, z, o);
                }
                else
                {
                    if (player->GetTeam() == ALLIANCE)
                        player->TeleportTo(0, -8833.37f, 628.62f, 94.00f, 1.06f);//Stormwind
                    else
                        player->TeleportTo(1, 1569.59f, -4397.63f, 16.06f, 0.54f);//Orgrimmar
                }

                ObjectAccessor::SaveAllPlayers();//Save
            }
        }

        if (sConfigMgr->GetBoolDefault("GM.Skip.Deathknight.Starter.Enable", true))
        {
            if (player->GetSession()->GetSecurity() >= SEC_MODERATOR && player->GetAreaId() == 4342)
            {
                if (!firstLogin)
                    return;
                player->SetLevel(DKL);
                player->LearnSpell(53428, false);//runeforging
                player->LearnSpell(53441, false);//runeforging
                player->LearnSpell(53344, false);//runeforging
                player->LearnSpell(62158, false);//runeforging
                player->LearnSpell(33391, false);//journeyman riding
                player->LearnSpell(54586, false);//runeforging credit
                player->LearnSpell(48778, false);//acherus deathcharger
                player->LearnSkillRewardedSpells(776, 375);//Runeforging
                player->LearnSkillRewardedSpells(960, 375);//Runeforging
                player->EquipNewItem(EQUIPMENT_SLOT_HEAD, 38661, true);//Greathelm of the Scourge Champion
                player->EquipNewItem(EQUIPMENT_SLOT_WRISTS, 38666, true);//Plated Saronite Bracers
                player->EquipNewItem(EQUIPMENT_SLOT_WAIST, 38668, true);//The Plaguebringer's Girdle
                player->EquipNewItem(EQUIPMENT_SLOT_HANDS, 38667, true);//Bloodbane's Gauntlets of Command
                player->EquipNewItem(EQUIPMENT_SLOT_CHEST, 38665, true);//Saronite War Plate
                player->EquipNewItem(EQUIPMENT_SLOT_LEGS, 38669, true);//Engraved Saronite Legplates
                player->EquipNewItem(EQUIPMENT_SLOT_SHOULDERS, 38663, true);// Blood-Soaked Saronite Plated Spaulders
                player->EquipNewItem(EQUIPMENT_SLOT_FEET, 38670, true);//Greaves of the Slaughter
                player->EquipNewItem(EQUIPMENT_SLOT_TRINKET1, 38675, true);//Signet of the Dark Brotherhood
                player->EquipNewItem(EQUIPMENT_SLOT_TRINKET2, 38674, true);//Soul Harvester's Charm
                player->EquipNewItem(EQUIPMENT_SLOT_FINGER1, 38671, true);//Valanar's Signet Ring
                player->EquipNewItem(EQUIPMENT_SLOT_FINGER2, 38672, true);// Keleseth's Signet Ring
                player->AddItem(39320, true);//Sky Darkener's Shroud of Blood
                player->AddItem(38664, true);//Sky Darkener's Shroud of the Unholy
                player->AddItem(39322, true);//Shroud of the North Wind
                player->AddItem(38632, true);//Greatsword of the Ebon Blade
                player->AddItem(6948, true);//Hearthstone
                player->AddItem(38707, true);//Runed Soulblade
                player->AddItem(40483, true);//Insignia of the Scourge

                if (player->GetQuestStatus(12657) == QUEST_STATUS_NONE)//The Might Of The Scourge
                {
                    player->AddQuest(sObjectMgr->GetQuestTemplate(12657), nullptr);
                    player->RewardQuest(sObjectMgr->GetQuestTemplate(12657), false, player);
                }
                if (player->GetQuestStatus(12801) == QUEST_STATUS_NONE)//The Light of Dawn
                {
                    player->AddQuest(sObjectMgr->GetQuestTemplate(12801), nullptr);
                    player->RewardQuest(sObjectMgr->GetQuestTemplate(12801), false, player);
                }
                if (player->GetTeam() == ALLIANCE && player->GetQuestStatus(13188) == QUEST_STATUS_NONE)//Where Kings Walk
                    player->AddQuest(sObjectMgr->GetQuestTemplate(13188), nullptr);
                else if (player->GetTeam() == HORDE && player->GetQuestStatus(13189) == QUEST_STATUS_NONE)//Saurfang's Blessing
                    player->AddQuest(sObjectMgr->GetQuestTemplate(13189), nullptr);

                if (sConfigMgr->GetBoolDefault("Skip.DeathKnight.EndLocation.Override.Enable", false))
                {
                    int mapid = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.MapID", 1);
                    float x = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.X", 16226.2);
                    float y = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.Y", 16257);
                    float z = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.Z", 13.2);
                    float o = sConfigMgr->GetFloatDefault("Skip.DeathKnight.EndLocation.Override.O", 1.65);
                    player->TeleportTo(mapid, x, y, z, o);
                }
                else
                {
                    if (player->GetTeam() == ALLIANCE)
                        player->TeleportTo(0, -8833.37f, 628.62f, 94.00f, 1.06f);//Stormwind
                    else
                        player->TeleportTo(1, 1569.59f, -4397.63f, 16.06f, 0.54f);//Orgrimmar
                }

                ObjectAccessor::SaveAllPlayers();//Save
            }
        }
    }
};

void AddSC_skip_StarterArea()
{
    new Trinitycore_skip_deathknight_announce;
    new Trinitycore_skip_deathknight;
}
