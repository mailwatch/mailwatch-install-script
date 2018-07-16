CREATE TABLE IF NOT EXISTS `bayes_expire` (
  `id` int(11) NOT NULL DEFAULT '0',
  `runtime` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_global_vars` (
  `variable` varchar(30) NOT NULL DEFAULT '',
  `value` varchar(200) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_seen` (
  `id` int(11) NOT NULL DEFAULT '0',
  `msgid` varchar(200) NOT NULL DEFAULT '',
  `flag` char(1) NOT NULL DEFAULT '',
  `lastupdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_token` (
  `id` int(11) NOT NULL DEFAULT '0',
  `token` char(5) NOT NULL DEFAULT '',
  `spam_count` int(11) NOT NULL DEFAULT '0',
  `ham_count` int(11) NOT NULL DEFAULT '0',
  `atime` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `bayes_vars` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(200) NOT NULL DEFAULT '',
  `spam_count` int(11) NOT NULL DEFAULT '0',
  `ham_count` int(11) NOT NULL DEFAULT '0',
  `token_count` int(11) NOT NULL DEFAULT '0',
  `last_expire` int(11) NOT NULL DEFAULT '0',
  `last_atime_delta` int(11) NOT NULL DEFAULT '0',
  `last_expire_reduce` int(11) NOT NULL DEFAULT '0',
  `oldest_token_age` int(11) NOT NULL DEFAULT '2147483647',
  `newest_token_age` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `bayes_expire`
 ADD KEY `bayes_expire_idx1` (`id`);

ALTER TABLE `bayes_global_vars`
 ADD PRIMARY KEY (`variable`);

ALTER TABLE `bayes_seen`
 ADD PRIMARY KEY (`id`,`msgid`);

ALTER TABLE `bayes_token`
 ADD PRIMARY KEY (`id`,`token`), ADD KEY `bayes_token_idx1` (`id`,`atime`);

ALTER TABLE `bayes_vars`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `bayes_vars_idx1` (`username`);

INSERT INTO bayes_global_vars VALUES ('VERSION','3');
