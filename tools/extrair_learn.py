#!/usr/bin/env python3
# Extrai o LEARN do app para website/learn_textos.json: as fichas
# (include/rasgo/performance/PerformanceStarLearn.hpp), os nomes de controle
# (performanceStarControlNames + a tabela de tradução do app), os nomes de
# exibição e as famílias. A fonte única continua sendo o código.
# Uso, da raiz do projeto: python3 tools/extrair_learn.py > website/learn_textos.json
import json, pathlib, re, sys

RAIZ = pathlib.Path(__file__).resolve().parent.parent
learn = (RAIZ / "include/rasgo/performance/PerformanceStarLearn.hpp").read_text(encoding="utf-8")
sky = (RAIZ / "include/rasgo/performance/PerformanceSky.hpp").read_text(encoding="utf-8")
app = (RAIZ / "src/app/PerformanceApp.cpp").read_text(encoding="utf-8")
LIT = r'"((?:[^"\\]|\\.)*)"'
un = lambda s: s.replace('\\"', '"').replace("\\\\", "\\")
LANGS = ["pt", "en", "fr", "es"]

def corpo_funcao(texto, assinatura):
    i = texto.index(assinatura)
    return texto[i:texto.index("\n}\n", i)]

# Fichas: depois de cada corpo, 28 literais (7 campos x 4 idiomas).
partes = re.split(r"\{PerformanceModuleBody::(\w+),", learn.split("kPerformanceStarLearn{{", 1)[1].split("}};", 1)[0])[1:]
fichas = []
for corpo, bloco in zip(partes[0::2], partes[1::2]):
    lits = [un(x) for x in re.findall(LIT, bloco)]
    assert len(lits) == 28, (corpo, len(lits))
    campo = lambda n: dict(zip(LANGS, lits[4 * n:4 * n + 4]))
    fichas.append({"corpo": corpo, "rapido": campo(0), "como": campo(1),
                   "controles": [campo(2), campo(3), campo(4)], "ouvir": campo(5), "origem": campo(6)})

# Nomes de controle (português) por estrela e a tradução do app.
nomes = corpo_funcao(sky, "performanceStarControlNames(")
controles = {m.group(1): [un(x) for x in re.findall(LIT, m.group(2))]
             for m in re.finditer(r"case PerformanceModuleBody::(\w+): return \{([^}]*)\};", nomes)}
traducao = {}
for linha in re.finditer(r'\{' + LIT + r', ' + LIT + r', ' + LIT + r', ' + LIT + r'\},', app.split("kNames{{", 1)[1].split("}};", 1)[0]):
    pt, en, fr, es = (un(x) for x in linha.groups())
    traducao[pt] = {"pt": pt, "en": en, "fr": fr, "es": es}

# Nomes de exibição e famílias.
exibicao = {}
bloco = app[app.index("static juce::String moduleDisplayName(PerformanceModuleBody"):]
bloco = bloco[:bloco.index("\n    }\n")]
for m in re.finditer(r"case PerformanceModuleBody::(\w+): return (?:utf8\()?" + LIT, bloco):
    exibicao[m.group(1)] = un(m.group(2))
funcao = corpo_funcao(sky, "performanceStarFunction(")
familia_de, pendentes = {}, []
for m in re.finditer(r"case PerformanceModuleBody::(\w+):( return PerformanceStarFunction::(\w+);)?", funcao):
    pendentes.append(m.group(1))
    if m.group(3):
        for c in pendentes: familia_de[c] = m.group(3)
        pendentes = []
FAMILIAS = {"Tone": {"pt": "TOM", "en": "TONE", "fr": "TON", "es": "TONO"},
            "Texture": {"pt": "TEXTURA", "en": "TEXTURE", "fr": "TEXTURE", "es": "TEXTURA"},
            "Voice": {"pt": "VOZ", "en": "VOICE", "fr": "VOIX", "es": "VOZ"},
            "Pulse": {"pt": "PULSO", "en": "PULSE", "fr": "PULSATION", "es": "PULSO"},
            "Control": {"pt": "CONTROLE", "en": "CONTROL", "fr": "CONTRÔLE", "es": "CONTROL"}}

for f in fichas:
    c = f["corpo"]
    f["nome"] = exibicao[c]
    f["familia"] = familia_de.get(c, "Tone")
    f["nomes_controles"] = [traducao.get(n, {l: n for l in LANGS}) for n in controles[c]]
json.dump({"familias": FAMILIAS, "fichas": fichas}, sys.stdout, ensure_ascii=False, indent=1)
