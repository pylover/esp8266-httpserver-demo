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

assert-eq "UNS resolve" ${ADDR} `uns resolve ${NAME}`
assert-eq "Small binary response" \
  `md5 < assets/favicon-16x16.png` \
  `uns h get home.nodemcu/favicon.ico | md5`

assert-eq "URL encoded form" \
  "foo=bar baz=qux " \
  "$(uns h post home.nodemcu/urlencoded foo=bar baz=qux)"


tmp=$(tempfile)
echo -e "bar" > ${tmp}
assert-eq "Small multipart form" \
  "foo=bar " \
  "$(uns h upload home.nodemcu/multipart @foo=${tmp})"
rm ${tmp}

# TODO:
# - MULTIPART BIG
# - Stream
