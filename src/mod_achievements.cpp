/*
<--------------------------------------------------------------------------->
- Developer(s): Lille Carl, Grim/Render
- Complete: %100
- ScriptName: 'AccountAchievements'
- Comment: Tested and Works.
- Orginial Creator: Lille Carl
- Edited: Render/Grim
<--------------------------------------------------------------------------->
*/

#include "mod_achievements.h"

#include "Config.h"
#include "Chat.h"
#include "Player.h"

void AccountAchievementsWorldScript::LoadAchievementWhitelist()
{
    auto qResult = WorldDatabase.Query("SELECT achievement_id FROM account_achievements_whitelist");

    if (!qResult)
    {
        LOG_WARN("module", ">> Loaded 0 account achievements from account_achievements_whitelist.");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = qResult->Fetch();

        uint32 achievementId = fields[0].Get<uint32>();

        achievementWhitelist.emplace(achievementId);

        count++;
    }
    while (qResult->NextRow());

    LOG_INFO("module", Acore::StringFormatFmt(">> Loaded {} account achievements from account_achievements_whitelist.", count));
}

void AccountAchievementsWorldScript::OnAfterConfigLoad(bool reload)
{
    if (reload)
    {
        achievementWhitelist.clear();
    }

    LoadAchievementWhitelist();
}

void AccountAchievementsPlayerScript::OnLogin(Player* player)
{
    if (!sConfigMgr->GetOption<bool>("Account.Achievements.Enable", true))
    {
        return;
    }

    if (sConfigMgr->GetOption<bool>("Account.Achievements.Announce", true))
    {
        ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00AccountAchievements |rmodule.");
    }

    QueryResult result1 = CharacterDatabase.Query("SELECT guid, race FROM characters WHERE account = {}", player->GetSession()->GetAccountId());

    if (!result1)
        return;

    std::vector<uint32> guids;

    do
    {
        Field* fields = result1->Fetch();

        uint32 race = fields[1].Get<uint8>();

        if ((Player::TeamIdForRace(race) == player->GetTeamId()))
        {
            guids.push_back(result1->Fetch()[0].Get<uint32>());
        }

    } while (result1->NextRow());

    std::vector<uint32> achievements;

    for (auto& i : guids)
    {
        QueryResult result2 = CharacterDatabase.Query("SELECT achievement FROM character_achievement WHERE guid = {}", i);

        if (!result2)
            continue;

        do
        {
            uint32 achievementId = result2->Fetch()[0].Get<uint32>();

            // Only include whitelisted achievements.
            if (!AchievementWhitelisted(achievementId))
            {
                continue;
            }

            achievements.push_back(achievementId);
        } while (result2->NextRow());
    }

    for (auto& i : achievements)
    {
        AddAchievements(player, i);
    }
}

void AccountAchievementsPlayerScript::AddAchievements(Player* player, uint32 achievementId)
{
    if(player->HasAchieved(achievementId))
    {
        return;
    }

    auto achievement = sAchievementStore.LookupEntry(achievementId);
    if (!achievement)
    {
        LOG_ERROR("module", Acore::StringFormatFmt("Tried to add achievement '{}' to player '{}' but the achievement does not exist!", player->GetPlayerName(), achievementId));
        return;
    }

    player->CompletedAchievement(achievement);
}

bool AccountAchievementsPlayerScript::AchievementWhitelisted(uint32 achievementId)
{
    return achievementWhitelist.find(achievementId) != achievementWhitelist.end();
}

void AddAccountAchievementsScripts()
{
    new AccountAchievementsWorldScript();
	new AccountAchievementsPlayerScript();
}
