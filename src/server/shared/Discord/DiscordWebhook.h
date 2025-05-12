#ifndef DISCORDWEBHOOK_H
#define DISCORDWEBHOOK_H

#include <string>
#include DiscordWebhookMgr.h

class DiscordWebhook
{
public:
    // Constructor that accepts a webhook URL
    DiscordWebhook(const std::string& webhookUrl);

    // Sends a message to the Discord webhook
    void SendMessage(const std::string& message);

private:
    std::string m_webhookUrl;
    
    // Helper function to send an HTTP POST request
    bool SendHttpRequest(const std::string& jsonData);
};

#endif // DISCORDWEBHOOK_H