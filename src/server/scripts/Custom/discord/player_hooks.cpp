#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Log.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>
#include <sstream>
#include <memory>

class DiscordWebhookPlayerLogin : public PlayerScript
{
public:
    DiscordWebhookPlayerLogin() : PlayerScript("DiscordWebhookPlayerLogin") { }

    void OnLogin(Player* player) override
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "Webhook URL is not configured.");
            return;
        }

        std::string name = player->GetName();
        std::string ip = player->GetSession()->GetRemoteAddress();
        std::string message = "🟢 **Player Logged In**\nName: `" + name + "`\nIP: `" + ip + "`";

        SendDiscordWebhook(webhookUrl, message);
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
    new DiscordWebhookPlayerLogin();
}