-- TDB 335.24111 characters
UPDATE `updates` SET `state`='ARCHIVED';
ALTER TABLE `character_queststatus_seasonal` ADD `completedTime` bigint NOT NULL DEFAULT '0' AFTER `event`;

UPDATE `character_queststatus_seasonal` SET `completedTime` = UNIX_TIMESTAMP();

DELETE FROM `worldstates` WHERE `entry` BETWEEN 1 AND 85;
ALTER TABLE `quest_tracker`
  MODIFY COLUMN `id` int UNSIGNED NOT NULL DEFAULT 0 FIRST,
  ADD UNIQUE INDEX `idx_latest_quest_for_character`(`id`, `character_guid`, `quest_accept_time` DESC);
