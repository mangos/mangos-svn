-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:19 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `quests`
-- 
-- Creation: Aug 26, 2005 at 12:24 AM
-- Last update: Aug 26, 2005 at 12:24 AM
-- 

DROP TABLE IF EXISTS `quests`;
CREATE TABLE `quests` (
  `questId` bigint(20) unsigned NOT NULL auto_increment,
  `zoneId` bigint(20) unsigned NOT NULL default '0',
  `title` longtext collate latin1_general_ci,
  `details` longtext collate latin1_general_ci,
  `objectives` longtext collate latin1_general_ci,
  `completedText` longtext collate latin1_general_ci,
  `incompleteText` longtext collate latin1_general_ci,
  `targetGuid` bigint(20) unsigned NOT NULL default '0',
  `questItemId1` bigint(20) unsigned NOT NULL default '0',
  `questItemId2` bigint(20) unsigned NOT NULL default '0',
  `questItemId3` bigint(20) unsigned NOT NULL default '0',
  `questItemId4` bigint(20) unsigned NOT NULL default '0',
  `questItemCount1` bigint(20) unsigned NOT NULL default '0',
  `questItemCount2` bigint(20) unsigned NOT NULL default '0',
  `questItemCount3` bigint(20) unsigned NOT NULL default '0',
  `questItemCount4` bigint(20) unsigned NOT NULL default '0',
  `questMobId1` bigint(20) unsigned NOT NULL default '0',
  `questMobId2` bigint(20) unsigned NOT NULL default '0',
  `questMobId3` bigint(20) unsigned NOT NULL default '0',
  `questMobId4` bigint(20) unsigned NOT NULL default '0',
  `questMobCount1` bigint(20) unsigned NOT NULL default '0',
  `questMobCount2` bigint(20) unsigned NOT NULL default '0',
  `questMobCount3` bigint(20) unsigned NOT NULL default '0',
  `questMobCount4` bigint(20) unsigned NOT NULL default '0',
  `numChoiceRewards` int(5) unsigned NOT NULL default '0',
  `choiceItemId1` bigint(20) unsigned NOT NULL default '0',
  `choiceItemId2` bigint(20) unsigned NOT NULL default '0',
  `choiceItemId3` bigint(20) unsigned NOT NULL default '0',
  `choiceItemId4` bigint(20) unsigned NOT NULL default '0',
  `choiceItemId5` bigint(20) unsigned NOT NULL default '0',
  `choiceItemCount1` bigint(20) unsigned NOT NULL default '0',
  `choiceItemCount2` bigint(20) unsigned NOT NULL default '0',
  `choiceItemCount3` bigint(20) unsigned NOT NULL default '0',
  `choiceItemCount4` bigint(20) unsigned NOT NULL default '0',
  `choiceItemCount5` bigint(20) unsigned NOT NULL default '0',
  `numItemRewards` int(5) unsigned NOT NULL default '0',
  `rewardItemId1` bigint(20) unsigned NOT NULL default '0',
  `rewardItemId2` bigint(20) unsigned NOT NULL default '0',
  `rewardItemId3` bigint(20) unsigned NOT NULL default '0',
  `rewardItemId4` bigint(20) unsigned NOT NULL default '0',
  `rewardItemId5` bigint(20) unsigned NOT NULL default '0',
  `rewardItemCount1` bigint(20) unsigned NOT NULL default '0',
  `rewardItemCount2` bigint(20) unsigned NOT NULL default '0',
  `rewardItemCount3` bigint(20) unsigned NOT NULL default '0',
  `rewardItemCount4` bigint(20) unsigned NOT NULL default '0',
  `rewardItemCount5` bigint(20) unsigned NOT NULL default '0',
  `rewardGold` bigint(20) unsigned NOT NULL default '0',
  `questXp` bigint(20) unsigned NOT NULL default '0',
  `originalGuid` bigint(20) unsigned NOT NULL default '0',
  `requiredLevel` bigint(20) unsigned NOT NULL default '0',
  `previousQuest` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`questId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 18432 kB' AUTO_INCREMENT=1 ;
