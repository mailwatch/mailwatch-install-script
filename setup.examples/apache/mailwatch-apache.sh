#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WebFolder="$1"
echo $WebFolder
if ( type "apache2" > /dev/null 2>&1 ); then
  apacheBin="apache2"
else
  apacheBin="httpd"
fi

#backup old config
if [ -f /etc/apache2/conf-available/mailwatch.conf ]; then
    mv /etc/apache2/conf-available/mailwatch.conf /etc/apache2/conf-available/mailwatch.conf.old
fi

cp "$DIR/etc/apache2/conf-available/mailwatch.conf" /etc/apache2/conf-available/mailwatch.conf

if [[ -z $($apacheBin -v | grep "/2.4.") ]]; then
    sed -i -e "s/ALLGRANTED/Require all granted/" "/etc/apache2/conf-available/mailwatch.conf"
else
    sed -i -e "s/ALLGRANTED/Order allow,deny\n  Allow from all/" "/etc/apache2/conf-available/mailwatch.conf"
fi
sed -i -e "s~WEBFOLDER~$WebFolder~" "/etc/apache2/conf-available/mailwatch.conf"

# a2enconf mailwatch.conf
ln -f -r -s /etc/apache2/conf-available/mailwatch.conf /etc/apache2/conf-enabled/mailwatch.conf

a2enmod ssl
service apache2 reload
