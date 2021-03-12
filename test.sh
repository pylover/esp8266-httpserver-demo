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

assert-eq "Streaming" \
  "$(printf 'Foo\r\nBar\r\nBaz\r\nQux\r\n')" \
  "$(uns http DOWNLOAD ${NAME})"

# Multipart, streaming
tmp1=$(tempfile)
tmp2=$(tempfile)
dd if=/dev/urandom bs=1 count=100000 of=${tmp1} >> /dev/null 2>&1

uns http -b download ${NAME}/multipartstreams > ${tmp2} & \
  uns http upload ${NAME}/multipartstreams @foo=${tmp1} > /dev/null
assert-eq "Multipart streaming" "$(cat $tmp1 | md5)" "$(cat $tmp2 | md5)"
rm ${tmp1}
rm ${tmp2}


# Multipart, single field
tmp=$(tempfile)
echo -n "bar" > ${tmp}
assert-eq "Small multipart form, single field" \
  "foo=bar " \
  "$(uns http echo ${NAME}/multipartforms @foo=${tmp})"
rm ${tmp}


# Multipart, multiple fields
tmpbar=$(tempfile)
tmpbaz=$(tempfile)
echo -n "bar" > ${tmpbar}
echo -n "baz" > ${tmpbaz}
assert-eq "Small multipart form, multipart fields" \
  "foo=bar qux=baz " \
  "$(uns http echo ${NAME}/multipartforms @foo=${tmpbar} @qux=${tmpbaz})"
rm ${tmpbar} ${tmpbaz}

echo -e "$LK"
make map6user2 >> /dev/null
echo -e "$C"
binsize user/.output/eagle/debug/lib/libuser.a
binsize httpd/.output/eagle/debug/lib/libhttpd.a
binsize uns/.output/eagle/debug/lib/libuns.a 
echo '--------------------------'
binsize bin/upgrade/user2.4096.new.6.bin
