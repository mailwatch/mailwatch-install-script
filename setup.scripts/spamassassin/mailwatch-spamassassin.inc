#!/bin/bash
# Bash Menu Script Example

function prepare-spamassassin-install() {
    logprint ""
    if ! ( type "spamassassin" > /dev/null 2>&1 ) ; then
        logprint "Spamassassin not installed. Going to install it"
        installSpamassassin="y"
    else
        logprint "Spamassassin already installed"
        installSpamassassin="n"
    fi
}

function install-spamassassin() {
    ############ Configure mysql server ################
    if [ "$installSpamassassin" == "y" ]; then
        logprint "Start install of spamassassin"
        $PM install spamassassin
    else
        logprint "Not installing spamassassin"
    fi
}
