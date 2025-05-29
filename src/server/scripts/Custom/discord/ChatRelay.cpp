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
        ChannelMgr* cMgr = ChannelMgr::getSingletonPtr();
        if (!cMgr)
            return;

        Channel* channel = cMgr->GetChannel(0, "world", player, false, nullptr);
        if (!channel)
            return;

        std::string playerName = player->GetName();
        std::string content = "[WORLD] **" + playerName + "**: " + msg;

        TC_LOG_INFO("chatrelay", "%s", content.c_str());

        // Send message to Discord
        SendDiscordMessage(content);
    }
};

void AddChatRelayScript()
{
    new ChatRelayScript();
}