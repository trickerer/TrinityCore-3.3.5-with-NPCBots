#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"
#include "WorldSession.h"
#include <unordered_map>
#include <cstdlib>
#include <ctime>

std::unordered_map<uint64, std::string> g_DiscordCodes;

class discord_verify_commandscript : public CommandScript
{
public:
    discord_verify_commandscript() : CommandScript("discord_verify_commandscript") {}

    Trinity::ChatCommands::ChatCommandTable GetCommands() const override
    {
        using namespace Trinity::ChatCommands;

        static ChatCommandTable discordVerifyCommandTable =
        {
            ChatCommandBuilder("getdiscordcode")
                .SetHandler(&discord_verify_commandscript::HandleGetDiscordCode)
                .SetSecurity(SEC_PLAYER)
        };

        static ChatCommandTable commandTable =
        {
            ChatCommandBuilder("discordverify", discordVerifyCommandTable)
        };

        return commandTable;
    }

    static bool HandleGetDiscordCode(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        std::string code = GenerateCode(6);
        g_DiscordCodes[player->GetGUID()] = code;

        handler->PSendSysMessage("Join our Discord and DM the bot with this code: |cff00ff00%s|r", code.c_str());
        return true;
    }

    static std::string GenerateCode(size_t length)
    {
        static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i)
            result += charset[rand() % (sizeof(charset) - 1)];
        return result;
    }
};

void AddSC_discord_verify_commandscript()
{
    new discord_verify_commandscript();
}
