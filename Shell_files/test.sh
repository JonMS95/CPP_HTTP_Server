#!/bin/bash

DEFAULT_USER_PORT=55555
DEFAULT_CONN_NUM=2

CONFIG_FILE="config.xml"

PATH_TO_THIS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PATH_TO_LIB_ROOT="$(dirname ${PATH_TO_THIS})"
PATH_TO_TEST_DEPS="$( xmlstarlet sel -t -v "config/Tests/Dependencies/@Dest" ${CONFIG_FILE})"
PATH_TO_TEST_DEP_DYN_LIBS=${PATH_TO_LIB_ROOT}/${PATH_TO_TEST_DEPS}/Dynamic_libraries

export LD_LIBRARY_PATH=${PATH_TO_TEST_DEP_DYN_LIBS}

echo
echo "*******************************"
echo "Testing 'main' executable file."
echo "*******************************"
# # HTTP
./Tests/Executable_files/main -r ${DEFAULT_USER_PORT} -t 1800 -y 1800 -p -m 10 -e "~/Desktop/scripts/C++/CPP_HTTP_Server/Tests/Dependency_files/Data/HTML_tutorial"
# # HTTPS
# ./Tests/Executable_files/main -r ${DEFAULT_USER_PORT} -t 1800 -p -m 10 -s -c ~/Desktop/scripts/certificate_test/certificate.crt -k ~/Desktop/scripts/certificate_test/private.key
