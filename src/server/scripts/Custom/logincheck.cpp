#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Log.h"

class TestLoginHook : public PlayerScript
{
public:
    TestLoginHook() : PlayerScript("TestLoginHook") {}

    void OnLogin(Player* player, bool /*firstLogin*/)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(">> TEST: Login hook triggered.");
        TC_LOG_INFO("custom", "TestLoginHook: OnLogin triggered for player: %s", player->GetName().c_str());
    }
};

void AddSC_TestLoginHook()
{
    new TestLoginHook();
}