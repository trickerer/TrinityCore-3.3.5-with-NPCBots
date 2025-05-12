#ifndef DISCORD_WEBHOOK_MGR_H
#define DISCORD_WEBHOOK_MGR_H

#include <string>

static void SendDiscordMessage(const std::string& message);
void SendBattlegroundDiscordWebhook(const std::string& webhookUrl, const std::string& battlegroundName, uint32 alliancePlayers, uint32 hordePlayers);

#endif // DISCORD_WEBHOOK_MGR_H
