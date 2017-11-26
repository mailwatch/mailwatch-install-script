#!/bin/bash

function logprint {
    echo -e "$1"
    echo -e "$1" >> /root/mailwatchInstall.log
}

function ask() {
    message="$1"
    var="$2"
    if [ -z "${!var}" ]; then
        read -r -p "$message" "$var"
    else
        logprint "$var set by setup-config. Using that value"
    fi
}

function detectos() {
    if ! [ -z "$confirmPackageInstalls" ] && [ "$confirmPackageInstalls" == "y" ]; then
        packageConfirm=" -y "
        #may add export DEBIAN_FRONTEND=noninteractive (for deb based) so that the install is completely unattanded
        #(see https://serverfault.com/questions/143968/automate-the-installation-of-postfix-on-ubuntu)
    fi
    if [[ $(cat /etc/*release | grep CentOS) ]]; then
        OS="CentOS"
        PM="yum $packageConfirm"
        #find OS version
        if rpm -q --qf "%{VERSION}" $(rpm -q --whatprovides redhat-release) | grep 5; then
            OSVersion="5"
        elif rpm -q --qf "%{VERSION}" $(rpm -q --whatprovides redhat-release) | grep 6; then
            OSVersion="6"
        elif rpm -q --qf "%{VERSION}" $(rpm -q --whatprovides redhat-release) | grep 7; then
            OSVersion="7"
        fi
    elif [[ $(cat /etc/*release | grep ^NAME | grep Red) ]]; then
        OS="RedHat"
        PM="yum $packageConfirm"
    elif [[ $(cat /etc/*release | grep -i Debian) ]]; then
        OS="Debian"
        PM="apt-get $packageConfirm"
    else
        logprint "OS NOT SUPPORTED - Please perform a manual install"
        exit 1;
    fi

    #Find Architecture (i686/x86_64)
    if [[ $(arch | grep 'x86_64') ]]; then
        ARCH="64"
    elif [[ $(arch | grep 'i686') ]]; then
        ARCH="32"
    else
        ARCH="unknown"
    fi
    logprint "System is $OS on $ARCH architecture"
}

function prepare-webserver() {
    logprint ""
    if [ -z "$WebServer"]; then
        if ( type "httpd" > /dev/null 2>&1 ) || ( type "apache2" > /dev/null 2>&1 ); then
            WebServer="apache"
            logprint "Detected installed web server apache. We will use it for MailWatch"
        elif ( type "nginx" > /dev/null 2>&1 ); then
            WebServer="nginx"
            logprint "Detected installed web server nginx. We will use it for MailWatch"
        else
            logprint "We're unable to find your webserver. We support Apache and Nginx. (default: Apache)";
            echo "Do you wish to install a webserver?"
            echo "1 - Apache"
            echo "2 - Nginx"
            echo "n - do not install or configure"
            echo;
            ask  "Select Webserver: " webserverSelect
            if [[ $webserverSelect =~ ^([nN][oO])$ ]]; then
                #do not install or configure webserver
                logprint "No WebServer will be configured"
                WebServer="skip"
            elif [ $webserverSelect == 1 ] || [ -z "$webserverSelect" ]; then
                #Apache
                logprint "Using apache"
                WebServer="apache"
            elif [ $webserverSelect == 2 ]; then
                #Nginx
                logprint "Using nginx"
                WebServer="nginx"
            fi

        fi
    elif [ "$WebServer" != "apache" ] && [ "$WebServer" != "nginx" ] && [ "$WebServer" != "skip" ]; then
        logprint "Invalid WebServer defined (valid values are 'apache', 'nginx' and 'skip'"
        exit 1
    else
        logprint "Using WebServer $WebServer defined by setup config"
    fi
}

function download-mailwatch() {
    mkdir -p "$MailWatchTmpDir"
    logprint "Downloading MailWatch version $MailWatchVersion"
    wget -O "$TmpDir/MW.tar.gz" "https://github.com/mailwatch/MailWatch/archive/$MailWatchVersion.tar.gz"
    logprint "Extracting MailWatch files to $TmpDir/mailwatch"
    tar -xf "$TmpDir/MW.tar.gz" -C "$TmpDir/mailwatch"
    # test if mailwatch was successfully downloaded
    if [ ! -d "$MailWatchTmpDir/mailscanner" ]; then
        logprint "Problem occured while downloading MailWatch. Stopping setup"
        exit
    fi
}

function check-and-prepare-update() {
    if [ -d $WebFolder ]; then
        ask "Folder $WebFolder already exists. Old files except config.php and skin.css will be removed.  Do you want to continue?(y/n)[n]: " overwriteWebFolder
        if [ -z $overwriteWebFolder ] || [ "$overwriteWebFolder" == "n" ]; then
            logprint "Stopping setup on user request"
            exit
        fi
        ask "Are you upgrading mailwatch? Requires valid conf.php in the web files directory. (y/n)[y]" upgrading
    fi
}

function install-mailwatch() {
    download-mailwatch
    ################# Copy MailWatch files and set permissions ##################
    if [ -z $upgrading ] || [ "$upgrading" == "y" ]; then
        if [ -f "$WebFolder/conf.php" ]; then
            IsUpgrade=1
            logprint "Saving old MailWatch config.php file"
            cp "$WebFolder/conf.php" "$TmpDir/mailwatch_conf.php.old"
        else
            logprint "You wanted to upgrade MailWatch but we couldn't find an old config file. Stopping setup."
            exit
        fi
        if [ -f "$WebFolder/skin.css" ]; then
            logprint "Saving old MailWatch skin.css file"
            cp "$WebFolder/skin.css" "$TmpDir/mailwatch_skin.css.old"
        else
           logprint "No old MailWatch skin.css file found"
        fi
    else
        logprint "No Upgrade. All existing content in the given folder will be overwritten"
    fi
    if [ -z $WebFolder ]; then
        rm -R $WebFolder
        logprint "Clearing web files directory"
    fi

    #copy web files
    logprint "Moving MailWatch web files to new folder"
    mkdir -p $WebFolder
    cp -r "$MailWatchTmpDir"/mailscanner/* $WebFolder
}


function configure-sqlcredentials() {
    logprint ""
    if [ "$IsUpgrade" == 0 ]; then
        logprint "Setting up sql credentials"
        #get sql credentials
        ask "SQL user for mailwatch[mailwatch]:" SqlUser
        if [ -z $SqlUser ]; then
            SqlUser="mailwatch"
        fi
        ask "SQL password for mailwatch[mailwatch]:" SqlPwd
        if [ -z $SqlPwd ]; then
            SqlPwd="mailwatch"
        fi
        ask "SQL database for mailwatch[mailscanner]:" SqlDb
        if [  -z $SqlDb ]; then
            SqlDb="mailscanner"
        fi
        ask "SQL host of database[localhost]:" SqlHost
        if [ -z $SqlHost ]; then
            SqlHost="localhost"
        fi
        logprint "Using sql credentials user: $SqlUser; password: $SqlPwd; db: $SqlDb; host: $SqlHost"
    else
        SqlUser=$(grep "define('DB_USER" "$TmpDir/mailwatch_conf.php.old" | head -n 1 |  cut -d"'" -f 4)
        SqlPwd=$(grep "define('DB_PASS" "$TmpDir/mailwatch_conf.php.old" | head -n 1 |  cut -d"'" -f 4)
        SqlDb=$(grep "define('DB_NAME" "$TmpDir/mailwatch_conf.php.old" | head -n 1 |  cut -d"'" -f 4)
        SqlHost=$(grep "define('DB_HOST" "$TmpDir/mailwatch_conf.php.old" | head -n 1 |  cut -d"'" -f 4)
    fi
}

function configure-mailwatch() {
    logprint "Settings permissions for web directory"
    chown root:"$Webuser" $WebFolder/temp
    chmod g+rw $WebFolder/temp

    ###############apply general MailWatch settings #######################
    logprint "Apply MailWatch settings to conf.php"
    if [ "$IsUpgrade" == 0 ]; then
        cp "$WebFolder/conf.php.example" "$WebFolder/conf.php"
        sed -i -e "s/^define('DB_USER', '.*')/define('DB_USER', '$SqlUser')/" $WebFolder/conf.php
        sed -i -e "s/^define('DB_PASS', '.*')/define('DB_PASS', '$SqlPwd')/" $WebFolder/conf.php
        sed -i -e "s/^define('DB_HOST', '.*')/define('DB_HOST', '$SqlHost')/" $WebFolder/conf.php
        sed -i -e "s/^define('DB_NAME', '.*')/define('DB_NAME', '$SqlDb')/" $WebFolder/conf.php
        sed -i -e "s~^define('MAILWATCH_HOME', '.*')~define('MAILWATCH_HOME', '$WebFolder')~" $WebFolder/conf.php
   else
       logprint "Upgrading MailWatch conf and db"
       cp "$TmpDir/mailwatch_conf.php.old" "$WebFolder/conf.php"
       sed -i -e "s~^define('MAILWATCH_HOME', '.*')~define('MAILWATCH_HOME', '$WebFolder')~" $WebFolder/conf.php
       "$MailWatchTmpDir/upgrade.php" --skip-user-confirm "$WebFolder/functions.php"
       if [ -f "$TmpDir/mailwatch_skin.css.old" ]; then
           logprint "Restoring old skin.css file"
           cp "$TmpDir/mailwatch_skin.css.old" "$WebFolder/skin.css"
       fi
   fi
}

function configure-cronjobs() {
    sed -i -e "s+^\$pathToFunctions = '.*';+\$pathToFunctions = '$WebFolder/functions.php';+" $MailWatchTmpDir/tools/Cron_jobs/mailwatch_quarantine_maint.php
    sed -i -e "s+^\$pathToFunctions = '.*';+\$pathToFunctions = '$WebFolder/functions.php';+" $MailWatchTmpDir/tools/Cron_jobs/mailwatch_quarantine_report.php
    sed -i -e "s+^\$pathToFunctions = '.*';+\$pathToFunctions = '$WebFolder/functions.php';+" $MailWatchTmpDir/tools/Cron_jobs/mailwatch_db_clean.php
    sed -i -e "s+^\$pathToFunctions = '.*';+\$pathToFunctions = '$WebFolder/functions.php';+" $MailWatchTmpDir/tools/Cron_jobs/mailwatch_geoip_update.php
    chmod +x $MailWatchTmpDir/tools/Cron_jobs/mailwatch_quarantine_maint.php
    chmod +x $MailWatchTmpDir/tools/Cron_jobs/mailwatch_quarantine_report.php
    chmod +x $MailWatchTmpDir/tools/Cron_jobs/mailwatch_db_clean.php
    chmod +x $MailWatchTmpDir/tools/Cron_jobs/mailwatch_geoip_update.php
    chmod +x $MailWatchTmpDir/tools/Cron_jobs/mailwatch
    chmod +x $MailWatchTmpDir/tools/Cron_jobs/mailwatch.cron.monthly
    cp $MailWatchTmpDir/tools/Cron_jobs/mailwatch_quarantine_maint.php /usr/local/bin/mailwatch_quarantine_maint.php
    cp $MailWatchTmpDir/tools/Cron_jobs/mailwatch_quarantine_report.php /usr/local/bin/mailwatch_quarantine_report.php
    cp $MailWatchTmpDir/tools/Cron_jobs/mailwatch_db_clean.php /usr/local/bin/mailwatch_db_clean.php
    cp $MailWatchTmpDir/tools/Cron_jobs/mailwatch_geoip_update.php /usr/local/bin/mailwatch_geoip_update.php
    cp $MailWatchTmpDir/tools/Cron_jobs/mailwatch /etc/cron.daily/mailwatch
    cp $MailWatchTmpDir/tools/Cron_jobs/mailwatch.cron.monthly /etc/cron.monthly/mailwatch

    sed -i -e "s/q_days=.*/q_days=0/" /etc/MailScanner/defaults
}

function configure-sudo() {
    logprint ""
    if ! ( type "sudo" > /dev/null 2>&1 ) ; then
        ask "Some functions need sudo (eg. SpamAssasin Lint, MailScanner Lint). Do you want to install it? (y/n)[y]: " installSudo
        if [ -z $installSudo ] || [ "$installSudo" == "y" ]; then
            logprint "Installing sudo"
            $PM install sudo
        else
            logprint "Not installing sudo."
        fi
    fi
    if ! ( type "sudo" > /dev/null 2>&1 ) ; then
        logprint "Sudo is installed so we are configuring it for use with MailWatch"
        sed -i -e "s/User_Alias MAILSCANNER = .*/User_Alias MAILSCANNER = $Webuser" $MailWatchTmpDir/tools/sudo/mailwatch
        cp $MailWatchTmpDir/tools/sudo/mailwatch /etc/sudoers.d/mailwatch
    fi
}
