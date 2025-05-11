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

bool serverShuttingDown = false;  // Global flag to track server shutdown

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
        // Set the flag that the server is shutting down
        serverShuttingDown = true;

        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");

        std::stringstream messageStream;
        messageStream << "🛑 **Server is restarting, 1 min downtime..**\nRealm: **" << realmName << "**";
        SendDiscordWebhook(webhookUrl, messageStream.str());
    }

    // You can modify the Notify function if required
    void Notify(Player* player, bool loggingIn)
    {
        // Check if the server is shutting down, and prevent notifications if true
        if (serverShuttingDown)
        {
            TC_LOG_INFO("server.hooks", "Server is shutting down, not sending login/logout notifications.");
            return;
        }

        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "No webhook URL configured!");
            return;
        }

        std::string name = player->GetName();
        std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER ? "🛡️ " : "";
        std::string status = loggingIn ? "🟢 **Logged In**" : "🔴 **Logged Out**";

        std::ostringstream messageStream;
        messageStream << gmTag << "**Player " << status << "**\nName: `" << name << "`";

        TC_LOG_INFO("player.hooks", "Sending webhook for player: {}", name);
        TC_LOG_INFO("player.hooks", "Message content: {}", messageStream.str());

        SendDiscordWebhook(webhookUrl, messageStream.str());
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
			std::string avatarUrl = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");
			if (!avatarUrl.empty())
				json.set("avatar_url", avatarUrl);

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