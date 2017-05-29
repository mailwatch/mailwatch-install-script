#!/bin/bash
# Bash Menu Script Example

MailScannerTmpDir="$TmpDir/mailscanner/"

function install-mailscanner() {
    logprint ""
    if [ "$PM" == "yum" ]; then
        MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.rhel.tar.gz"
    elif [ "$PM" == "apt-get" ]; then
        MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.deb.tar.gz"
    fi

    read -p "Install/upgrade MailScanner version $MailScannerVersion?:(y/n)[y]: " installMailScanner
    if [ -z $installMailScanner ] || [ "$installMailScanner" == "y" ]; then
        logprint "Starting MailScanner install"
        mkdir -p /tmp/mailwatchinstall/mailscanner
        logprint "Downloading current MailScanner release $MailScannerVersion:"
        wget -O "$MailScannerTmpDir/MailScanner.deb.tar.gz"  "$MailScannerDownloadPath"
        logprint "Extracting mailscanner files:"
        tar -xzf "$MailScannerTmpDir/MailScanner.deb.tar.gz" -C "$MailScannerTmpDir/"
        logprint "Starting MailScanner install script"
        "$MailScannerTmpDir/MailScanner-$MailScannerVersion/install.sh"
        logprint "MailScanner install finished."
        EndNotice="$EndNotice \n * Adjust /etc/MailScanner.conf to your needs \n * Set run_mailscanner=1 in /etc/MailScanner/defaults"
        sleep 1
    else
        logprint "Not installing MailScanner"
    fi

    logprint "Stopping mailscanner"
    service mailscanner stop

    logprint ""
    logprint "Installing Encoding::FixLatin"
    cpan -i Encoding::FixLatin
}

function configure-mailscanner() {
    logprint "Adjusting perl files"
    sed -i -e "s/my (\$db_name) = '.*'/my (\$db_name) = '$SqlDb'/"   "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"
    sed -i -e "s/my (\$db_host) = '.*'/my (\$db_host) = '$SqlHost'/" "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"
    sed -i -e "s/my (\$db_user) = '.*'/my (\$db_user) = '$SqlUser'/" "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"
    sed -i -e "s/my (\$db_pass) = '.*'/my (\$db_pass) = '$SqlPwd'/"  "$MailWatchTmpDir/MailScanner_perl_scripts/MailWatchConf.pm"

    logprint "Copying perl files to MailScanner"
    cp "$MailWatchTmpDir"/MailScanner_perl_scripts/*.pm /etc/MailScanner/custom/

    logprint "Restart mailscanner service"
    service mailscanner restart
}

