#!/usr/bin/env bash
# Diagnóstico sem escrita para decidir se este host pode iniciar o ensaio real
# do Rasgo Synth Asterismus (antes Performance). Não abre o app, não toca áudio e não substitui
# medição de xruns/latência ou escuta.
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
binary_rel="src/app/RasgoSynthAsterismusApp_artefacts/Release/Rasgo Synth Asterismus"
app_path="${RASGO_ASTERISMUS_APP:-$(ls -t "$script_dir"/.local-build/host-*/"$binary_rel" "$script_dir/build-app/$binary_rel" 2>/dev/null | head -n 1 || true)}"
failure=0

ok() { printf 'OK   %s\n' "$*"; }
no() { printf 'FALTA %s\n' "$*"; failure=1; }
info() { printf 'INFO %s\n' "$*"; }

if [[ -x "$app_path" ]]; then
    ok "binário Release encontrado"
else
    no "binário Release ausente: compile RasgoSynthAsterismusApp"
fi

mount_options="$(findmnt -no OPTIONS --target "$script_dir" 2>/dev/null || true)"
if [[ -n "$mount_options" && ",$mount_options," == *",rw,"* ]]; then
    ok "volume de trabalho rw ($mount_options)"
else
    no "volume de trabalho sem confirmação rw (${mount_options:-indisponível})"
fi

available_kib="$(awk '/^MemAvailable:/ { print $2; exit }' /proc/meminfo 2>/dev/null || echo 0)"
if [[ "$available_kib" =~ ^[0-9]+$ ]] && (( available_kib >= 4194304 )); then
    ok "memória disponível: $(( available_kib / 1024 )) MiB"
else
    no "memória disponível abaixo de 4 GiB (${available_kib:-0} KiB)"
fi

if [[ -n "${WAYLAND_DISPLAY-}" ]]; then
    ok "sessão Wayland declarada (${WAYLAND_DISPLAY})"
elif [[ -n "${DISPLAY-}" ]]; then
    if command -v wmctrl >/dev/null 2>&1 && wmctrl -l >/dev/null 2>&1; then
        ok "servidor X acessível (${DISPLAY})"
    elif command -v wmctrl >/dev/null 2>&1; then
        no "DISPLAY declarado mas servidor X não acessível (${DISPLAY})"
    else
        info "DISPLAY declarado (${DISPLAY}); wmctrl ausente, confirmar janela manualmente"
    fi
else
    no "nenhuma sessão gráfica declarada (DISPLAY/WAYLAND_DISPLAY)"
fi

if [[ -d /dev/snd ]]; then
    device_count="$(find /dev/snd -mindepth 1 -maxdepth 1 -printf '%f\n' 2>/dev/null | wc -l)"
    ok "kernel expõe /dev/snd ($device_count nós)"
else
    no "nenhum dispositivo de áudio exposto em /dev/snd"
fi

if command -v pactl >/dev/null 2>&1; then
    if pactl info >/dev/null 2>&1; then
        server_name="$(pactl info 2>/dev/null | awk -F': ' '/^Server Name:/ { print $2; exit }')"
        sink_name="$(pactl info 2>/dev/null | awk -F': ' '/^Default Sink:/ { print $2; exit }')"
        ok "servidor de áudio acessível (${server_name:-não identificado}; saída ${sink_name:-não identificada})"
    else
        info "pactl não acessa um servidor; ALSA/JUCE ainda pode ser escolhido no host"
    fi
else
    info "pactl ausente; confirmar o dispositivo no rodapé do app"
fi

if (( failure != 0 )); then
    printf '\nNÃO INICIAR O ENSAIO REAL AINDA. Corrija os itens FALTA e repita este diagnóstico.\n' >&2
    exit 1
fi

printf '\nPRONTO PARA ABRIR O HOST. Ainda é obrigatório executar e registrar o ensaio de áudio real.\n'
