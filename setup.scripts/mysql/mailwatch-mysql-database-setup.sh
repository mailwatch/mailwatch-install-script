#!/bin/bash
# Bash Menu Script Example
set +o history

InstallFilesFolder=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd "./../.." && pwd )

source "$InstallFilesFolder/version.inc"

TmpDir="/tmp/mailwatchinstall"
MailWatchTmpDir="$TmpDir/mailwatch/MailWatch-${MailWatchVersion/v/}"

source "$InstallFilesFolder/setup.scripts/mailwatch/mailwatch.inc"
source "$InstallFilesFolder/setup.scripts/mysql/mailwatch-mysql.inc"

prepare-mysql-install
configure-sql-credentials

download-mailwatch
setup-database

set -o history
