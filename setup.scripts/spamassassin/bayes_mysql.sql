
CREATE TABLE IF NOT EXISTS `bayes_expire` (
  `id` int(11) NOT NULL DEFAULT '0',
  `runtime` int(11) NOT NULL DEFAULT '0',
  KEY `bayes_expire_idx1` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_global_vars` (
  `variable` varchar(30) NOT NULL DEFAULT '',
  `value` varchar(200) NOT NULL DEFAULT '',
  PRIMARY KEY (`variable`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_seen` (
  `id` int(11) NOT NULL DEFAULT '0',
  `msgid` varchar(200) NOT NULL DEFAULT '',
  `flag` char(1) NOT NULL DEFAULT '',
  `lastupdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`msgid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_token` (
  `id` int(11) NOT NULL DEFAULT '0',
  `token` binary(5) NOT NULL DEFAULT '',
  `spam_count` int(11) NOT NULL DEFAULT '0',
  `ham_count` int(11) NOT NULL DEFAULT '0',
  `atime` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`token`),
  KEY `bayes_token_idx1` (`id`,`atime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_vars` (
`id` int(11) NOT NULL,
  `username` varchar(200) NOT NULL DEFAULT '',
  `spam_count` int(11) NOT NULL DEFAULT '0',
  `ham_count` int(11) NOT NULL DEFAULT '0',
  `token_count` int(11) NOT NULL DEFAULT '0',
  `last_expire` int(11) NOT NULL DEFAULT '0',
  `last_atime_delta` int(11) NOT NULL DEFAULT '0',
  `last_expire_reduce` int(11) NOT NULL DEFAULT '0',
  `oldest_token_age` int(11) NOT NULL DEFAULT '2147483647',
  `newest_token_age` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `bayes_vars_idx1` (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

REPLACE INTO bayes_global_vars VALUES ('VERSION','3');
