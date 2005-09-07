-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:15 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `char_actions`
-- 
-- Creation: Aug 21, 2005 at 10:31 AM
-- Last update: Aug 21, 2005 at 10:31 AM
-- 

DROP TABLE IF EXISTS `char_actions`;
CREATE TABLE `char_actions` (
  `charId` int(6) NOT NULL default '0',
  `button` int(2) unsigned NOT NULL default '0',
  `action` int(6) unsigned NOT NULL default '0',
  `type` int(3) unsigned NOT NULL default '0',
  `misc` int(3) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
