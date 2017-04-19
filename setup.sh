#!/bin/bash
# Bash Menu Script Example
InstallFilesFolder=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MailScannerVersion="5.0.3-7"

TmpDir="/tmp/mailwatchinstall"
MailScannerTmpDir="$TmpDir/mailscanner/"
#MailWatchVersion="v1.2.2"
#MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-1.2.2"
MailWatchVersion="develop"
MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-develop"
IsUpgrade=0

EndNotice=""

if cat /etc/*release | grep CentOS; then
    OS="CentOS"
    PM="yum"
    MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.rhel.tar.gz"
    #find OS version
    if cat /etc/*release | grep 5; then
        OSVersion="5"
    elif cat /etc/*release | grep 6; then
        OSVersion="6"
    elif cat /etc/*release | grep 7; then
        OSVersion="7"
    fi
elif cat /etc/*release | grep ^NAME | grep Red; then
    OS="RedHat"
    PM="yum"
    MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.rhel.tar.gz"
elif cat /etc/*release | grep ^NAME | grep Fedora; then
    OS="Fedora"
    PM="yum"
    MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.rhel.tar.gz"
elif cat /etc/*release | grep -i Debian; then
    OS="Debian"
    PM="apt-get"
    MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.deb.tar.gz"
else
    echo "OS NOT SUPPORTED - Please perform a manual install"
    exit 1;
fi

function logprint {
    echo -e "$1"
    echo -e "$1" >> /root/mailwatchInstall.log
}
logprint "Clearing temp dir"
rm -rf /tmp/mailwatchinstall/*

if ! ( type "wget" > /dev/null 2>&1 ) ; then
    logprint "Install script depends on wget."
    $PM install wget
fi

mkdir -p "$MailWatchTmpDir"
logprint "Downloading MailWatch version $MailWatchVersion"
wget -O "$TmpDir/MW.tar.gz" "https://github.com/mailwatch/MailWatch/archive/$MailWatchVersion.tar.gz"
logprint "Extracting MailWatch files"
tar -xf "$TmpDir/MW.tar.gz" -C "$TmpDir/mailwatch"
# test if mailwatch was successfully downloaded
if [ ! -d "$MailWatchTmpDir/mailscanner" ]; then
    logprint "Problem occured while downloading MailWatch. Stopping setup"
    exit
fi


################################# MailScanner install ##########################
logprint ""
read -p "Install/upgrade MailScanner version $MailScannerVersion?:(y/n)[y]: " installMailScanner
if [ -z $installMailScanner ] || [ "$installMailScanner" == "y" ]; then
    logprint "Starting MailScanner install"
    mkdir -p /tmp/mailwatchinstall/mailscanner
    logprint "Downloading current MailScanner release $MailScannerVersion:"
    wget -O "$MailScannerTmpDir/MailScanner.deb.tar.gz"  "$MailScannerDownloadPath"
    logprint "Extracting mailscanner files:"
    tar -xzf "$MailScannerTmpDir/MailScanner.deb.tar.gz" -C "$MailScannerTmpDir/"
    logprint "Starting MailScanner install script"
    "$MailScannerTmpDir/MailScanner-$MailScannerVersion/install.sh"
    logprint "MailScanner install finished."
    EndNotice="$EndNotice \n * Adjust /etc/MailScanner.conf to your needs \n * Set run_mailscanner=1 in /etc/MailScanner/defaults"
    sleep 1
else
   logprint "Not installing MailScanner"
fi

logprint "Stopping mailscanner"
service mailscanner stop

logprint ""
if [[ -z $(grep mtagroup /etc/group) ]]; then
    logprint "Creating group mtagroup"
    groupadd mtagroup
fi

logprint ""
logprint "Installing Encoding::FixLatin"
cpan -i Encoding::FixLatin


##ask directory for web files
logprint ""
read -p "In what location should MailWatch be installed (web files directory)?[/var/www/html/mailscanner/]:" WebFolder
if [ -z $WebFolder ]; then
    WebFolder="/var/www/html/mailscanner/"
fi
logprint "Using web directory $WebFolder"

################# Copy MailWatch files and set permissions ##################
if [ -d $WebFolder ]; then
   read -p "Folder $WebFolder already exists. Old files except config will be removed. Do you want to continue?(y/n)[n]: " response
   if [ -z $response ]; then
       logprint "Stopping setup on user request"
       exit
   elif [ "$response" == "n" ]; then
       logprint "Stopping setup on user request"
       exit
   fi
   read -p "Are you upgrading mailwatch? Requires valid conf.php in the web files directory. (y/n)[y]" upgrading
   if [ -z $upgrading ] || [ "$upgrading" == "y" ]; then
       if [ -f "$WebFolder/conf.php" ]; then
           IsUpgrade=1
           logprint "Saving old MailWatch config file"
           cp "$WebFolder/conf.php" "$TmpDir/mailwatch_conf.php.old"
       else
           logprint "You wanted to upgrade MailWatch but we couldn't find an old config file. Stopping setup."
           exit
       fi
   else
      logprint "No Upgrade. All existing content related to mailwatch will be overwritten"
   fi
   logprint "Clearing web files directory"
   rm -R $WebFolder
fi

#copy web files
logprint "Moving MailWatch web files to new folder"
mkdir -p $WebFolder
cp -r "$MailWatchTmpDir"/mailscanner/* $WebFolder

############ Ask for sql credentiails ################
logprint ""
if [ "$IsUpgrade" == 0 ]; then
    logprint "Setting up sql credentials"
    #get sql credentials
    read -p "SQL user for mailwatch[mailwatch]:" SqlUser
    if [ -z $SqlUser ]; then
        SqlUser="mailwatch"
    fi
    read -p "SQL password for mailwatch[mailwatch]:" SqlPwd
    if [ -z $SqlPwd ]; then
        SqlPwd="mailwatch"
    fi
    read -p "SQL database for mailwatch[mailscanner]:" SqlDb
    if [  -z $SqlDb ]; then
        SqlDb="mailscanner"
    fi
    read -p "SQL host of database[localhost]:" SqlHost
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

############ Configure mysql server ################
logprint ""
if ! ( type "mysqld" > /dev/null 2>&1 ) ; then
    read -p "No mysql server found. Do you want to install mariadb as sql server?(y/n)[y]: " response
    if [ -z $response ] || [ $response == "y" ]; then
        logprint "Start install of mariadb"
        if [ $OS == "CentOS" ]; then
            logprint "Adding MariaDB 10.1 Repo"
            cat yum.repo/$OS-$OSVersion-MariaDB.repo > /etc/yum.repos.d/MariaDB.repo
            $PM clean all
            $PM install MariaDB-server MariaDB-client
            #TODO::Check for other rpm variants
        else
            $PM install mariadb-server mariadb-client
            mysqlInstalled="1"
        fi
        logprint "MariaDB installed - now we need to secure the installation"
        logprint "Please enter the required details in the next step - for security, please enter a password"
        mysql_secure_installation
    else
        mysqlInstalled="0"
        logprint "Not installing mariadb."
    fi
else
    mysqlInstalled="1"
    logprint "Found installed mysql server and will use that"
fi

if [ "$IsUpgrade" == 1 ]; then
    logprint "Not creating sql db because we are running an upgrade"
elif [ "$mysqlInstalled" == "1" ]; then
    read -p "Root sql user (with rights to create db)[root]:" SqlRoot
    if [ -z $SqlRoot ]; then
        SqlRoot="root"
    fi
    logprint "Creating sql database and setting permission. You now need to enter the password of the root sql user twice"
    mysql -u $SqlRoot -p < "$MailWatchTmpDir/create.sql"
    mysql -u $SqlRoot -p --execute="GRANT ALL ON $SqlDb.* TO $SqlUser@localhost IDENTIFIED BY '$SqlPwd'; GRANT FILE ON *.* TO $SqlUser@localhost IDENTIFIED BY '$SqlPwd'; FLUSH PRIVILEGES"

    read -p "Enter an admin user for the MailWatch web interface: " MWAdmin
    read -p "Enter password for the admin: " MWAdminPwd
    logprint "Create MailWatch web gui admin"
    mysql -u $SqlUser -p$SqlPwd $SqlDb --execute="REPLACE INTO users SET username = '$MWAdmin', password = MD5('$MWAdminPwd'), fullname = 'Admin', type = 'A';"
else
    echo "You have to create the database yourself!"
    EndNotice="$EndNotice \n * create the database, a sql user with access to the db and following properties user: $SqlUser; password: $SqlPwd; db: $SqlDb; host: $SqlHost"
    EndNotice="$EndNotice \n * create an admin account for the web gui"
fi

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

logprint ""
read -p "MailWatch requires the php packages php5 php5-gd and php5-mysqlnd. Do you want to install them if missing?(y/n)[y]: " installPhp
if [ -z $installPhp ] || [ "$installPhp" == "y" ]; then
    logprint "Installing required php packages"
    if [ "$OS" == "Debian" ] || [ "$OS" == "Ubuntu" ]; then
        $PM install php5 php5-gd php5-mysqlnd
    else
        $PM install php php-gd php-mysqlnd
    fi
else
    logprint "Not installing php packages. You have to check them manually."
    EndNotice= "$EndNotice \n * check for installed php5 php5-gd and php5-mysqlnd"
fi

# configure webserver
logprint ""
case $WebServer in
    "apache")
        logprint "Creating config for apache"
        "$InstallFilesFolder/setup.examples/apache/mailwatch-apache.sh" "$WebFolder"
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

###############fix web dir permissions#################################
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
fi

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
            "$InstallFilesFolder/setup.examples/postfix/mailwatch-postfix.sh" "$Webuser"
            sleep 1
            break
            ;;

        "exim")
            logprint "Configure MailScanner for use with exim"
            "$InstallFilesFolder/setup.examples/exim/mailwatch-exim.sh" "$Webuser"
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

logprint "Adjusting perl files"
sed -i -e "s/my (\$db_name) = '.*'/my (\$db_name) = '$SqlDb'/"   "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"
sed -i -e "s/my (\$db_host) = '.*'/my (\$db_host) = '$SqlHost'/" "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"
sed -i -e "s/my (\$db_user) = '.*'/my (\$db_user) = '$SqlUser'/" "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"
sed -i -e "s/my (\$db_pass) = '.*'/my (\$db_pass) = '$SqlPwd'/"  "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"

logprint "Copying perl files to MailScanner"
cp "$MailWatchTmpDir"/MailScanner_perl_scripts/*.pm /etc/MailScanner/custom/

logprint "Restart mailscanner service"
service mailscanner restart
#todo relay files
logprint "Install finished!"
logprint "Next steps you have to do are:"
logprint "$EndNotice"
logprint " * adjust your mta and web server configs"
logprint " * adjust the MailWatch config $WebFolder/conf.php"
echo ""
echo "You can find the log file at /root/mailwatchInstall.log"
