#!/bin/bash
# Bash Menu Script Example
InstallFilesFolder=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MailScannerVersion="5.0.6-5"

TmpDir="/tmp/mailwatchinstall"
#MailWatchVersion="v1.2.6"
#MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-1.2.6"
MailWatchVersion="develop"
MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-develop"
IsUpgrade=0

EndNotice=""

source "$InstallFilesFolder/setup.scripts/mailwatch/mailwatch-setup-config.inc"
source "$InstallFilesFolder/setup.scripts/mailwatch/mailwatch.inc"
source "$InstallFilesFolder/setup.scripts/mailscanner/mailwatch-mailscanner.inc"
source "$InstallFilesFolder/setup.scripts/mysql/mailwatch-mysql.inc"
source "$InstallFilesFolder/setup.scripts/php/mailwatch-php.inc"

if ! [ -d $TmpDir ]; then
    logprint "Warning: temporary directory from previous install found."
    logprint "This usally means that a previous install is still running or was interrupted."
    logprint "In case of an upgrade you should make sure that your old conf.php and skin.css are still"
    logprint "in the web folder or backup the ones from the temporary folder $TmpDir before continuing."
fi

detectos

if ! ( type "wget" > /dev/null 2>&1 ) ; then
    logprint "Install script depends on wget."
    $PM install wget
    logprint ""
fi

if ! ( type "tar" > /dev/null 2>&1 ) ; then
    logprint "Install script depends on tar."
       $PM install tar
    logprint ""
fi

if [[ -z $(grep mtagroup /etc/group) ]]; then
    logprint "Creating group mtagroup"
    groupadd mtagroup
fi

##ask directory for web files
ask "In what location should MailWatch be installed (web files directory)?[/opt/mailwatch/public/]:" WebFolder
if [ -z $WebFolder ]; then
    WebFolder="/opt/mailwatch/public/"
fi
logprint "Using web directory $WebFolder"

check-and-prepare-update

#also detects MTA
prepare-mailscanner
prepare-webserver
configure-sqlcredentials


install-mailscanner
install-mailwatch

install-mysql

configure-mysql

######################Configure web server ########################
logprint ""
if [ "$WebServer" == "apache" ]; then
    #Apache
    logprint "Installing apache"
    if [ $PM == "yum" ]; then
        $PM install httpd
    else
        $PM install apache2
    fi
    logprint "Creating config for apache"
    "$InstallFilesFolder/setup.scripts/apache/mailwatch-apache.sh" "$WebFolder"
    if [[ "$PM" == "yum"* ]]; then
        Webuser="apache"
    else
        Webuser="www-data"
    fi
elif [ "$WebServer" == "nginx" ]; then
    #Nginx
    logprint "Installing nginx"
    $PM install nginx
    if [[ "$PM" == "yum"* ]]; then
        Webuser="nginx"
    else
        Webuser="www-data"
    fi
    TODO
    logprint "Nginx automatic configuration not available yet"
else
    logprint "Skipping web server install"
    EndNotice="$EndNotice \n * you need to configure your webserver for directory $WebFolder."
    ask "MailWatch needs to assign permissions to the webserver user. What user is your webserver running at?: " Webuser
fi

install-php

configure-mailwatch

#####################apply adjustments for MTAs ########################
if [ "$MTA" == "postfix" ]; then
    logprint "Configure MailScanner for use with postfix"
    "$InstallFilesFolder/setup.scripts/postfix/mailwatch-postfix.sh" "$Webuser"
elif [ "$MTA" == "exim" ];then
    logprint "Configure MailScanner for use with exim"
    "$InstallFilesFolder/setup.scripts/exim/mailwatch-exim.sh" "$Webuser"
elif [ "$MTA" == "sendmail" ]; then
    logprint "Sendmail is not yet supported. If you have experience with sendmail and want to help support this please contact us at https://github.com/mailwatch/mailwatch-install-script"
    #TODO
else
    logprint "Not configuring mta"
fi
sleep 1

configure-cronjobs
configure-mailscanner
configure-sudo

#todo relay files
logprint "Install finished!"

logprint "Cleanup temporary data"
rm -rf /tmp/mailwatchinstall/*
resetVariables

logprint ""
logprint "Next steps you have to do are:"
logprint "$EndNotice"
logprint " * adjust your mta and web server configs"
logprint " * adjust the MailWatch config ${WebFolder}conf.php"

resetVariables

echo ""
echo "You can find the log file at /root/mailwatchInstall.log"

