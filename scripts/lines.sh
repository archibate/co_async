#!/bin/bash
fd [ch]pp co_async -x wc -l {} | awk '{print $1}' | paste -d+ -s | bc
