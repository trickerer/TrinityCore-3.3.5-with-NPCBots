#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ChannelMgr.h"
#include "Log.h"
#include "Config.h"

#include <curl/curl.h>


void SendDiscordMessage(const std::string& message)
{
    std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
    std::string avatarUrl  = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");

    if (webhookUrl.empty())
    {
        TC_LOG_ERROR("module", "Webhook URL is empty. Check your config (Webhook.URL)");
        return;
    }

    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        // Escape double quotes in message
        std::string escapedMessage = message;
        size_t pos = 0;
        while ((pos = escapedMessage.find("\"", pos)) != std::string::npos)
        {
            escapedMessage.replace(pos, 1, "\\\"");
            pos += 2;
        }

        // Construct JSON payload
        std::string payload = "{\"content\": \"" + escapedMessage + "\"";
        if (!avatarUrl.empty())
            payload += ", \"avatar_url\": \"" + avatarUrl + "\"";
        payload += "}";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            TC_LOG_ERROR("module", "CURL failed: {}", curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

class ChatRelayScript : public PlayerScript
{
public:
    ChatRelayScript() : PlayerScript("ChatRelayScript") {}

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {
        if (type != CHAT_MSG_CHANNEL)
            return;

        if (player->GetSession()->GetLastChannelName() != "world")
            return;

        std::string playerName = player->GetName();
        std::string content = "[WORLD] **" + playerName + "**: " + msg;

        TC_LOG_INFO("chatrelay", "%s", content.c_str());

        SendDiscordMessage(content);
    }
};

void AddChatRelayScript()
{
    new ChatRelayScript();
}
