#! /usr/bin/env bash

ADDR=192.168.8.162
NAME="home.nodemcu"

# ANSI COLORS start
R="\033[31m"
G="\033[32m"
Y="\033[33m"
C="\033[0m"
function nor () {
  echo -e "${C}$@"
}
function red () {
  echo -e "${R}$@${C}"
}
function grn () {
  echo -e "${G}$@${C}"
}
# ANSI COLORS end


function assert-eq () {
  local exp="${2}"
  local act="${3}"

  if [[ ${exp} == ${act} ]]; then 
    grn "Passed:${C} ${1}"
  else
    red "Failed:${C} ${1} ${Y}(Values are not equal)"
    nor "  exptected: -> ${exp}"
    nor "  actual:    -> ${act}"
    nor
  fi
}


function md5 () {
  md5sum -b | cut -c -32 
}

assert-eq "UNS resolve" ${ADDR} `uns resolve --force ${NAME}`
assert-eq "Small binary response" \
  `md5 < assets/favicon-16x16.png` \
  `uns http get home.nodemcu/favicon.ico | md5`

assert-eq "URL encoded form" \
  "foo=bar baz=qux " \
  "$(uns http post home.nodemcu/urlencoded foo=bar baz=qux)"


# Multipart, single field
tmp=$(tempfile)
echo -e "bar" > ${tmp}
assert-eq "Small multipart form, single field" \
  "foo=bar " \
  "$(uns http upload home.nodemcu/multipart @foo=${tmp})"
rm ${tmp}


# Multipart, multiple fields
tmpbar=$(tempfile)
tmpbaz=$(tempfile)
echo -e "bar" > ${tmpbar}
echo -e "baz" > ${tmpbaz}
assert-eq "Small multipart form, multipart fields" \
  "foo=bar qux=baz " \
  "$(uns http upload home.nodemcu/multipart @foo=${tmpbar} @qux=${tmpbaz})"
rm ${tmpbar} ${tmpbaz}


# TODO:
# - MULTIPART BIG
# - Stream
