ALTER TABLE `account` CHANGE `password` `I` VARCHAR( 40 ) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT 'authentification hash';
UPDATE `account` SET `I`=SHA1(CONCAT(UPPER(`username`),':',UPPER(`I`)));
