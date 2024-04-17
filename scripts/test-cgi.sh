#!/bin/bash
echo "content-type: text/plain"
echo
echo "method: $HTTP_METHOD"
echo "path: $HTTP_PATH"
echo "user-agent: $HTTP_HEADER_user_agent"
echo "query: $HTTP_GET_query"
