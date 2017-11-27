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

function prepare-php-rpc() {
    logprint ""
    if ! [ -z "$missingPhpRpcPackages" ]; then
        ask "Do you want to use RPC for this server (requires php-xml)? (y/n)[y]: " installRpc
        if [ -z "$installRpc" ] || [ "$installRpc" == "y" ]; then
            logprint "Installing required php-xml package"
            phpPackagesToInstall="$phpPackagesToInstall $missingPhpRpcPackages"
        else
            logprint "Not installing php-xml package. You have to install it manually to use RPC."
        fi
    else
        logprint "Packages needed for RPC already installed"
    fi
}

function prepare-php-install() {
    if [ "$OS" == "Debian" ]; then
        if [[ -z $(apt-cache search php | cut -d " " -f 1 | grep '^php$') ]]; then
            missingPhpCorePackages=$(getMissingDebianPackages "php5" "php5-mysqlnd" "php5-curl")
            missingPhpLdapPackages=$(getMissingDebianPackages "php5-ldap")
            missingPhpRpcPackages=$(getMissingDebianPackages "php5-xml")
        else
            missingPhpCorePackages=$(getMissingDebianPackages "php" "php-mysql" "php-curl" "php-mbstring")
            missingPhpLdapPackages=$(getMissingDebianPackages "php-ldap")
            missingPhpRpcPackages=$(getMissingDebianPackages "php-xml")
        fi
    else
        missingPhpCorePackages=$(getMissingRpmPackages "php" "php-mysqlnd" "php-curl")
        missingPhpLdapPackages=$(getMissingRpmPackages "php-ldap")
        missingPhpRpcPackages=$(getMissingRpmPackages "php-xml")
    fi
    prepare-php-core
    prepare-php-ldap
    prepare-php-rpc
}

function install-php() {
    if ! [ -z "$missingPhpWebserverPackages" ]; then
        phpPackagesToInstall="$phpPackagesToInstall $missingPhpWebserverPackages"
    fi
    if ! [ -z "$phpPackagesToInstall" ]; then
        $PM install ${phpPackagesToInstall}
    fi
}
