CREATE TABLE IF NOT EXISTS `report_scheduler` (
	`day` varchar(15) NOT NULL DEFAULT '0',
	`n0` tinyint(1) NOT NULL DEFAULT '0',
	`n1` tinyint(1) NOT NULL DEFAULT '0',
	`n2` tinyint(1) NOT NULL DEFAULT '0',
	`n3` tinyint(1) NOT NULL DEFAULT '0',
	`n4` tinyint(1) NOT NULL DEFAULT '0',
	`n5` tinyint(1) NOT NULL DEFAULT '0',
	`n6` tinyint(1) NOT NULL DEFAULT '0',
	`n7` tinyint(1) NOT NULL DEFAULT '0',
	`n8` tinyint(1) NOT NULL DEFAULT '0',
	`n9` tinyint(1) NOT NULL DEFAULT '0',
	`n10` tinyint(1) NOT NULL DEFAULT '0',
	`n11` tinyint(1) NOT NULL DEFAULT '0',
	`n12` tinyint(1) NOT NULL DEFAULT '0',
	`n13` tinyint(1) NOT NULL DEFAULT '0',
	`n14` tinyint(1) NOT NULL DEFAULT '0',
	`n15` tinyint(1) NOT NULL DEFAULT '0',
	`n16` tinyint(1) NOT NULL DEFAULT '0',
	`n17` tinyint(1) NOT NULL DEFAULT '0',
	`n18` tinyint(1) NOT NULL DEFAULT '0',
	`n19` tinyint(1) NOT NULL DEFAULT '0',
	`n20` tinyint(1) NOT NULL DEFAULT '0',
	`n21` tinyint(1) NOT NULL DEFAULT '0',
	`n22` tinyint(1) NOT NULL DEFAULT '0',
	`n23` tinyint(1) NOT NULL DEFAULT '0',
	UNIQUE KEY `day` (`day`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
