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
        // Fetch Webhook URL from configuration
        std::string webhookUrl = sConfigMgr->GetStringDefault("Webhook.URL", "");
        if (webhookUrl.empty())
        {
            TC_LOG_ERROR("server.hooks", "Webhook URL is not configured.");
            return;
        }

        // Send Discord Webhook
        SendDiscordWebhook(webhookUrl, "✅ **Server has started successfully!**");
    }

    void SendDiscordWebhook(const std::string& url, const std::string& message)
    {
        try
        {
            // Parse the URL
            Poco::URI uri(url);
            std::string path = uri.getPathAndQuery();
            if (path.empty()) path = "/";

            // Format the payload
            std::string payload = "{\"content\":\"" + message + "\"}";

            // Set up HTTP client session
            Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, "HTTP/1.1");
            request.setContentType("application/json");
            request.setContentLength(payload.length());

            // Send request with payload
            std::ostream& os = session.sendRequest(request);
            os << payload;

            // Get and process the response from Discord
            Poco::Net::HTTPResponse response;
            std::istream& rs = session.receiveResponse(response);
            std::stringstream ss;
            Poco::StreamCopier::copyStream(rs, ss);

            // Log the response for debugging
            std::string responseBody = ss.str();
			if (!responseBody.empty())
				TC_LOG_INFO("server.hooks", "Discord webhook sent. Response: %s", responseBody.c_str());
			else
				TC_LOG_INFO("server.hooks", "Discord webhook sent. No response body received.");

            // Check for error in response
            if (responseBody.find("error") != std::string::npos)
            {
                TC_LOG_ERROR("server.hooks", "Discord Webhook Error: %s", responseBody.c_str());
            }
        }
        catch (const Poco::Exception& ex)
        {
            // Log exceptions and errors
            TC_LOG_ERROR("server.hooks", "Discord webhook failed: %s", ex.displayText().c_str());
        }
    }
};

void AddDiscordWebhookServerHookScripts()
{
    new DiscordWebhookServerHook();
}