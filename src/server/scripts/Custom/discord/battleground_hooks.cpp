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
    // Ensure webhookUrl is declared before its usage
    std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");

    // Check if webhook URL is valid before sending it
    if (!webhookUrl.empty())
    {
        uint32 alliancePlayers = GetPlayersCountByTeam(ALLIANCE);
        uint32 hordePlayers = GetPlayersCountByTeam(HORDE);

        // Ensure the webhook function is correctly called here
        SendBattlegroundDiscordWebhook(webhookUrl, GetName(), alliancePlayers, hordePlayers);
    }

    // Proceed with other logic in the function
    SetStartTime(0);
    SetLastResurrectTime(0);
    AddToBGFreeSlotQueue();
    sBattlegroundMgr->AddBattleground(this);

    // Log additional information for rated battlegrounds
    if (m_IsRated)
    {
        TC_LOG_DEBUG("bg.arena", "Arena match type: {} for Team1Id: {} - Team2Id: {} started.", m_ArenaType, m_ArenaTeamIds[TEAM_ALLIANCE], m_ArenaTeamIds[TEAM_HORDE]);
    }
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




void AddBattlegroundDiscordHookScripts()
{
    //new BattlegroundScript_DiscordHook();
}
