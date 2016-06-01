CREATE TABLE IF NOT EXISTS `ci_users_groups` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `user_id` int(9) unsigned NOT NULL,
  `group_id` int(9) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `group_id` (`group_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;
