CREATE TABLE IF NOT EXISTS `ci_user_perm` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `user_id` int(5) unsigned NOT NULL,
  `perm_id` int(5) unsigned NOT NULL,
  `value` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `perm_id` (`perm_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;
