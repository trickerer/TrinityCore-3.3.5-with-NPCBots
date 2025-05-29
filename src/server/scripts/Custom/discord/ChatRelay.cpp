#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ChannelMgr.h"
#include "Log.h"

class ChatRelayScript : public PlayerScript
{
public:
    ChatRelayScript() : PlayerScript("ChatRelayScript") {}

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {

        // Get the "world" channel (realm ID 0)
        ChannelMgr* cMgr = ChannelMgr::forTeam(TEAM_NEUTRAL);
        Channel* channel = cMgr->GetChannel(0, "world", player, false, nullptr);
        if (!channel)
            return;


        std::string playerName = player->GetName();
        TC_LOG_INFO("chatrelay", "[WORLD CHANNEL] {}: {}", playerName, msg);

        // Here you could add your Discord webhook or other integration
    }
};

void AddChatRelayScript()
{
    new ChatRelayScript();
}