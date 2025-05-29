#include "DiscordWebhookMgr.h"
#include "Config.h"
#include "Log.h"

#include <curl/curl.h>
#include <string>

// Poco includes
#define format __poco_format_workaround
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#undef format

/*
void SendBattlegroundDiscordWebhook(const std::string& battlegroundName, const std::string& teamAName, unsigned int teamAPlayers, unsigned int teamBPlayers, const std::string& teamBName)
{
    std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
    std::string avatarUrl  = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");

    if (webhookUrl.empty())
    {
        TC_LOG_ERROR("module", "Webhook URL is empty. Check your config (Webhook.URL)");
        return;
    }

    // Construct the message
    std::string message = battlegroundName + " has started with " +
                          std::to_string(teamAPlayers) + " " + teamAName + " players and " +
                          std::to_string(teamBPlayers) + " " + teamBName + " players.";

    // Escape double quotes (optional but safe)
    size_t pos = 0;
    while ((pos = message.find("\"", pos)) != std::string::npos)
    {
        message.replace(pos, 1, "\\\"");
        pos += 2;
    }

    // Construct JSON payload
    std::string payload = "{\"content\": \"" + message + "\"";
    if (!avatarUrl.empty())
        payload += ", \"avatar_url\": \"" + avatarUrl + "\"";
    payload += "}";

    CURL* curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            TC_LOG_ERROR("module", "CURL failed: {}", curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
*/

void SendDiscordMessage(const std::string& message)
{
    if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", true))
        return;
    
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


void SendDiscordMessageWorld(const std::string& message)
{
    if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", true))
        return;
    
    std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook2.URL", "");
    std::string avatarUrl  = sConfigMgr->GetStringDefault("Webhook2.AvatarURL", "");

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


