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
#include "PlayerScript.h"

// Inherit from PlayerScript (which is already part of TrinityCore)
class DiscordWebhookPlayerActivity : public PlayerScript
{
public:
    // Constructor to set up the script
    DiscordWebhookPlayerActivity() : PlayerScript("DiscordWebhookPlayerActivity") { }

    // This function is called when a player logs in
    void OnLogin(Player* player) override
    {
        Notify(player, true); // Call the Notify function when the player logs in
    }

    // This function is called when a player logs out
    void OnLogout(Player* player) override
    {
        Notify(player, false); // Call the Notify function when the player logs out
    }

private:
    bool IsWebhookEnabled()
    {
        // Check if the Webhook is enabled in the config file
        return sConfigMgr->GetBoolDefault("Webhook.Enabled", true);
    }

    void Notify(Player* player, bool loggingIn)
    {
        // Fetch the Webhook URL from the configuration
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            return;  // No webhook URL configured, exit
        }

        // Player's name and status (Logged In/Logged Out)
        std::string name = player->GetName();
        std::string gmTag = player->IsGameMaster() ? "🛡️ " : ""; // Add "🛡️" for GM players
        std::string status = loggingIn ? "🟢 **Logged In**" : "🔴 **Logged Out**";

        // Build the message to send to Discord
        std::ostringstream messageStream;
        messageStream << gmTag << "**Player " << status << "**\nName: `" << name << "`";

        // Send the webhook with the message
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

            // Create an HTTPS session for the request
            std::unique_ptr<Poco::Net::HTTPClientSession> session;
            if (uri.getScheme() == "https")
                session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
            else
                session = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());

            // Create the POST request
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, "HTTP/1.1");
            request.setHost(uri.getHost());
            request.setContentType("application/json");
            request.setContentLength(static_cast<int>(payload.size()));

            // Send the request
            std::ostream& os = session->sendRequest(request);
            os << payload;

            // Receive the response
            Poco::Net::HTTPResponse response;
            std::istream& rs = session->receiveResponse(response);
            std::stringstream ss;
            Poco::StreamCopier::copyStream(rs, ss);

            std::string responseBody = ss.str();

            // Log the response
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
    new DiscordWebhookPlayerActivity(); // Register the script to be loaded
}
