#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"

class instance_stormwind_vault : public InstanceMapScript
{
public:
    instance_stormwind_vault() : InstanceMapScript("instance_stormwind_vault", 35) { }

    struct instance_stormwind_vault_InstanceMapScript : public InstanceScript
    {
        instance_stormwind_vault_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

        void Initialize()
        {
            // Initialization logic if needed
        }

        void OnPlayerEnter(Player* player) override
        {
            // Optional logic
        }

        void Update(uint32 /*diff*/) override
        {
            // Optional logic
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_stormwind_vault_InstanceMapScript(map);
    }
};

void AddSC_instance_stormwind_vault()
{
    new instance_stormwind_vault();
}
