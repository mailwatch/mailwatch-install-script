#!/bin/bash

function install-webserver {
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

    if [ "$IsUpgrade" == "1" ]; then
        logprint "Updating: not configuring apache"
    else
        ## mwApacheVhostConf basic virtual hosts config file (can be enable via symlink; default: enabled)
        ## mwApacheDirConf config file for use as virtual directory (can be enable via symlink; default: disabled)
        if [ "$apacheBin" == "httpd" ]; then
            mwApacheVhostConf="/etc/httpd/conf.d/ssl_mailwatch.conf.dist"
            mwApacheDirConf="/etc/httpd/conf.d/dir_mailwatch.conf.dist"
        elif [ "$apacheBin" == "apache2" ]; then
            mwApacheVhostConf="/etc/apache2/sites-available/mailwatch-ssl.conf"
            mwApacheDirConf="/etc/apache2/conf-available/mailwatch-dir.conf"
        fi

        logprint "Using apache conf file $mwApacheVhostConf"

        cp "$InstallFilesFolder/setup.scripts/apache/etc/apache2/sites-available/ssl_mailwatch.conf" "$mwApacheVhostConf"
        cp "$InstallFilesFolder/setup.scripts/apache/etc/apache2/conf-available/mailwatch.conf" "$mwApacheDirConf"

        logprint "Patching apache conf file"

        if [[ -n $("$apacheBin" -v | grep "/2.4.") ]]; then
            sed -i -e "s/ALLGRANTED/Require all granted/" "$mwApacheVhostConf"
            sed -i -e "s/ALLGRANTED/Require all granted/" "$mwApacheDirConf"
        else
            sed -i -e "s/ALLGRANTED/Order allow,deny\n  Allow from all/" "$mwApacheVhostConf"
            sed -i -e "s/ALLGRANTED/Order allow,deny\n  Allow from all/" "$mwApacheDirConf"
        fi

        #set hostname for virtual host
        escapedHostname="${mwHost/\./\\\.}"
        sed -i -e "s~MWHOSTNAME~$escapedHostname~" "$mwApacheVhostConf"

        #set MailWatch directory
        sed -i -e "s~WEBFOLDER~$WebFolder~" "$mwApacheVhostConf"
        sed -i -e "s~WEBFOLDER~$WebFolder~" "$mwApacheDirConf"


        #enable the virtual host config if mailwatch.conf doesn't exist yet
        if [ "$apacheBin" == "httpd" ]; then
            if [ ! -f /etc/httpd/conf.d/mailwatch.conf ]; then
                ln -s "$mwApacheVhostConf" /etc/httpd/conf.d/mailwatch.conf
            fi
        elif [ "$apacheBin" == "apache2" ]; then
            a2enmod ssl
            if [ ! -f /etc/apache2/sites-enabled/mailwatch.conf ]; then
                ln -s "$mwApacheVhostConf" /etc/apache2/sites-enabled/mailwatch.conf
            fi
        fi
    fi

    service "$apacheBin" reload
    if [[ "$PM" == "yum"* ]]; then
        Webuser="apache"
    else
        Webuser="www-data"
    fi
}
