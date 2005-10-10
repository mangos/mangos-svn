# MySQL-Front 3.2  (Build 7.19)

# Host: localhost    Database: mangos
# ------------------------------------------------------
# Server version 4.0.26-nt

USE `mangos`;

#
# Table structure for table npc_gossip
#

DROP TABLE IF EXISTS `npc_gossip`;
CREATE TABLE `npc_gossip` (
  `NPC_GUID` int(11) NOT NULL default '0',
  `TEXTID` int(30) NOT NULL default '0',
  PRIMARY KEY  (`NPC_GUID`)
) TYPE=MyISAM;

#
# Dumping data for table npc_gossip
#

/*!40101 SET NAMES  */;

INSERT INTO `npc_gossip` VALUES (63933,63933);
INSERT INTO `npc_gossip` VALUES (999993,999993);
INSERT INTO `npc_gossip` VALUES (999994,999994);
INSERT INTO `npc_gossip` VALUES (999995,999995);
INSERT INTO `npc_gossip` VALUES (999996,999996);
INSERT INTO `npc_gossip` VALUES (999997,999997);
INSERT INTO `npc_gossip` VALUES (999998,999998);
INSERT INTO `npc_gossip` VALUES (999999,999999);

/*!40101 SET NAMES latin1 */;

