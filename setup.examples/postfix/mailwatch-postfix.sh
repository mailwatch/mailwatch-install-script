#!/bin/bash
# Configuration script for mailwatch with postfix
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
Webuser="$1"

service postfix stop

cp "$DIR/etc/MailScanner/conf.d/00_mailwatch.conf" /etc/MailScanner/conf.d/00_mailwatch.conf
cp "$DIR/etc/postfix/header_checks" /etc/postfix/header_checks
echo "header_checks = regexp:/etc/postfix/header_checks" >> /etc/postfix/main.cf

# restart required to create hold folder (will create error message because of still missing permissions)
service postfix start
sleep 3
service postfix stop

# "Setting file permissions for use of postfix"
mkdir -p /var/spool/MailScanner/spamassassin/
chown -R postfix:mtagroup /var/spool/MailScanner/spamassassin
chown -R postfix:postfix /var/spool/MailScanner/incoming
chown -R postfix:postfix /var/spool/MailScanner/quarantine

chown    postfix:postfix /var/spool/postfix
chown -R postfix:mtagroup /var/spool/postfix/{incoming,hold}
chmod -R g+r /var/spool/postfix/{hold,incoming}
chown -R postfix:postfix /var/spool/postfix/{active,bounce,flush,maildrop,pid}

# restart again to notice new permissions
service postfix start
