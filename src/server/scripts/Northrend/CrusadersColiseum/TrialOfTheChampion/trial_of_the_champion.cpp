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

/* ScriptData
SDName: Trial Of the Champion
SD%Complete:
SDComment:
SDCategory: trial_of_the_champion
EndScriptData */

/* ContentData
npc_announcer_toc5
EndContentData */

#include "ScriptMgr.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "TemporarySummon.h"
#include "trial_of_the_champion.h"
#include "Vehicle.h"

enum Yells
{
    SAY_INTRO_1         = 0,
    SAY_INTRO_2         = 1,
    SAY_INTRO_3         = 2,
    SAY_AGGRO           = 3,
    SAY_PHASE_2         = 4,
    SAY_PHASE_3         = 5,
    SAY_KILL_PLAYER     = 6,
    SAY_DEATH           = 7
};

enum Gossip
{
    GOSSIP_START_EVENT1_MID = 10614,  //  I'm ready to start challenge.
    GOSSIP_START_EVENT1_OID = 0,
    GOSSIP_START_EVENT2_MID = 10614,  //  I'm ready for the next challenge.
    GOSSIP_START_EVENT2_OID = 1
};

#define ORIENTATION             4.714f

/*######
## npc_announcer_toc5
######*/

// Spawn deliberately behind the main gate.
// Arena-side reference from the user: 735.809 / 661.920 / 412.394 facing 4.714.
// With that facing, the gate lies to the north (+Y), so the parade must originate farther north.
const Position SpawnPosition = {746.261f, 684.000f, 411.681f, 4.65f};

class npc_announcer_toc5 : public CreatureScript
{
public:
    npc_announcer_toc5() : CreatureScript("npc_announcer_toc5") { }

    struct npc_announcer_toc5AI : public ScriptedAI
    {
        npc_announcer_toc5AI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();

            uiSummonTimes = 0;
            uiPosition = 0;
            uiLesserChampions = 0;

            uiFirstBoss = 0;
            uiSecondBoss = 0;
            uiThirdBoss = 0;

            uiArgentChampion = 0;

            uiPhase = 0;
            uiTimer = 0;

            me->SetReactState(REACT_PASSIVE);
            me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);

            SetGrandChampionsForEncounter();
            SetArgentChampion();
        }

        InstanceScript* instance;

        uint8 uiSummonTimes;
        uint8 uiPosition;
        uint8 uiLesserChampions;

        uint32 uiArgentChampion;

        uint32 uiFirstBoss;
        uint32 uiSecondBoss;
        uint32 uiThirdBoss;

        uint32 uiPhase;
        uint32 uiTimer;

        ObjectGuid uiVehicle1GUID;
        ObjectGuid uiVehicle2GUID;
        ObjectGuid uiVehicle3GUID;

        GuidList Champion1List;
        GuidList Champion2List;
        GuidList Champion3List;

        void NextStep(uint32 uiTimerStep, bool bNextStep = true, uint8 uiPhaseStep = 0)
        {
            uiTimer = uiTimerStep;
            if (bNextStep)
                ++uiPhase;
            else
                uiPhase = uiPhaseStep;
        }

        void SetMainGate(bool open)
        {
            if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                instance->HandleGameObject(gate->GetGUID(), open);
        }

        void SetData(uint32 uiType, uint32 /*uiData*/) override
        {
            switch (uiType)
            {
                case DATA_START:
                    Talk(SAY_INTRO_1);

                    // Presentation pass:
                    // open gate -> Grand Champion first -> three mounted-looking lesser champions behind him
                    SetMainGate(true);
                    DoSummonGrandChampion(uiFirstBoss);

                    // Give the whole group time to clear the doorway, then close it.
                    NextStep(10000, false, 10);
                    break;
                case DATA_IN_POSITION: // movement done - compatibility fallback
                    // Entrance sequencing is timer-driven because the old vehicle waypoint callback
                    // is unreliable in this branch. Never reopen the gate from this callback.
                    SetMainGate(false);
                    break;
                case DATA_LESSER_CHAMPIONS_DEFEATED:
                {
                    ++uiLesserChampions;
                    GuidList TempList;
                    if (uiLesserChampions == 3 || uiLesserChampions == 6)
                    {
                        switch (uiLesserChampions)
                        {
                            case 3:
                                TempList = Champion2List;
                                break;
                            case 6:
                                TempList = Champion3List;
                                break;
                        }

                        for (GuidList::const_iterator itr = TempList.begin(); itr != TempList.end(); ++itr)
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, *itr))
                                AggroAllPlayers(summon);
                    }else if (uiLesserChampions == 9)
                        StartGrandChampionsAttack();

                    break;
                }
            }
        }

        void StartGrandChampionsAttack()
        {
            Creature* pGrandChampion1 = ObjectAccessor::GetCreature(*me, uiVehicle1GUID);
            Creature* pGrandChampion2 = ObjectAccessor::GetCreature(*me, uiVehicle2GUID);
            Creature* pGrandChampion3 = ObjectAccessor::GetCreature(*me, uiVehicle3GUID);

            if (pGrandChampion1 && pGrandChampion2 && pGrandChampion3)
            {
                AggroAllPlayers(pGrandChampion1);
                AggroAllPlayers(pGrandChampion2);
                AggroAllPlayers(pGrandChampion3);
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (uiPointId == 1)
                me->SetFacingTo(ORIENTATION);
        }

        void DoSummonGrandChampion(uint32 uiBoss)
        {
            ++uiSummonTimes;

            uint32 VEHICLE_TO_SUMMON1 = 0;
            uint32 LESSER_TO_SUMMON = 0;

            // Alliance players fight the Horde champion set in this script.
            // Grand Champions keep their original mounted encounter.
            // Lesser champions are now standalone 353xx NPCs and never use 333xx vehicles.
            switch (uiBoss)
            {
                case 0:
                    VEHICLE_TO_SUMMON1 = VEHICLE_MOKRA_SKILLCRUSHER_MOUNT;
                    LESSER_TO_SUMMON = 35314; // Orgrimmar Champion
                    break;
                case 1:
                    VEHICLE_TO_SUMMON1 = VEHICLE_ERESSEA_DAWNSINGER_MOUNT;
                    LESSER_TO_SUMMON = 35326; // Silvermoon Champion
                    break;
                case 2:
                    VEHICLE_TO_SUMMON1 = VEHICLE_RUNOK_WILDMANE_MOUNT;
                    LESSER_TO_SUMMON = 35325; // Thunder Bluff Champion
                    break;
                case 3:
                    VEHICLE_TO_SUMMON1 = VEHICLE_ZUL_TORE_MOUNT;
                    LESSER_TO_SUMMON = 35323; // Sen'jin Champion
                    break;
                case 4:
                    VEHICLE_TO_SUMMON1 = VEHICLE_DEATHSTALKER_VESCERI_MOUNT;
                    LESSER_TO_SUMMON = 35327; // Undercity Champion
                    break;
                default:
                    return;
            }

            if (Creature* pBoss = me->SummonCreature(VEHICLE_TO_SUMMON1, SpawnPosition))
            {
                switch (uiSummonTimes)
                {
                    case 1:
                    {
                        uiVehicle1GUID = pBoss->GetGUID();
                        ObjectGuid uiGrandChampionBoss1;
                        if (Vehicle* pVehicle = pBoss->GetVehicleKit())
                            if (Unit* unit = pVehicle->GetPassenger(0))
                                uiGrandChampionBoss1 = unit->GetGUID();

                        instance->SetGuidData(DATA_GRAND_CHAMPION_VEHICLE_1, uiVehicle1GUID);
                        instance->SetGuidData(DATA_GRAND_CHAMPION_1, uiGrandChampionBoss1);
                        pBoss->AI()->SetData(1, 0);
                        break;
                    }
                    case 2:
                    {
                        uiVehicle2GUID = pBoss->GetGUID();
                        ObjectGuid uiGrandChampionBoss2;
                        if (Vehicle* pVehicle = pBoss->GetVehicleKit())
                            if (Unit* unit = pVehicle->GetPassenger(0))
                                uiGrandChampionBoss2 = unit->GetGUID();

                        instance->SetGuidData(DATA_GRAND_CHAMPION_VEHICLE_2, uiVehicle2GUID);
                        instance->SetGuidData(DATA_GRAND_CHAMPION_2, uiGrandChampionBoss2);
                        pBoss->AI()->SetData(2, 0);
                        break;
                    }
                    case 3:
                    {
                        uiVehicle3GUID = pBoss->GetGUID();
                        ObjectGuid uiGrandChampionBoss3;
                        if (Vehicle* pVehicle = pBoss->GetVehicleKit())
                            if (Unit* unit = pVehicle->GetPassenger(0))
                                uiGrandChampionBoss3 = unit->GetGUID();

                        instance->SetGuidData(DATA_GRAND_CHAMPION_VEHICLE_3, uiVehicle3GUID);
                        instance->SetGuidData(DATA_GRAND_CHAMPION_3, uiGrandChampionBoss3);
                        pBoss->AI()->SetData(3, 0);
                        break;
                    }
                    default:
                        return;
                }

                // Three standalone lesser champions escort each Grand Champion into the arena.
                // They start passive/non-attackable through JustSummoned(), then the announcer
                // activates the three waves with AggroAllPlayers().
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (Creature* pAdd = me->SummonCreature(LESSER_TO_SUMMON, SpawnPosition, TEMPSUMMON_CORPSE_DESPAWN))
                    {
                        switch (uiSummonTimes)
                        {
                            case 1:
                                Champion1List.push_back(pAdd->GetGUID());
                                break;
                            case 2:
                                Champion2List.push_back(pAdd->GetGUID());
                                break;
                            case 3:
                                Champion3List.push_back(pAdd->GetGUID());
                                break;
                        }

                        switch (i)
                        {
                            case 0:
                                pAdd->GetMotionMaster()->MoveFollow(pBoss, 2.0f, float(M_PI));
                                break;
                            case 1:
                                pAdd->GetMotionMaster()->MoveFollow(pBoss, 2.0f, float(M_PI) / 2);
                                break;
                            case 2:
                                pAdd->GetMotionMaster()->MoveFollow(pBoss, 2.0f, float(M_PI) / 2 + float(M_PI));
                                break;
                        }
                    }
                }
            }
        }

        void DoStartArgentChampionEncounter()
        {
            me->GetMotionMaster()->MovePoint(1, 735.81f, 661.92f, 412.39f);

            if (me->SummonCreature(uiArgentChampion, SpawnPosition))
            {
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (Creature* pTrash = me->SummonCreature(NPC_ARGENT_LIGHWIELDER, SpawnPosition))
                        pTrash->AI()->SetData(i, 0);
                    if (Creature* pTrash = me->SummonCreature(NPC_ARGENT_MONK, SpawnPosition))
                        pTrash->AI()->SetData(i, 0);
                    if (Creature* pTrash = me->SummonCreature(NPC_PRIESTESS, SpawnPosition))
                        pTrash->AI()->SetData(i, 0);
                }
            }
        }

        void SetGrandChampionsForEncounter()
        {
            uiFirstBoss = urand(0, 4);

            while (uiSecondBoss == uiFirstBoss || uiThirdBoss == uiFirstBoss || uiThirdBoss == uiSecondBoss)
            {
                uiSecondBoss = urand(0, 4);
                uiThirdBoss = urand(0, 4);
            }
        }

        void SetArgentChampion()
        {
           uint8 uiTempBoss = urand(0, 1);

           switch (uiTempBoss)
           {
                case 0:
                    uiArgentChampion = NPC_EADRIC;
                    break;
                case 1:
                    uiArgentChampion = NPC_PALETRESS;
                    break;
           }
        }

        void StartEncounter()
        {
            me->RemoveNpcFlag(UNIT_NPC_FLAG_GOSSIP);

            if (instance->GetBossState(BOSS_BLACK_KNIGHT) == NOT_STARTED)
            {
                if (instance->GetBossState(BOSS_ARGENT_CHALLENGE_E) == NOT_STARTED && instance->GetBossState(BOSS_ARGENT_CHALLENGE_P) == NOT_STARTED)
                {
                    if (instance->GetBossState(BOSS_GRAND_CHAMPIONS) == NOT_STARTED)
                        SetData(DATA_START, 0);

                    if (instance->GetBossState(BOSS_GRAND_CHAMPIONS) == DONE)
                        DoStartArgentChampionEncounter();
                }

               if ((instance->GetBossState(BOSS_GRAND_CHAMPIONS) == DONE &&
                   instance->GetBossState(BOSS_ARGENT_CHALLENGE_E) == DONE) ||
                   instance->GetBossState(BOSS_ARGENT_CHALLENGE_P) == DONE)
                    me->SummonCreature(VEHICLE_BLACK_KNIGHT, 769.834f, 651.915f, 447.035f, 0);
            }
        }

        void AggroAllPlayers(Creature* temp)
        {
            if (!temp)
                return;

            temp->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            temp->SetFaction(14);
            temp->RemoveNpcFlag(UNIT_NPC_FLAG_SPELLCLICK);

            temp->RemoveUnitFlag(
                UNIT_FLAG_NON_ATTACKABLE |
                UNIT_FLAG_NOT_ATTACKABLE_1 |
                UNIT_FLAG_UNINTERACTIBLE |
                UNIT_FLAG_IMMUNE_TO_PC |
                UNIT_FLAG_IMMUNE_TO_NPC
            );

            temp->SetImmuneToPC(false);
            temp->SetImmuneToNPC(false);
            temp->SetReactState(REACT_AGGRESSIVE);

            Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                if (Player* player = itr->GetSource())
                {
                    if (player->IsGameMaster() || !player->IsAlive())
                        continue;

                    AddThreat(player, 1.0f, temp);
                }
            }

            if (temp->AI())
                temp->AI()->DoZoneInCombat();
        }

       void UpdateAI(uint32 uiDiff) override
        {
            ScriptedAI::UpdateAI(uiDiff);

            if (uiTimer <= uiDiff)
            {
                switch (uiPhase)
                {
                    // First group has just entered.
                    case 10:
                        SetMainGate(false);
                        NextStep(3000, false, 1);
                        break;

                    // Second entrance.
                    case 1:
                        Talk(SAY_INTRO_2);
                        SetMainGate(true);
                        DoSummonGrandChampion(uiSecondBoss);
                        NextStep(10000, false, 11);
                        break;

                    case 11:
                        SetMainGate(false);
                        NextStep(3000, false, 2);
                        break;

                    // Third entrance.
                    case 2:
                        Talk(SAY_INTRO_3);
                        SetMainGate(true);
                        DoSummonGrandChampion(uiThirdBoss);
                        NextStep(10000, false, 12);
                        break;

                    case 12:
                        SetMainGate(false);

                        // The parade is complete. Put the announcer in his arena position,
                        // pause briefly, then release the first lesser wave.
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(1, 735.81f, 661.92f, 412.39f);
                        NextStep(5000, false, 3);
                        break;

                    // Wave 1 begins only after all three ceremonial entrances are finished.
                    case 3:
                        if (!Champion1List.empty())
                        {
                            for (GuidList::const_iterator itr = Champion1List.begin(); itr != Champion1List.end(); ++itr)
                                if (Creature* summon = ObjectAccessor::GetCreature(*me, *itr))
                                    AggroAllPlayers(summon);
                        }

                        // Stop the presentation timer until deaths trigger waves 2/3.
                        uiPhase = 0;
                        uiTimer = 0;
                        break;
                }
            }
            else
                uiTimer -= uiDiff;

            if (!UpdateVictim())
                return;
        }

        void JustSummoned(Creature* summon) override
        {
            if (instance->GetBossState(BOSS_GRAND_CHAMPIONS) == NOT_STARTED)
            {
                summon->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }

        void SummonedCreatureDespawn(Creature* /*summon*/) override
        {
            // Lesser-wave progression is handled immediately by npc_toc5_lesser_champion::JustDied().
            // The old implementation counted despawning 333xx vehicles, which are no longer used.
        }

        bool OnGossipHello(Player* player) override
        {
            // Disable gossip only after the final encounter is actually finished.
            // The old condition disabled gossip immediately when Paletress was DONE,
            // which prevented starting the Black Knight encounter.
            if (instance->GetBossState(BOSS_BLACK_KNIGHT) == DONE)
                return false;

            if (instance->GetBossState(BOSS_GRAND_CHAMPIONS) == NOT_STARTED &&
                instance->GetBossState(BOSS_ARGENT_CHALLENGE_E) == NOT_STARTED &&
                instance->GetBossState(BOSS_ARGENT_CHALLENGE_P) == NOT_STARTED &&
                instance->GetBossState(BOSS_BLACK_KNIGHT) == NOT_STARTED)
            {
                InitGossipMenuFor(player, GOSSIP_START_EVENT1_MID);
                AddGossipItemFor(player, GOSSIP_START_EVENT1_MID, GOSSIP_START_EVENT1_OID, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                SendGossipMenuFor(player, player->GetGossipTextId(GOSSIP_START_EVENT1_MID, me), me->GetGUID());
            }
            else
            {
                InitGossipMenuFor(player, GOSSIP_START_EVENT2_MID);
                AddGossipItemFor(player, GOSSIP_START_EVENT2_MID, GOSSIP_START_EVENT2_OID, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                SendGossipMenuFor(player, player->GetGossipTextId(GOSSIP_START_EVENT2_MID, me), me->GetGUID());
            }

            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);
            if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                CloseGossipMenuFor(player);
                StartEncounter();
            }
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_announcer_toc5AI>(creature);
    }
};


/*######
## npc_toc5_lesser_champion
##
## Replacement for the broken 333xx lesser vehicle phase.
## 353xx champion NPCs fight as ordinary creatures and report their death
## directly to the announcer so wave progression no longer depends on vehicles.
######*/

class npc_toc5_lesser_champion : public CreatureScript
{
public:
    npc_toc5_lesser_champion() : CreatureScript("npc_toc5_lesser_champion") { }

    struct npc_toc5_lesser_championAI : public ScriptedAI
    {
        npc_toc5_lesser_championAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (!instance)
                return;

            if (Creature* announcer = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ANNOUNCER)))
                if (announcer->AI())
                    announcer->AI()->SetData(DATA_LESSER_CHAMPIONS_DEFEATED, 0);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_toc5_lesser_championAI>(creature);
    }
};

void AddSC_trial_of_the_champion()
{
    new npc_announcer_toc5();
    new npc_toc5_lesser_champion();
}
