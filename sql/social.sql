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
-- Table structure for table `social`
-- 
-- Creation: Aug 26, 2005 at 12:25 AM
-- Last update: Aug 26, 2005 at 12:25 AM
-- 

DROP TABLE IF EXISTS `social`;
CREATE TABLE `social` (
  `charname` varchar(21) collate latin1_general_ci NOT NULL default '',
  `guid` int(6) NOT NULL default '0',
  `friendid` int(6) NOT NULL default '0',
  `flags` varchar(21) collate latin1_general_ci NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
