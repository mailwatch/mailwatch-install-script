#!/bin/bash

function install-webserver {
#todo only configure on install. not on update
#todo: vhost ssl, centos/redhat config
    #Apache
    logprint "Creating config for apache"
    if ( type "apache2" > /dev/null 2>&1 ); then
        apacheBin="apache2"
    elif ( type "httpd" > /dev/null 2>&1 ); then
        apacheBin="httpd"
    else
        logprint "Installing apache"
        if [ "$PM" == "yum" ]; then
            $PM install httpd mod_ssl
            apacheBin="httpd"
        else
            $PM install apache2
            if [[ -z $(apt-cache search php | cut -d " " -f 1 | grep "^libapache2-mod-php$") ]]; then
                missingPhpWebserverPackages=$(getMissingDebianPackages "libapache2-mod-php5")
            else
                missingPhpWebserverPackages=$(getMissingDebianPackages "libapache2-mod-php")
            fi
            apacheBin="apache2"
        fi
    fi

    if [ "$apacheBin" == "httpd" ]; then
        mwApacheConf="/etc/httpd/conf.d/mailwatch.conf
    elif [ "$apachebin" == "apache2" ]; then
        #backup old config
        mwApacheConf="/etc/apache2/sites-available/mailwatch.conf
    fi

    if [ -f "$mwApacheConf" ]; then
        mv "$mwApacheConf" "$mwApacheConf.old"
    fi

#    cp "$InstallFilesFolder/setup.scripts/apache/etc/apache2/conf-available/mailwatch.conf" /etc/apache2/conf-available/mailwatch.conf
    cp "$InstallFilesFolder/setup.scripts/apache/etc/apache2/sites-available/mailwatch.conf" "$mwApacheConf"

#    if [[ -n $($apacheBin -v | grep "/2.4.") ]]; then
#        sed -i -e "s/ALLGRANTED/Require all granted/" "/etc/apache2/conf-available/mailwatch.conf"
#    else
#        sed -i -e "s/ALLGRANTED/Order allow,deny\n  Allow from all/" "/etc/apache2/conf-available/mailwatch.conf"
#    fi
    sed -i -e "s~WEBFOLDER~$WebFolder~" "$mwApacheConf"

    if [ "$apacheBin" == "apache2" ]; then
        # a2enconf mailwatch.conf
        a2enmod ssl
        ln -f -r -s "$mwApacheConf" /etc/apache2/conf-enabled/mailwatch.conf
    fi

    service "$apacheBin" reload
    if [[ "$PM" == "yum"* ]]; then
        Webuser="apache"
    else
        Webuser="www-data"
    fi
}
