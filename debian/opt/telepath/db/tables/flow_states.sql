 CREATE TABLE IF NOT EXISTS `flow_states` (
`SID` varchar(70) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`RID` bigint(20) NOT NULL,
`business_id` int(11) NOT NULL DEFAULT 0,
`status` tinyint(1) NOT NULL DEFAULT 0,
UNIQUE KEY `R_B` (`RID`,`business_id`),
KEY `SID` (`SID`),
KEY `RID` (`RID`),
KEY `business_id` (`business_id`)   
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
