#!/bin/bash
# Bash Menu Script Example

function install-mysql() {
    ############ Configure mysql server ################
    logprint ""
    if ! ( type "mysqld" > /dev/null 2>&1 ) ; then
        ask "No mysql server found. Do you want to install mariadb as sql server?(y/n)[y]: " installMariadb
        if [ -z $installMariadb ] || [ $installMariadb == "y" ]; then
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
}

function configure-mysql() {
    if [ "$IsUpgrade" == 1 ]; then
        logprint "Not creating sql db because we are running an upgrade"
    elif [ "$mysqlInstalled" == "1" ]; then
        ask "Root sql user (with rights to create db)[root]: " SqlRoot
        if [ -z $SqlRoot ]; then
            SqlRoot="root"
        fi
        ask "Passwort for root sql user: " SqlRootPwd
        logprint "Creating sql database and setting permission."
        mysql -u $SqlRoot -p$SqlRootPwd < "$MailWatchTmpDir/create.sql"
        mysql -u $SqlRoot -p$SqlRootPwd --execute="GRANT ALL ON $SqlDb.* TO $SqlUser@localhost IDENTIFIED BY '$SqlPwd'; GRANT FILE ON *.* TO $SqlUser@localhost IDENTIFIED BY '$SqlPwd'; FLUSH PRIVILEGES"
        SqlRootPwd=

        ask "Enter an admin user for the MailWatch web interface: " MWAdmin
        ask "Enter password for the admin: " MWAdminPwd
        logprint "Create MailWatch web gui admin"
        mysql -u $SqlUser -p$SqlPwd $SqlDb --execute="REPLACE INTO users SET username = '$MWAdmin', password = MD5('$MWAdminPwd'), fullname = 'Admin', type = 'A';"
    else
        echo "You have to create the database yourself!"
        EndNotice="$EndNotice \n * create the database, a sql user with access to the db and following properties user: $SqlUser; password: $SqlPwd; db: $SqlDb; host: $SqlHost"
        EndNotice="$EndNotice \n * create an admin account for the web gui"
    fi
}

