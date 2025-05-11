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


// Declare the function before its use
void SendBattlegroundDiscordWebhook(const std::string& webhookUrl, const std::string& battlegroundName, uint32 alliancePlayers, uint32 hordePlayers);

// Function definitions

void Battleground::StartBattleground()
{
    TC_LOG_INFO("bg.hooks", "StartBattleground called for: %s", GetName().c_str());

    // Add this block:
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (!webhookUrl.empty())
        {
            uint32 alliancePlayers = GetPlayersCountByTeam(ALLIANCE);
            uint32 hordePlayers = GetPlayersCountByTeam(HORDE);

            SendBattlegroundDiscordWebhook(webhookUrl, GetName(), alliancePlayers, hordePlayers);
        }
    }

    uint32 alliancePlayers = bg->GetPlayersCountByTeam(ALLIANCE);
    uint32 hordePlayers = bg->GetPlayersCountByTeam(HORDE);
    SendBattlegroundDiscordWebhook(webhookUrl, bg->GetName(), alliancePlayers, hordePlayers);
}

void SendBattlegroundDiscordWebhook(const std::string& webhookUrl, const std::string& battlegroundName, uint32 alliancePlayers, uint32 hordePlayers)
{
    try
    {
        Poco::URI uri(webhookUrl);
        std::string path = uri.getPathAndQuery();
        if (path.empty()) path = "/";

        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("content", "⚔️ **" + battlegroundName + " Started!**\n"
                            + "**Players:** " + std::to_string(alliancePlayers) + " Alliance vs "
                            + std::to_string(hordePlayers) + " Horde");

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

class BattlegroundScript_DiscordHook : public BattlegroundScript
{
public:
    BattlegroundScript_DiscordHook() : BattlegroundScript("BattlegroundScript_DiscordHook")
    {
        TC_LOG_INFO("bg.hooks", "BattlegroundScript_DiscordHook registered successfully.");
    }

    void OnBattlegroundStart(Battleground* bg)
    {
        TC_LOG_INFO("bg.hooks", "OnBattlegroundStart triggered for: %s", bg->GetName().c_str());

        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("bg.hooks", "Webhook URL is not configured.");
            return;
        }

        uint32 alliancePlayers = bg->GetPlayersCountByTeam(ALLIANCE);
        uint32 hordePlayers = bg->GetPlayersCountByTeam(HORDE);

        TC_LOG_INFO("bg.hooks", "Sending webhook with %u Alliance vs %u Horde", alliancePlayers, hordePlayers);

        SendBattlegroundDiscordWebhook(webhookUrl, bg->GetName(), alliancePlayers, hordePlayers);
    }
};


void AddBattlegroundDiscordHookScripts()
{
    new BattlegroundScript_DiscordHook();
}
