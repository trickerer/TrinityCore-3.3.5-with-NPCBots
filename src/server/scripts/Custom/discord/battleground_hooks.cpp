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
#include "Player.h"
#include "Config.h"
#include "Log.h"

class DiscordWebhookBattlegroundScript : public BGScript
{
public:
    DiscordWebhookBattlegroundScript() : BGScript("DiscordWebhookBattlegroundScript") { }

    void OnBattlegroundStart(Battleground* bg)
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("bg.hooks", "Webhook URL is not configured.");
            return;
        }

        std::string bgName = bg->GetName();
        std::stringstream message;
        message << "⚔️ **Battleground Started!**\n"
                << "**Name:** " << bgName << "\n"
                << "**Players:** "
                << bg->GetPlayersCountByTeam(ALLIANCE) << " Alliance vs "
                << bg->GetPlayersCountByTeam(HORDE) << " Horde";

        SendDiscordWebhook(webhookUrl, message.str());
    }

private:
    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty()) path = "/";

            Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
            json->set("content", message);

            std::stringstream payload;
            Poco::JSON::Stringifier::stringify(json, payload);

            std::unique_ptr<Poco::Net::HTTPClientSession> session;
            if (uri.getScheme() == "https")
                session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
            else
                session = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());

            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, "HTTP/1.1");
            request.setContentType("application/json");
            request.setContentLength((int)payload.str().size());

            std::ostream& os = session->sendRequest(request);
            os << payload.str();

            Poco::Net::HTTPResponse response;
            std::istream& rs = session->receiveResponse(response);
            std::stringstream ss;
            Poco::StreamCopier::copyStream(rs, ss);
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("bg.hooks", "Failed to send Discord webhook: %s", ex.displayText().c_str());
        }
    }
};

void AddBattlegroundDiscordHookScripts()
{
    new DiscordWebhookBattlegroundScript();
}