#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ChannelMgr.h"
#include "Log.h"
#include "Config.h"

#include <curl/curl.h>
#include <json/json.h>

void SendDiscordMessage(const std::string& content)
{
    std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        TC_LOG_ERROR("chatrelay", "Failed to initialize CURL");
        return;
    }

    Json::Value root;
    root["content"] = content;
    Json::StreamWriterBuilder writer;
    const std::string jsonData = Json::writeString(writer, root);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, webhookURL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)jsonData.size());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        TC_LOG_ERROR("chatrelay", "CURL send to Discord failed: %s", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

class ChatRelayScript : public PlayerScript
{
public:
    ChatRelayScript() : PlayerScript("ChatRelayScript") {}

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {
        ChannelMgr* cMgr = ChannelMgr::forTeam(TEAM_NEUTRAL);
        Channel* channel = cMgr->GetChannel(0, "world", player, false, nullptr);
        if (!channel)
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
