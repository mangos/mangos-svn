-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:10 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `char_actions`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `char_actions`;
CREATE TABLE IF NOT EXISTS `char_actions` (
  `charId` int(6) NOT NULL default '0',
  `button` int(2) unsigned NOT NULL default '0',
  `action` int(6) unsigned NOT NULL default '0',
  `type` int(3) unsigned NOT NULL default '0',
  `misc` int(3) NOT NULL default '0'
) TYPE=MyISAM;
