CREATE TABLE IF NOT EXISTS `case_requests` (
  `case_id` int(11) unsigned NOT NULL,
  `RID` bigint(20) NOT NULL,
  UNIQUE KEY `case_id` (`case_id`,`RID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
