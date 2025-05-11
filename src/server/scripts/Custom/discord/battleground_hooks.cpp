// Workaround for GCC 13
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

#include "ScriptMgr.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Config.h"
#include "Log.h"

class DiscordWebhookBattlegroundHook : public BGScript
{
public:
    DiscordWebhookBattlegroundHook() : BGScript("DiscordWebhookBattlegroundHook") { }

    void OnBattlegroundStarted(Battleground* bg)
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        std::string avatarUrl  = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("bg.hooks", "Webhook URL is not configured.");
            return;
        }

        std::string name = bg->GetName();
        std::stringstream messageStream;
        messageStream << "⚔️ **Battleground Started!**\nMap: **" << name << "**";

        SendDiscordWebhook(webhookUrl, messageStream.str(), avatarUrl);
    }

private:
    void SendDiscordWebhook(const std::string& url, const std::string& message, const std::string& avatarUrl)
    {
        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty())
                path = "/";

            Poco::JSON::Object json;
            json.set("content", message);
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

            TC_LOG_INFO("bg.hooks", "Discord webhook sent. Response: %s", ss.str().c_str());
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("bg.hooks", "Discord webhook failed: %s", ex.displayText().c_str());
        }
    }
};

// Register
void AddBattlegroundDiscordHookScripts()
{
    new DiscordWebhookBattlegroundHook();
} 
