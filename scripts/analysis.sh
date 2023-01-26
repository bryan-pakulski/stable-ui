#!/bin/bash
# cppcheck must be installed
cd ../
mkdir -p .cppcheck
mkdir -p .cppcheck/analysis
cppcheck --cppcheck-build-dir=../.cppcheck/analysis --project=../build/compile_commands.json --suppress=*:*ThirdParty\* --suppress=*:*build\* --enable=all --xml 2> .cppcheck/report.xml
cppcheck-htmlreport  --file=.cppcheck/report.xml --title=stable-ui --report-dir=.cppcheck/html_report --source-dir=.