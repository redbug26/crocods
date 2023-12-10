#!/bin/bash

docker run -p 8080:80 -v /Users/miguelvanhove/Dropbox/Sources/rg350/crocods/emscripten/web:/usr/share/nginx/html:ro nginx
