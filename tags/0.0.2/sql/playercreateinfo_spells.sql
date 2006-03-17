# phpMyAdmin SQL Dump
# version 2.5.3
# http://www.phpmyadmin.net
#
# Host: localhost
# Generation Time: Sep 26, 2005 at 01:34 AM
# Server version: 4.0.15
# PHP Version: 4.3.3
#
# Database : `mangos`
#

# --------------------------------------------------------

#
# Table structure for table `playercreateinfo_spells`
#
# Creation: Sep 25, 2005 at 09:07 PM
# Last update: Sep 26, 2005 at 01:09 AM
#

DROP TABLE IF EXISTS `playercreateinfo_spells`;
CREATE TABLE `playercreateinfo_spells` (
  `CreateId` varchar(255) default NULL,
  `Spell` varchar(255) default NULL,
  `Note` varchar(255) default NULL
) TYPE=MyISAM;

#
# Dumping data for table `playercreateinfo_spells`
#