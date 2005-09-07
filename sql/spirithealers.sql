-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:19 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `spirithealers`
-- 
-- Creation: Aug 26, 2005 at 12:25 AM
-- Last update: Aug 26, 2005 at 12:26 AM
-- 

DROP TABLE IF EXISTS `spirithealers`;
CREATE TABLE `spirithealers` (
  `X` float default NULL,
  `Y` float default NULL,
  `Z` float default NULL,
  `F` float default NULL,
  `name_id` int(8) default NULL,
  `zoneId` int(16) default NULL,
  `mapId` int(16) default NULL,
  `faction_id` int(32) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
