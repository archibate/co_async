#!/bin/bash
echo "#pragma once" > co_async/co_async.hpp
fd hpp co_async/ | grep -v 'co_async/co_async.hpp' | awk '{print "#include <"$1">";}' >> co_async/co_async.hpp
