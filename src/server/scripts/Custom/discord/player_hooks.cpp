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
#include <thread>

#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"
#include "server_shutdown.h"
#include "DBCStores.h"
#include "AchievementMgr.h"
#include "DatabaseEnv.h"
#include "../../scripts/Custom/discord/DiscordWebhookMgr.h"

static std::unordered_set<uint64> LoggedInGuids;

class DiscordWebhookPlayerActivity : public PlayerScript
{
public:
    DiscordWebhookPlayerActivity() : PlayerScript("DiscordWebhookPlayerActivity")
    {
        TC_LOG_INFO("player.hooks", "DiscordWebhookPlayerActivity script loaded.");
    }

    void OnAchievementEarned(Player* player, AchievementEntry const* achievement) //override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "No webhook URL configured!");
            return;
        }

        const std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");
        const std::string name = player->GetName();
        const std::string achievementName = GetLocalizedAchievementName(achievement->ID);

        const std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER
            ? (player->GetSession()->GetSecurity() > 3 ? "🧪 " : "⚙️ ")
            : "👤 ";

        std::ostringstream messageStream;
        messageStream << "🌍 [" << realmName << "] "  // Realm name prefix
                      << gmTag << "🏆 Achievement Earned by `" << name << "`: **" << achievementName << "**";

        SendDiscordWebhookAsync(webhookUrl, messageStream.str());
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        uint64 guid = player->GetGUID();

        if (!LoggedInGuids.insert(guid).second)
            return;

        Notify(player, true);
    }

    void OnLogout(Player* player) override
    {
        if (!sConfigMgr->GetBoolDefault("Webhook.Enabled", false))
            return;

        if (serverShuttingDown)
        {
            const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
            if (!webhookUrl.empty())
            {
                const std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");
                const std::string message = "🌍 [" + realmName + "] 👢 All online players have been logged out..";
                SendDiscordWebhookAsync(webhookUrl, message);
            }
            return;
        }

        LoggedInGuids.erase(player->GetGUID());
        Notify(player, false);
    }

private:
    void Notify(Player* player, bool loggingIn)
    {
        const std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("player.hooks", "No webhook URL configured!");
            return;
        }

        const std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");

        const std::string name = player->GetName();
        const uint8 level = player->GetLevel();
        const std::string gmTag = player->GetSession()->GetSecurity() > SEC_PLAYER 
                          ? (player->GetSession()->GetSecurity() > 3 ? "🧪 " : "⚙️ ")
                          : "👤 ";
        const std::string status = loggingIn ? "🟢 Logged In" : "🛑 Logged Out";

        std::ostringstream messageStream;
        messageStream << "🌍 [" << realmName << "] " << gmTag << status << " `" << name << "` (Level " << static_cast<int>(level) << ")";
        SendDiscordWebhookAsync(webhookUrl, messageStream.str());

        if (!loggingIn)
        {
            uint32 guid = player->GetGUID().GetCounter();
            QueryResult result = CharacterDatabase.PQuery("SELECT in_world_channel FROM world_channel_flags WHERE guid = {}", guid);
            if (result)
            {
                const std::string status2 = "🛑 Left World Channel";

                std::ostringstream messageStream2;
                messageStream2 << "🌍 [" << realmName << "] "
                               << gmTag << status2 << " `" << name << "` (Level " << static_cast<int>(level) << ")";

                SendDiscordMessageWorld(messageStream2.str());
            }
        }
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
        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty())
                path = "/";

            const std::string avatarUrl = sConfigMgr->GetStringDefault("Webhook.AvatarURL", "");
            //const std::string realmName = sConfigMgr->GetStringDefault("WorldServer.RealmName", "Unknown Realm");
            const std::string realmName = "";

            Poco::JSON::Object json;

            // Add realm name to the message
            std::string fullMessage = realmName + " : " + message;
            json.set("content", fullMessage);

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
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("player.hooks", "Webhook failed: {}", ex.displayText());
        }
    }

    void SendDiscordWebhookAsync(const std::string& url, const std::string& message)
    {
        std::thread([this, url, message]()
        {
            try
            {
                SendDiscordWebhook(url, message);
            }
            catch (const std::exception& e)
            {
                TC_LOG_ERROR("player.hooks", "Exception in webhook thread: %s", e.what());
            }
        }).detach();
    }
};

void AddSC_DiscordWebhookPlayerActivity()
{
    new DiscordWebhookPlayerActivity();
}
