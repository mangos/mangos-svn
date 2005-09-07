-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:18 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `mail`
-- 
-- Creation: Aug 26, 2005 at 12:23 AM
-- Last update: Aug 26, 2005 at 12:23 AM
-- 

DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `mailId` bigint(20) unsigned NOT NULL default '0',
  `sender` bigint(20) unsigned NOT NULL default '0',
  `reciever` bigint(20) unsigned NOT NULL default '0',
  `subject` longtext collate latin1_general_ci,
  `body` longtext collate latin1_general_ci,
  `item` bigint(20) unsigned NOT NULL default '0',
  `time` bigint(20) unsigned NOT NULL default '0',
  `money` bigint(20) unsigned NOT NULL default '0',
  `COD` bigint(20) unsigned NOT NULL default '0',
  `checked` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`mailId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 18432 kB';
