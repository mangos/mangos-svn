-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 11, 2005 at 08:26 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `quests`
-- 
-- Creation: Sep 11, 2005 at 08:21 PM
-- Last update: Sep 11, 2005 at 08:23 PM
-- 

DROP TABLE IF EXISTS `quests`;
CREATE TABLE `quests` (
  `questId` bigint(20) NOT NULL default '0',
  `zoneId` bigint(20) NOT NULL default '0',
  `questFlags` bigint(20) NOT NULL default '0',
  `title` longtext,
  `details` longtext,
  `objectives` longtext,
  `completedText` longtext,
  `incompleteText` longtext,
  `secondText` longtext,
  `partText1` longtext,
  `partText2` longtext,
  `partText3` longtext,
  `partText4` longtext,
  `reqLevel` bigint(20) NOT NULL default '0',
  `questLevel` bigint(20) NOT NULL default '0',
  `prevQuests` bigint(20) NOT NULL default '0',
  `previousQuest1` bigint(20) NOT NULL default '0',
  `previousQuest2` bigint(20) NOT NULL default '0',
  `previousQuest3` bigint(20) NOT NULL default '0',
  `previousQuest4` bigint(20) NOT NULL default '0',
  `previousQuest5` bigint(20) NOT NULL default '0',
  `previousQuest6` bigint(20) NOT NULL default '0',
  `previousQuest7` bigint(20) NOT NULL default '0',
  `previousQuest8` bigint(20) NOT NULL default '0',
  `previousQuest9` bigint(20) NOT NULL default '0',
  `previousQuest10` bigint(20) NOT NULL default '0',
  `lprevQuests` bigint(20) NOT NULL default '0',
  `lpreviousQuest1` bigint(20) NOT NULL default '0',
  `lpreviousQuest2` bigint(20) NOT NULL default '0',
  `lpreviousQuest3` bigint(20) NOT NULL default '0',
  `lpreviousQuest4` bigint(20) NOT NULL default '0',
  `lpreviousQuest5` bigint(20) NOT NULL default '0',
  `lpreviousQuest6` bigint(20) NOT NULL default '0',
  `lpreviousQuest7` bigint(20) NOT NULL default '0',
  `lpreviousQuest8` bigint(20) NOT NULL default '0',
  `lpreviousQuest9` bigint(20) NOT NULL default '0',
  `lpreviousQuest10` bigint(20) NOT NULL default '0',
  `lockQuests` bigint(20) NOT NULL default '0',
  `lockQuest1` bigint(20) NOT NULL default '0',
  `lockQuest2` bigint(20) NOT NULL default '0',
  `lockQuest3` bigint(20) NOT NULL default '0',
  `lockQuest4` bigint(20) NOT NULL default '0',
  `lockQuest5` bigint(20) NOT NULL default '0',
  `lockQuest6` bigint(20) NOT NULL default '0',
  `lockQuest7` bigint(20) NOT NULL default '0',
  `lockQuest8` bigint(20) NOT NULL default '0',
  `lockQuest9` bigint(20) NOT NULL default '0',
  `lockQuest10` bigint(20) NOT NULL default '0',
  `questItemId1` bigint(20) NOT NULL default '0',
  `questItemId2` bigint(20) NOT NULL default '0',
  `questItemId3` bigint(20) NOT NULL default '0',
  `questItemId4` bigint(20) NOT NULL default '0',
  `questItemCount1` bigint(20) NOT NULL default '0',
  `questItemCount2` bigint(20) NOT NULL default '0',
  `questItemCount3` bigint(20) NOT NULL default '0',
  `questItemCount4` bigint(20) NOT NULL default '0',
  `questMobId1` bigint(20) NOT NULL default '0',
  `questMobId2` bigint(20) NOT NULL default '0',
  `questMobId3` bigint(20) NOT NULL default '0',
  `questMobId4` bigint(20) NOT NULL default '0',
  `questMobCount1` bigint(20) NOT NULL default '0',
  `questMobCount2` bigint(20) NOT NULL default '0',
  `questMobCount3` bigint(20) NOT NULL default '0',
  `questMobCount4` bigint(20) NOT NULL default '0',
  `choiceRewards` bigint(20) NOT NULL default '0',
  `choiceItemId1` bigint(20) NOT NULL default '0',
  `choiceItemId2` bigint(20) NOT NULL default '0',
  `choiceItemId3` bigint(20) NOT NULL default '0',
  `choiceItemId4` bigint(20) NOT NULL default '0',
  `choiceItemId5` bigint(20) NOT NULL default '0',
  `choiceItemId6` bigint(20) NOT NULL default '0',
  `choiceItemCount1` bigint(20) NOT NULL default '0',
  `choiceItemCount2` bigint(20) NOT NULL default '0',
  `choiceItemCount3` bigint(20) NOT NULL default '0',
  `choiceItemCount4` bigint(20) NOT NULL default '0',
  `choiceItemCount5` bigint(20) NOT NULL default '0',
  `choiceItemCount6` bigint(20) NOT NULL default '0',
  `itemRewards` bigint(20) NOT NULL default '0',
  `rewardItemId1` bigint(20) NOT NULL default '0',
  `rewardItemId2` bigint(20) NOT NULL default '0',
  `rewardItemId3` bigint(20) NOT NULL default '0',
  `rewardItemId4` bigint(20) NOT NULL default '0',
  `rewardItemCount1` bigint(20) NOT NULL default '0',
  `rewardItemCount2` bigint(20) NOT NULL default '0',
  `rewardItemCount3` bigint(20) NOT NULL default '0',
  `rewardItemCount4` bigint(20) NOT NULL default '0',
  `rewardGold` bigint(20) NOT NULL default '0',
  `repFaction1` bigint(20) NOT NULL default '0',
  `repFaction2` bigint(20) NOT NULL default '0',
  `repValue1` bigint(20) NOT NULL default '0',
  `repValue2` bigint(20) NOT NULL default '0',
  `srcItem` bigint(20) NOT NULL default '0',
  `nextQuest` bigint(20) NOT NULL default '0',
  `learnSpell` bigint(20) NOT NULL default '0',
  `timeMinutes` bigint(20) NOT NULL default '0',
  `questType` bigint(20) NOT NULL default '0',
  `questRaces` bigint(20) NOT NULL default '0',
  `questClass` bigint(20) NOT NULL default '0',
  `questTrSkill` bigint(20) NOT NULL default '0',
  `questBehavior` bigint(20) NOT NULL default '0',
  `locationid` bigint(20) NOT NULL default '0',
  `locationx` float NOT NULL default '0',
  `locationy` float NOT NULL default '0',
  `locationopt` bigint(20) NOT NULL default '0',
  PRIMARY KEY  (`questId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

