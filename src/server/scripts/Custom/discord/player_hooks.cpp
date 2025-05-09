#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Log.h"
#include "WorldSession.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <sstream>
#include <memory>

class DiscordWebhookPlayerActivity : public PlayerScript
{
public:
    DiscordWebhookPlayerActivity() : PlayerScript("DiscordWebhookPlayerActivity") { }

    // Handle player login (Override the correct method here)
    void OnLogin(Player* player) override
    {
        Notify(player, true);
    }

    // Handle player logout (Override the correct method here)
    void OnLogout(Player* player) override
    {
        Notify(player, false);
    }

private:
    void Notify(Player* player, bool loggingIn)
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "Webhook URL is not configured.");
            return;
        }

        std::string name = player->GetName();
        std::string gmTag = player->IsGameMaster() ? "🛡️ " : "";
        std::string status = loggingIn ? "🟢 **Logged In**" : "🔴 **Logged Out**";

        std::string message = gmTag + "**Player " + status + "**\nName: `" + name + "`";

        SendDiscordWebhook(webhookUrl, message);
    }

    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        try
        {
            Poco::JSON::Object json;
            json.set("content", message);
            std::stringstream jsonStream;
            Poco::JSON::Stringifier::stringify(json, jsonStream);
            std::string payload = jsonStream.str();

            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty())
                path = "/";

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

            TC_LOG_INFO("player.hooks", "Webhook status: %d %s", response.getStatus(), response.getReason().c_str());
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("player.hooks", "Webhook failed: %s", ex.displayText().c_str());
        }
    }
};

void AddDiscordWebhookPlayerLoginScripts()
{
    new DiscordWebhookPlayerActivity();
}
