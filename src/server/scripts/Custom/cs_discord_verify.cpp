#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"
#include <unordered_map>
#include <ctime>

std::unordered_map<uint64, std::string> g_DiscordCodes;

class discord_verify_commandscript : public CommandScript
{
public:
    discord_verify_commandscript() : CommandScript("discord_verify_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commands =
        {
            { "getdiscordcode", SEC_PLAYER, false, &HandleGetDiscordCode, "", "" }
        };
        return commands;
    }

    static bool HandleGetDiscordCode(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        uint64 guid = player->GetGUID();

        std::string code = GenerateCode(6);
        g_DiscordCodes[guid] = code;

        handler->PSendSysMessage("Join our Discord and DM the bot with this code: |cffffcc00%s|r", code.c_str());
        return true;
    }

    static std::string GenerateCode(size_t length)
    {
        static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string result;
        result.reserve(length);

        std::srand(std::time(nullptr));
        for (size_t i = 0; i < length; ++i)
            result += charset[rand() % (sizeof(charset) - 1)];

        return result;
    }
};

void AddSC_discord_verify_commandscript()
{
    new discord_verify_commandscript();
}