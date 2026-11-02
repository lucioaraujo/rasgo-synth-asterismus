#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# O binário: RASGO_ASTERISMUS_APP, se definido; senão o build mais recente
# entre .local-build/host-* e build-app (as pastas build-* anteriores a
# 2026-10-11 apontam para o caminho antigo do projeto).
binary_rel="src/app/RasgoSynthAsterismusApp_artefacts/Release/Rasgo Synth Asterismus"
app_path="${RASGO_ASTERISMUS_APP:-}"
if [[ -z "$app_path" ]]; then
    app_path="$(ls -t "$script_dir"/.local-build/host-*/"$binary_rel" "$script_dir/build-app/$binary_rel" 2>/dev/null | head -n 1 || true)"
fi

if [[ ! -x "$app_path" ]]; then
    printf 'Binário não encontrado ou não executável: %s\n' "$app_path" >&2
    printf 'Compile primeiro (ver CONTINUIDADE.md): cmake --build .local-build/host-AAAAMMDD --target RasgoSynthAsterismusApp\n' >&2
    exit 1
fi

# Gates de entrada da governança: não iniciar um processo que possa gravar
# estado/recordings quando o volume está somente leitura ou há pressão severa
# de memória na máquina.
mount_options="$(findmnt -no OPTIONS --target "$script_dir" 2>/dev/null || true)"
if [[ -n "$mount_options" && ",$mount_options," != *",rw,"* ]]; then
    printf 'Volume do Performance não está montado para escrita: %s\n' "$mount_options" >&2
    exit 1
fi

available_kib="$(awk '/^MemAvailable:/ { print $2; exit }' /proc/meminfo 2>/dev/null || echo 0)"
if [[ "$available_kib" =~ ^[0-9]+$ ]] && (( available_kib < 4194304 )); then
    printf 'Memória disponível abaixo de 4 GiB; sessão não iniciada por segurança: %s KiB\n' "$available_kib" >&2
    exit 1
fi

if [[ -z "${DISPLAY-}" && -z "${WAYLAND_DISPLAY-}" ]]; then
    printf 'Nenhuma sessão gráfica detectada (DISPLAY/WAYLAND_DISPLAY); abra o host em uma sessão gráfica.\n' >&2
    exit 1
fi

if [[ -n "${DISPLAY-}" ]] && command -v wmctrl >/dev/null 2>&1 \
    && ! wmctrl -l >/dev/null 2>&1; then
    printf 'DISPLAY configurado, mas o servidor gráfico não aceita conexão; sessão não iniciada.\n' >&2
    exit 1
fi

exec "$app_path" "$@"
