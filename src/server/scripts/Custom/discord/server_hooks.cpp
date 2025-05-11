// Workaround for GCC 13: avoid ambiguity between std::format and Poco::format
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

// TrinityCore headers
#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "World.h"
#include "WorldSession.h"
#include "server_shutdown.h"

// Global variable to hold the countdown timer and remaining time
int shutdownTimer = 0;  // Time in seconds until the server shuts down
bool countdownInProgress = false;  // Flag to check if a countdown is in progress

class DiscordWebhookServerHook : public WorldScript
{
public:
    DiscordWebhookServerHook() : WorldScript("DiscordWebhookServerHook") { }

    void OnStartup() override
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");

        std::stringstream messageStream;
        messageStream << "✅ **Server is online**\nRealm: **" << realmName << "**";
        SendDiscordWebhook(webhookUrl, messageStream.str());
    }

    void OnShutdown() override
    {
        // Prevent starting multiple countdowns
        if (countdownInProgress)
            return;

        countdownInProgress = true;
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        // Start countdown, example 30 seconds before shutdown
        shutdownTimer = 30;  // Set countdown time (in seconds)
        
        // Start sending periodic messages to Discord
        SendShutdownCountdown(webhookUrl);

        // Optionally, add a final message right before shutdown
        std::stringstream messageStream;
        messageStream << "🛑 **Server is restarting, 1 min downtime..**\nRealm: **" << sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm") << "**";
        SendDiscordWebhook(webhookUrl, messageStream.str());
    }

    // This function sends periodic countdown messages every 10 seconds
    void SendShutdownCountdown(const std::string& webhookUrl)
    {
        // Schedule the periodic countdown
        World::AddWorldTimedTask(10, [this, webhookUrl]() {
            if (shutdownTimer > 0)
            {
                std::stringstream messageStream;
                messageStream << "⚠️ **Server restarting in " << shutdownTimer << " seconds**";
                SendDiscordWebhook(webhookUrl, messageStream.str());

                // Decrease the timer
                shutdownTimer -= 10;

                // If time is up, send the final shutdown message
                if (shutdownTimer <= 0)
                {
                    std::stringstream finalMessage;
                    finalMessage << "🛑 **Server is shutting down now!**";
                    SendDiscordWebhook(webhookUrl, finalMessage.str());
                    
                    // You can trigger the actual shutdown here if needed
                    // Example: World::StopAll();
                }
                else
                {
                    // Reschedule the next countdown update
                    SendShutdownCountdown(webhookUrl);
                }
            }
        });
    }

private:
    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty())
                path = "/";

            Poco::JSON::Object json;
            json.set("content", message);

            std::stringstream ssPayload;
            Poco::JSON::Stringifier::stringify(json, ssPayload);
            std::string payload = ssPayload.str();

            std::unique_ptr<Poco::Net::HTTPClientSession> session;
            if (uri.getScheme() == "https")
                session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
            else
                session = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());

            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, "HTTP/1.1");
            request.setHost(uri.getHost());
            request.setContentType("application/json");
            request.setContentLength(static_cast<int>(payload.size()));

            std::ostream& os = session->sendRequest(request);
            os << payload;

            Poco::Net::HTTPResponse response;
            std::istream& rs = session->receiveResponse(response);

            std::stringstream ss;
            Poco::StreamCopier::copyStream(rs, ss);
            std::string responseBody = ss.str();

            if (!responseBody.empty())
                TC_LOG_INFO("server.hooks", "Webhook response body: %s", responseBody.c_str());
            else
                TC_LOG_INFO("server.hooks", "Webhook response body is empty (expected for 204).");
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("server.hooks", "Discord webhook failed: %s", ex.displayText().c_str());
        }
    }
};

// Register the script
void AddDiscordWebhookServerHookScripts()
{
    new DiscordWebhookServerHook();
}