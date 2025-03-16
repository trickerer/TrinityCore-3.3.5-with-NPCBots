--
SET @PERMISSION_START = 70001;
SET @PERMISSION_END   = 70033;

DELETE FROM `rbac_permissions` WHERE id BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
('70001','Command: npcbot'),
('70002','Command: npcbot add'),
('70003','Command: npcbot remove'),
('70004','Command: npcbot spawn'),
('70005','Command: npcbot move'),
('70006','Command: npcbot delete'),
('70007','Command: npcbot lookup'),
('70008','Command: npcbot revive'),
('70009','Command: npcbot reloadconfig'),
('70010','Command: npcbot info'),
('70011','Command: npcbot hide'),
('70012','Command: npcbot unhide'),
('70013','Command: npcbot recall'),
('70014','Command: npcbot kill'),
('70015','Command: npcbot debug raid'),
('70016','Command: npcbot debug mount'),
('70017','Command: npcbot debug spellvisual'),
('70018','Command: npcbot debug states'),
('70019','Command: npcbot toggle flags'),
('70020','Command: npcbot set faction'),
('70021','Command: npcbot set owner'),
('70022','Command: npcbot set spec'),
('70023','Command: npcbot command standstill'),
('70024','Command: npcbot command stopfully'),
('70025','Command: npcbot command follow'),
('70026','Command: npcbot distance attack short'),
('70027','Command: npcbot distance attack long'),
('70028','Command: npcbot distance attack'),
('70029','Command: npcbot distance'),
('70030','Command: npcbot order cast'),
('70031','Command: npcbot vehicle eject'),
('70032','Command: npcbot dump load'),
('70033','Command: npcbot dump write');
--
SET @PERMISSION_START = 70001;
SET @PERMISSION_END   = 70033;

DELETE FROM `rbac_linked_permissions` WHERE linkedId BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
('199','70001'),
('197','70002'),
('197','70003'),
('197','70004'),
('197','70005'),
('197','70006'),
('197','70007'),
('197','70008'),
('197','70009'),
('199','70010'),
('199','70011'),
('199','70012'),
('199','70013'),
('199','70014'),
('197','70015'),
('197','70016'),
('197','70017'),
('197','70018'),
('197','70019'),
('197','70020'),
('197','70021'),
('197','70022'),
('199','70023'),
('199','70024'),
('199','70025'),
('199','70026'),
('199','70027'),
('199','70028'),
('199','70029'),
('199','70030'),
('199','70031'),
('196','70032'),
('196','70033');
--
SET @PERMISSION_START = 70034;
SET @PERMISSION_END   = 70034;

DELETE FROM `rbac_permissions` WHERE id BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
('70034','Command: npcbot spawned');
--
SET @PERMISSION_START = 70034;
SET @PERMISSION_END   = 70034;

DELETE FROM `rbac_linked_permissions` WHERE linkedId BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
('196','70034');
--
SET @PERMISSION_START = 70035;
SET @PERMISSION_END   = 70035;

DELETE FROM `rbac_permissions` WHERE id BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
('70035','Command: npcbot command walk');
--
SET @PERMISSION_START = 70035;
SET @PERMISSION_END   = 70035;

DELETE FROM `rbac_linked_permissions` WHERE linkedId BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
('199','70035');
--
SET @PERMISSION_START = 70036;
SET @PERMISSION_END   = 70036;

DELETE FROM `rbac_permissions` WHERE id BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
('70036','Command: npcbot createnew');
--
SET @PERMISSION_START = 70036;
SET @PERMISSION_END   = 70036;

DELETE FROM `rbac_linked_permissions` WHERE linkedId BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
('196','70036');
--
SET @PERMISSION_START = 70037;
SET @PERMISSION_END   = 70037;

DELETE FROM `rbac_permissions` WHERE id BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
('70037','Command: npcbot sendto');
--
SET @PERMISSION_START = 70037;
SET @PERMISSION_END   = 70037;

DELETE FROM `rbac_linked_permissions` WHERE linkedId BETWEEN @PERMISSION_START AND @PERMISSION_END;
INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
('199','70037');
--
UPDATE `rbac_linked_permissions` SET `id`='197' WHERE (`id`='196') AND (`linkedId`='70034');
