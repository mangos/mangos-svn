# MySQL-Front 3.2  (Build 7.19)

# Host: localhost    Database: mangos
# ------------------------------------------------------
# Server version 4.0.26-nt

USE `mangos`;

#
# Table structure for table npc_text
#

DROP TABLE IF EXISTS `npc_text`;
CREATE TABLE `npc_text` (
  `ID` int(11) NOT NULL default '0',
  `TEXT` longtext NOT NULL,
  PRIMARY KEY  (`ID`)
) TYPE=MyISAM;

#
# Dumping data for table npc_text
#

/*!40101 SET NAMES  */;

INSERT INTO `npc_text` VALUES (63933,'Hey Dumbass!');
INSERT INTO `npc_text` VALUES (639331,'WoooHOOO!!!');
INSERT INTO `npc_text` VALUES (999993,'What is it $c? Can\'t you see I\'m busy?');
INSERT INTO `npc_text` VALUES (999994,'What is it $N? Can\'t you see I\'m busy?');
INSERT INTO `npc_text` VALUES (999995,'Greetings $c.\r\n\r\nI\'m sorry but I don\'t currently have any quests for you.\r\n');
INSERT INTO `npc_text` VALUES (999996,'Greetings $N.\r\n\r\nI\'m sorry but I don\'t currently have any quests for you.\r\n');
INSERT INTO `npc_text` VALUES (999997,'Greetings $c, how may I help you?');
INSERT INTO `npc_text` VALUES (999998,'Greetings $N, how may I help you?\r\n');
INSERT INTO `npc_text` VALUES (999999,'Greetings $c $N, how may I help you?\r\n');

/*!40101 SET NAMES latin1 */;

