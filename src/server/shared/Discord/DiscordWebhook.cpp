#include "DiscordWebhook.h"
#include <iostream>
#include <sstream>
#include DiscordWebhookMgr.h

DiscordWebhook::DiscordWebhook(const std::string& webhookUrl)
    : m_webhookUrl(webhookUrl)
{
}

void DiscordWebhook::SendMessage(const std::string& message)
{
    // Create a JSON payload with the message
    std::stringstream jsonPayload;
    jsonPayload << "{\"content\": \"" << message << "\"}";

    // Send the HTTP POST request with the JSON payload
    if (SendHttpRequest(jsonPayload.str()))
    {
        std::cout << "Message sent successfully to Discord!" << std::endl;
    }
    else
    {
        std::cout << "Failed to send message to Discord!" << std::endl;
    }
}

bool DiscordWebhook::SendHttpRequest(const std::string& jsonData)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        return false;
    }

    // Set the URL and headers
    curl_easy_setopt(curl, CURLOPT_URL, m_webhookUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

    // Set content-type header
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    // Clean up and return success or failure
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return res == CURLE_OK;
}