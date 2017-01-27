#!/usr/bin/awk -f

/^start/ { name = $2 }
/^end/ { print name, $3 / 1e6 }
