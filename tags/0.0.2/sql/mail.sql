-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:24 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `mail`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `mail`;
CREATE TABLE IF NOT EXISTS `mail` (
  `mailId` bigint(20) unsigned NOT NULL default '0',
  `sender` bigint(20) unsigned NOT NULL default '0',
  `reciever` bigint(20) unsigned NOT NULL default '0',
  `subject` longtext,
  `body` longtext,
  `item` bigint(20) unsigned NOT NULL default '0',
  `time` bigint(20) unsigned NOT NULL default '0',
  `money` bigint(20) unsigned NOT NULL default '0',
  `COD` bigint(20) unsigned NOT NULL default '0',
  `checked` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`mailId`)
) TYPE=MyISAM COMMENT='InnoDB free: 18432 kB';
