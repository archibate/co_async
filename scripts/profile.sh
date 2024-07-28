which tee > /dev/null || exit
which perl > /dev/null || exit
which perf > /dev/null || exit
paranoid=$(cat /proc/sys/kernel/perf_event_paranoid)
[[ "x$paranoid" != "x-1" ]] && (echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid > /dev/null)
dir=$(mktemp -d)
cache=$HOME/.cache/profile
test -d $cache || mkdir -p $cache
_process() {
    trap - SIGINT
    echo -- Perf scripting...
    perf script > $dir/out.perf
    echo -- Getting stackcollapse-perf.pl...
    test -f $cache/stackcollapse-perf.pl || curl -sSL https://mirror.ghproxy.com/https://raw.githubusercontent.com/brendangregg/FlameGraph/master/stackcollapse-perf.pl -o $cache/stackcollapse-perf.pl
    echo -- Getting flamegraph.pl...
    test -f $cache/flamegraph.pl || curl -sSL https://mirror.ghproxy.com/https://raw.githubusercontent.com/brendangregg/FlameGraph/master/flamegraph.pl -o $cache/flamegraph.pl
    echo -- Folding perf result...
    perl $cache/stackcollapse-perf.pl $dir/out.perf > $dir/out.folded
    echo -- Generating SVG file...
    perl $cache/flamegraph.pl $dir/out.folded > $dir/out.svg
    cp $dir/out.svg $cache/out.svg
    echo -- SVG file: $cache/out.svg
    firefox $cache/out.svg
    [[ "x$paranoid" != "x-1" ]] && (echo $paranoid | sudo tee /proc/sys/kernel/perf_event_paranoid > /dev/null)
    exit
}
trap _process SIGINT
echo -- Perf recording...
perf record -F 99 -a --call-graph dwarf -- "$@" || echo -- Exit code: $?
_process
