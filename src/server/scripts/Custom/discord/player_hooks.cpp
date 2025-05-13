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

#include <unordered_set>

static std::unordered_set<uint64> LoggedInGuids;

// TrinityCore headers
#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"
#include "server_shutdown.h"


class DiscordWebhookPlayerActivity : public PlayerScript
{
public:
    DiscordWebhookPlayerActivity() : PlayerScript("DiscordWebhookPlayerActivity")
	{
		TC_LOG_INFO("player.hooks", "DiscordWebhookPlayerActivity script loaded.");
	}

    void OnLogin(Player* player, bool /*firstLogin*/)
	{
		uint64 guid = player->GetGUID();

		if (LoggedInGuids.find(guid) != LoggedInGuids.end())
			return; // Already notified

		LoggedInGuids.insert(guid);

		TC_LOG_INFO("player.hooks", "OnLogin function triggered for: {}", player->GetName());
		Notify(player, true); 
	}

    void OnLogout(Player* player)
	{
		if (serverShuttingDown)
		{
			std::string message = "All online players have been logged out..";
			std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
			if (!webhookUrl.empty())
			{
				SendDiscordWebhook(webhookUrl, message);  // You need to call the SendDiscordWebhook function
			}

			// Prevent further logout actions when the server is shutting down
			return;
		}
		uint64 guid = player->GetGUID();
		LoggedInGuids.erase(guid);

		TC_LOG_INFO("player.hooks", "Player logged out: {}", player->GetName());
		Notify(player, false);  
	}

private:
    void Notify(Player* player, bool loggingIn)
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "No webhook URL configured!");
            return;  
        }

        std::string name = player->GetName();
        std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER ? "🛡️ **GM " : " **Player";
        std::string status = loggingIn ? "🟢 **Logged In**" : "🛑 **Logged Out**";

        std::ostringstream messageStream;
        messageStream << gmTag << "" << status << "**\nName: `" << name << "`";

        TC_LOG_INFO("player.hooks", "Sending webhook for player: {}", name);
        TC_LOG_INFO("player.hooks", "Message content: {}", messageStream.str());

        SendDiscordWebhook(webhookUrl, messageStream.str());
    }

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

            std::stringstream payloadStream;
            json.stringify(payloadStream);
            std::string payload = payloadStream.str();

            TC_LOG_INFO("player.hooks", "Payload being sent: {}", payload.c_str());

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
                TC_LOG_INFO("player.hooks", "Webhook response body: {}", responseBody);
            else
                TC_LOG_INFO("player.hooks", "Webhook response body is empty (expected for 204).");
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_INFO("player.hooks", "Webhook failed: {}", ex.displayText());
        }
    }
};


// Register the script
void AddSC_DiscordWebhookPlayerActivity()
{
    new DiscordWebhookPlayerActivity();
}
