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

        void Register() override
        {
            // No custom handlers needed for apply/remove in 3.3.5a
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
