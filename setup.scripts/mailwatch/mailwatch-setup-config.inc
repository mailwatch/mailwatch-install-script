#!/bin/bash

#defines if MailScanner should be installed/upgraded
installMailScanner=
#Path to new/old web directory
WebFolder=
#If WebFolder exists this confirms to overwrite it
overwriteWebFolder=
#When WebFolder exists this will save the old conf.php, skin.css
upgrading=

#Sql settings, when not upgrading this user gets access to the MW db
SqlUser=
SqlPwd=
SqlDb=
SqlHost=
#install mariadb-server if no sql server was found
installMariadb=
#if "y" skips the mysql_secure_installation during the script. with "n" its run in non interactive mode which is a bit hackish and if something goes wrong can result in loss of root access to mysql
#if undefined it asks interactively during install (y/n)
skipSecureSqlInstallation=
#sql user with rights to create the sql database and assign permissions to it (not used on upgrade)
SqlRoot=
#pwd for sql root user
SqlRootPwd=

#which web server shall be installed (1: apache, 2: nginx, others: skip)
WebServer=
#when skipping web server install, the permissions will be assigned to this user
WebUser=

#which mta to configure for MW: 1:postfix, 2:exim, 3:sendmail, 4/n/others:skip
useMta=
#use/install clamav by mailscanner (y/n)
useClamav=
#use Cpan to install missing packages (y/n)
useCpan=
# ignore dependecies for MailScanner
ignoreDeps=
#RAM disk size for uMailScanner
ramdiskSize=

#php install: will install/upgrade neccessary packets with system package manager
# base php packets need for MW
installPhp=
# packages for use with ldap
installLdap=
# packages for use with rpc
installRpc=

#credentials for new MW admin (not used on upgrade)
MWAdmin=
MWAdminPwd=

#install sudo (required for MailScanner Lint over the web interface)
installSudo=

#confirm each installation of packages done with the package manager (y/n). Default is n.
confirmPackageInstalls=

function resetVariables() {
    installMailScanner=
    WebFolder=
    overwriteWebFolder=
    upgrading=
    SqlUser=
    SqlPwd=
    SqlDb=
    SqlHost=
    webserverSelect=
    WebUser=
    useMta=
    installPhp=
    installLdap=
    installRpc=
    installMariadb=
    installSudo=
    SqlRoot=
    SqlRootPwd=
    MWAdmin=
    MWAdminPwd=
    confirmPackageInstalls=
    EndNotice=
}
