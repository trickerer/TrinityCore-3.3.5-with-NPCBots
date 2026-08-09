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
SDName: Instance Trial of the Champion
SDComment:
SDCategory: Trial Of the Champion
EndScriptData */

#include "ScriptMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "Player.h"
#include "trial_of_the_champion.h"

constexpr uint32 ToCEncounterCount = 4;

class instance_trial_of_the_champion : public InstanceMapScript
{
public:
    instance_trial_of_the_champion() : InstanceMapScript(ToCScriptName, 650) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_trial_of_the_champion_InstanceMapScript(map);
    }

    struct instance_trial_of_the_champion_InstanceMapScript : public InstanceScript
    {
        instance_trial_of_the_champion_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(ToCEncounterCount);
            uiMovementDone = 0;
            uiGrandChampionsDeaths = 0;
            uiGrandChampionMountsDefeated = 0;
            uiGrandChampionGroundDelay = 0;
            bGrandChampionGroundPending = false;
            uiArgentSoldierDeaths = 0;
            teamInInstance = 0;

            bDone = false;
        }

        uint32 teamInInstance;
        uint16 uiMovementDone;
        uint16 uiGrandChampionsDeaths;
        uint8 uiGrandChampionMountsDefeated;
        uint32 uiGrandChampionGroundDelay;
        bool bGrandChampionGroundPending;
        uint8 uiArgentSoldierDeaths;

        ObjectGuid uiAnnouncerGUID;
        ObjectGuid uiMainGateGUID;
        ObjectGuid uiGrandChampionVehicle1GUID;
        ObjectGuid uiGrandChampionVehicle2GUID;
        ObjectGuid uiGrandChampionVehicle3GUID;
        ObjectGuid uiGrandChampion1GUID;
        ObjectGuid uiGrandChampion2GUID;
        ObjectGuid uiGrandChampion3GUID;
        ObjectGuid uiChampionLootGUID;
        ObjectGuid uiArgentChampionGUID;

        GuidList VehicleList;

        bool bDone;

        void OnPlayerEnter(Player* player) override
        {
            if (!teamInInstance)
                teamInInstance = player->GetTeam();
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case VEHICLE_ARGENT_WARHORSE:
                case VEHICLE_ARGENT_BATTLEWORG:
                    VehicleList.push_back(creature->GetGUID());
                    break;
                case NPC_EADRIC:
                case NPC_PALETRESS:
                    uiArgentChampionGUID = creature->GetGUID();
                    break;
                case NPC_JAEREN:
                case NPC_ARELAS:
                    uiAnnouncerGUID = creature->GetGUID();
                    break;
                default:
                    break;
            }
        }

        uint32 GetCreatureEntry(ObjectGuid::LowType /*guidLow*/, CreatureData const* data) override
        {
            if (!teamInInstance)
            {
                Map::PlayerList const& players = instance->GetPlayers();
                if (!players.isEmpty())
                    if (Player* player = players.begin()->GetSource())
                        teamInInstance = player->GetTeam();
            }

            uint32 entry = data->id;
            switch (entry)
            {
                case VEHICLE_MOKRA_SKILLCRUSHER_MOUNT:
                    return teamInInstance == HORDE ? VEHICLE_MARSHAL_JACOB_ALERIUS_MOUNT : VEHICLE_MOKRA_SKILLCRUSHER_MOUNT;
                case VEHICLE_ERESSEA_DAWNSINGER_MOUNT:
                    return teamInInstance == HORDE ? VEHICLE_AMBROSE_BOLTSPARK_MOUNT : VEHICLE_ERESSEA_DAWNSINGER_MOUNT;
                case VEHICLE_RUNOK_WILDMANE_MOUNT:
                    return teamInInstance == HORDE ? VEHICLE_COLOSOS_MOUNT : VEHICLE_RUNOK_WILDMANE_MOUNT;
                case VEHICLE_ZUL_TORE_MOUNT:
                    return teamInInstance == HORDE ? VEHICLE_EVENSONG_MOUNT : VEHICLE_ZUL_TORE_MOUNT;
                case VEHICLE_DEATHSTALKER_VESCERI_MOUNT:
                    return teamInInstance == HORDE ? VEHICLE_LANA_STOUTHAMMER_MOUNT : VEHICLE_DEATHSTALKER_VESCERI_MOUNT;
                case NPC_JAEREN:
                    return teamInInstance == HORDE ? NPC_ARELAS : NPC_JAEREN;
                default:
                    return entry;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_MAIN_GATE:
                    uiMainGateGUID = go->GetGUID();
                    break;
                case GO_CHAMPIONS_LOOT:
                case GO_CHAMPIONS_LOOT_H:
                    uiChampionLootGUID = go->GetGUID();
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case BOSS_GRAND_CHAMPIONS:
                    if (state == IN_PROGRESS)
                    {
                        for (ObjectGuid guid : VehicleList)
                            if (Creature* summon = instance->GetCreature(guid))
                                summon->RemoveFromWorld();
                    }
                    else if (state == DONE)
                    {
                        // Final completion is handled by DATA_GRAND_CHAMPION_DEFEATED.
                        // DONE is an encounter state and must not be abused as a per-boss death counter.
                    }
                    break;
                case BOSS_ARGENT_CHALLENGE_E:
                    if (state == DONE)
                    {
                        if (Creature* pAnnouncer = instance->GetCreature(uiAnnouncerGUID))
                        {
                            pAnnouncer->GetMotionMaster()->MovePoint(0, 748.309f, 619.487f, 411.171f);
                            pAnnouncer->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
                            pAnnouncer->SummonGameObject(instance->IsHeroic() ? GO_EADRIC_LOOT_H : GO_EADRIC_LOOT, 746.59f, 618.49f, 411.09f, 1.42f, QuaternionData(), 25h);
                        }
                    }
                    break;
                case BOSS_ARGENT_CHALLENGE_P:
                    if (state == DONE)
                    {
                        if (Creature* pAnnouncer = instance->GetCreature(uiAnnouncerGUID))
                        {
                            pAnnouncer->GetMotionMaster()->MovePoint(0, 748.309f, 619.487f, 411.171f);
                            pAnnouncer->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
                            pAnnouncer->SummonGameObject(instance->IsHeroic() ? GO_PALETRESS_LOOT_H : GO_PALETRESS_LOOT, 746.59f, 618.49f, 411.09f, 1.42f, QuaternionData(), 25h);
                        }
                    }
                    break;
            }
            return true;
        }

        void Update(uint32 diff) override
        {
            if (!bGrandChampionGroundPending)
                return;

            if (uiGrandChampionGroundDelay > diff)
            {
                uiGrandChampionGroundDelay -= diff;
                return;
            }

            uiGrandChampionGroundDelay = 0;
            bGrandChampionGroundPending = false;

            // This activates the already proven FORCE_STAND_GROUND boss AI.
            SetBossState(BOSS_GRAND_CHAMPIONS, IN_PROGRESS);
        }

        void SetData(uint32 uiType, uint32 uiData) override
        {
            switch (uiType)
            {
                case DATA_MOVEMENT_DONE:
                    uiMovementDone = uiData;
                    if (uiMovementDone == 3)
                    {
                        if (Creature* pAnnouncer =  instance->GetCreature(uiAnnouncerGUID))
                            pAnnouncer->AI()->SetData(DATA_IN_POSITION, 0);
                    }
                    break;
                case DATA_GRAND_CHAMPION_MOUNT_DEFEATED:
                {
                    if (uiGrandChampionMountsDefeated < 3)
                        ++uiGrandChampionMountsDefeated;

                    if (uiGrandChampionMountsDefeated == 3)
                    {
                        // Mounted phase is over. Force every player off tournament vehicles.
                        Map::PlayerList const& players = instance->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        {
                            if (Player* player = itr->GetSource())
                            {
                                if (player->GetVehicle())
                                    player->ExitVehicle();
                            }
                        }

                        // Clean up the defeated tournament mounts.
                        ObjectGuid vehicleGuids[3] =
                        {
                            uiGrandChampionVehicle1GUID,
                            uiGrandChampionVehicle2GUID,
                            uiGrandChampionVehicle3GUID
                        };

                        for (ObjectGuid const& guid : vehicleGuids)
                            if (Creature* vehicle = instance->GetCreature(guid))
                                vehicle->DespawnOrUnsummon();

                        ObjectGuid championGuids[3] =
                        {
                            uiGrandChampion1GUID,
                            uiGrandChampion2GUID,
                            uiGrandChampion3GUID
                        };

                        Position const groundPositions[3] =
                        {
                            {739.678f, 662.541f, 412.393f, 4.49f},
                            {746.710f, 661.020f, 411.690f, 4.60f},
                            {754.340f, 660.700f, 412.390f, 4.79f}
                        };

                        for (uint8 i = 0; i < 3; ++i)
                        {
                            if (Creature* champion = instance->GetCreature(championGuids[i]))
                            {
                                if (champion->GetVehicle())
                                    champion->ExitVehicle();

                                champion->CombatStop(true);
                                champion->GetMotionMaster()->Clear();
                                champion->StopMoving();

                                // Remove any fake-death/vehicle residue and force a clean standing update.
                                champion->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);
                                champion->setDeathState(ALIVE);
                                champion->ClearUnitState(UNIT_STATE_DIED);
                                champion->SetEmoteState(EMOTE_STATE_NONE);
                                champion->SetStandState(UNIT_STAND_STATE_STAND);
                                champion->SetFullHealth();

                                // Force a fresh world-position/movement update after the vehicle exit.
                                champion->NearTeleportTo(
                                    groundPositions[i].GetPositionX(),
                                    groundPositions[i].GetPositionY(),
                                    groundPositions[i].GetPositionZ(),
                                    groundPositions[i].GetOrientation()
                                );

                                champion->SetStandState(UNIT_STAND_STATE_STAND);

                                // Wait phase: set protection ONCE and then leave the champion
                                // completely untouched for 30 seconds.
                                champion->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                                champion->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
                                champion->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
                                champion->SetImmuneToPC(true);
                                champion->SetImmuneToNPC(true);
                                champion->SetReactState(REACT_PASSIVE);
                            }
                        }

                        bGrandChampionGroundPending = true;
                        uiGrandChampionGroundDelay = 30000;
                    }
                    break;
                }
                case DATA_GRAND_CHAMPION_DEFEATED:
                {
                    if (uiGrandChampionsDeaths < 3)
                        ++uiGrandChampionsDeaths;

                    if (uiGrandChampionsDeaths == 3)
                    {
                        // Complete the encounter exactly once, after all three ground bosses are dead.
                        SetBossState(BOSS_GRAND_CHAMPIONS, DONE);

                        if (Creature* pAnnouncer = instance->GetCreature(uiAnnouncerGUID))
                        {
                            pAnnouncer->GetMotionMaster()->MovePoint(0, 748.309f, 619.487f, 411.171f);
                            pAnnouncer->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);

                            pAnnouncer->SummonGameObject(
                                instance->IsHeroic() ? GO_CHAMPIONS_LOOT_H : GO_CHAMPIONS_LOOT,
                                746.59f, 618.49f, 411.09f, 1.42f,
                                QuaternionData(), 25h);
                        }
                    }
                    break;
                }
                case DATA_ARGENT_SOLDIER_DEFEATED:
                    uiArgentSoldierDeaths = uiData;
                    if (uiArgentSoldierDeaths == 9)
                    {
                        if (Creature* pBoss = instance->GetCreature(uiArgentChampionGUID))
                        {
                            pBoss->GetMotionMaster()->MovePoint(0, 746.88f, 618.74f, 411.06f);

                            // Eadric/Paletress are spawned with friendly faction 35.
                            // Switch them to hostile and fully enable combat after all 9 soldiers are defeated.
                            pBoss->SetFaction(14);
                            pBoss->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                            pBoss->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
                            pBoss->SetImmuneToPC(false);
                            pBoss->SetReactState(REACT_AGGRESSIVE);
                        }
                    }
                    break;
            }

            if (uiData == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 uiData) const override
        {
            switch (uiData)
            {
                case DATA_MOVEMENT_DONE: return uiMovementDone;
                case DATA_GRAND_CHAMPION_DEFEATED: return uiGrandChampionsDeaths;
                case DATA_ARGENT_SOLDIER_DEFEATED: return uiArgentSoldierDeaths;
            }

            return 0;
        }

        ObjectGuid GetGuidData(uint32 uiData) const override
        {
            switch (uiData)
            {
                case DATA_ANNOUNCER: return uiAnnouncerGUID;
                case DATA_MAIN_GATE: return uiMainGateGUID;

                case DATA_GRAND_CHAMPION_VEHICLE_1: return uiGrandChampionVehicle1GUID;
                case DATA_GRAND_CHAMPION_VEHICLE_2: return uiGrandChampionVehicle2GUID;
                case DATA_GRAND_CHAMPION_VEHICLE_3: return uiGrandChampionVehicle3GUID;
                case DATA_GRAND_CHAMPION_1: return uiGrandChampion1GUID;
                case DATA_GRAND_CHAMPION_2: return uiGrandChampion2GUID;
                case DATA_GRAND_CHAMPION_3: return uiGrandChampion3GUID;
            }

            return ObjectGuid::Empty;
        }

        void SetGuidData(uint32 uiType, ObjectGuid uiData) override
        {
            switch (uiType)
            {
                case DATA_GRAND_CHAMPION_VEHICLE_1:
                    uiGrandChampionVehicle1GUID = uiData;
                    break;
                case DATA_GRAND_CHAMPION_VEHICLE_2:
                    uiGrandChampionVehicle2GUID = uiData;
                    break;
                case DATA_GRAND_CHAMPION_VEHICLE_3:
                    uiGrandChampionVehicle3GUID = uiData;
                    break;
                case DATA_GRAND_CHAMPION_1:
                    uiGrandChampion1GUID = uiData;
                    break;
                case DATA_GRAND_CHAMPION_2:
                    uiGrandChampion2GUID = uiData;
                    break;
                case DATA_GRAND_CHAMPION_3:
                    uiGrandChampion3GUID = uiData;
                    break;
            }
        }

        void WriteSaveDataMore(std::ostringstream& stream) override
        {
            stream << uiGrandChampionsDeaths << ' ' << uiMovementDone;
        }

        void ReadSaveDataMore(std::istringstream& stream) override
        {
            stream >> uiGrandChampionsDeaths >> uiMovementDone;
        }
    };
};

void AddSC_instance_trial_of_the_champion()
{
    new instance_trial_of_the_champion();
}
