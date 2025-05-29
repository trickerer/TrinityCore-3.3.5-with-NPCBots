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

#include <sstream>
#include <memory>
#include <thread>

#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "World.h"
#include "WorldSession.h"
#include "server_shutdown.h"
#include "../../scripts/Custom/discord/DiscordWebhookMgr.h"


bool serverShuttingDown = false;  // Global flag to track server shutdown

class DiscordWebhookServerHook : public WorldScript
{
public:
    DiscordWebhookServerHook() : WorldScript("DiscordWebhookServerHook") { }

    void OnStartup() override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        const std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");
        std::stringstream messageStream;
        messageStream << "✅ **Server is online** - Realm: " << realmName;

        SendDiscordWebhookAsync(webhookUrl, messageStream.str());
        // SEND TO WORLD CHAT
        SendDiscordMessageWorld(messageStream.str());
    }

    void OnShutdown() override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        serverShuttingDown = true;

        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        const std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");
        std::stringstream messageStream;
        messageStream << "🛑 **Server is restarting, 2 mins downtime..** Realm: " << realmName;

        SendDiscordWebhookAsync(webhookUrl, messageStream.str());
        // SEND TO WORLD CHAT
        SendDiscordMessageWorld(messageStream.str());
    }

    void Notify(Player* player, bool loggingIn)
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        if (serverShuttingDown)
            return;

        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "No webhook URL configured!");
            return;
        }

        const std::string name = player->GetName();
        const std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER ? "🛡️ " : "👤 ";
        const std::string status = loggingIn ? "🟢 Logged In" : "🛑 Logged Out";

        std::ostringstream messageStream;
        messageStream << gmTag << status << " `" << name << "`";

        SendDiscordWebhookAsync(webhookUrl, messageStream.str());
        // SEND TO WORLD CHAT
        SendDiscordMessageWorld(messageStream.str());
    }

private:
    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty())
                path = "/";

            Poco::JSON::Object json;
            json.set("content", message);

            const std::string avatarUrl = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");
            if (!avatarUrl.empty())
                json.set("avatar_url", avatarUrl);

            std::stringstream ssPayload;
            Poco::JSON::Stringifier::stringify(json, ssPayload);
            const std::string payload = ssPayload.str();

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
            // Optional: Log the response if needed
            // std::string responseBody = ss.str();
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("server.hooks", "Discord webhook failed: {}", ex.displayText());
        }
    }

    void SendDiscordWebhookAsync(const std::string& url, const std::string& message)
    {
        std::thread([this, url, message]()
        {
            try
            {
                SendDiscordWebhook(url, message);
            }
            catch (const std::exception& e)
            {
                TC_LOG_ERROR("server.hooks", "Exception in webhook thread: %s", e.what());
            }
        }).detach(); // Run the thread independently
    }
};

// Register the script
void AddDiscordWebhookServerHookScripts()
{
    new DiscordWebhookServerHook();
}
