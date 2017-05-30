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
    if cat /etc/*release | grep CentOS; then
        OS="CentOS"
        PM="yum"
        #find OS version
        if rpm -q --qf "%{VERSION}" $(rpm -q --whatprovides redhat-release) | grep 5; then
            OSVersion="5"
        elif rpm -q --qf "%{VERSION}" $(rpm -q --whatprovides redhat-release) | grep 6; then
            OSVersion="6"
        elif rpm -q --qf "%{VERSION}" $(rpm -q --whatprovides redhat-release) | grep 7; then
            OSVersion="7"
        fi
    elif cat /etc/*release | grep ^NAME | grep Red; then
        OS="RedHat"
        PM="yum"
    elif cat /etc/*release | grep -i Debian; then
        OS="Debian"
        PM="apt-get"
    else
        echo "OS NOT SUPPORTED - Please perform a manual install"
        exit 1;
    fi

    #Find Architecture (i686/x86_64)
    if arch | grep 'x86_64' ; then
        ARCH="64"
    elif arch | grep 'i686' ; then
        ARCH="32"
    else
        ARCH="unknown"
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


function install-mailwatch() {
    ################# Copy MailWatch files and set permissions ##################
    if [ -d $WebFolder ]; then
        ask "Folder $WebFolder already exists. Old files except config.php and skin.css will be removed.  Do you want to continue?(y/n)[n]: " overwriteWebFolder
        if [ -z $overwriteWebFolder ] || [ "$overwriteWebFolder" == "n" ]; then
            logprint "Stopping setup on user request"
            exit
        fi
        ask "Are you upgrading mailwatch? Requires valid conf.php in the web files directory. (y/n)[y]" upgrading
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
        logprint "Clearing web files directory"
        rm -R $WebFolder
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
    chown root:"$Webuser" $WebFolder/images
    chmod ug+rwx $WebFolder/images
    chown root:"$Webuser" $WebFolder/images/cache
    chmod ug+rwx $WebFolder/images/cache
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
       "$MailWatchTmpDir/upgrade.php" "$WebFolder/functions.php"
       if [ -f "$TmpDir/mailwatch_skin.css.old" ]; then
           logprint "Restoring old skin.css file"
           cp "$TmpDir/mailwatch_skin.css.old" "$WebFolder/skin.css"
       fi
   fi
}

