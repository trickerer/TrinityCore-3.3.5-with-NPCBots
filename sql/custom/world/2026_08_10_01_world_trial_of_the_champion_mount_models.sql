-- Trial of the Champion: assign cosmetic mount display IDs.
-- MountCreatureID is unused by the TrinityCore 3.3.5 branch.

INSERT IGNORE INTO creature_template_addon (entry)
VALUES
    (35314),
    (35323),
    (35325),
    (35326),
    (35327);

UPDATE creature_template_addon AS addon
JOIN creature_template AS model
    ON model.entry =
        CASE addon.entry
            WHEN 35314 THEN 33320 -- Orgrimmar Wolf
            WHEN 35323 THEN 33321 -- Darkspear Raptor
            WHEN 35325 THEN 33322 -- Thunder Bluff Kodo
            WHEN 35326 THEN 33323 -- Silvermoon Hawkstrider
            WHEN 35327 THEN 33324 -- Forsaken Warhorse
        END
SET addon.mount = model.modelid1,
    addon.MountCreatureID = 0
WHERE addon.entry IN (35314,35323,35325,35326,35327);
