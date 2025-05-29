#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ChannelMgr.h"

class ChatRelayScript : public PlayerScript
{
public:
    ChatRelayScript() : PlayerScript("ChatRelayScript") {}

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {

        // Get the "world" channel (realm ID 0)
        Channel* channel = cMgr->GetChannel(0, "world", player, false, nullptr);
        if (!channel)
            return;

        // Check if player is a member of the "world" channel
        if (!channel->HasMember(player->GetGUID()))
            return;

        // Only proceed if message type is say / yell / whatever you want, or just log all
        // For example, let's just print all messages from the world channel:
        std::string playerName = player->GetName();
        TC_LOG_INFO("chatrelay", "[WORLD CHANNEL] <%s>: %s", playerName.c_str(), msg.c_str());

        // Here you could add your Discord webhook or other integration
    }
};

void AddChatRelayScript()
{
    new ChatRelayScript();
}