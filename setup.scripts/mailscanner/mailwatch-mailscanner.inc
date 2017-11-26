#!/bin/bash
# Bash Menu Script Example

MailScannerTmpDir="$TmpDir/mailscanner/"

function prepare-mailscanner() {
    if [ "$IsUpgrade" != "1" ]; then
        ask 'Which MTA do you want to use with MailWatch? (it should already be installed):
  MTAs: 1:postfix (default), 2:exim, 3:sendmail, 4: skip :' useMta
        if [ "$useMta" == "1" ] || [ -z "$useMta" ]; then
            MTA="postfix"
        elif [ "$useMta" == "2" ];then
           MTA="exim"
        elif [ "$useMta" == "3" ]; then
            MTA="sendmail"
            logprint "Sendmail is not yet supported. If you have experience with sendmail and want to help support this please contact us at https://github.com/mailwatch/mailwatch-install-script"
            exit
        else
            MTA="none"
        fi
        logprint "Selected mta $MTA"
    else
        oldMTA=$(grep -hr MTA /etc/MailScanner/MailScanner.conf /etc/MailScanner/conf.d/ | grep -v "#" | cut -d"=" -f2 | sed -e 's/[[:space:]]*//' | grep -v -e '^[[:space:]]*$' | tail -n 1)
        if [ "$oldMTA" != "postfix" ] && [ "$oldMTA" != "exim" ] && [ "$oldMTA" != "sendmail" ]; then
            logprint "Couldn't detect old MTA from MailScanner conf files"
            exit 1
        else
            MTA=$oldMTA
        fi
    fi

    ask 'Do you want to use Clamav? (y/n)[y]:' useClamav
    useClamav=${useClamav:-y}
    useClamav=${useClamav,,}
    if [ "$useClamav" != "y" ] && [ "$useClamav" != "n" ]; then
        logprint "Invalid option given for use of Clamav!!!"
        exit 1;
    fi

    ask 'Do you want to use CPAN to install missing packages? (y/n)[y]:' useCpan
    useCpan=${useCpan:-y}
    useCpan=${useCpan,,}
    if [ "$useCpan" != "y" ] && [ "$useCpan" != "n" ]; then
        logprint "Invalid option given for use of CPAN!!!"
        exit 1;
    fi

    ask 'Do you want to ignore the dependecies of MailScanner? (y/n)[n]:' ignoreDeps
    ignoreDeps=${ignoreDeps:-n}
    ignoreDeps=${ignoreDeps,,}
    if [ "$ignoreDeps" != "y" ] && [ "$ignoreDeps" != "n" ]; then
        logprint "Invalid option given for use of ignoreDeps!!!"
        exit 1;
    fi

    ask 'Do you want to use a RAM disk with mailscanner for incoming and outgoing mails? Enter a size in MB here [256]:' ramdiskSize
    ramdiskSize=${ramdiskSize:-256}
    if ! [[ "$ramdiskSize" =~ [0-9]+ ]]; then
        logprint "Invalid option given for RAM disk size. It has to be an integer!!!"
        exit 1;
    fi
}

function install-mailscanner() {
    logprint ""
    if [[ "$PM" == "yum"* ]]; then
        MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.rhel.tar.gz"
    elif [[ "$PM" == "apt-get"* ]]; then
        MailScannerDownloadPath="https://s3.amazonaws.com/msv5/release/MailScanner-$MailScannerVersion.deb.tar.gz"
    fi

    ask "Install/upgrade MailScanner version $MailScannerVersion?:(y/n)[y]: " installMailScanner
    if [ -z $installMailScanner ] || [ "$installMailScanner" == "y" ]; then
        logprint "Starting MailScanner install"
        mkdir -p /tmp/mailwatchinstall/mailscanner
        logprint "Downloading current MailScanner release $MailScannerVersion:"
        wget -O "$MailScannerTmpDir/MailScanner.deb.tar.gz"  "$MailScannerDownloadPath"
        logprint "Extracting mailscanner files:"
        tar -xzf "$MailScannerTmpDir/MailScanner.deb.tar.gz" -C "$MailScannerTmpDir/"
        logprint "Starting MailScanner install script"

        "$MailScannerTmpDir/MailScanner-$MailScannerVersion/install.sh" "--MTA=$MTA"  "--installClamav=$useClamav" "--installCPAN=$useCpan" "--ignoreDeps=$ignoreDeps" "--ramdiskSize=$ramdiskSize"
        if [[ "$?" != "0" ]]; then
            echo "Some error occured while installing MailScanner."
            exit 1
        fi

        logprint "MailScanner install finished."
        EndNotice="$EndNotice \n * Adjust MailScanner config by creating an adjusted config file in /etc/MailScanner/conf.d/ \n * Set run_mailscanner=1 in /etc/MailScanner/defaults"
        sleep 1
    else
        logprint "Not installing MailScanner"
    fi

    logprint "Stopping mailscanner"
    service mailscanner stop

    install-fixlatin
    install-digestsha1
}

function install-fixlatin() {
    perl -MEncoding::FixLatin -e 1 &> /dev/null
    if [ "$?" == "0" ]; then
        echo "Encoding::FixLatin already installed"
    else
        echo "Encoding::FixLatin not installed. Installing it now."
        cpan -i Encoding::FixLatin
    fi
}

function install-digestsha1() {
    perl -MDigest::SHA1 -e 1 &> /dev/null
    if [ "$?" == "0" ]; then
        echo "Digest::SHA1 already installed"
    else
        echo "Digest::SHA1 not installed. Installing it now."
        cpan -i Digest::SHA1
    fi
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

