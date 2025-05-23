#include "ScriptMgr.h"
#include "Chat.h"
#include "CreatureAI.h"
#include "CreatureGroups.h"
#include "DatabaseEnv.h"
#include "FollowMovementGenerator.h"
#include "GameTime.h"
#include "Language.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MovementDefines.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Player.h"
#include "RBAC.h"
#include "SmartEnum.h"
#include "Transport.h"
#include "World.h"
#include "WorldSession.h"
#include <unordered_map>
#include <ctime>
#include <cstdlib>

using namespace Trinity::ChatCommands;

std::unordered_map<uint64, std::string> g_DiscordCodes;

class discord_verify_commandscript : public CommandScript
{
public:
    discord_verify_commandscript() : CommandScript("discord_verify_commandscript") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        static ChatCommandTable discordVerifyCommandTable =
        {
            { "getdiscordcode", HandleGetDiscordCode, SEC_PLAYER, Console::No }
        };

        static ChatCommandTable commandTable =
        {
            { "discordverify", discordVerifyCommandTable }
        };

        return commandTable;
    }

    static bool HandleGetDiscordCode(ChatHandler* handler)
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