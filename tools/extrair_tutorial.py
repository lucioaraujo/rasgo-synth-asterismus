#!/usr/bin/env python3
# Extrai o tutorial do app (src/app/PerformanceApp.cpp, 4 idiomas) para website/tutorial_textos.json.
# Uso, da raiz do projeto: python3 tools/extrair_tutorial.py src/app/PerformanceApp.cpp > website/tutorial_textos.json
import re, json, sys
src = open(sys.argv[1], encoding='utf-8').read()
# O texto mora em tutorialSource(): um literal cru R"RASGO(...)RASGO" por idioma,
# na ordem English, French, Spanish, Portuguese (o default).
corpo = src[src.index('const char* tutorialSource(SupportLanguage language)'):]
corpo = corpo[:corpo.index('\n}\n')]
en, fr, es, pt = re.findall(r'R"RASGO\((.*?)\)RASGO"', corpo, re.S)
json.dump({'pt': pt, 'en': en, 'fr': fr, 'es': es}, sys.stdout, ensure_ascii=False)
