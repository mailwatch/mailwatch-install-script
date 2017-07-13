#!/bin/bash

function install-php-core() {
    logprint ""
    logprint "MailWatch requires the php packages php php-gd, php-mysqlnd, php-curl and libapache2-mod-php/php-fpm." 
    ask " Do you want to install them if missing?(y/n)[y]: " installPhp
    if [ -z $installPhp ] || [ "$installPhp" == "y" ]; then
        logprint "Installing required php packages"
        # since debian 9 and ubuntu 16 packages are php php-gd etc. instead of php5.
        # So we test if the php package is available over apt and if that's not the case we install php5
        if [ "$OS" == "Debian" ]; then
            if [[ -z $(apt-cache search php | grep "^php ") ]]; then
                if [[ "$WebServer" == "apache" ]]; then
                    $PM install libapache2-mod-php5
                elif [[ "$WebServer" == "nginx" ]]; then
                    $PM install php5-fpm
                fi
                $PM install php5 php5-gd php5-mysqlnd php5-curl
            else
                if [[ "$WebServer" == "apache" ]]; then
                    $PM install libapache2-mod-php
                elif [[ "$WebServer" == "nginx" ]]; then
                    $PM install php-fpm
                fi
                $PM install php php-gd php-mysql php-curl php-mbstring
            fi
        else
# TODO: check if specific dependency has to be installed for apache/nginx so only one webserver will be installed
#            if [[ "$WebServer" == "apache" ]]; then
#               $PM install libapache2-mod-php
#            elif [[ "$WebServer" == "nginx" ]]; then
#                $PM install  php-fpm
#            fi
            $PM install php php-gd php-mysqlnd php-curl
        fi
    else
        logprint "Not installing php packages. You have to check them manually."
        EndNotice= "$EndNotice \n * check for installed php5 php5-gd and php5-mysqlnd"
    fi
}

function install-php-ldap() {
    logprint ""
    ask "Do you want to use LDAP for user authentication (requires php-ldap)? (y/n)[y]: " installLdap
    if [ -z $installLdap ] || [ "$installLdap" == "y" ]; then
        logprint "Installing required php-ldap package"
        if [ "$OS" == "Debian" ]; then
            if [[ -z $(apt-cache search php-ldap | grep "^php-ldap ") ]]; then
                $PM install php5-ldap
            else
                $PM install php-ldap
            fi
        else
            $PM install php-ldap
        fi
    else
        logprint "Not installing php-ldap package. You have to install it manually to use LDAP."
    fi
}

function install-php-rpc() {
    logprint ""
    ask "Do you want to use RPC for this server (requires php-xml)? (y/n)[y]: " installRpc
    if [ -z $installRpc ] || [ "$installLdap" == "y" ]; then
        logprint "Installing required php-xml package"
        if [ "$OS" == "Debian" ]; then
             if [[ -z $(apt-cache search php-xml | grep "^php-xml ") ]]; then
                $PM install php5-xml
            else
                $PM install php-xml
            fi
        else
            $PM install php-xml
        fi
    else
        logprint "Not installing php-xml package. You have to install it manually to use RPC."
    fi
}

function install-php() {
    install-php-core
    install-php-ldap
    install-php-rpc
}

