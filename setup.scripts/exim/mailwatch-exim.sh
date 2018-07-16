#!/bin/bash
# Script to apply adjustments for Exim
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
Webuser="$1"
OS="$2"
if [ "$OS" == "Debian" ] || [ "$OS" == "Ubuntu" ]; then
    EximUser="Debian-exim"
    EximGroup="Debian-exim"
    Service="exim4"
    SpoolDir="/var/spool/exim4"
else
    if [ "$OS" == "RedHat" ]; then
        sed -i -e "s/Debian-exim/exim/" "$DIR/etc/MailScanner/conf.d/00_mailwatch.conf"
        sed -i -e "s~/usr/sbin/exim4~/usr/sbin/exim~" "$DIR/etc/MailScanner/conf.d/00_mailwatch.conf"
    fi
    EximUser="exim"
    EximGroup="exim"
    Service="exim"
    SpoolDir="/var/spool/exim"
fi

service "$Service" stop

if [ "$OS" == "Debian" ] || [ "$OS" == "Ubuntu" ]; then
	// Install "minimum" Exim version (Debian - Ubuntu)
	cp -f "$DIR/etc/default/exim4" /etc/default/exim4
	cp -f "$DIR"/etc/exim4/conf.d/main/01_mailscanner_config /etc/exim4/conf.d/main/.
	cp -f "$DIR"/etc/logrotate.d/exim4-outgoing /etc/logrotate.d/.
	// Install "full" Exim version
	//cp -f "$DIR"/etc/exim4/conf.d/main/00_mailscanner_listmacrosdefs /etc/exim4/conf.d/main/.
	//cp -f "$DIR"/etc/exim4/mailscanner_acldefs /etc/exim4/.
	//cp -f "$DIR"/etc/exim4/hubbed_hosts /etc/exim4/.
	//cp -f "$DIR"/etc/exim4/relay_domains /etc/exim4/.
	//cp -f "$DIR"/etc/exim4/update-exim4.conf.conf /etc/exim4/.
	///usr/sbin/update-exim4.conf
else
	// Install for RedHat
	// Todo
fi

// Copy MailScanner configuration
cp "$DIR/etc/MailScanner/conf.d/00_mailwatch.conf" /etc/MailScanner/conf.d/00_mailwatch.conf

// Adjust rights for Exim
usermod -a -G "$EximGroup" clamav
usermod -a -G mtagroup clamav
usermod -a -G mtagroup "$EximUser"
usermod -a -G mtagroup mail
usermod -a -G mtagroup "$Webuser"

chown -R root:root /etc/exim4
chmod -R 644 /etc/exim4
chmod 755 /etc/exim4/conf.d/
chown root:"$EximGroup" /etc/exim4/passwd.client
chmod 640 /etc/exim4/passwd.client

chown "$EximUser":adm /var/log/exim4/
chmod 750 /var/log/exim4/
chown "$EximUser":adm /var/log/exim4_outgoing/
chmod 750 /var/log/exim4_outgoing/

chown "$EximUser":"$EximGroup" "$SpoolDir"
chmod 750 /var/spool/exim4/
mkdir /var/spool/exim4_outgoing/
chown "$EximUser":"$EximGroup" /var/spool/exim4_outgoing/
chmod 750 /var/spool/exim4_outgoing/

chown "$EximUser":"$EximGroup" "$SpoolDir//db"
chmod 750 "$SpoolDir/db"
chown "$EximUser":"$EximGroup" "$SpoolDir/input"
chmod 750 "$SpoolDir/input"
chown "$EximUser":"$EximGroup" "$SpoolDir/msglog"
chmod 750 "$SpoolDir/msglog"

mkdir /var/spool/exim4_outgoing/db
chown "$EximUser":"$EximGroup" /var/spool/exim4_outgoing/db
chmod 750 /var/spool/exim4_outgoing/db
mkdir /var/spool/exim4_outgoing/input
chown "$EximUser":"$EximGroup" /var/spool/exim4_outgoing/input
chmod 750 /var/spool/exim4_outgoing/input
mkdir /var/spool/exim4_outgoing/msglog
chown "$EximUser":"$EximGroup" /var/spool/exim4_outgoing/msglog
chmod 750 /var/spool/exim4_outgoing/msglog

# Save currrent crontabs
crontab -l > /tmp/crontab.current
# Add Exim crons
cat >> /tmp/crontab.current << EOF
# For Exim4 ingoing
0 6 * * * /usr/sbin/exim_tidydb -t 1d /var/spool/exim4 callout > /dev/null 2>&1
# For Exim4 outgoing
0 6 * * * /usr/sbin/exim_tidydb -t 1d /var/spool/exim4_outgoing retry > /dev/null 2>&1
0 6 * * * /usr/sbin/exim_tidydb -t 1d /var/spool/exim4_outgoing wait-remote_smtp > /dev/null 2>&1
EOF
# Import the updated cron
crontab /tmp/crontab.current
rm /tmp/crontab.current

if [[ -z $(grep -r "mailer-daemon: postmaster" /etc/aliases) ]]; then
    echo "mail-daemon: postmaster" >> /etc/aliases
fi
if [[ -z $(grep -r "postmaster: root" /etc/aliases) ]]; then
    echo "postmaster: root" >> /etc/aliases
fi
if [[ -z $(grep -r "hostmaster: root" /etc/aliases) ]]; then
    echo "hostmaster: root" >> /etc/aliases
fi
if [[ -z $(grep -r "webmaster: root" /etc/aliases) ]]; then
    echo "webmaster: root" >> /etc/aliases
fi
if [[ -z $(grep -r "www: root" /etc/aliases) ]]; then
    echo "www: root" >> /etc/aliases
fi
if [[ -z $(grep -r "clamav: root" /etc/aliases) ]]; then
    echo "clamav: root" >> /etc/aliases
fi

if [ -f /etc/sudoers.d/mailwatch ]; then
    sed -i -e "s~#Cmnd_Alias EXIM_QUEUE_IN = /usr/sbin/exim -bpc~Cmnd_Alias EXIM_QUEUE_IN = /usr/sbin/exim -bpc~"
    sed -i -e "s~#Cmnd_Alias EXIM_QUEUE_OUT = /usr/sbin/exim -bpc -DOUTGOING~Cmnd_Alias EXIM_QUEUE_OUT = /usr/sbin/exim -bpc -DOUTGOING~"
    sed -i -e "s~#Cmnd_Alias SENDMAIL_QUEUE_IN = /usr/bin/mailq -bp -OQueueDirectory=/var/spool/mqueue.in~Cmnd_Alias SENDMAIL_QUEUE_IN = /usr/bin/mailq -bp -OQueueDirectory=/var/spool/mqueue.in~"
    sed -i -e "s~#Cmnd_Alias SENDMAIL_QUEUE_OUT = /usr/bin/mailq -bp~Cmnd_Alias SENDMAIL_QUEUE_OUT = /usr/bin/mailq -bp~"
    sed -i -e "s~#MAILSCANNER ALL= NOPASSWD: MAILSCANLINT, SPAMASSASSINLINT, EXIM_QUEUE_IN, EXIM_QUEUE_OUT, SENDMAIL_QUEUE_IN, SENDMAIL_QUEUE_OUT~MAILSCANNER ALL= NOPASSWD: MAILSCANLINT, SPAMASSASSINLINT, EXIM_QUEUE_IN, EXIM_QUEUE_OUT, SENDMAIL_QUEUE_IN, SENDMAIL_QUEUE_OUT~"
fi

service "$Service" start
