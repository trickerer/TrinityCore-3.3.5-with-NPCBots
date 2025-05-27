// Scripted by Biglad & Bob
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "InstanceScript.h"
#include "CreatureAIImpl.h"
#include "ScriptedGossip.h"
#include "Unit.h"

using namespace std::chrono;

enum Spells
{
    SPELL_CLEAVE            = 40504,
    SPELL_LEAP              = 40727,
    SPELL_HOWL_OF_VOID      = 8715,
    SPELL_CHRONO_BURN       = 43757,
    SPELL_ECHO_SLAM         = 53399,
    SPELL_BERSERK           = 37023, 
    SPELL_UNSTABLE_RIFT     = 39175,
};

enum Events
{
    EVENT_CLEAVE = 1,
    EVENT_LEAP = 2,
    EVENT_GNOLL_REINFORCEMENTS = 3,
    EVENT_HOWL_OF_VOID = 4,
    EVENT_CHRONO_BURN = 5,
    EVENT_UNSTABLE_RIFT = 6,
    EVENT_ECHO_SLAM = 7,
    EVENT_MEMORY_OVERLOAD = 8,
};

enum NPCs
{
    NPC_GNOLL_ADDS      = 3494300,
    NPC_PLAYER_CLONE    = 3494301,
    NPC_CHANNELING_ADD  = 3494302,
};

enum Data
{
    DATA_MEMORY_OF_HOGGER = 0,
};

enum Yells
{
    SAY_AGGRO   = 0,
    SAY_PHASE_2 = 1,
    SAY_PHASE_3 = 2,
    SAY_DEATH   = 3
};

enum GnollSpells
{
    SPELL_GNOLL_ATTACK = 40504, 
};

class npc_riverpaw_hideflayer : public CreatureScript
{
public:
    npc_riverpaw_hideflayer() : CreatureScript("npc_riverpaw_hideflayer") { }

    struct npc_riverpaw_hideflayerAI : public ScriptedAI
    {
        npc_riverpaw_hideflayerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            events.Reset();
            events.ScheduleEvent(1, milliseconds(5000)); // Cleave every 5s
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == 1)
                {
                    DoCastVictim(40504); // Cleave
                    events.ScheduleEvent(1, milliseconds(5000));
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_riverpaw_hideflayerAI(creature);
    }
};

class npc_riverpaw_pack_warder : public CreatureScript
{
public:
    npc_riverpaw_pack_warder() : CreatureScript("npc_riverpaw_pack_warder") { }

    struct npc_riverpaw_pack_warderAI : public ScriptedAI
    {
        npc_riverpaw_pack_warderAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            events.Reset();
            events.ScheduleEvent(1, milliseconds(8000)); // Charge
            events.ScheduleEvent(2, milliseconds(6000)); // Rend
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == 1)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, 42702); // Charge
                    events.ScheduleEvent(1, milliseconds(10000));
                }
                else if (eventId == 2)
                {
                    DoCastVictim(11977); // Rend
                    events.ScheduleEvent(2, milliseconds(6000));
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_riverpaw_pack_warderAI(creature);
    }
};

class npc_riverpaw_bone_chanter : public CreatureScript
{
public:
    npc_riverpaw_bone_chanter() : CreatureScript("npc_riverpaw_bone_chanter") { }

    struct npc_riverpaw_bone_chanterAI : public ScriptedAI
    {
        npc_riverpaw_bone_chanterAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            events.Reset();
            events.ScheduleEvent(1, milliseconds(2000)); // Shadowbolt
            events.ScheduleEvent(2, milliseconds(8000)); // Shadow Word: Pain
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == 1)
                {
                    DoCastVictim(9613); // Shadowbolt
                    events.ScheduleEvent(1, milliseconds(3000));
                }
                else if (eventId == 2)
                {
                    DoCastVictim(2767); // Shadow Word: Pain
                    events.ScheduleEvent(2, milliseconds(10000));
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_riverpaw_bone_chanterAI(creature);
    }
};

class npc_memory_gnoll_add : public CreatureScript
{
public:
    npc_memory_gnoll_add() : CreatureScript("npc_memory_gnoll_add") {}

    struct npc_memory_gnoll_addAI : public ScriptedAI
    {
        npc_memory_gnoll_addAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 spellTimer;

        void Reset() override
        {
            spellTimer = 7000; // cast every 7 seconds
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (spellTimer <= diff)
            {
                DoCastVictim(SPELL_GNOLL_ATTACK);
                spellTimer = 7000;
            }
            else
                spellTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_memory_gnoll_addAI(creature);
    }
};

class boss_memory_of_hogger : public CreatureScript
{
public:
    boss_memory_of_hogger() : CreatureScript("boss_memory_of_hogger") {}

    struct boss_memory_of_hoggerAI : public BossAI
    {
        boss_memory_of_hoggerAI(Creature* creature) : BossAI(creature, DATA_MEMORY_OF_HOGGER) {}
        bool phaseTwo = false;
        bool phaseThree = false;

        void Reset() override
        {
            _Reset();
            phaseTwo = false;
            phaseThree = false;
            // Apply freeze/slow/movement-impairing immunities
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SLOW_ATTACK, true);

            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_DECREASE_SPEED, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_ROOT, true);
        }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_CLEAVE, milliseconds(6000));
            events.ScheduleEvent(EVENT_LEAP, milliseconds(20000));
            events.ScheduleEvent(EVENT_GNOLL_REINFORCEMENTS, milliseconds(30000));
            BossAI::JustEngagedWith(who); // important for encounter logic
        }

        void EnterEvadeMode() //override
        {
            BossAI::EnterEvadeMode();
            phaseTwo = false;
            phaseThree = false;
            me->Yell("You have Failed!! Do Not Test Me!!", LANG_UNIVERSAL);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void DamageTaken(Unit* attacker, uint32& damage) //override
        {
            //NOTHING TODO
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            if (!phaseTwo && HealthBelowPct(70))
            {
                phaseTwo = true;
                Talk(SAY_PHASE_2);
                events.Reset();
                events.ScheduleEvent(EVENT_HOWL_OF_VOID, milliseconds(25000));
                events.ScheduleEvent(EVENT_CHRONO_BURN, milliseconds(15000));
                events.ScheduleEvent(EVENT_UNSTABLE_RIFT, milliseconds(10000));
                events.ScheduleEvent(EVENT_GNOLL_REINFORCEMENTS, milliseconds(30000));
            }

            if (!phaseThree && HealthBelowPct(30))
            {
                phaseThree = true;
                Talk(SAY_PHASE_3);
                DoCast(me, SPELL_BERSERK);
                events.Reset();
                DoCast(me, SPELL_BERSERK, true);
                events.ScheduleEvent(EVENT_ECHO_SLAM, milliseconds(15000));
                events.ScheduleEvent(EVENT_MEMORY_OVERLOAD, milliseconds(40000));
                events.ScheduleEvent(EVENT_GNOLL_REINFORCEMENTS, milliseconds(10000));
            }

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CLEAVE:
                        DoCastVictim(SPELL_CLEAVE);
                        events.ScheduleEvent(EVENT_CLEAVE, milliseconds(6000));
                        break;

                    case EVENT_LEAP:
                    {
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 200, true))
                            DoCast(target, SPELL_LEAP);
                        events.ScheduleEvent(EVENT_LEAP, milliseconds(20000));
                        break;
                    }

                    case EVENT_GNOLL_REINFORCEMENTS:
                        me->Yell("Come Forth My Minions, Assist Me!", LANG_UNIVERSAL);
                        for (int i = 0; i < 3; ++i)
                        {
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 200, true))
                            {
                                Creature* gnoll = me->SummonCreature(NPC_GNOLL_ADDS,
                                -90.582489,
                                136.845551,
                                -40.375099,
                                0.f,
                                TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,
                                milliseconds(30000));

                                if (gnoll && me->GetVictim())
                                    gnoll->AI()->AttackStart(me->GetVictim());
                            }
                        }
                        events.ScheduleEvent(EVENT_GNOLL_REINFORCEMENTS, milliseconds(50000));
                        break;

                    case EVENT_HOWL_OF_VOID:
                        DoCast(me, SPELL_HOWL_OF_VOID);
                        //me->Yell("VOID!!!", LANG_UNIVERSAL);
                        for (int i = 0; i < 2; ++i)
                        {
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 200, true))
                            {
                                Creature* clone = me->SummonCreature(NPC_PLAYER_CLONE,
                                target->GetPositionX(),
                                target->GetPositionY(),
                                target->GetPositionZ(),
                                0.f,
                                TEMPSUMMON_TIMED_DESPAWN,
                                milliseconds(20000));

                                if (clone && target)
                                    clone->AI()->AttackStart(target);
                            }
                        }
                        events.ScheduleEvent(EVENT_HOWL_OF_VOID, milliseconds(45000));
                        break;

                    case EVENT_CHRONO_BURN:
                        //me->Yell("CHRONO BURN!!", LANG_UNIVERSAL);
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 200, true))
                            DoCast(target, SPELL_CHRONO_BURN);
                        events.ScheduleEvent(EVENT_CHRONO_BURN, milliseconds(15000));
                        break;

                    case EVENT_UNSTABLE_RIFT:
                        // Optional hazard here
                        DoCast(me, SPELL_UNSTABLE_RIFT);
                        //me->Yell("SUNSTABLE RIFT NOT ACTIVE YET!", LANG_UNIVERSAL);
                        events.ScheduleEvent(EVENT_UNSTABLE_RIFT, milliseconds(45000));
                        break;

                    case EVENT_ECHO_SLAM:
                        //me->Yell("SLAM", LANG_UNIVERSAL);
                        DoCast(me, SPELL_ECHO_SLAM);
                        events.ScheduleEvent(EVENT_ECHO_SLAM, milliseconds(15000));
                        break;

                    case EVENT_MEMORY_OVERLOAD:
                        //me->Yell("MEMORY OVER LOAD!", LANG_UNIVERSAL);
                        for (int i = 0; i < 4; ++i)
                        {
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 200, true))
                            {    
                                Creature* channeler = me->SummonCreature(NPC_CHANNELING_ADD,
                                -90.582489,
                                136.845551,
                                -40.375099,
                                0.f,
                                TEMPSUMMON_TIMED_DESPAWN,
                                milliseconds(50000));

                            if (channeler && me->GetVictim())
                                channeler->AI()->AttackStart(me->GetVictim());
                            }
                        }
                        events.ScheduleEvent(EVENT_MEMORY_OVERLOAD, milliseconds(50000));
                        break;

                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_memory_of_hoggerAI(creature);
    }
};

void AddSC_boss_memory_of_hogger()
{
    new boss_memory_of_hogger();
    new npc_memory_gnoll_add();
    new npc_riverpaw_hideflayer();
    new npc_riverpaw_pack_warder();
    new npc_riverpaw_bone_chanter();
}