#!/bin/bash

cat html/header.html

echo '<p>'
sed -e 's/^\(Q:.*\)$/<\/p>\n\<p>\n<b>\1<\/b><p>/g'<  ../../FAQ
echo '</p>'

cat html/footer.html

