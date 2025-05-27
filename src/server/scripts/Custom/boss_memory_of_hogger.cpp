// Scripted by Biglad & Bob
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "CreatureAIImpl.h"

enum Spells
{
    SPELL_CLEAVE            = 40504,
    SPELL_LEAP              = 57057,
    SPELL_HOWL_OF_VOID      = 38684,
    SPELL_CHRONO_BURN       = 67479,
    SPELL_ECHO_SLAM         = 32014,
};

enum Events
{
    EVENT_CLEAVE = 1,
    EVENT_LEAP,
    EVENT_GNOLL_REINFORCEMENTS,
    EVENT_HOWL_OF_VOID,
    EVENT_CHRONO_BURN,
    EVENT_UNSTABLE_RIFT,
    EVENT_ECHO_SLAM,
    EVENT_MEMORY_OVERLOAD,
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
    // add other boss IDs here if needed
};

class boss_memory_of_hogger : public CreatureScript
{
public:
    boss_memory_of_hogger() : CreatureScript("boss_memory_of_hogger") {}

    struct boss_memory_of_hoggerAI : public BossAI
    {
        boss_memory_of_hoggerAI(Creature* creature) : BossAI(creature, DATA_MEMORY_OF_HOGGER) {}

        void Reset() override
        {
            _Reset();
        }

        void EnterCombat(Unit* /*who*/) //override
        {
            Talk(0); // Aggro
            events.ScheduleEvent(EVENT_CLEAVE, 6000);
            events.ScheduleEvent(EVENT_LEAP, 20000);
            events.ScheduleEvent(EVENT_GNOLL_REINFORCEMENTS, 30000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(3); // Death
            _JustDied();
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) //override
        {
            if (!phaseTwo && HealthBelowPct(70))
            {
                phaseTwo = true;
                Talk(1); // Phase 2
                events.Reset();
                events.ScheduleEvent(EVENT_HOWL_OF_VOID, 25000);
                events.ScheduleEvent(EVENT_CHRONO_BURN, 15000);
                events.ScheduleEvent(EVENT_UNSTABLE_RIFT, 10000);
            }
            else if (!phaseThree && HealthBelowPct(30))
            {
                phaseThree = true;
                Talk(2); // Phase 3
                events.Reset();
                DoCast(me, SPELL_BERSERK, true);
                events.ScheduleEvent(EVENT_ECHO_SLAM, 15000);
                events.ScheduleEvent(EVENT_MEMORY_OVERLOAD, 40000);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
                case EVENT_CLEAVE:
                    DoCastVictim(SPELL_CLEAVE);
                    events.Repeat(6000);
                    break;
                case EVENT_LEAP:
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                        me->CastSpell(target, SPELL_LEAP, false);
                    events.Repeat(20000);
                    break;
                }
                case EVENT_GNOLL_REINFORCEMENTS:
                    for (int i = 0; i < 3; ++i)
                        me->SummonCreature(NPC_GNOLL_ADDS, me->GetPositionX()+irand(-5,5), me->GetPositionY()+irand(-5,5), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                    events.Repeat(30000);
                    break;
                case EVENT_HOWL_OF_VOID:
                    DoCastAOE(SPELL_HOWL_OF_VOID);
                    for (int i = 0; i < 2; ++i)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            me->SummonCreature(NPC_PLAYER_CLONE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
                    }
                    events.Repeat(25000);
                    break;
                case EVENT_CHRONO_BURN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        me->CastSpell(target, SPELL_CHRONO_BURN, false);
                    events.Repeat(15000);
                    break;
                case EVENT_UNSTABLE_RIFT:
                    // Optional: Cast visual-only spell or trigger room effect
                    events.Repeat(45000);
                    break;
                case EVENT_ECHO_SLAM:
                    DoCastAOE(SPELL_ECHO_SLAM);
                    events.Repeat(15000);
                    break;
                case EVENT_MEMORY_OVERLOAD:
                    for (int i = 0; i < 4; ++i)
                        me->SummonCreature(NPC_CHANNELING_ADD, me->GetPositionX()+irand(-8,8), me->GetPositionY()+irand(-8,8), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
                    events.Repeat(40000);
                    break;
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool phaseTwo = false;
        bool phaseThree = false;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_memory_of_hoggerAI(creature);
    }
};

// Optional Texts
void AddSC_boss_memory_of_hogger()
{
    new boss_memory_of_hogger();
}
