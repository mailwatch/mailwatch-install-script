#!/bin/bash
# Bash Menu Script Example
InstallFilesFolder=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MailScannerVersion="5.0.3-7"

TmpDir="/tmp/mailwatchinstall"
#MailWatchVersion="v1.2.4"
#MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-1.2.4"
MailWatchVersion="develop"
MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-develop"
IsUpgrade=0

EndNotice=""

source "$InstallFilesFolder/setup.scripts/mailwatch/mailwatch-setup-config.inc"
source "$InstallFilesFolder/setup.scripts/mailwatch/mailwatch.inc"
source "$InstallFilesFolder/setup.scripts/mailscanner/mailwatch-mailscanner.inc"
source "$InstallFilesFolder/setup.scripts/mysql/mailwatch-mysql.inc"
source "$InstallFilesFolder/setup.scripts/php/mailwatch-php.inc"

logprint "Clearing temp dir $TmpDir"
rm -rf /tmp/mailwatchinstall/*

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

logprint ""
if [[ -z $(grep mtagroup /etc/group) ]]; then
    logprint "Creating group mtagroup"
    groupadd mtagroup
fi

download-mailwatch

install-mailscanner

##ask directory for web files
logprint ""
ask "In what location should MailWatch be installed (web files directory)?[/opt/mailwatch/public/]:" WebFolder
if [ -z $WebFolder ]; then
    WebFolder="/opt/mailwatch/public/"
fi
logprint "Using web directory $WebFolder"

install-mailwatch

configure-sqlcredentials

install-mysql

configure-mysql

######################Configure web server ########################
logprint ""
if ( type "httpd" > /dev/null 2>&1 ) || ( type "apache2" > /dev/null 2>&1 ); then
    WebServer="apache"
    logprint "Detected installed web server apache. We will use it for MailWatch"
elif ( type "nginx" > /dev/null 2>&1 ); then
    WebServer="nginx"
    logprint "Detected installed web server nginx. We will use it for MailWatch"
else
    echo "We're unable to find your webserver.  We support Apache and Nginx";echo;
    echo "Do you wish to install a webserver?"
    echo "1 - Apache"
    echo "2 - Nginx"
    echo "n - do not install or configure"
    echo;
    ask "Select Webserver: " webserverSelect
    if [[ $webserverSelect =~ ^([nN][oO])$ ]]; then
        #do not install or configure webserver
        WebServer="skip"
    elif [ $webserverSelect == 1 ]; then
        #Apache
        logprint "Installing apache"
        if [ $PM == "yum" ]; then
            $PM install httpd
        else
            $PM install apache2
        fi
        WebServer="apache"
    elif [ $webserverSelect == 2 ]; then
        #Nginx
        logprint "Installing nginx"
        $PM install nginx
        WebServer="nginx"
    else
        WebServer="skip"
    fi
fi

install-php

# configure webserver
logprint ""
case $WebServer in
#TODO::Check compatibility with RPM version
    "apache")
        logprint "Creating config for apache"
        "$InstallFilesFolder/setup.scripts/apache/mailwatch-apache.sh" "$WebFolder"
        if [[ "$PM" == "yum"* ]]; then
            Webuser="apache"
        else
            Webuser="www-data"
        fi
        sleep 1
        ;;
    "nginx")
       #TODO
        if [[ "$PM" == "yum"* ]]; then
            Webuser="nginx"
        else
            Webuser="www-data"
        fi
        logprint "not available yet"
        sleep 1
        ;;
    "skip")
        logprint "Skipping web server install"
        EndNotice="$EndNotice \n * you need to configure your webserver for directory $WebFolder."
        ask "MailWatch needs to assign permissions to the webserver user. What user is your webserver running at?: " Webuser
        sleep 1
        ;;
esac

configure-mailwatch

#####################apply adjustments for MTAs ########################
logprint 'Which MTA do you want to use with MailWatch? (it should already be installed):'
ask 'MTAs: 1:postfix, 2:exim, 3:sendmail, 4/n: skip ' useMta
logprint "Selected mta $useMta"
if [ "$useMta" == "1" ]; then
    logprint "Configure MailScanner for use with postfix"
    "$InstallFilesFolder/setup.scripts/postfix/mailwatch-postfix.sh" "$Webuser"
elif [ "$useMta" == "2" ];then
    logprint "Configure MailScanner for use with exim"
    "$InstallFilesFolder/setup.scripts/exim/mailwatch-exim.sh" "$Webuser"
elif [ "$useMta" == "3" ]; then
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
logprint ""
logprint "Next steps you have to do are:"
logprint "$EndNotice"
logprint " * adjust your mta and web server configs"
logprint " * adjust the MailWatch config ${WebFolder}conf.php"

resetVariables

echo ""
echo "You can find the log file at /root/mailwatchInstall.log"
