/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scarlet_monastery.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "EventMap.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SmartAI.h"
#include <ScriptedGossip.h>
#include "TemporarySummon.h"
#include <EasternKingdoms/ScarletMonastery/boss_high_inquisitor_fairbanks.cpp>

Position const BunnySpawnPosition = { 1776.27f, 1348.74f, 19.20f };
Position const EarthBunnySpawnPosition = { 1765.28f, 1347.46f, 18.55f, 6.17f };
Position const HeadlessHorsemanSpawnPosition = { 1765.00f, 1347.00f, 15.00f };
Position const HeadlessHorsemanHeadSpawnPosition = { 1788.54f, 1348.05f, 18.88f }; // Guessed

ObjectData const creatureData[] =
{
    { NPC_HEADLESS_HORSEMAN_HEAD, DATA_HORSEMAN_HEAD     },
    { NPC_HEADLESS_HORSEMAN,      DATA_HEADLESS_HORSEMAN },
    { NPC_FLAME_BUNNY,            DATA_FLAME_BUNNY       },
    { NPC_EARTH_BUNNY,            DATA_EARTH_BUNNY       },
    { NPC_SIR_THOMAS,             DATA_THOMAS            },
    { NPC_MOGRAINE,               DATA_MOGRAINE          },
    { NPC_VORREL,                 DATA_VORREL            },
    { NPC_WHITEMANE,              DATA_WHITEMANE         },
    { 0,                          0                      } // END
};

ObjectData const gameObjectData[] =
{
    { GO_PUMPKIN_SHRINE,        DATA_PUMPKIN_SHRINE        },
    { GO_HIGH_INQUISITORS_DOOR, DATA_HIGH_INQUISITORS_DOOR },
    { GO_LOOSELY_TURNED_SOIL,   DATA_LOOSELY_TURNED_SOIL   },
    { 0,                        0                          } // END
};

enum AshbringerEventMisc
{
    AURA_OF_ASHBRINGER = 28282,
    NPC_SCARLET_MYRIDON = 4295,
    NPC_SCARLET_DEFENDER = 4298,
    NPC_SCARLET_CENTURION = 4301,
    NPC_SCARLET_SORCERER = 4294,
    NPC_SCARLET_WIZARD = 4300,
    NPC_SCARLET_ABBOT = 4303,
    NPC_SCARLET_MONK = 4540,
    NPC_SCARLET_CHAMPION = 4302,
    NPC_SCARLET_CHAPLAIN = 4299,
    NPC_FAIRBANKS = 4542,
    NPC_FAIRBANKS_NEW = 764542,
    NPC_COMMANDER_MOGRAINE = 3976,
    NPC_COMMANDER_MOGRAINE_NEW = 763976,
    FACTION_FRIENDLY_TO_ALL = 35,
};

enum ScarletMonasteryTrashMisc
{
    SAY_WELCOME = 0,
    AURA_ASHBRINGER = 28282,
    //FACTION_FRIENDLY_TO_ALL = 35,
    NPC_HIGHLORD_MOGRAINE = 16440,
    SPELL_COSMETIC_CHAIN = 45537,
    SPELL_COSMETIC_EXPLODE = 45935,
    SPELL_FORGIVENESS = 28697,
};

class instance_scarlet_monastery : public InstanceMapScript
{
public:
    instance_scarlet_monastery() : InstanceMapScript(SMScriptName, 189) { }

    struct instance_scarlet_monastery_InstanceMapScript : public InstanceScript
    {
        instance_scarlet_monastery_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
            LoadObjectData(creatureData, gameObjectData);
            _horsemanState = NOT_STARTED;
        }

        void HandleStartEvent()
        {
            _horsemanState = IN_PROGRESS;
            for (uint32 data : {DATA_PUMPKIN_SHRINE, DATA_LOOSELY_TURNED_SOIL})
                if (GameObject* gob = GetGameObject(data))
                    gob->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

            instance->SummonCreature(NPC_HEADLESS_HORSEMAN_HEAD, HeadlessHorsemanHeadSpawnPosition);
            instance->SummonCreature(NPC_FLAME_BUNNY, BunnySpawnPosition);
            instance->SummonCreature(NPC_EARTH_BUNNY, EarthBunnySpawnPosition);
            _events.ScheduleEvent(EVENT_ACTIVE_EARTH_EXPLOSION, 1s + 500ms);
            _events.ScheduleEvent(EVENT_SPAWN_HEADLESS_HORSEMAN, 3s);
            _events.ScheduleEvent(EVENT_DESPAWN_OBJECTS, 10s);
            if (Creature* thomas = GetCreature(DATA_THOMAS))
                thomas->DespawnOrUnsummon();
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
            case DATA_START_HORSEMAN_EVENT:
                if (_horsemanState != IN_PROGRESS)
                    HandleStartEvent();
                break;
            case DATA_HORSEMAN_EVENT_STATE:
                _horsemanState = data;
                break;
            case DATA_PREPARE_RESET:
                _horsemanState = NOT_STARTED;
                for (uint32 data : {DATA_FLAME_BUNNY, DATA_EARTH_BUNNY})
                    if (Creature* bunny = GetCreature(data))
                        bunny->DespawnOrUnsummon();
                break;
            default:
                break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case DATA_HORSEMAN_EVENT_STATE:
                return _horsemanState;
            default:
                return 0;
            }
        }

        void Update(uint32 diff) override
        {
            if (_events.Empty())
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE_EARTH_EXPLOSION:
                    if (Creature* earthBunny = GetCreature(DATA_EARTH_BUNNY))
                        earthBunny->CastSpell(earthBunny, SPELL_EARTH_EXPLOSION);
                    break;
                case EVENT_SPAWN_HEADLESS_HORSEMAN:
                    if (TempSummon* horseman = instance->SummonCreature(NPC_HEADLESS_HORSEMAN, HeadlessHorsemanSpawnPosition))
                        horseman->AI()->DoAction(ACTION_HORSEMAN_EVENT_START);
                    break;
                case EVENT_DESPAWN_OBJECTS:
                    for (uint32 data : {DATA_PUMPKIN_SHRINE, DATA_LOOSELY_TURNED_SOIL})
                        if (GameObject* gob = GetGameObject(data))
                            gob->RemoveFromWorld();
                    break;
                default:
                    break;
                }
            }
        }

        void OnPlayerEnter(Player* player)
        {
            if (player->HasAura(AURA_OF_ASHBRINGER))
            {
                std::list<Creature*> ScarletList;
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_MYRIDON, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_DEFENDER, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_CENTURION, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_SORCERER, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_WIZARD, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_ABBOT, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_MONK, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_CHAMPION, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_CHAPLAIN, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_COMMANDER_MOGRAINE, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_FAIRBANKS, 4000.0f);
                if (!ScarletList.empty())
                    for (std::list<Creature*>::iterator itr = ScarletList.begin(); itr != ScarletList.end(); itr++)
                    {
                        (*itr)->SetFaction(FACTION_FRIENDLY_TO_ALL);

                        //If Fairbanks or Mograine
                        if ((*itr)->GetEntry() == NPC_FAIRBANKS)
                        {
                            (*itr)->SummonCreature(NPC_FAIRBANKS_NEW, (*itr)->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);
                            (*itr)->RemoveFromWorld();
                        }
                        else if ((*itr)->GetEntry() == NPC_COMMANDER_MOGRAINE)
                        {
                            (*itr)->SummonCreature(NPC_COMMANDER_MOGRAINE_NEW, (*itr)->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);
                            (*itr)->RemoveFromWorld();
                        }
                    }
            }
        }

        void OnPlayerAreaUpdate(Player* player, uint32 /*oldArea*/, uint32 /*newArea*/)
        {
            if (player->HasAura(AURA_OF_ASHBRINGER))
            {
                std::list<Creature*> ScarletList;
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_MYRIDON, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_DEFENDER, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_CENTURION, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_SORCERER, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_WIZARD, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_ABBOT, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_MONK, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_CHAMPION, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_SCARLET_CHAPLAIN, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_COMMANDER_MOGRAINE, 4000.0f);
                player->GetCreatureListWithEntryInGrid(ScarletList, NPC_FAIRBANKS, 4000.0f);
                if (!ScarletList.empty())
                    for (std::list<Creature*>::iterator itr = ScarletList.begin(); itr != ScarletList.end(); itr++)
                    {
                        (*itr)->SetFaction(FACTION_FRIENDLY_TO_ALL);

                        //If Fairbanks or Mograine
                        if ((*itr)->GetEntry() == NPC_FAIRBANKS)
                        {
                            (*itr)->SummonCreature(NPC_FAIRBANKS_NEW, (*itr)->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);
                            (*itr)->RemoveFromWorld();
                        }
                        else if ((*itr)->GetEntry() == NPC_COMMANDER_MOGRAINE)
                        {
                            (*itr)->SummonCreature(NPC_COMMANDER_MOGRAINE_NEW, (*itr)->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN);
                            (*itr)->RemoveFromWorld();
                        }
                    }
            }
        }

    private:
        EventMap _events;
        uint32 _horsemanState;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_scarlet_monastery_InstanceMapScript(map);
    }
};

class npc_scarlet_guard : public CreatureScript
{
public:
    npc_scarlet_guard() : CreatureScript("npc_scarlet_guard") { }

    struct npc_scarlet_guardAI : public SmartAI
    {
        npc_scarlet_guardAI(Creature* creature) : SmartAI(creature) { }

        void Reset()
        {
            SayAshbringer = false;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who && who->GetDistance2d(me) < 12.0f)
            {
                if (Player* player = who->ToPlayer())
                {
                    if (player->HasAura(AURA_ASHBRINGER) && !SayAshbringer)
                    {
                        Talk(SAY_WELCOME);
                        me->SetFaction(FACTION_FRIENDLY_TO_ALL);
                        me->SetSheath(SHEATH_STATE_UNARMED);
                        me->SetFacingToObject(player);
                        me->SetStandState(UNIT_STAND_STATE_KNEEL);
                        me->AddAura(SPELL_AURA_MOD_ROOT, me);
                        me->CastSpell(me, SPELL_AURA_MOD_ROOT, true);
                        SayAshbringer = true;
                    }
                }
            }

            SmartAI::MoveInLineOfSight(who);
        }
    private:
        bool SayAshbringer = false;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_guardAI(creature);
    }
};

class npc_mograine : public CreatureScript
{
public:
    npc_mograine() : CreatureScript("npc_scarlet_commander_mograine") { }

    struct npc_mograineAI : public SmartAI
    {
        npc_mograineAI(Creature* creature) : SmartAI(creature) { }

        uint32 AshbringerEvent(uint32 uiSteps)
        {
            Creature* mograine = me->FindNearestCreature(NPC_HIGHLORD_MOGRAINE, 200.0f);

            switch (uiSteps)
            {
            case 1:
                me->GetMotionMaster()->MovePoint(0, 1152.039795f, 1398.405518f, 32.527878f);
                return 2 * IN_MILLISECONDS;
            case 2:
                me->SetSheath(SHEATH_STATE_UNARMED);
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                return 2 * IN_MILLISECONDS;
            case 3:
                Talk(3);
                return 10 * IN_MILLISECONDS;
            case 4:
                me->SummonCreature(NPC_HIGHLORD_MOGRAINE, 1065.130737f, 1399.350586f, 30.763723f, 6.282961f, TEMPSUMMON_TIMED_DESPAWN, std::chrono::milliseconds(400000))->SetName("Highlord Mograine");
                me->FindNearestCreature(NPC_HIGHLORD_MOGRAINE, 200.0f)->SetFaction(FACTION_FRIENDLY_TO_ALL);
                return 30 * IN_MILLISECONDS;
            case 5:
                mograine->StopMoving();
                mograine->AI()->Talk(0);
                mograine->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                return 4 * IN_MILLISECONDS;
            case 6:
                me->SetStandState(UNIT_STAND_STATE_STAND);
                return 2 * IN_MILLISECONDS;
            case 7:
                Talk(4);
                return 4 * IN_MILLISECONDS;
            case 8:
                mograine->AI()->Talk(1);
                return 11 * IN_MILLISECONDS;
            case 9:
                mograine->HandleEmoteCommand(EMOTE_ONESHOT_BATTLE_ROAR);
                return 4 * IN_MILLISECONDS;
            case 10:
                me->SetSheath(SHEATH_STATE_UNARMED);
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                Talk(5);
                return 2 * IN_MILLISECONDS;
            case 11:
                mograine->CastSpell(me, SPELL_FORGIVENESS, false);
                return 1 * IN_MILLISECONDS;
            case 12:
                mograine->CastSpell(me, SPELL_COSMETIC_CHAIN, true);
                return 0.5 * IN_MILLISECONDS;
            case 13:
                mograine->AI()->Talk(2);
                mograine->DespawnOrUnsummon(std::chrono::milliseconds(3 * IN_MILLISECONDS));
                mograine->Kill(me, me, true);
                return 0;
            default:
                if (mograine)
                    mograine->DespawnOrUnsummon(std::chrono::milliseconds(0));
                return 0;
            }
        }

        void Reset()
        {
            SayAshbringer = false;
            timer = 0;
            step = 1;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who && who->GetDistance2d(me) < 15.0f)
                if (Player* player = who->ToPlayer())
                    if (player->HasAura(AURA_ASHBRINGER) && !SayAshbringer)
                    {
                        me->SetFaction(FACTION_FRIENDLY_TO_ALL);
                        me->SetSheath(SHEATH_STATE_UNARMED);
                        me->SetStandState(UNIT_STAND_STATE_KNEEL);
                        me->SetFacingToObject(player);
                        me->Yell("Bow down! Kneel before the Ashbringer! A new dawn approaches, brothers and sisters! Our message will be delivered to the filth of this world through the chosen one!", LANG_UNIVERSAL, player);
                        SayAshbringer = true;
                    }

            SmartAI::MoveInLineOfSight(who);
        }

        void UpdateAI(uint32 diff)
        {
            timer = timer - diff;
            if (SayAshbringer && step < 15)
            {
                if (timer <= 0)
                {
                    timer = AshbringerEvent(step);
                    step++;
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private:
        bool SayAshbringer = false;
        int timer = 0;
        int step = 1;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mograineAI(creature);
    }
};

class npc_fairbanks : public CreatureScript
{
public:
    npc_fairbanks() : CreatureScript("npc_fairbanks") { }

    struct npc_fairbanksAI : public ScriptedAI
    {
        npc_fairbanksAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            SayAshbringer = false;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who && who->GetDistance2d(me) < 2.0f)
                if (Player* player = who->ToPlayer())
                    if (player->HasAura(AURA_ASHBRINGER) && !SayAshbringer)
                    {
                        me->SetFaction(FACTION_FRIENDLY_TO_ALL);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        me->SetSheath(SHEATH_STATE_UNARMED);
                        me->CastSpell(me, 57767, true);
                        me->SetDisplayId(16179);
                        me->SetStandState(UNIT_STAND_STATE_SIT);
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        me->SetFacingToObject(player);
                        SayAshbringer = true;
                    }
                    else
                    {
                        BossAI::BossAI( me, DATA_HIGH_INQUISITOR_FAIRBANKS);
                    }
            ScriptedAI::MoveInLineOfSight(who);
            
        }

        bool OnGossipHello(Player* plr) override
        {
            AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "Curse? What's going on here, Fairbanks?", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(plr, 100100, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* plr, uint32 Sender, uint32 uiAction) override
        {
            uint32 const action = plr->PlayerTalkClass->GetGossipOptionAction(uiAction);
            ClearGossipMenuFor(plr);

            switch (action)
            {
            case 1:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "Mograine?", GOSSIP_SENDER_MAIN, 2);
                SendGossipMenuFor(plr, 100101, me->GetGUID());
                return true;
            case 2:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "What do you mean?", GOSSIP_SENDER_MAIN, 3);
                SendGossipMenuFor(plr, 100102, me->GetGUID());
                return true;
            case 3:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "I still do not fully understand.", GOSSIP_SENDER_MAIN, 4);
                SendGossipMenuFor(plr, 100103, me->GetGUID());
                return true;
            case 4:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "Incredible story. So how did he die?", GOSSIP_SENDER_MAIN, 5);
                SendGossipMenuFor(plr, 100104, me->GetGUID());
                return true;
            case 5:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "You mean...", GOSSIP_SENDER_MAIN, 6);
                SendGossipMenuFor(plr, 100105, me->GetGUID());
                return true;
            case 6:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "How do you know all of this?", GOSSIP_SENDER_MAIN, 7);
                SendGossipMenuFor(plr, 100106, me->GetGUID());
                return true;
            case 7:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "A thousand? For one man?", GOSSIP_SENDER_MAIN, 8);
                SendGossipMenuFor(plr, 100107, me->GetGUID());
                return true;
            case 8:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_EXCLAMATION);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "Yet? Yet what?", GOSSIP_SENDER_MAIN, 9);
                SendGossipMenuFor(plr, 100108, me->GetGUID());
                return true;
            case 9:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "And did he?", GOSSIP_SENDER_MAIN, 10);
                SendGossipMenuFor(plr, 100109, me->GetGUID());
                return true;
            case 10:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_NO);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "Continue please, Fairbanks.", GOSSIP_SENDER_MAIN, 11);
                SendGossipMenuFor(plr, 100110, me->GetGUID());
                return true;
            case 11:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "You mean...", GOSSIP_SENDER_MAIN, 12);
                SendGossipMenuFor(plr, 100111, me->GetGUID());
                return true;
            case 12:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "You were right, Fairbanks. That is tragic.", GOSSIP_SENDER_MAIN, 13);
                SendGossipMenuFor(plr, 100112, me->GetGUID());
                return true;
            case 13:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "And you did...", GOSSIP_SENDER_MAIN, 14);
                SendGossipMenuFor(plr, 100113, me->GetGUID());
                return true;
            case 14:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "You tell an incredible tale, Fairbanks. What of the blade? Is it beyond redemption?", GOSSIP_SENDER_MAIN, 15);
                SendGossipMenuFor(plr, 100114, me->GetGUID());
                return true;
            case 15:
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                AddGossipItemFor(plr, GOSSIP_ICON_CHAT, "But his son is dead.", GOSSIP_SENDER_MAIN, 16);
                SendGossipMenuFor(plr, 100115, me->GetGUID());
                return true;
            case 16:
                SendGossipMenuFor(plr, 100116, me->GetGUID());
                // todo: we need to play these 3 emote in sequence, we play only the last one right now.
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_NO);
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_TALK);
                me->HandleEmoteCommand(Emote::EMOTE_ONESHOT_POINT_NO_SHEATHE);
                return true;
            }

            return true;
        }

    private:
        bool SayAshbringer = false;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fairbanksAI(creature);
    }
};


void AddSC_instance_scarlet_monastery()
{
    new instance_scarlet_monastery();
    new npc_scarlet_guard();
    new npc_fairbanks();
    new npc_mograine();
}
