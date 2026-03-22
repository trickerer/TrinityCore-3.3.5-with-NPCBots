
/*Table structure for table `characters_npcbot` */

DROP TABLE IF EXISTS `characters_npcbot`;

CREATE TABLE `characters_npcbot` (
  `entry` int unsigned NOT NULL COMMENT 'creature_template.entry',
  `owner` int unsigned NOT NULL DEFAULT '0' COMMENT 'characters.guid (lowguid)',
  `roles` int unsigned NOT NULL COMMENT 'bitmask: tank(1),dps(2),heal(4),ranged(8)',
  `spec` tinyint unsigned NOT NULL DEFAULT '1',
  `faction` int unsigned NOT NULL DEFAULT '35',
  `hire_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `shared_owners` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `equipMhEx` int unsigned NOT NULL DEFAULT '0',
  `equipOhEx` int unsigned NOT NULL DEFAULT '0',
  `equipRhEx` int unsigned NOT NULL DEFAULT '0',
  `equipHead` int unsigned NOT NULL DEFAULT '0',
  `equipShoulders` int unsigned NOT NULL DEFAULT '0',
  `equipChest` int unsigned NOT NULL DEFAULT '0',
  `equipWaist` int unsigned NOT NULL DEFAULT '0',
  `equipLegs` int unsigned NOT NULL DEFAULT '0',
  `equipFeet` int unsigned NOT NULL DEFAULT '0',
  `equipWrist` int unsigned NOT NULL DEFAULT '0',
  `equipHands` int unsigned NOT NULL DEFAULT '0',
  `equipBack` int unsigned NOT NULL DEFAULT '0',
  `equipBody` int unsigned NOT NULL DEFAULT '0',
  `equipFinger1` int unsigned NOT NULL DEFAULT '0',
  `equipFinger2` int unsigned NOT NULL DEFAULT '0',
  `equipTrinket1` int unsigned NOT NULL DEFAULT '0',
  `equipTrinket2` int unsigned NOT NULL DEFAULT '0',
  `equipNeck` int unsigned NOT NULL DEFAULT '0',
  `spells_disabled` longtext COLLATE utf8mb4_unicode_ci,
  `miscvalues` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

/*Data for the table `characters_npcbot` */

/*Table structure for table `characters_npcbot_gear_set` */

DROP TABLE IF EXISTS `characters_npcbot_gear_set`;

CREATE TABLE `characters_npcbot_gear_set` (
  `owner` int unsigned NOT NULL DEFAULT '0',
  `set_id` tinyint unsigned NOT NULL DEFAULT '0',
  `set_name` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`owner`,`set_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Bot equipment sets system';

/*Data for the table `characters_npcbot_gear_set` */

/*Table structure for table `characters_npcbot_gear_set_item` */

DROP TABLE IF EXISTS `characters_npcbot_gear_set_item`;

CREATE TABLE `characters_npcbot_gear_set_item` (
  `owner` int unsigned NOT NULL DEFAULT '0',
  `set_id` tinyint unsigned NOT NULL DEFAULT '0',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `item_id` mediumint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`owner`,`set_id`,`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Bot equipment sets system';

/*Data for the table `characters_npcbot_gear_set_item` */

/*Table structure for table `characters_npcbot_gear_storage` */

DROP TABLE IF EXISTS `characters_npcbot_gear_storage`;

CREATE TABLE `characters_npcbot_gear_storage` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `item_guid` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`),
  KEY `existing_player` (`guid`),
  CONSTRAINT `characters_npcbot_gear_storage_ibfk_1` FOREIGN KEY (`item_guid`) REFERENCES `item_instance` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `existing_player` FOREIGN KEY (`guid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Bot item storage system';

/*Data for the table `characters_npcbot_gear_storage` */

/*Table structure for table `characters_npcbot_group_member` */

DROP TABLE IF EXISTS `characters_npcbot_group_member`;

CREATE TABLE `characters_npcbot_group_member` (
  `guid` int unsigned NOT NULL,
  `entry` int unsigned NOT NULL,
  `memberFlags` tinyint unsigned NOT NULL DEFAULT '0',
  `subgroup` tinyint unsigned NOT NULL DEFAULT '0',
  `roles` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

/*Data for the table `characters_npcbot_group_member` */

/*Table structure for table `characters_npcbot_logs` */

DROP TABLE IF EXISTS `characters_npcbot_logs`;

CREATE TABLE `characters_npcbot_logs` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `entry` int unsigned NOT NULL DEFAULT '0',
  `owner` int NOT NULL DEFAULT '-1',
  `mapid` int NOT NULL DEFAULT '-1',
  `inmap` tinyint NOT NULL DEFAULT '-1',
  `inworld` tinyint NOT NULL DEFAULT '-1',
  `type` smallint unsigned NOT NULL DEFAULT '0',
  `param1` varchar(51) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `param2` varchar(51) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `param3` varchar(51) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `param4` varchar(51) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `param5` varchar(51) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=552981 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

/*Data for the table `characters_npcbot_logs` */

/*Table structure for table `characters_npcbot_settings` */

DROP TABLE IF EXISTS `characters_npcbot_settings`;

CREATE TABLE `characters_npcbot_settings` (
  `owner` int unsigned NOT NULL,
  `dist_follow` tinyint unsigned NOT NULL DEFAULT '30',
  `dist_attack` tinyint unsigned NOT NULL DEFAULT '0',
  `attack_range_mode` tinyint unsigned NOT NULL DEFAULT '1',
  `attack_angle_mode` tinyint unsigned NOT NULL DEFAULT '1',
  `engage_delay_dps` int unsigned NOT NULL DEFAULT '0',
  `engage_delay_heal` int unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

/*Data for the table `characters_npcbot_settings` */

/*Table structure for table `characters_npcbot_stats` */

DROP TABLE IF EXISTS `characters_npcbot_stats`;

CREATE TABLE `characters_npcbot_stats` (
  `entry` int unsigned NOT NULL DEFAULT '0',
  `maxhealth` int unsigned NOT NULL DEFAULT '0',
  `maxpower` int unsigned NOT NULL DEFAULT '0',
  `strength` int unsigned NOT NULL DEFAULT '0',
  `agility` int unsigned NOT NULL DEFAULT '0',
  `stamina` int unsigned NOT NULL DEFAULT '0',
  `intellect` int unsigned NOT NULL DEFAULT '0',
  `spirit` int unsigned NOT NULL DEFAULT '0',
  `armor` int unsigned NOT NULL DEFAULT '0',
  `defense` int unsigned NOT NULL DEFAULT '0',
  `resHoly` int unsigned NOT NULL DEFAULT '0',
  `resFire` int unsigned NOT NULL DEFAULT '0',
  `resNature` int unsigned NOT NULL DEFAULT '0',
  `resFrost` int unsigned NOT NULL DEFAULT '0',
  `resShadow` int unsigned NOT NULL DEFAULT '0',
  `resArcane` int unsigned NOT NULL DEFAULT '0',
  `blockPct` float unsigned NOT NULL DEFAULT '0',
  `dodgePct` float unsigned NOT NULL DEFAULT '0',
  `parryPct` float unsigned NOT NULL DEFAULT '0',
  `critPct` float unsigned NOT NULL DEFAULT '0',
  `attackPower` int unsigned NOT NULL DEFAULT '0',
  `spellPower` int unsigned NOT NULL DEFAULT '0',
  `spellPen` int unsigned NOT NULL DEFAULT '0',
  `hastePct` float unsigned NOT NULL DEFAULT '0',
  `hitBonusPct` float unsigned NOT NULL DEFAULT '0',
  `expertise` int unsigned NOT NULL DEFAULT '0',
  `armorPenPct` float unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

/*Data for the table `characters_npcbot_stats` */

/*Table structure for table `characters_npcbot_transmog` */

DROP TABLE IF EXISTS `characters_npcbot_transmog`;

CREATE TABLE `characters_npcbot_transmog` (
  `entry` int unsigned NOT NULL,
  `slot` tinyint unsigned NOT NULL,
  `item_id` int unsigned NOT NULL DEFAULT '0',
  `fake_id` int NOT NULL DEFAULT '-1',
  PRIMARY KEY (`entry`,`slot`),
  CONSTRAINT `bot_id` FOREIGN KEY (`entry`) REFERENCES `characters_npcbot` (`entry`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

/*Data for the table `characters_npcbot_transmog` */