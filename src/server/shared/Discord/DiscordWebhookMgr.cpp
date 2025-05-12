#include "DiscordWebhookMgr.h"
#include "Config.h"
#include "Log.h"

#include <curl/curl.h>

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

void SendBattlegroundDiscordWebhook(const std::string& webhookUrl, const std::string& battlegroundName, uint32 alliancePlayers, uint32 hordePlayers)
{
    CURL* curl = curl_easy_init();
    if (curl)
    {
        // Construct the message with battleground information
        std::string message = battlegroundName + " has started with " + std::to_string(alliancePlayers) + " Alliance players and " + std::to_string(hordePlayers) + " Horde players.";

        // Prepare JSON payload
        std::string jsonPayload = "{\"content\": \"" + message + "\"}";

        // Set options for CURL
        curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, "Content-Type: application/json");

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "Error sending battleground webhook: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
}


void DiscordWebhookMgr::SendDiscordMessage(const std::string& message)
{
    // Example code to send a POST request to the Discord webhook (using libcurl)
    CURL* curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "YOUR_DISCORD_WEBHOOK_URL");

        // Create JSON body
        std::string jsonBody = "{\"content\": \"" + message + "\"}";

        // Set up the request
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, "Content-Type: application/json");

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            // Handle error
            std::cerr << "Error sending message to Discord: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
}


