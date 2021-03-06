#!/bin/bash

function prepare-php-core() {
    logprint ""
    if ! [ -z "$missingPhpCorePackages" ]; then
        ask "MailWatch requires the php packages $missingPhpCorePackages. Do you want to install them?(y/n)[y]: " installPhp
        if [ -z "$installPhp" ] || [ "$installPhp" == "y" ]; then
            logprint "Installing required php packages"
            phpPackagesToInstall="$phpPackagesToInstall $missingPhpCorePackages"
        else
            logprint "Not installing php packages. You have to check them manually."
            EndNotice= "$EndNotice \n * check for installed $missingPhpCorePackages"
        fi
    else
        logprint "PHP core packages already installed"
    fi
}

function prepare-php-ldap() {
    logprint ""
    if ! [ -z "$missingPhpLdapPackages" ]; then
        ask "Do you want to use LDAP for user authentication (requires php-ldap)? (y/n)[y]: " installLdap
        if [ -z "$installLdap" ] || [ "$installLdap" == "y" ]; then
            logprint "Installing required php-ldap package"
            phpPackagesToInstall="$phpPackagesToInstall $missingPhpLdapPackages"
        else
            logprint "Not installing php-ldap package. You have to install it manually to use LDAP."
        fi
    else
        logprint "Packages needed for user authentication over LDAP already installed"
    fi
}

function prepare-php-install() {
    if [ "$OS" == "Debian" ]; then
        if [[ -z $(apt-cache search php | cut -d " " -f 1 | grep '^php$') ]]; then
            missingPhpCorePackages=$(getMissingDebianPackages "php5" "php5-mysqlnd" "php5-curl")
            missingPhpLdapPackages=$(getMissingDebianPackages "php5-ldap")
        else
            missingPhpCorePackages=$(getMissingDebianPackages "php" "php-mysql" "php-curl" "php-mbstring" "php-xml")
            missingPhpLdapPackages=$(getMissingDebianPackages "php-ldap")
        fi
    else
        #curl in php-common
        missingPhpCorePackages=$(getMissingRpmPackages "php" "php-mysqlnd" "php-common" "php-xml")
        missingPhpLdapPackages=$(getMissingRpmPackages "php-ldap")
    fi
    prepare-php-core
    prepare-php-ldap
}

function install-php() {
    if ! [ -z "$missingPhpWebserverPackages" ]; then
        phpPackagesToInstall="$phpPackagesToInstall $missingPhpWebserverPackages"
    fi
    if ! [ -z "$phpPackagesToInstall" ]; then
        $PM install ${phpPackagesToInstall}
    fi
}
