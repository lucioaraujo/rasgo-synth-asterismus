#!/usr/bin/env python3
# Extrai o tutorial do app (src/app/PerformanceApp.cpp, 4 idiomas) para website/tutorial_textos.json.
# Uso, da raiz do projeto: python3 tools/extrair_tutorial.py src/app/PerformanceApp.cpp > website/tutorial_textos.json
import re, json, sys
src = open(sys.argv[1], encoding='utf-8').read()
def literals(chunk):
    out = ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', chunk))
    return out.encode('utf-8').decode('unicode_escape').encode('latin-1').decode('utf-8')
def block(start_marker, end_marker):
    i = src.index(start_marker); j = src.index(end_marker, i) + len(end_marker)
    return literals(src[i:j])
pt = block('"1. COMEÇAR · SEED E TERRITÓRIO', 'deve ser revisto, redesenhado ou retirado.")')
en = block('"1. START · SEED AND TERRITORY', 'clear musical consequence.")')
fr = block('"1. COMMENCER · SEED ET TERRITOIRE', 'conséquence musicale claire.")')
es = block('"1. COMENZAR · SEED Y TERRITORIO', 'consecuencia musical clara.")')
json.dump({'pt': pt, 'en': en, 'fr': fr, 'es': es}, sys.stdout, ensure_ascii=False)
