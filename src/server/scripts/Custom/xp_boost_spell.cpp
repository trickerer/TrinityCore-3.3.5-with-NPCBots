#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuraEffects.h"

class spell_xp_boost_aura : public SpellScriptLoader
{
public:
    spell_xp_boost_aura() : SpellScriptLoader("spell_xp_boost_aura") { }

    class spell_xp_boost_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_xp_boost_aura_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
                target->ToPlayer()->SetFlag(PLAYER_FLAGS_EXTRA, PLAYER_FLAGS_EXTRA_XP_BOOST);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* target = GetTarget())
                target->ToPlayer()->RemoveFlag(PLAYER_FLAGS_EXTRA, PLAYER_FLAGS_EXTRA_XP_BOOST);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_xp_boost_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_xp_boost_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_xp_boost_aura_AuraScript();
    }
};

void AddSC_spell_xp_boost_aura()
{
    new spell_xp_boost_aura();
}
