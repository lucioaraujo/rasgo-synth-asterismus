#!/usr/bin/env bash
# Nome antigo (o Asterismus se chamava Rasgo Synth Performance até 27/10/2026):
# atalho para run_asterismus.sh, mantido para quem já usa este comando.
exec "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/run_asterismus.sh" "$@"
