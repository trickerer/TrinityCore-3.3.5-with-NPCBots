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
#include <sstream>
#include <memory>

#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"
#include "server_shutdown.h"
#include "DBCStores.h"
#include "AchievementMgr.h"

// Remove ThreadPool forward declaration and extern
// namespace Threading { class ThreadPool; }
// extern Threading::ThreadPool* sThreadPool;

static std::unordered_set<uint64> LoggedInGuids;

class DiscordWebhookPlayerActivity : public PlayerScript
{
public:
    DiscordWebhookPlayerActivity() : PlayerScript("DiscordWebhookPlayerActivity")
    {
        TC_LOG_INFO("player.hooks", "DiscordWebhookPlayerActivity script loaded.");
    }

    void OnAchievementEarned(Player* player, AchievementEntry const* achievement) // override NOT USED!!
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "No webhook URL configured!");
            return;
        }

        const std::string name = player->GetName();
        const std::string achievementName = GetLocalizedAchievementName(achievement->ID);
        std::ostringstream messageStream;

        const std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER ? "🛡️ " : "👤 ";
        messageStream << gmTag << "🏆 Achievement Earned by `" << name << "`: **" << achievementName << "**";

        try
        {
            SendDiscordWebhook(webhookUrl, messageStream.str());
        }
        catch (const std::exception& e)
        {
            TC_LOG_ERROR("player.hooks", "Exception sending achievement webhook: {}", e.what());
        }
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        const uint64 guid = player->GetGUID();

        if (LoggedInGuids.find(guid) != LoggedInGuids.end())
            return;

        LoggedInGuids.insert(guid);

        TC_LOG_INFO("player.hooks", "OnLogin function triggered for: {}", player->GetName());
        Notify(player, true);
    }

    void OnLogout(Player* player) override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        if (serverShuttingDown)
        {
            const std::string message = "👢 All online players have been logged out..";
            const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
            if (!webhookUrl.empty())
            {
                SendDiscordWebhook(webhookUrl, message);
            }
            return;
        }

        const uint64 guid = player->GetGUID();
        LoggedInGuids.erase(guid);

        Notify(player, false);
    }

private:
    void Notify(Player* player, bool loggingIn)
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "No webhook URL configured!");
            return;
        }

        const std::string name = player->GetName();
        const uint8 level = player->GetLevel();
        const std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER ? "🛡️ " : "👤 ";
        const std::string status = loggingIn ? "🟢 Logged In" : "🛑 Logged Out";

        std::ostringstream messageStream;
        messageStream << gmTag << status << " `" << name << "` (Level " << static_cast<int>(level) << ")";

        SendDiscordWebhook(webhookUrl, messageStream.str());
    }

    static std::string GetLocalizedAchievementName(uint32 id)
    {
        const AchievementEntry* achievement = sAchievementStore.LookupEntry(id);
        if (!achievement)
            return "Unknown Achievement";

        const uint8 locale = sWorld->GetDefaultDbcLocale();

        if (achievement->Title[locale] && achievement->Title[locale][0] != '\0')
            return std::string(achievement->Title[locale]);
        else if (achievement->Title[0])
            return std::string(achievement->Title[0]);

        return "Unnamed Achievement";
    }

    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty())
                path = "/";

            Poco::JSON::Object json;
            json.set("content", message);
            const std::string avatarUrl = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");
            if (!avatarUrl.empty())
                json.set("avatar_url", avatarUrl);

            std::stringstream payloadStream;
            json.stringify(payloadStream);
            const std::string payload = payloadStream.str();

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

            // Optional: log the response or status if needed
            // std::string responseBody = ss.str();
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("player.hooks", "Webhook failed: {}", ex.displayText());
        }
    }
};

void AddSC_DiscordWebhookPlayerActivity()
{
    new DiscordWebhookPlayerActivity();
}
