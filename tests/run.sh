#!/bin/sh
# Runs every test program and compares what it prints with the expected output.
# tests/programs.py writes the small programs into tests/build/.
# tests/samples/ (not in the repo) can hold more: name.ppm, name.out, name.in.
cd "$(dirname "$0")/.." || exit 1
python3 tests/programs.py tests/build || exit 1

pass=0
fail=0
for dir in tests/build tests/samples; do
    [ -d "$dir" ] || continue
    for ppm in "$dir"/*.ppm; do
        name=${ppm%.ppm}
        [ -f "$name.out" ] || continue
        input=/dev/null
        [ -f "$name.in" ] && input=$name.in
        if timeout 10 ./piet "$ppm" < "$input" > "$name.got" 2>/dev/null && cmp -s "$name.got" "$name.out"; then
            pass=$((pass + 1))
            echo "ok    $ppm"
        else
            fail=$((fail + 1))
            echo "FAIL  $ppm (got $name.got, expected $name.out)"
        fi
    done
done
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
