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

    #test if bayes sql module is already configured
    bayesSqlOptionExists=$(grep -e "^[[:space:]]*bayes_store_module[[:space:]]*Mail::SpamAssassin:BayesStore::MySQL")
    if [ -z "$bayesSqlOptionExists" ]; then
        ask "Do you want to use Bayes-Sql backend?[y]" useBayesSql
        if [ -z "$useBayesSql" ] || [ "$useBayesSql" == "y" ]; then
            logprint "Bayes-Sql is going to be installed"
            installBayesSql="y"
        else
            logprint "Not installing Bayes-Sql"
            installBayesSql="n"
        fi
    else
        logprint "Bayes-Sql already configured"
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
}

function install-bayesSql() {
    if [ "$installBayesSql" != "n" ]; then
        logprint "Configure Bayes-Sql"
        mysql -u "$SqlRoot" -p"$SqlRootPwd" -h "$SqlHost" "$SqlDb" < "$InstallFilesFolder/setup.scripts/spamassassin/bayesSql_mysql.sql"

        cat <<EOF >> /etc/MailScanner/spamassassin.conf
bayes_store_module              Mail::SpamAssassin::BayesStore::MySQL
bayes_sql_dsn                   DBI:mysql:$SqlDb:$SqlHost
bayes_sql_username              $SqlUser
bayes_sql_password              $SqlPass
bayes_sql_override_username     $SqlUser
EOF
    fi
}
