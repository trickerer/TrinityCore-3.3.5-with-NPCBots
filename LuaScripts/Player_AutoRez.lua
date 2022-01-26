--[[
    EmuDevs <http://emudevs.com/forum.php>
    Eluna Lua Engine <https://github.com/ElunaLuaEngine/Eluna>
    Eluna Scripts <https://github.com/ElunaLuaEngine/Scripts>
    Eluna Wiki <http://wiki.emudevs.com/doku.php?id=eluna>
    -= Script Information =-
    * Script Type: Auto Rez
    * Npc: Carla Lightbringer <376581>
--]]

-- Configurable
local SpawnMessage = 'Greetings $N, it looks like you are in need of assistance!'
local RezMessage = '... Normally there is a fee, but I can waive it this time...'
local RezSpellToCast = 72429
local LightbringerNPC = 376581
local spawnHeight = 9
local timeTillDespawn = 15

-- Events
local PLAYER_EVENT_ON_KILLED_BY_CREATURE = 8 -- (event, killer, killed)
local TEMPSUMMON_TIMED_OR_DEAD_DESPAWN = 1 -- despawns after a specified time OR when the creature disappears

-- SQL Update for new NPC
local creatureSQL = 'REPLACE INTO `creature_template` (`entry`,`difficulty_entry_1`,`difficulty_entry_2`,`difficulty_entry_3`,`KillCredit1`,`KillCredit2`,`modelid1`,`modelid2`,`modelid3`,`modelid4`,`name`,`subname`,`IconName`,`gossip_menu_id`,`minlevel`,`maxlevel`,`exp`,`faction`,`npcflag`,`speed_walk`,`speed_run`,`scale`,`rank`,`dmgschool`,`BaseAttackTime`,`RangeAttackTime`,`BaseVariance`,`RangeVariance`,`unit_class`,`unit_flags`,`unit_flags2`,`dynamicflags`,`family`,`type`,`type_flags`,`lootid`,`pickpocketloot`,`skinloot`,`PetSpellDataId`,`VehicleId`,`mingold`,`maxgold`,`AIName`,`MovementType`,`HoverHeight`,`HealthModifier`,`ManaModifier`,`ArmorModifier`,`DamageModifier`,`ExperienceModifier`,`RacialLeader`,`movementId`,`RegenHealth`,`mechanic_immune_mask`,`spell_school_immune_mask`,`flags_extra`,`ScriptName`,`VerifiedBuild`) VALUES (' ..LightbringerNPC.. ',0,0,0,0,0,24991,0,0,0,\'Carla Lightbringer\',\'Johnston Division Commander\',NULL,0,70,80,2,35,0,1,1.14286,0,2,0,2000,2000,0,0,1,0,2048,2,0,6,2,0,0,0,0,0,0,0,\'\',0,4,1,1,1,1,1,0,0,1,8388625,0,320,\'\',12340)'

WorldDBQuery(creatureSQL)

-- On Player Death by Creature we send the Calvery
function OnPlayerKilledByCreature(event, killer, player)
		
	-- Spawn Healer
	local creature = killer:SpawnCreature(
		LightbringerNPC, 
		player:GetX(), 
		player:GetY(), 
		player:GetZ() + spawnHeight, 
		player:GetO(), 
		TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,
		timeTillDespawn)
	
	-- Whisper the Player
	creature:SendUnitWhisper(SpawnMessage, 0, player, false)
	creature:SendUnitWhisper(RezMessage, 0, player, false)
	creature:CastSpell(player, RezSpellToCast)
	
	-- Random Move
	creature:MoveRandom(20)
	
	
end

RegisterPlayerEvent(PLAYER_EVENT_ON_KILLED_BY_CREATURE, OnPlayerKilledByCreature)

