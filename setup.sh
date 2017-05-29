#!/bin/bash
# Bash Menu Script Example
InstallFilesFolder=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MailScannerVersion="5.0.3-7"

TmpDir="/tmp/mailwatchinstall"
#MailWatchVersion="v1.2.2"
#MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-1.2.2"
MailWatchVersion="develop"
MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-develop"
IsUpgrade=0

EndNotice=""

source "$InstallFilesFolder/setup.scripts/mailwatch/mailwatch.inc"
source "$InstallFilesFolder/setup.scripts/mailscanner/mailwatch-mailscanner.inc"
source "$InstallFilesFolder/setup.scripts/mysql/mailwatch-mysql.inc"
source "$InstallFilesFolder/setup.scripts/php/mailwatch-php.inc"

logprint "Clearing temp dir $TmpDir"
rm -rf /tmp/mailwatchinstall/*

detectos

if ! ( type "wget" > /dev/null 2>&1 ) ; then
    logprint "Install script depends on wget but it is missing. Installing it."
    $PM install wget
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
read -p "In what location should MailWatch be installed (web files directory)?[/var/www/html/mailscanner/]:" WebFolder
if [ -z $WebFolder ]; then
    WebFolder="/var/www/html/mailscanner/"
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
    read -r -p "Select Webserver: " response
    if [[ $response =~ ^([nN][oO])$ ]]; then
        #do not install or configure webserver
        WebServer="skip"
    elif [ $response == 1 ]; then
        #Apache
        logprint "Installing apache"
        if [ $PM == "yum" ]; then
            $PM install httpd
        else
            $PM install apache2
        fi
        WebServer="apache"
    elif [ $response == 2 ]; then
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
    "apache")
        logprint "Creating config for apache"
        "$InstallFilesFolder/setup.scripts/apache/mailwatch-apache.sh" "$WebFolder"
        if [ $PM == "yum" ]; then
            Webuser="nginx"
        else
            Webuser="www-data"
        fi
        sleep 1
        ;;
    "nginx")
       #TODO
        if [ $PM == "yum" ]; then
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
        read -p "MailWatch needs to assign permissions to the webserver user. What user is your webserver running at?: " Webuser
        sleep 1
        ;;
esac

configure-mailwatch

#####################apply adjustments for MTAs ########################
PS3='Which MTA do you want to use with MailWatch? (it should already be installed):'
options=("sendmail" "postfix" "exim" "skip")
select opt in "${options[@]}"
do
    logprint "Selected mta $opt"
    case $opt in
        "sendmail")
            logprint "Not yet supported"
#TODO
            sleep 1
            break
            ;;
        "postfix")
            logprint "Configure MailScanner for use with postfix"
            "$InstallFilesFolder/setup.scripts/postfix/mailwatch-postfix.sh" "$Webuser"
            sleep 1
            break
            ;;

        "exim")
            logprint "Configure MailScanner for use with exim"
            "$InstallFilesFolder/setup.scripts/exim/mailwatch-exim.sh" "$Webuser"
            sleep 1
            break
            ;;
        *)
            logprint "Not configuring mta"
            sleep 1
            break
            ;;
    esac
done

configure-mailscanner

#todo relay files
logprint "Install finished!"
logprint "Next steps you have to do are:"
logprint "$EndNotice"
logprint " * adjust your mta and web server configs"
logprint " * adjust the MailWatch config $WebFolder/conf.php"
echo ""
echo "You can find the log file at /root/mailwatchInstall.log"
