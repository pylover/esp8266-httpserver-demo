#! /usr/bin/env bash

source testing.sh

ADDR=192.168.8.162
NAME="home.nodemcu"

assert-eq "UNS resolve" ${ADDR} `uns resolve --short ${NAME}`
assert-eq "Simple GET" \
  'Index' \
  `uns http get ${NAME}`

assert-eq "HTTP Headers" "Server: esp8266-HTTPd/2.0.0
Content-Length: 0
Connection: keep-alive
Host: 192.168.8.162
User-Agent: python-requests/2.22.0
Accept-Encoding: gzip, deflate
Accept: */* -H'foo: bar'" \
  "`uns http --include-headers echo ${NAME}/headers` -H'foo: bar'"


assert-eq "Small binary response" \
  `md5 < assets/favicon-16x16.png` \
  `uns http get ${NAME}/favicon.ico | md5`


assert-eq "Querystring" \
  "foo=bar baz=qux " \
  "$(uns http echo "${NAME}/queries?foo=bar&baz=qux")"


exit 0

assert-eq "URL encoded form" \
  "foo=bar baz=qux " \
  "$(uns http post ${NAME}/urlencoded foo=bar baz=qux)"


# Multipart, single field
tmp=$(tempfile)
echo -e "bar" > ${tmp}
assert-eq "Small multipart form, single field" \
  "foo=bar " \
  "$(uns http upload ${NAME}/multipart @foo=${tmp})"
rm ${tmp}


# Multipart, multiple fields
tmpbar=$(tempfile)
tmpbaz=$(tempfile)
echo -e "bar" > ${tmpbar}
echo -e "baz" > ${tmpbaz}
assert-eq "Small multipart form, multipart fields" \
  "foo=bar qux=baz " \
  "$(uns http upload ${NAME}/multipart @foo=${tmpbar} @qux=${tmpbaz})"
rm ${tmpbar} ${tmpbaz}


# TODO:
# - MULTIPART BIG
# - Stream
