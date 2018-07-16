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

    ask "Do you want to use Bayes-Sql backend?[y]" useBayesSql
    if [ -z "$useBayesSql" ] || [ "$useBayesSql" == "y" ]; then
        logprint "Bayes-Sql is going to be installed"
        installBayesSql="y"
    else
        logprint "Not installing Bayes-Sql"
        installBayesSql="n"
    fi
}

function install-spamassassin() {
    logprint ""
    if [ "$installSpamassassin" == "y" ]; then
        logprint "Start install of spamassassin"
        $PM install spamassassin
    else
        logprint "Not installing spamassassin"
    fi
    if [ "$installBayesSql" == "y" ]; then
        logprint "Configure Bayes-Sql"
        #mysql -u "$SqlRoot" -p"$SqlRootPwd" -h "$SqlHost" < "$InstallFilesFolder/setup.scripts/spamassassin/bayesSql.sql"
        #todo do whatever is needed to config spamassassin to use it; create sql file
    fi
}
