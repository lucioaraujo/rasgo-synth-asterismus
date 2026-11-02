#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
unexpected=()

while IFS= read -r path; do
    unexpected+=("$path")
done < <(find "$project_dir" \
    -path "$project_dir/.git" -prune -o \
    -path "$project_dir/build" -prune -o \
    -path "$project_dir/build-*" -prune -o \
    -path "$project_dir/.local-build" -prune -o \
    -path "$project_dir/third_party" -prune -o \
    -type f \( -name '*.tmp' -o -name '.*.tmp' -o -name '*.part' -o -name '*.lock' \) -print)

if ((${#unexpected[@]} > 0)); then
    printf 'Artefatos temporários inesperados no projeto:\n' >&2
    printf '  %s\n' "${unexpected[@]}" >&2
    exit 1
fi

printf 'OK: nenhum temporário inesperado no projeto. Builds e recordings locais permanecem fora da verificação.\n'
