#! /usr/bin/env bash

source testing.sh

ADDR=192.168.8.162
NAME="home.nodemcu"

assert-eq "UNS resolve" ${ADDR} `uns resolve --short ${NAME}`
assert-eq "Simple GET" \
  'Index' \
  `uns http get ${NAME}`

assert-eq "HTTP Headers" "\
200 OK HTTP/1.1
Server: esp8266-HTTPd/2.0.0
Connection: keep-alive
Content-Length: 0
Host: 192.168.8.162
User-Agent: python-requests/2.22.0
Accept-Encoding: gzip, deflate
Accept: */* -H'foo: bar'" \
  "`uns http --include-headers echo ${NAME}/headers` -H'foo: bar'"


assert-eq "Small binary response" \
  `md5 < assets/favicon-16x16.png` \
  `uns http -b get ${NAME}/favicon.ico | md5`


assert-eq "Querystring" \
  "foo=bar baz=qux " \
  "$(uns http echo "${NAME}/queries?foo=bar&baz=qux")"


assert-eq "URL encoded form" \
  "foo=bar baz=qux " \
  "$(uns http ECHO ${NAME}/urlencodedforms foo=bar baz=qux)"


# Multipart, streaming
tmp1=$(tempfile)
tmp2=$(tempfile)
dd if=/dev/urandom bs=1 count=1000000 of=${tmp1} >> /dev/null 2>&1

uns http -b download ${NAME}/multipartforms > ${tmp2} & \
  assert-eq "Multipart streaming" \
  "$(printf 'Ok\r\n')" \
  "$(uns http upload ${NAME}/multipartforms @foo=${tmp1})"

assert-eq "HTTP Streaming" \
  "$(cat $tmp1 | md5)" \
  "$(cat $tmp2 | md5)"

#rm ${tmp1}
#rm ${tmp2}

exit 0

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
