#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <string>
#include "WebhookMgr.h"
#include "Chat.h"
#include <csignal>

WebhookMgr* WebhookMgr::instance()
{
    static WebhookMgr instance;
    return &instance;
}

WebhookMgr::~WebhookMgr()
{
    Stop(); // Ensure the worker thread is stopped
}

void WebhookMgr::ScheduleMessage(const std::string& rawMessage)
{
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _messageQueue.push(rawMessage);
    }
    _condition.notify_one(); // Notify the worker thread that a message is available
}

void WebhookMgr::Start()
{
    _stopWorker = false;
    _workerThread = std::thread(&WebhookMgr::ProcessMessages, this);
}

void WebhookMgr::Stop()
{
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _stopWorker = true; // Signal the thread to stop
    }
    _condition.notify_all(); // Wake up the thread if it’s waiting

    if (_workerThread.joinable())
    {
        _workerThread.join(); // Wait for the thread to finish
    }
}

void WebhookMgr::ProcessMessages()
{
    while (true) {
        std::string message;

        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _condition.wait(lock, [this]
                {
                return !_messageQueue.empty() || _stopWorker;
                });

            // Exit immediately if stopWorker is set
            if (_stopWorker)
            {
                // Clear the queue
                std::queue<std::string> emptyQueue;
                std::swap(_messageQueue, emptyQueue);
                break;
            }

            // Retrieve the next message
            message = _messageQueue.front();
            _messageQueue.pop();
        }

        // Send the message (rate-limited)
        SendDiscordWebhook(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(600)); // Break for rate limit. Currently, 5 requests per 2 seconds
    }

    LOG_INFO("server.worldserver", "Webhook processor closed.");
}

void WebhookMgr::SendDiscordWebhook(const std::string& rawMessage)
{
    try {
        // Extract Discord API path from the webhook URL
        const std::string host = "discord.com";
        const std::string port = "443";

        if (std::empty(_webhookUrl))
        {
            LOG_ERROR("server.loading", "The webhook url is empty. Please provide one.");
            Stop();
            return;
        }

        size_t apiStart = _webhookUrl.find("/api/webhooks");
        if (apiStart == std::string::npos)
        {
            LOG_ERROR("server.worldserver", "Invalid webhook url provided. Stopping module.");
            Stop();
            return;
        }
        const std::string apiPath = _webhookUrl.substr(apiStart);

        // Format the message into JSON
        std::string formattedMessage = R"({"content":")" + rawMessage + R"("})";

        // Boost.Asio context and resolver setup
        boost::asio::io_context io_context;
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23_client);
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream(io_context, ssl_context);

        // Resolve Discord's address
        auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(stream.next_layer(), endpoints);

        // Perform the SSL handshake
        stream.handshake(boost::asio::ssl::stream_base::client);

        // Construct the HTTP POST request
        std::string request =
            "POST " + apiPath + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Content-Type: application/json\r\n" +
            "Content-Length: " + std::to_string(formattedMessage.size()) + "\r\n" +
            "Connection: close\r\n\r\n" +
            formattedMessage;

        // Send the request
        boost::asio::write(stream, boost::asio::buffer(request));

        // Read the response
        boost::asio::streambuf response;
        boost::asio::read_until(stream, response, "\r\n");

        // Print the response status
        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;
        response_stream >> http_version >> status_code;
        std::getline(response_stream, status_message);

        if(status_code != 204)
        {
            std::ostringstream ss;
            ss << "Failed to send webhook.HTTP Status : " << status_code << " " << status_message << std::endl;
            LOG_ERROR("server.worldserver", ss.str());
        }
    }
    catch (const std::exception& e)
    {
        std::ostringstream ss;
        ss << "Error: " << e.what() << std::endl;
        LOG_ERROR("server.worldserver", ss.str());
    }
}

void WebhookMgr::SetWebhookUrl(std::string& url)
{
    _webhookUrl = url;
}

