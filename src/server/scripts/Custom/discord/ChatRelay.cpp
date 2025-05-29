#include "Player.h"
#include "ScriptMgr.h"
#include "Chat.h"
#include "Log.h"
#include "DiscordWebhookMgr.h" // Your header where SendDiscordMessage is declared

class ChatRelayScript : public PlayerScript
{
public:
    ChatRelayScript() : PlayerScript("ChatRelayScript") {}

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {
        // We want only world channel messages
        // CHAT_MSG_CHANNEL = 7 for channel chat; world channel typically named "world"
        if (type == CHAT_MSG_CHANNEL)
        {
            if (channelName == "world")
            {
                if (msg.find("[Discord]:") == std::string::npos)
                {
                    // Compose the message to send to Discord webhook
                    std::string discordMsg = "[World] [" + player->GetName() + "]: " + msg;

                    // Send to Discord
                    SendDiscordMessage(discordMsg);
                }
            }
        }
    }
};

void AddSC_chat_relay_script()
{
    new ChatRelayScript();
}