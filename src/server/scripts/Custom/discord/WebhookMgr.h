
#ifndef _WEBHOOK_MGR_H
#define _WEBHOOK_MGR_H

#include "Config.h"
#include "Chat.h"

class WebhookMgr {

private:
    std::string _webhookUrl;
    std::thread _workerThread; // Store the thread
    std::atomic<bool> _stopWorker{ false }; // Atomic flag to signal the thread to stop
    std::queue<std::string> _messageQueue; // The message queue
    std::mutex _queueMutex;
    std::condition_variable _condition;
    void ProcessMessages();
    void SendDiscordWebhook(const std::string& message);

public:
    static WebhookMgr* instance();
    ~WebhookMgr();
    void ScheduleMessage(const std::string& rawMessage);
    void SetWebhookUrl(std::string& url);
    void Start();
    void Stop();
};

#define sWebhookMgr WebhookMgr::instance()

#endif
