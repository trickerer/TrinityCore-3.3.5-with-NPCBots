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
#include "Player.h"  // Include Player.h to ensure PlayerScript is available

// PlayerScript should be properly inherited
class DiscordWebhookPlayerActivity : public PlayerScript
{
public:
    DiscordWebhookPlayerActivity() : PlayerScript("DiscordWebhookPlayerActivity") { }

    // Handle player login - no override keyword in TrinityCore 3.3.5a
    void OnLogin(Player* player) 
	{
		TC_LOG_INFO("player.hooks", "Player %s has logged in.", player->GetName().c_str());
		Notify(player, true);  // Notify when player logs in
	}


    // Handle player logout - no override keyword in TrinityCore 3.3.5a
    void OnLogout(Player* player) 
    {
        Notify(player, false); // Notify when player logs out
    }

private:
    bool IsWebhookEnabled()
    {
        // Check if the Webhook is enabled in the config file
        return sConfigMgr->GetBoolDefault("Webhook.Enabled", true);
    }

    void Notify(Player* player, bool loggingIn)
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            return;  // No webhook URL configured
        }

        std::string name = player->GetName();
        std::string gmTag = player->IsGameMaster() ? "🛡️ " : "";
        std::string status = loggingIn ? "🟢 **Logged In**" : "🔴 **Logged Out**";

        // Use stringstream to build the message
        std::ostringstream messageStream;
        messageStream << gmTag << "**Player " << status << "**\nName: `" << name << "`";

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

            std::string payload = "{\"content\":\"" + message + "\"}";

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

            // Log the response if needed
            if (!responseBody.empty())
                TC_LOG_INFO("player.hooks", "Webhook response body: %s", responseBody.c_str());
            else
                TC_LOG_INFO("player.hooks", "Webhook response body is empty (expected for 204).");
        }
        catch (const Poco::Exception& ex)
        {
            // Log the error if the webhook fails
            TC_LOG_ERROR("player.hooks", "Webhook failed: %s", ex.displayText().c_str());
        }
    }
};

// Register the script
void AddDiscordWebhookPlayerLoginScripts()
{
    new DiscordWebhookPlayerActivity();
}
