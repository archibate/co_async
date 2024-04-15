#!/bin/bash
echo "content-type: text/plain"
echo
echo "method: $HTTP_METHOD"
echo "path: $HTTP_PATH"
echo "query: $HTTP_GET_query"
