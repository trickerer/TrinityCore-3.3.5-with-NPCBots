#ifndef DISCORD_WEBHOOK_MGR_H
#define DISCORD_WEBHOOK_MGR_H

#include <string>

void SendDiscordMessage(const std::string& message);
void SendDiscordMessageWorld(const std::string& message);
void SendBattlegroundDiscordWebhook(const std::string& webhookUrl, const std::string& battlegroundName, uint32 alliancePlayers, uint32 hordePlayers, const std::string& bracket);

#endif // DISCORD_WEBHOOK_MGR_H
