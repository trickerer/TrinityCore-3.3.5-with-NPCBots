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
SDName: Argent Challenge Encounter.
SD%Complete: 50 %
SDComment: AI for Argent Soldiers are not implemented. AI from bosses need more improvements.
SDCategory: Trial of the Champion
EndScriptData */

#include "ScriptMgr.h"
#include "Containers.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedEscortAI.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "trial_of_the_champion.h"
#include "Player.h"
/*
enum Yells
{
    // Eadric the Pure
    SAY_INTRO                   = 0,
    SAY_AGGRO                   = 1,
    EMOTE_RADIANCE              = 2,
    EMOTE_HAMMER_RIGHTEOUS      = 3,
    SAY_HAMMER_RIGHTEOUS        = 4,
    SAY_KILL_PLAYER             = 5,
    SAY_DEFEATED                = 6,

    // Argent Confessor Paletress
    SAY_INTRO_1                 = 0,
    SAY_INTRO_2                 = 1,
    SAY_AGGRO                   = 2,
    SAY_MEMORY_SUMMON           = 3,
    SAY_MEMORY_DEATH            = 4,
    SAY_KILL_PLAYER             = 5,
    SAY_DEFEATED                = 6,

    // Memory of X
    EMOTE_WAKING_NIGHTMARE      = 0
};
*/
enum Spells
{
    // Server-side Trial of the Champion achievement marker for Eadric/Paletress.
    SPELL_PALETRESS_CREDIT       = 68574,
    SPELL_EADRIC_CREDIT          = 68575,
    // Server-side Trial of the Champion achievement credit markers
    // Eadric the Pure
    SPELL_EADRIC_ACHIEVEMENT    = 68197,
    SPELL_HAMMER_JUSTICE        = 66863,
    SPELL_HAMMER_RIGHTEOUS      = 66867,
    SPELL_RADIANCE              = 66935,
    SPELL_VENGEANCE             = 66865,

    // Paletress
    SPELL_SMITE                 = 66536,
    SPELL_SMITE_H               = 67674,
    SPELL_HOLY_FIRE             = 66538,
    SPELL_HOLY_FIRE_H           = 67676,
    SPELL_RENEW                 = 66537,
    SPELL_RENEW_H               = 67675,
    SPELL_HOLY_NOVA             = 66546,
    SPELL_SHIELD                = 66515,
    SPELL_CONFESS               = 66680,
    SPELL_SUMMON_MEMORY         = 66545,

    // Memory of X (Summon)
    SPELL_MEMORY_ALGALON        = 66715,
    SPELL_MEMORY_ARCHIMONDE     = 66704,
    SPELL_MEMORY_CHROMAGGUS     = 66697,
    SPELL_MEMORY_CYANIGOSA      = 66709,
    SPELL_MEMORY_DELRISSA       = 66706,
    SPELL_MEMORY_ECK            = 66710,
    SPELL_MEMORY_ENTROPIUS      = 66707,
    SPELL_MEMORY_GRUUL          = 66702,
    SPELL_MEMORY_HAKKAR         = 66698,
    SPELL_MEMORY_HEIGAN         = 66712,
    SPELL_MEMORY_HEROD          = 66694,
    SPELL_MEMORY_HOGGER         = 66543,
    SPELL_MEMORY_IGNIS          = 66713,
    SPELL_MEMORY_ILLIDAN        = 66705,
    SPELL_MEMORY_INGVAR         = 66708,
    SPELL_MEMORY_KALITHRESH     = 66700,
    SPELL_MEMORY_LUCIFRON       = 66695,
    SPELL_MEMORY_MALCHEZAAR     = 66701,
    SPELL_MEMORY_MUTANUS        = 66692,
    SPELL_MEMORY_ONYXIA         = 66711,
    SPELL_MEMORY_THUNDERAAN     = 66696,
    SPELL_MEMORY_VANCLEEF       = 66691,
    SPELL_MEMORY_VASHJ          = 66703,
    SPELL_MEMORY_VEKNILASH      = 66699,
    SPELL_MEMORY_VEZAX          = 66714,

    // Memory
    SPELL_OLD_WOUNDS            = 66620,
    SPELL_OLD_WOUNDS_H          = 67679,
    SPELL_SHADOWS_PAST          = 66619,
    SPELL_SHADOWS_PAST_H        = 67678,
    SPELL_WAKING_NIGHTMARE      = 66552,
    SPELL_WAKING_NIGHTMARE_H    = 67677
};

class OrientationCheck
{
    public:
        explicit OrientationCheck(Unit* _caster) : caster(_caster) { }
        bool operator()(WorldObject* object)
        {
            return !object->isInFront(caster, 2.5f) || !object->IsWithinDist(caster, 40.0f);
        }

    private:
        Unit* caster;
};

// 66862, 67681 - Radiance
class spell_eadric_radiance : public SpellScriptLoader
{
    public:
        spell_eadric_radiance() : SpellScriptLoader("spell_eadric_radiance") { }
        class spell_eadric_radiance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_radiance_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(OrientationCheck(GetCaster()));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_eadric_radiance_SpellScript();
        }
};

class boss_eadric : public CreatureScript
{
public:
    boss_eadric() : CreatureScript("boss_eadric") { }
    struct boss_eadricAI : public ScriptedAI
    {
        boss_eadricAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
            instance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
            creature->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        }

        void Initialize()
        {
            uiVenganceTimer = 10000;
            uiRadianceTimer = 16000;
            uiHammerJusticeTimer = 25000;
            uiResetTimer = 5000;

            bDone = false;
        }

        InstanceScript* instance;

        uint32 uiVenganceTimer;
        uint32 uiRadianceTimer;
        uint32 uiHammerJusticeTimer;
        uint32 uiResetTimer;

        bool bDone;

        void Reset() override
        {
            Initialize();
        }

        void DamageTaken(Unit* /*done_by*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            if (damage >= me->GetHealth() && !bDone)
            {
                // Scripted defeat: do NOT evade/home-run through the gate.
                damage = 0;
                me->SetHealth(1);
                me->AttackStop();
                me->CombatStop(true);
                me->SetReactState(REACT_PASSIVE);
                me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                me->SetImmuneToPC(true);

                // 68574 searches for ENEMY units around the caster.
                // Cast it while the boss is still hostile so players are valid targets.
                me->CastSpell((Unit*)nullptr, SPELL_EADRIC_CREDIT, true);

                if (instance)
                    instance->DoUpdateAchievementCriteria(
                        ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET,
                        SPELL_EADRIC_CREDIT);

                me->SetFaction(FACTION_FRIENDLY);

                // Eadric does not actually die in this scripted defeat flow,
                // so explicitly fire his server-side achievement credit marker.

                bDone = true;
                uiResetTimer = 3000;
            }
        }

        void MovementInform(uint32 MovementType, uint32 /*Data*/) override
        {
            if (MovementType != POINT_MOTION_TYPE)
                return;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bDone)
            {
                if (uiResetTimer <= uiDiff)
                {
                    // Encounter is already won at 1 HP. Finish it here so loot/announcer
                    // do not depend on pathfinding or MovementInform.
                    instance->SetBossState(BOSS_ARGENT_CHALLENGE_E, DONE);

                    me->GetMotionMaster()->MovePoint(0, 746.87f, 665.87f, 411.75f);
                    me->DespawnOrUnsummon(5s);

                    bDone = false;
                    uiResetTimer = 0;
                }
                else
                    uiResetTimer -= uiDiff;

                return;
            }

            if (!UpdateVictim())
                return;

            if (uiHammerJusticeTimer <= uiDiff)
            {
                me->InterruptNonMeleeSpells(true);

                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 250, true))
                {
                    if (target->IsAlive())
                    {
                        DoCast(target, SPELL_HAMMER_JUSTICE);
                        DoCast(target, SPELL_HAMMER_RIGHTEOUS);
                    }
                }
                uiHammerJusticeTimer = 25000;
            } else uiHammerJusticeTimer -= uiDiff;

            if (uiVenganceTimer <= uiDiff)
            {
                DoCast(me, SPELL_VENGEANCE);

                uiVenganceTimer = 10000;
            } else uiVenganceTimer -= uiDiff;

            if (uiRadianceTimer <= uiDiff)
            {
                DoCastAOE(SPELL_RADIANCE);

                uiRadianceTimer = 16000;
            } else uiRadianceTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<boss_eadricAI>(creature);
    }
};

class boss_paletress : public CreatureScript
{
public:
    boss_paletress() : CreatureScript("boss_paletress") { }

    struct boss_paletressAI : public ScriptedAI
    {
        boss_paletressAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
            instance = creature->GetInstanceScript();

            creature->SetReactState(REACT_PASSIVE);
            creature->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            creature->RestoreFaction();
        }

        void Initialize()
        {
            uiHolyFireTimer = urand(9000, 12000);
            uiHolySmiteTimer = urand(5000, 7000);
            uiRenewTimer = urand(2000, 5000);

            uiResetTimer = 7000;

            bHealth = false;
            bDone = false;
        }

        InstanceScript* instance;
        ObjectGuid MemoryGUID;

        bool bHealth;
        bool bDone;

        uint32 uiHolyFireTimer;
        uint32 uiHolySmiteTimer;
        uint32 uiRenewTimer;
        uint32 uiResetTimer;

        void Reset() override
        {
            me->RemoveAllAuras();

            Initialize();

            if (Creature* pMemory = ObjectAccessor::GetCreature(*me, MemoryGUID))
                if (pMemory->IsAlive())
                    pMemory->RemoveFromWorld();
        }

        void SetData(uint32 uiId, uint32 /*uiValue*/) override
        {
            if (uiId == 1)
                me->RemoveAura(SPELL_SHIELD);
        }

        void DamageTaken(Unit* /*done_by*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
        {
            if (damage >= me->GetHealth() && !bDone)
            {
                // Scripted defeat: do NOT evade/home-run through the gate.
                damage = 0;
                me->SetHealth(1);
                me->AttackStop();
                me->CombatStop(true);
                me->SetReactState(REACT_PASSIVE);
                me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                me->SetImmuneToPC(true);

                // 68574 searches for ENEMY units around the caster.
                // Cast it while the boss is still hostile so players are valid targets.
                me->CastSpell((Unit*)nullptr, SPELL_PALETRESS_CREDIT, true);

                if (instance)
                    instance->DoUpdateAchievementCriteria(
                        ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET,
                        SPELL_PALETRESS_CREDIT);

                me->SetFaction(FACTION_FRIENDLY);

                // Paletress also survives at 1 HP, so normal creature death credit
                // never happens. Fire the server-side achievement marker explicitly.

                bDone = true;
                uiResetTimer = 3000;
            }
        }

        void MovementInform(uint32 MovementType, uint32 Point) override
        {
            if (MovementType != POINT_MOTION_TYPE || Point != 0)
                return;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bDone)
            {
                if (uiResetTimer <= uiDiff)
                {
                    // Encounter is already won at 1 HP. Finish it here so loot/announcer
                    // do not depend on pathfinding or MovementInform.
                    instance->SetBossState(BOSS_ARGENT_CHALLENGE_P, DONE);

                    me->GetMotionMaster()->MovePoint(0, 746.87f, 665.87f, 411.75f);
                    me->DespawnOrUnsummon(5s);

                    bDone = false;
                    uiResetTimer = 0;
                }
                else
                    uiResetTimer -= uiDiff;

                return;
            }

            if (!UpdateVictim())
                return;

            if (uiHolyFireTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 250, true))
                {
                    if (target->IsAlive())
                        DoCast(target, SPELL_HOLY_FIRE);
                }
                 if (me->HasAura(SPELL_SHIELD))
                    uiHolyFireTimer = 13000;
                else
                    uiHolyFireTimer = urand(9000, 12000);
            } else uiHolyFireTimer -= uiDiff;

            if (uiHolySmiteTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 250, true))
                {
                    if (target->IsAlive())
                        DoCast(target, SPELL_SMITE);
                }
                if (me->HasAura(SPELL_SHIELD))
                    uiHolySmiteTimer = 9000;
                else
                    uiHolySmiteTimer = urand(5000, 7000);
            } else uiHolySmiteTimer -= uiDiff;

            if (me->HasAura(SPELL_SHIELD))
            {
                if (uiRenewTimer <= uiDiff)
                {
                    me->InterruptNonMeleeSpells(true);
                    uint8 uiTarget = urand(0, 1);
                    switch (uiTarget)
                    {
                        case 0:
                            DoCast(me, SPELL_RENEW);
                            break;
                        case 1:
                            if (Creature* pMemory = ObjectAccessor::GetCreature(*me, MemoryGUID))
                                if (pMemory->IsAlive())
                                    DoCast(pMemory, SPELL_RENEW);
                            break;
                    }
                    uiRenewTimer = urand(15000, 17000);
                } else uiRenewTimer -= uiDiff;
            }

            if (!bHealth && !HealthAbovePct(25))
            {
                me->InterruptNonMeleeSpells(true);
                DoCastAOE(SPELL_HOLY_NOVA, false);
                DoCast(me, SPELL_SHIELD);
                DoCastAOE(SPELL_SUMMON_MEMORY, false);
                DoCastAOE(SPELL_CONFESS, false);

                bHealth = true;
            }

            DoMeleeAttackIfReady();
        }

        void JustSummoned(Creature* summon) override
        {
            MemoryGUID = summon->GetGUID();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<boss_paletressAI>(creature);
    }
};

class npc_memory : public CreatureScript
{
public:
    npc_memory() : CreatureScript("npc_memory") { }

    struct npc_memoryAI : public ScriptedAI
    {
        npc_memoryAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            uiOldWoundsTimer = 12000;
            uiShadowPastTimer = 5000;
            uiWakingNightmare = 7000;
        }

        uint32 uiOldWoundsTimer;
        uint32 uiShadowPastTimer;
        uint32 uiWakingNightmare;

        void Reset() override
        {
            Initialize();
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiOldWoundsTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                {
                    if (target->IsAlive())
                        DoCast(target, SPELL_OLD_WOUNDS);
                }
                uiOldWoundsTimer = 12000;
            }else uiOldWoundsTimer -= uiDiff;

            if (uiWakingNightmare <= uiDiff)
            {
                DoCast(me, SPELL_WAKING_NIGHTMARE);
                uiWakingNightmare = 7000;
            }else uiWakingNightmare -= uiDiff;

            if (uiShadowPastTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 1))
                {
                    if (target->IsAlive())
                        DoCast(target, SPELL_SHADOWS_PAST);
                }
                uiShadowPastTimer = 5000;
            }else uiShadowPastTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (TempSummon* summ = me->ToTempSummon())
                if (Unit* summoner = summ->GetSummonerUnit())
                    if (summoner->IsAlive())
                        summoner->GetAI()->SetData(1, 0);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_memoryAI>(creature);
    }
};

class npc_argent_soldier : public CreatureScript
{
public:
    npc_argent_soldier() : CreatureScript("npc_argent_soldier") { }

    struct npc_argent_soldierAI : public EscortAI
    {
        npc_argent_soldierAI(Creature* creature) : EscortAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_DEFENSIVE);
            SetDespawnAtEnd(false);
            Initialize();
        }

        InstanceScript* instance;
        uint8 uiWaypoint;
        uint32 abilityTimer1;
        uint32 abilityTimer2;
        uint32 abilityTimer3;
        uint32 smiteTimer;
        bool finalMeditationDone;

        enum ArgentSoldierSpells
        {
            // Argent Lightwielder - 35309
            SPELL_CLEAVE               = 15284,
            SPELL_UNBALANCING_STRIKE   = 26613,
            SPELL_BLAZING_LIGHT        = 67254,

            // Argent Monk - 35305
            SPELL_FLURRY_OF_BLOWS      = 67233,
            SPELL_PUMMEL               = 67235,
            SPELL_FINAL_MEDITATION     = 67255,

            // Argent Priestess - 35307
            SPELL_FOUNTAIN_OF_LIGHT    = 67194,
            SPELL_MIND_CONTROL         = 67229,
            SPELL_PRIESTESS_SMITE      = 48123,
            SPELL_PRIESTESS_SWP        = 48125,
        };

        void Initialize()
        {
            uiWaypoint = 0;
            abilityTimer1 = urand(4000, 7000);
            abilityTimer2 = urand(7000, 11000);
            abilityTimer3 = urand(1000, 2000);
            finalMeditationDone = false;
            smiteTimer = urand(3000, 5000);

            // Priestess should establish her Fountain shortly after combat begins.
            if (me->GetEntry() == NPC_PRIESTESS)
            {
                abilityTimer1 = urand(5000, 8000);   // Shadow Word: Pain
                abilityTimer2 = urand(12000, 18000); // Heroic Mind Control
                abilityTimer3 = urand(2000, 4000);   // Fountain of Light
            }
        }

        void Reset() override
        {
            Initialize();
            me->SetReactState(REACT_DEFENSIVE);
        }

        void WaypointReached(uint32 waypointId, uint32 /*pathId*/) override
        {
            if (waypointId != 0)
                return;

            switch (uiWaypoint)
            {
                case 0: me->SetFacingTo(5.81f); break;
                case 1: me->SetFacingTo(4.60f); break;
                case 2: me->SetFacingTo(2.79f); break;
            }
        }

        void SetData(uint32 uiType, uint32 /*uiData*/) override
        {
            switch (me->GetEntry())
            {
                case NPC_ARGENT_LIGHWIELDER:
                    switch (uiType)
                    {
                        case 0: AddWaypoint(0, 712.14f, 628.42f, 411.88f, true); break;
                        case 1: AddWaypoint(0, 742.44f, 650.29f, 411.79f, true); break;
                        case 2: AddWaypoint(0, 783.33f, 615.29f, 411.84f, true); break;
                    }
                    break;
                case NPC_ARGENT_MONK:
                    switch (uiType)
                    {
                        case 0: AddWaypoint(0, 713.12f, 632.97f, 411.90f, true); break;
                        case 1: AddWaypoint(0, 746.73f, 650.24f, 411.56f, true); break;
                        case 2: AddWaypoint(0, 781.32f, 610.54f, 411.82f, true); break;
                    }
                    break;
                case NPC_PRIESTESS:
                    switch (uiType)
                    {
                        case 0: AddWaypoint(0, 715.06f, 637.07f, 411.91f, true); break;
                        case 1: AddWaypoint(0, 750.72f, 650.20f, 411.77f, true); break;
                        case 2: AddWaypoint(0, 779.77f, 607.03f, 411.81f, true); break;
                    }
                    break;
            }

            Start(false);
            uiWaypoint = uiType;
        }

        void AttackStart(Unit* who) override
        {
            if (!who)
                return;

            // Priestess is a ranged caster and must not chase into melee.
            if (me->GetEntry() == NPC_PRIESTESS)
                AttackStartCaster(who, 25.0f);
            else
                EscortAI::AttackStart(who);
        }

        void JustEngagedWith(Unit* who) override
        {
            if (!who)
                return;

            me->SetReactState(REACT_AGGRESSIVE);

            // Pull exactly the nearby trio. The three packs are far enough apart
            // that a 15 yd radius links one pack without waking the other two.
            std::list<Creature*> members;
            GetCreatureListWithEntryInGrid(members, me, NPC_ARGENT_LIGHWIELDER, 15.0f);
            GetCreatureListWithEntryInGrid(members, me, NPC_ARGENT_MONK, 15.0f);
            GetCreatureListWithEntryInGrid(members, me, NPC_PRIESTESS, 15.0f);

            for (Creature* member : members)
            {
                if (!member || member == me || !member->IsAlive() || member->IsInCombat())
                    continue;

                member->SetReactState(REACT_AGGRESSIVE);
                member->AI()->AttackStart(who);
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
        {
            if (me->GetEntry() == NPC_ARGENT_MONK && me->GetMap()->IsHeroic() && !finalMeditationDone && damage >= me->GetHealth())
            {
                finalMeditationDone = true;
                damage = 0;
                me->SetHealth(1);
                me->AttackStop();
                me->CombatStop(false);
                DoCast(me, SPELL_FINAL_MEDITATION, true);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            EscortAI::UpdateAI(diff);

            if (!UpdateVictim())
                return;

            switch (me->GetEntry())
            {
                case NPC_ARGENT_LIGHWIELDER:
                {
                    if (abilityTimer1 <= diff)
                    {
                        DoCastVictim(SPELL_CLEAVE);
                        abilityTimer1 = urand(5000, 8000);
                    }
                    else
                        abilityTimer1 -= diff;

                    if (abilityTimer2 <= diff)
                    {
                        DoCastVictim(SPELL_UNBALANCING_STRIKE);
                        abilityTimer2 = urand(9000, 13000);
                    }
                    else
                        abilityTimer2 -= diff;

                    if (abilityTimer3 <= diff)
                    {
                        if (Unit* target = DoSelectLowestHpFriendly(30.0f, 1))
                            DoCast(target, SPELL_BLAZING_LIGHT);
                        abilityTimer3 = urand(10000, 15000);
                    }
                    else
                        abilityTimer3 -= diff;

                    DoMeleeAttackIfReady();
                    break;
                }

                case NPC_ARGENT_MONK:
                {
                    if (abilityTimer1 <= diff)
                    {
                        DoCast(me, SPELL_FLURRY_OF_BLOWS);
                        abilityTimer1 = urand(10000, 15000);
                    }
                    else
                        abilityTimer1 -= diff;

                    if (abilityTimer2 <= diff)
                    {
                        if (Unit* victim = me->GetVictim())
                            if (victim->IsNonMeleeSpellCast(false))
                                DoCast(victim, SPELL_PUMMEL);
                        abilityTimer2 = urand(7000, 11000);
                    }
                    else
                        abilityTimer2 -= diff;

                    DoMeleeAttackIfReady();
                    break;
                }

                case NPC_PRIESTESS:
                {
                    // Documented ToC behaviour:
                    // Fountain of Light, Holy Smite, Shadow Word: Pain;
                    // Heroic additionally uses Mind Control.
                    Unit* victim = me->GetVictim();
                    if (!victim)
                        return;

                    // Keep mana available so the caster AI cannot stall.
                    if (me->GetMaxPower(POWER_MANA) > 0 &&
                        me->GetPower(POWER_MANA) < me->CountPctFromMaxPower(POWER_MANA, 30))
                    {
                        me->SetPower(POWER_MANA, me->CountPctFromMaxPower(POWER_MANA, 70));
                    }

                    // IMPORTANT: all timers tick every update, even while another spell is casting.
                    if (abilityTimer1 > diff)
                        abilityTimer1 -= diff;
                    else
                        abilityTimer1 = 0;

                    if (abilityTimer3 > diff)
                        abilityTimer3 -= diff;
                    else
                        abilityTimer3 = 0;

                    if (smiteTimer > diff)
                        smiteTimer -= diff;
                    else
                        smiteTimer = 0;

                    if (me->GetMap()->IsHeroic())
                    {
                        if (abilityTimer2 > diff)
                            abilityTimer2 -= diff;
                        else
                            abilityTimer2 = 0;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;

                    // Highest priority: establish/refresh the healing fountain.
                    if (abilityTimer3 == 0)
                    {
                        DoCast(me, SPELL_FOUNTAIN_OF_LIGHT);
                        abilityTimer3 = urand(28000, 35000);
                        return;
                    }

                    // Heroic: interruptible Mind Control attempt.
                    if (me->GetMap()->IsHeroic() && abilityTimer2 == 0)
                    {
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 30.0f, true))
                            DoCast(target, SPELL_MIND_CONTROL);

                        abilityTimer2 = urand(18000, 25000);
                        return;
                    }

                    // Maintain Shadow Word: Pain periodically.
                    if (abilityTimer1 == 0)
                    {
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 30.0f, true))
                            DoCast(target, SPELL_PRIESTESS_SWP);

                        abilityTimer1 = urand(12000, 16000);
                        return;
                    }

                    // Holy Smite has a short cooldown; it is a filler, not a machine gun.
                    if (smiteTimer == 0)
                    {
                        DoCast(victim, SPELL_PRIESTESS_SMITE);
                        smiteTimer = urand(4000, 6000);
                        return;
                    }

                    // Intentionally do nothing between casts. No melee fallback.
                    break;
                }
                default:
                    DoMeleeAttackIfReady();
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (instance)
                instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, instance->GetData(DATA_ARGENT_SOLDIER_DEFEATED) + 1);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_argent_soldierAI>(creature);
    }
};

uint32 const memorySpellId[25] =
{
    SPELL_MEMORY_ALGALON,
    SPELL_MEMORY_ARCHIMONDE,
    SPELL_MEMORY_CHROMAGGUS,
    SPELL_MEMORY_CYANIGOSA,
    SPELL_MEMORY_DELRISSA,
    SPELL_MEMORY_ECK,
    SPELL_MEMORY_ENTROPIUS,
    SPELL_MEMORY_GRUUL,
    SPELL_MEMORY_HAKKAR,
    SPELL_MEMORY_HEIGAN,
    SPELL_MEMORY_HEROD,
    SPELL_MEMORY_HOGGER,
    SPELL_MEMORY_IGNIS,
    SPELL_MEMORY_ILLIDAN,
    SPELL_MEMORY_INGVAR,
    SPELL_MEMORY_KALITHRESH,
    SPELL_MEMORY_LUCIFRON,
    SPELL_MEMORY_MALCHEZAAR,
    SPELL_MEMORY_MUTANUS,
    SPELL_MEMORY_ONYXIA,
    SPELL_MEMORY_THUNDERAAN,
    SPELL_MEMORY_VANCLEEF,
    SPELL_MEMORY_VASHJ,
    SPELL_MEMORY_VEKNILASH,
    SPELL_MEMORY_VEZAX
};

// 66545 - Summon Memory
class spell_paletress_summon_memory : public SpellScriptLoader
{
    public:
        spell_paletress_summon_memory() : SpellScriptLoader("spell_paletress_summon_memory") { }

        class spell_paletress_summon_memory_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_paletress_summon_memory_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(memorySpellId);
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                targets.clear();
                targets.push_back(target);
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                GetHitUnit()->CastSpell(GetHitUnit(), memorySpellId[urand(0, 24)], GetCaster()->GetGUID());
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_paletress_summon_memory_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_paletress_summon_memory_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_paletress_summon_memory_SpellScript();
        }
};

void AddSC_boss_argent_challenge()
{
    new boss_eadric();
    new spell_eadric_radiance();
    new boss_paletress();
    new npc_memory();
    new npc_argent_soldier();
    new spell_paletress_summon_memory();
}
