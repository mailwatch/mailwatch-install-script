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
    if [ -f /etc/MailScanner/spamassassin.conf ]; then
        bayesSqlOptionExists=$(grep -e "^[[:space:]]*bayes_store_module[[:space:]]*Mail::SpamAssassin::BayesStore::MySQL" /etc/MailScanner/spamassassin.conf)
    else
        bayesSqlOptionExists=""
    fi
    if [ -z "$bayesSqlOptionExists" ]; then
        ask "Do you want to use Bayes-Sql backend (y/n)?[y]" useBayesSql
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

    #test if bayes awl module is already configured
    if [ -f /etc/MailScanner/spamassassin.conf ]; then
        bayesAwlOptionExists=$(grep -e "^[[:space:]]*auto_whitelist_factory[[:space:]]*Mail::SpamAssassin::SQLBasedAddrList" /etc/MailScanner/spamassassin.conf)
    else
        bayesAwlOptionExists=""
    fi
    if [ -z "$bayesAwlOptionExists" ]; then
        ask "Do you want to use Bayes auto whitelist (y/n)?[y]" useBayesAwl
        if [ -z "$useBayesAwl" ] || [ "$useBayesAwl" == "y" ]; then
            logprint "Bayes-AWL is going to be installed"
            installBayesAwl="y"
        else
            logprint "Not installing Bayes-AWL"
            installBayesAwl="n"
        fi
    else
        logprint "Bayes-AWL already configured"
        installBayesAwl="n"
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
        mysql -u "$SqlUser" -p"$SqlPwd" -h "$SqlHost" "$SqlDb" < "$InstallFilesFolder/setup.scripts/spamassassin/bayes_mysql.sql"

        cat <<EOF >> /etc/MailScanner/spamassassin.conf
bayes_store_module              Mail::SpamAssassin::BayesStore::MySQL
bayes_sql_dsn                   DBI:mysql:$SqlDb:$SqlHost
bayes_sql_username              $SqlUser
bayes_sql_password              $SqlPwd
bayes_sql_override_username     $SqlUser
EOF
    fi
}

function install-bayesAwl() {
    if [ "$installBayesAwl" != "n" ]; then
        logprint "Configure Bayes-Awl"
        mysql -u "$SqlUser" -p"$SqlPwd" -h "$SqlHost" "$SqlDb" < "$InstallFilesFolder/setup.scripts/spamassassin/awl_mysql.sql"

        cat <<EOF >> /etc/MailScanner/spamassassin.conf
loadplugin                      Mail::SpamAssassin::Plugin::AWL
auto_whitelist_factory          Mail::SpamAssassin::SQLBasedAddrList
user_awl_dsn                    DBI:mysql:$SqlDb:$SqlHost
user_awl_sql_username           $SqlUser
user_awl_sql_password           $SqlPwd
EOF
    fi
}
