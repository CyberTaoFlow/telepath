CREATE TABLE IF NOT EXISTS `attributes` (
                `att_id` bigint(20) unsigned NOT NULL DEFAULT 0,
                `page_id` bigint(20) unsigned NOT NULL DEFAULT 0,
                `att_name` char(128) NOT NULL DEFAULT "",
                `att_alias` VARCHAR( 100 ) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
                `att_source` CHAR( 1 ) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT "",
                `structure` varchar(20) NOT NULL DEFAULT "",
                `excluded` tinyint(1) NOT NULL DEFAULT 0,
                `noisy` tinyint(1) NOT NULL DEFAULT 0,
                `mask` tinyint(1) NOT NULL DEFAULT 0,
                `weight` double NOT NULL DEFAULT 0,
                PRIMARY KEY (`page_id`,`att_id`),
                UNIQUE KEY `att_id` (`att_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
