CREATE TABLE IF NOT EXISTS `pages` (
		`app_id` bigint(20) unsigned NOT NULL DEFAULT 0,
		`subdomain_id` int(10) unsigned NOT NULL DEFAULT 0,
		`page_id` bigint(20) unsigned NOT NULL DEFAULT 0,
		`path` varchar(500) DEFAULT NULL,
		`display_path` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
		`title` varchar(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
		`is_entrypoint` tinyint(1) unsigned NOT NULL,
		`learning_data` tinyint(1) NOT NULL DEFAULT '1',
		`dirty` tinyint(1) NOT NULL DEFAULT '1',
		PRIMARY KEY (`page_id`),
		KEY `app_id` (`app_id`),
		KEY `path_index` (`display_path`),
		FULLTEXT KEY `title` (`title`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
