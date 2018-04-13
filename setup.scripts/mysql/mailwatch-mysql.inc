#!/bin/bash
# Bash Menu Script Example

function prepare-mysql-install() {
    if ! ( type "mysqld" > /dev/null 2>&1 ) ; then
        ask "No mysql server found. Do you want to install mariadb as sql server?(y/n)[y]: " installMariadb
        if [ -z $installMariadb ] || [ $installMariadb == "y" ]; then
            logprint "MariaDB will be installed"
            ask "Password that should be set for the database root user: (Input is not shown)" SqlRootPwd "pw"
        else
            logprint "No mysql server found and none will be installed. We assume that you have an other mysql server that you want to use. If not the setup will not work correctly."
            sleep 5
            ask "Root sql user (with rights to create db)[root]: " SqlRoot
            ask "Password for root sql user: (Input is not shown)" SqlRootPwd "pw"
        fi
    else
        mysqlInstalled="1"
        logprint "Found installed mysql server and will use that"
        ask "Root sql user (with rights to create db)[root]: " SqlRoot
        ask "Password for root sql user: (Input is not shown)" SqlRootPwd "pw"
    fi
    if [ -z "$SqlRoot" ]; then
        SqlRoot="root"
    fi

}

function install-mysql() {
    ############ Configure mysql server ################
    if [ "$mysqlInstalled" != "1" ] && ([ -z $installMariadb ] || [ $installMariadb == "y" ]); then
        logprint "Start install of mariadb"
        if [ $OS == "CentOS" ] ; then
            logprint "Adding MariaDB 10.1 Repo"
            cat $InstallFilesFolder/yum.repo/$OS-$OSVersion-MariaDB.$ARCH.repo > /etc/yum.repos.d/MariaDB.repo
            $PM clean all
            $PM install mariadb-server
            #TODO::Check for other rpm variants
        else
            $PM install mariadb-server mariadb-client
        fi

        #Check install was successful
        if ! ( type "mysqld" > /dev/null 2>&1 ) ; then
            logprint "MariaDB installation failed - You will have to install manually"
            mysqlInstalled="0"
        else
            mysqlInstalled="1"
            logprint "MariaDB installed - now we need to secure the installation"
            logprint "Please enter the required details in the next step - for security, please enter a password"
            if [ -z "$skipSecureSqlInstallation" ]; then
                mysql_secure_installation
            elif [ "$mysql_secure_installation" == "n" ]; then
                logprint "Running mysql_secure_installation unattended"
                # the content of following lines are piped to mysql_secure_installation: 1.line current root pw (default empty)
                # 2.line change pw, 3./4.line new root pw, 5. remove anonymous user, 6. disallow remote root, 7. remove test db, 8. reload privs
                cat <<EOF | mysql_secure_installation

y
$SqlRoot
$SqlRootPwd





EOF
            else
                logprint "Skipping mysql_secure_installation"
                EndNotice="$EndNotice \n * you should call mysql_secure_installation to make mariadb secure"
            fi
        fi
    fi

    if [ "$IsUpgrade" == 1 ]; then
        logprint "Not creating sql db because we are running an upgrade"
    elif [ "$mysqlInstalled" == "1" ]; then
        logprint "Creating sql database and setting permission."
        mysql -u "$SqlRoot" -p"$SqlRootPwd" -h "$SqlHost" < "$MailWatchTmpDir/create.sql"
        mysql -u "$SqlRoot" -p"$SqlRootPwd" -h "$SqlHost" --execute="GRANT ALL ON $SqlDb.* TO \"$SqlUser\"@\"$SqlHost\" IDENTIFIED BY '$SqlPwd'; GRANT FILE ON *.* TO \"$SqlUser\"@\"$SqlHost\" IDENTIFIED BY '$SqlPwd'; FLUSH PRIVILEGES"
        SqlRootPwd=

        ask "Enter an admin user for the MailWatch web interface: " MWAdmin
        ask "Enter password for the admin: " MWAdminPwd "pw"
        logprint "Create MailWatch web gui admin"
        mysql -u "$SqlUser" -p"$SqlPwd" -h "$SqlHost" "$SqlDb" --execute="REPLACE INTO users SET username = '$MWAdmin', password = MD5('$MWAdminPwd'), fullname = 'Admin', type = 'A';"
    else
        echo "You have to create the database yourself!"
        EndNotice="$EndNotice \n * create the database, a sql user with access to the db and following properties user: $SqlUser; password: $SqlPwd; db: $SqlDb; host: $SqlHost"
        EndNotice="$EndNotice \n * create an admin account for the web gui"
    fi
}

