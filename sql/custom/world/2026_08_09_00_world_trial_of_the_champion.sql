-- Trial of the Champion: standalone lesser-champion rewrite
-- Horde lesser champions fought by Alliance players.
-- They are no longer passengers on 333xx vehicles.

UPDATE creature_template
SET AIName = '',
    ScriptName = 'npc_toc5_lesser_champion',
    faction = 14,
    npcflag = 0,
    unit_flags = 0
WHERE entry IN (35314,35323,35325,35326,35327);

UPDATE creature_template_addon
SET mount = 0,
    MountCreatureID = 0
WHERE entry IN (35314,35323,35325,35326,35327);

-- Remove automatic passengers from the old lesser vehicles.
DELETE FROM vehicle_template_accessory
WHERE entry IN (33320,33321,33322,33323,33324);

-- Restore the reward-chest configuration used by the scripted encounters.
-- Clear addon flags so all six normal/heroic reward chests remain usable.
UPDATE gameobject_template
SET Data0 = 1634
WHERE entry IN (195709,195710,195374,195375,195323,195324);

UPDATE gameobject_template_addon
SET flags = 0
WHERE entry IN (195709,195710,195374,195375,195323,195324);
