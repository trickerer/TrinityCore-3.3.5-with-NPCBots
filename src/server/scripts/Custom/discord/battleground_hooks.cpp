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

class BattlegroundWS_DiscordHook : public BattlegroundWS
{
public:
    BattlegroundWS_DiscordHook() : BattlegroundWS() { }

    void StartingEventCloseDoors() override
    {
        BattlegroundWS::StartingEventCloseDoors(); // Keep original behavior

        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("bg.hooks", "Webhook URL is not configured.");
            return;
        }

        std::stringstream message;
        message << "⚔️ **Warsong Gulch Started!**\n"
                << "**Players:** "
                << GetPlayersCountByTeam(ALLIANCE) << " Alliance vs "
                << GetPlayersCountByTeam(HORDE) << " Horde";

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

class BG_WS_DiscordHookScript : public BattlegroundScript
{
public:
    BG_WS_DiscordHookScript() : BattlegroundScript("BG_WS_DiscordHookScript") { }

    Battleground* GetBattleground() const override
    {
        return new BattlegroundWS_DiscordHook();
    }
};

void AddBattlegroundDiscordHookScripts()
{
    new BG_WS_DiscordHookScript();
}
