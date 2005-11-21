# phpMyAdmin SQL Dump
# version 2.5.3
# http://www.phpmyadmin.net
#
# Host: localhost
# Generation Time: Sep 26, 2005 at 01:33 AM
# Server version: 4.0.15
# PHP Version: 4.3.3
#
# Database : `mangos`
#

# --------------------------------------------------------

#
# Table structure for table `playercreateinfo_skills`
#
# Creation: Sep 25, 2005 at 06:57 PM
# Last update: Sep 26, 2005 at 12:31 AM
# Last check: Sep 25, 2005 at 07:02 PM
#

DROP TABLE IF EXISTS `playercreateinfo_skills`;
CREATE TABLE `playercreateinfo_skills` (
  `CreateId` varchar(255) default NULL,
  `Skill` varchar(255) default NULL,
  `SkillMin` varchar(255) default NULL,
  `SkillMax` varchar(255) default NULL,
  `Note` varchar(255) default NULL
) TYPE=MyISAM;

#
# Dumping data for table `playercreateinfo_skills`
#