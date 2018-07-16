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
        if [[ "$PM" == "yum"* ]]; then
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
    logprint "Apache binary is $apacheBin"

    if [ "$apacheBin" == "httpd" ]; then
        mwApacheConf="/etc/httpd/conf.d/ssl_mailwatch.conf"
    elif [ "$apacheBin" == "apache2" ]; then
        mwApacheConf="/etc/apache2/sites-available/ssl_mailwatch.conf"
    fi

    logprint "Using apache conf file $mwApacheConf"

    #backup old config
    if [ -f "$mwApacheConf" ]; then
        mv "$mwApacheConf" "$mwApacheConf.old"
    fi

#    cp "$InstallFilesFolder/setup.scripts/apache/etc/apache2/conf-available/mailwatch.conf" /etc/apache2/conf-available/mailwatch.conf
    cp "$InstallFilesFolder/setup.scripts/apache/etc/apache2/sites-available/ssl_mailwatch.conf" "$mwApacheConf"

    logprint "Patching apache conf file"

    if [[ -n $("$apacheBin" -v | grep "/2.4.") ]]; then
        sed -i -e "s/ALLGRANTED/Require all granted/" "$mwApacheConf"
    else
        sed -i -e "s/ALLGRANTED/Order allow,deny\n  Allow from all/" "$mwApacheConf"
    fi

    escapedHostname="${mwHost/\./\\\.}"
    sed -i -e "s~MWHOSTNAME~$escapedHostname~" "$mwApacheConf"
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
