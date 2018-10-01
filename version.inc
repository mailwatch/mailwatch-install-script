#!/bin/bash
# MWIS Version 1.0

MailScannerVersion=$(wget https://mailscanner.info/downloads -q -O - | grep "Stable" | cut -d'>' -f2 | cut -d'<' -f1 | cut -d' ' -f2 | cut -d'v' -f2)
if [ "$?" != "0" ]; then
    logprint "Current MailScanner version could not be found"
    exit 1
else
    logprint "Current MailScanner version is $MailScannerVersion"
fi

MailWatchVersion=$(wget https://github.com/mailwatch/mailwatch/releases -q -O - | grep '<span class="css-truncate-target"' | grep -vi 'rc' | head -n1 | cut -d'>' -f2 | cut -d'<' -f1)
#1.2 stable release
#MailWatchVersion="v1.2.9"
#1.2 dev branch
#MailWatchVersion="1.2"

if [ "$?" != "0" ]; then
    logprint "Current MailWatch version could not be found"
    exit 1
else
    logprint "Current MailWatch version is $MailWatchVersion"
fi


