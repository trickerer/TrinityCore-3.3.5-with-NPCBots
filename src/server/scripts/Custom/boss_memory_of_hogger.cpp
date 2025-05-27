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
    SPELL_LEAP              = 57057,
    SPELL_HOWL_OF_VOID      = 38684,
    SPELL_CHRONO_BURN       = 67479,
    SPELL_ECHO_SLAM         = 32014,
    SPELL_BERSERK           = 47008,  // Added since you use it in phase 3
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
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void DamageTaken(Unit* attacker, uint32& damage) //override
        {
            if (!phaseTwo && HealthBelowPct(70))
            {
                me->Yell("PHASE2", LANG_UNIVERSAL);
                phaseTwo = true;
                Talk(SAY_PHASE_2);
                events.Reset();
                events.ScheduleEvent(EVENT_HOWL_OF_VOID, milliseconds(25000));
                events.ScheduleEvent(EVENT_CHRONO_BURN, milliseconds(15000));
                events.ScheduleEvent(EVENT_UNSTABLE_RIFT, milliseconds(10000));
            }
            else if (!phaseThree && HealthBelowPct(30))
            {
                me->Yell("PHASE3", LANG_UNIVERSAL);
                phaseThree = true;
                Talk(SAY_PHASE_3);
                events.Reset();
                DoCast(me, SPELL_BERSERK, true);
                events.ScheduleEvent(EVENT_ECHO_SLAM, milliseconds(15000));
                events.ScheduleEvent(EVENT_MEMORY_OVERLOAD, milliseconds(40000));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

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
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 50, true))
                            DoCast(target, SPELL_LEAP);
                        events.ScheduleEvent(EVENT_LEAP, milliseconds(20000));
                        break;
                    }

                    case EVENT_GNOLL_REINFORCEMENTS:
                        me->Yell("Come Forth My Minions, Assist Me!", LANG_UNIVERSAL);
                        for (int i = 0; i < 3; ++i)
                        {
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 50, true))
                            {
                                Creature* gnoll = me->SummonCreature(NPC_GNOLL_ADDS,
                                me->GetPositionX() + irand(-5, 5),
                                me->GetPositionY() + irand(-5, 5),
                                me->GetPositionZ(),
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
                        me->Yell("VOID!!!", LANG_UNIVERSAL);
                        for (int i = 0; i < 2; ++i)
                        {
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 50, true))
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
                        events.ScheduleEvent(EVENT_HOWL_OF_VOID, milliseconds(25000));
                        break;

                    case EVENT_CHRONO_BURN:
                        me->Yell("CHRONO BURN!!", LANG_UNIVERSAL);
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 50, true))
                            DoCast(target, SPELL_CHRONO_BURN);
                        events.ScheduleEvent(EVENT_CHRONO_BURN, milliseconds(15000));
                        break;

                    case EVENT_UNSTABLE_RIFT:
                        // Optional hazard here
                        events.ScheduleEvent(EVENT_UNSTABLE_RIFT, milliseconds(45000));
                        break;

                    case EVENT_ECHO_SLAM:
                        me->Yell("SLAM", LANG_UNIVERSAL);
                        DoCast(me, SPELL_ECHO_SLAM);
                        events.ScheduleEvent(EVENT_ECHO_SLAM, milliseconds(15000));
                        break;

                    case EVENT_MEMORY_OVERLOAD:
                        me->Yell("MEMORY OVER LOAD!", LANG_UNIVERSAL);
                        for (int i = 0; i < 4; ++i)
                        {
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 50, true))
                            {    
                                Creature* channeler = me->SummonCreature(NPC_CHANNELING_ADD,
                                me->GetPositionX() + irand(-8, 8),
                                me->GetPositionY() + irand(-8, 8),
                                me->GetPositionZ(),
                                0.f,
                                TEMPSUMMON_TIMED_DESPAWN,
                                milliseconds(20000));

                            if (channeler && me->GetVictim())
                                channeler->AI()->AttackStart(me->GetVictim());
                            }
                        }
                        events.ScheduleEvent(EVENT_MEMORY_OVERLOAD, milliseconds(40000));
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
}