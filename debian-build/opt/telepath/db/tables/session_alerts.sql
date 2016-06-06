CREATE TABLE IF NOT EXISTS `session_alerts` (
		`rule_id` int(11) NOT NULL,
		`rule_type` varchar(50) NOT NULL,
		`SID` varchar(100) NOT NULL,
		`IP` varchar(20) NOT NULL,
		`RID` bigint(20) NOT NULL,
		`page_id` bigint(128) unsigned NOT NULL,
		`att_id` bigint(20) NOT NULL,
		`numeric_score` int(10) NOT NULL,
		`literal_score` varchar(20) NOT NULL,
		`value` varchar(1024) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
		`date` int(20) NOT NULL,
		`rule_group` int unsigned not null default '0',
		KEY `SID` (`SID`),
		KEY `RID` (`RID`),
		KEY `page_id` (`page_id`),
		UNIQUE KEY `R_R` ( `rule_id` , `RID` )
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
