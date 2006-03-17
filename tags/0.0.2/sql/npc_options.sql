# MySQL-Front 3.2  (Build 7.19)

# Host: localhost    Database: mangos
# ------------------------------------------------------
# Server version 4.0.26-nt

USE `mangos`;

#
# Table structure for table npc_options
#

DROP TABLE IF EXISTS `npc_options`;
CREATE TABLE `npc_options` (
  `ID` int(11) NOT NULL auto_increment,
  `NPC_GUID` int(11) NOT NULL default '0',
  `ICON` int(5) default NULL,
  `OPTIONTEXT` text NOT NULL,
  `NPC_TEXT_ID` int(11) default '0',
  `SPECIAL` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) TYPE=MyISAM;

#
# Dumping data for table npc_options
#

/*!40101 SET NAMES  */;

INSERT INTO `npc_options` VALUES (1,63933,2,'test 2',639331,1);
INSERT INTO `npc_options` VALUES (2,63933,1,'yo yo ',639331,1);
INSERT INTO `npc_options` VALUES (3,999999,1,'Greetings $c $N, how may I help you?\r\n',999999,3);
INSERT INTO `npc_options` VALUES (4,999998,1,'Greetings $N, how may I help you?',999998,1);
INSERT INTO `npc_options` VALUES (5,999997,1,'Greetings $c, how may I help you?',999997,1);
INSERT INTO `npc_options` VALUES (6,999996,1,'Greetings $N.\r\n\r\nI\'m sorry but I don\'t currently have any quests for you.\r\n',999996,1);
INSERT INTO `npc_options` VALUES (7,999995,1,'Greetings $c.\r\n\r\nI\'m sorry but I don\'t currently have any quests for you.',999995,1);
INSERT INTO `npc_options` VALUES (8,999994,1,'What is it $N? Can\'t you see I\'m busy?',999994,1);
INSERT INTO `npc_options` VALUES (9,999993,1,'What is it $c? Can\'t you see I\'m busy?',999993,1);

/*!40101 SET NAMES latin1 */;

