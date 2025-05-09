#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>
#include <sstream>

class DiscordWebhookServerHook : public WorldScript
{
public:
    DiscordWebhookServerHook() : WorldScript("DiscordWebhookServerHook") { }

    void OnStartup() override
    {
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        SendDiscordWebhook(webhookUrl, "✅ **Server has started successfully!**");
    }

    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        try
        {
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty()) path = "/";

            std::string payload = "{\"content\":\"" + message + "\"}";

            Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, "HTTP/1.1");
            request.setContentType("application/json");
            request.setContentLength(payload.length());

            std::ostream& os = session.sendRequest(request);
            os << payload;

            Poco::Net::HTTPResponse response;
            std::istream& rs = session.receiveResponse(response);
            std::stringstream ss;
            Poco::StreamCopier::copyStream(rs, ss);

            TC_LOG_INFO("server.hooks", "Discord webhook sent. Response: %s", ss.str().c_str());
        }
        catch (const Poco::Exception& ex)
        {
            TC_LOG_ERROR("server.hooks", "Discord webhook failed: %s", ex.displayText().c_str());
        }
    }
};

void AddDiscordWebhookServerHookScripts()
{
    new DiscordWebhookServerHook();
}