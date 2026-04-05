#!/usr/bin/env bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KCC="$SCRIPT_DIR/../krypton/kcc.sh"

echo "[1/2] Krypton to C..."
bash "$KCC" run.k > ktop_tmp.c

echo "[2/2] C to exe..."
gcc ktop_tmp.c -o ktop -lm -w
rm -f ktop_tmp.c

echo "Done! Run: ./ktop [--sort cpu|mem|pid|name] [--refresh ms] [--no-color]"
