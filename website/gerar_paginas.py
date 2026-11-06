#!/usr/bin/env python3
"""Gera as quatro páginas do site do Rasgo Synth Asterismus (index.html em
português, a canônica, e en/fr/es.html) a partir de um modelo único e dos
textos de cada idioma, para as quatro nunca divergirem na estrutura. Depois,
`python3 seo.py` injeta os metadados (o bloco SEO não é escrito aqui).

    python3 gerar_paginas.py && python3 seo.py

Editar os TEXTOS aqui, não nos .html (eles são derivados).

Estrutura copiada do gerador do site do Trioio (lucioaraujo/trioio, commit
cfbd744, website/gerar_paginas.py; mesmo autor e licença), com o modelo e os
textos do Asterismus. Enquanto PUBLICADO = False, as páginas levam noindex,
o selo "em preparação" e nenhum link de download.
"""

import html
import pathlib
import sys

AQUI = pathlib.Path(__file__).parent
PORTAL = "https://rasgoinstruments.arquiviagem.net/"
REPO = "https://github.com/lucioaraujo/rasgo-synth-asterismus"
BLOB = REPO + "/blob/main"
# a versão e os pacotes do release (a CI: .github/workflows/package.yml);
# os nomes são os do CPack (CPACK_PACKAGE_NAME) — conferir na primeira release
VERSAO = "0.3.0"
PUBLICADO = True
# LEARN (7/11/2026): as fichas das estrelas descrevem a 0.3 (36 estrelas, o
# kit e o groove da DRUMS…). Enquanto o site for o da 0.2.0, as páginas não
# são escritas em website/ nem entram no menu; `--previa-learn DIR` as gera
# para leitura local. Na release 0.3: LEARN_PUBLICO = True.
LEARN_PUBLICO = True
RELEASE = REPO + "/releases/tag/v" + VERSAO
DL = REPO + "/releases/download/v" + VERSAO + "/"
PACOTES = [
    ("Windows", [("rasgo-synth-asterismus-%s-win64.exe" % VERSAO, "exe")]),
    ("macOS", [("rasgo-synth-asterismus-%s-Darwin.dmg" % VERSAO, "dmg")]),
    ("Linux", [("rasgo-synth-asterismus-%s-Linux.deb" % VERSAO, "deb"),
               ("rasgo-synth-asterismus-%s-Linux.tar.gz" % VERSAO, "tar.gz")]),
]

PAGINAS = [("index.html", "pt-BR", "PT"), ("en.html", "en", "EN"), ("fr.html", "fr", "FR"), ("es.html", "es", "ES")]

T = {
    "pt-BR": {
        "title": "Rasgo Synth Asterismus — um céu de estrelas que toca",
        "desc": "Rasgo Synth Asterismus: instrumento ao vivo em que cada módulo de síntese é uma estrela. As estrelas se relacionam, se movem por física, contam os seus ciclos e tocam; uma seed propõe o céu, o músico o toca e grava a peça.",
        "strip": "RASGO — família de instrumentos e pesquisa sonora",
        "skip": "Pular para o conteúdo",
        "langnav": "Idioma",
        "pill": "Versão " + VERSAO + " — em preparação" if not PUBLICADO else "Versão " + VERSAO + " — para Windows, macOS e Linux",
        "lead": "O Asterismus é um instrumento ao vivo em que cada módulo de síntese é uma estrela. As estrelas se relacionam, se movem por física, contam os seus próprios ciclos e tocam. Uma seed propõe o céu; o músico o toca, troca estrelas, desenha relações e grava a peça.",
        "s_shot": "A interface",
        "shot_alt": "O Asterismus tocando: o Eixo de Forças à esquerda, a Constelação no centro com as estrelas movidas pela física e os pares em foco, o mixer das oito estrelas à direita, a forma e o tempo que respira no cabeçalho.",
        "shot_cap": "À esquerda, o Eixo de Forças e os macros; no centro, a Constelação — as estrelas, os cabos e os pares em foco da trama; à direita, o mixer com um stem por estrela. No cabeçalho, a forma, o tempo que respira e a gravação.",
        "s_sky": "O céu",
        "sky": [
            ("Estrelas e seed", "36 estrelas de tom, textura, voz e pulso, quatro delas de controle, que não soam mas filtram como as outras tocam. A seed escolhe oito entre as 28 do seu sorteio, um centro tonal e um modo, uma forma e os ciclos: cada seed é outra peça; as outras oito entram pela BIBLIOTECA."),
            ("A trama", "A cada compasso o céu escolhe pares em foco e o que cada par faz: resposta, contágio, sombra, hoquetus, pedal, atrito, cadência. A peça percorre abertura, desenvolvimento, contraste e retorno, e termina numa coda."),
            ("O tempo que respira", "Andamento elástico, swing que vem de uma órbita, compassos agrupados em 3+3+2, uma estrela que deriva contra o pulso, eclipses trazidos por luas calculadas."),
            ("Movimento físico", "Gravidade, molas, pêndulos, bilhar, magnetismo, efeito e espuma. Os choques fortes soam, e cada estrela soa no estéreo onde está na Constelação."),
            ("Cortes e fita", "Em algumas partes o céu corta — parêntese, corte de montagem, apócrifo, decimação — e se gasta: fita ou disco, tape stop, spin back, varispeed, conforme a seed."),
        ],
        "s_play": "Tocar",
        "play": [
            ("A Constelação", "Clique numa estrela para pô-la em foco e arraste-a para mudar o seu lugar; clique num cabo para escolher o fenômeno e arraste-o para a intensidade."),
            ("Gestos e cenas", "ABRIR, RASGAR, REPARAR, STUTTER, DESLOCAR, HOLD e PANIC SAFE agem na estrela em foco. Quatro cenas e o PERCURSO seguem a forma."),
            ("O mixer e a biblioteca", "Um stem por estrela, com presença, nível, mute e solo, um filtro e envios de eco e sala; a VARIAÇÃO deixa o céu acender e apagar estrelas e a POLIFONIA diz quantas soam juntas. A BIBLIOTECA troca ou fixa a estrela de um corpo e abre o LEARN, a ficha de cada estrela."),
            ("O Eixo de Forças", "Dois pontos movem oito macros — autonomia, intervenção, densidade, tensão, memória, risco, espaço, profundidade. Com a trama, um anel mostra a energia e a tensão do céu."),
        ],
        "s_rec": "Gravar e guardar",
        "rec": [
            "REC grava a tomada: um WAV estéreo de 24 bits, um WAV por corpo e um manifesto JSON, numa pasta por tomada;",
            "FAIXA · DURAÇÃO grava uma peça de 1, 2, 3 ou 5 minutos: a forma do céu se encaixa na duração e a coda chega no fim;",
            "▶ OUVIR abre a última tomada no player do sistema;",
            "a MEMÓRIA guarda oito lugares com nome; SALVAR SESSÃO e ABRIR SESSÃO gravam e leem arquivos .asterismus — só parâmetros, nunca áudio.",
        ],
        "output": "Saída no padrão comum RASGO: ganho fixo, limitador com true peak 4×, teto de −1,41 dBTP, medição BS.1770 e dither na gravação. Se o sistema de som travar ou o dispositivo sumir, o app o reabre e a peça continua de onde estava.",
        "tutorial": "O TUTORIAL do app explica tudo passo a passo, e o LEARN apresenta cada estrela — em português, inglês, francês ou espanhol.",
        "s_dl": "Baixar",
        "dl_pending": "A primeira versão (0.2.0, prévia) está em preparação: os pacotes para Windows, macOS e Linux serão montados pela integração contínua. Até lá, o instrumento pode ser compilado a partir do código.",
        "dl_intro": "A versão " + VERSAO + " traz estrelas novas, os cortes e a fita do céu, filtros e envios por estrela, o groove da DRUMS e o LEARN (o que mudou está no CHANGELOG). O instrumento continua aberto, em desenvolvimento. Escolha o pacote do seu sistema; os passos estão no INSTALL.",
        "dl_os": {"Windows": "Windows 10 ou 11, 64 bits — instalador (.exe)", "macOS": "macOS 10.15 ou mais novo, Intel e Apple Silicon", "Linux": ".deb para Ubuntu e Debian; .tar.gz para as outras distribuições"},
        "dl_warn": "Na primeira abertura, o Windows e o macOS mostram um aviso de segurança: é esperado, porque o app não tem a assinatura paga das duas empresas. O guia de instalação diz onde clicar.",
        "dl_install": "Guia de instalação",
        "dl_all": "Todos os pacotes e as notas da versão",
        "dl_build": "Compilar a partir do código",
        "s_prov": "Proveniência",
        "prov": "O DSP das estrelas vem do Rasgo Synth Studio, numa cópia fixada com commit de origem e checksums; o estágio de saída e o medidor foram adaptados do Antitotem e do Navalha 2. Cada tomada registra no seu manifesto a seed, as estrelas e as relações de que veio.",
        "s_credits": "Créditos",
        "credits": "Lúcio de Araújo — concepção, direção artística, música e código. Licença GNU GPLv3 ou posterior; JUCE pela via AGPLv3. Créditos e avisos completos:",
        "s_state": "Estado",
        "state": "Em desenvolvimento, aberto. Compilado e testado no Linux (39 testes) e tocado pelo autor em hardware real; os pacotes de Windows, macOS e Linux são montados pela integração contínua.",
        "contact": "Contato",
        "footer_credits": "Créditos",
        "footer_license": "Licença GPLv3+",
        "dialog_title": "CONTATO",
        "dialog_copy_hint": "Copie o endereço de e-mail:",
        "dialog_copy": "Copiar e-mail",
        "dialog_close": "Fechar janela de contato",
        "copied": "E-mail copiado.",
        "fallback": "Selecione e copie o endereço acima.",
    },
    "en": {
        "title": "Rasgo Synth Asterismus — a sky of stars that plays",
        "desc": "Rasgo Synth Asterismus: a live instrument in which every synthesis module is a star. The stars relate, move by physics, count their own cycles and play; a seed proposes the sky, the musician plays it and records the piece.",
        "strip": "RASGO — a family of instruments and sound research",
        "skip": "Skip to content",
        "langnav": "Language",
        "pill": "Version " + VERSAO + " — in preparation" if not PUBLICADO else "Version " + VERSAO + " — for Windows, macOS and Linux",
        "lead": "Asterismus is a live instrument in which every synthesis module is a star. The stars relate to each other, move by physics, count their own cycles and play. A seed proposes the sky; the musician plays it, swaps stars, draws relations and records the piece.",
        "s_shot": "The interface",
        "shot_alt": "Asterismus playing: the Axis of Forces on the left, the Constellation in the centre with the stars moved by physics and the pairs in focus, the mixer of the eight stars on the right, the form and the breathing tempo in the header.",
        "shot_cap": "On the left, the Axis of Forces and the macros; in the centre, the Constellation — the stars, the cables and the web's pairs in focus; on the right, the mixer with one stem per star. In the header, the form, the breathing tempo and the recording.",
        "s_sky": "The sky",
        "sky": [
            ("Stars and seed", "36 tone, texture, voice and pulse stars, four of them control stars that do not sound but filter how the others play. The seed chooses eight among the 28 of its draw, a key and a mode, a form and the cycles: every seed is another piece; the other eight come in through the LIBRARY."),
            ("The web", "Each bar the sky chooses pairs in focus and what each pair does: answer, contagion, shadow, hocket, pedal, friction, cadence. The piece goes through opening, development, contrast and return, and ends with a coda."),
            ("Time that breathes", "An elastic tempo, a swing that comes from an orbit, bars grouped as 3+3+2, a star that drifts against the pulse, eclipses brought by computed moons."),
            ("Physical motion", "Gravity, springs, pendulums, billiards, magnetism, spin and foam. Strong collisions sound, and each star sounds in the stereo field where it is in the Constellation."),
            ("Cuts and tape", "In some parts the sky cuts — parenthesis, montage cut, apocryphal, decimation — and wears: tape or vinyl, tape stop, spin back, varispeed, as the seed chooses."),
        ],
        "s_play": "Playing",
        "play": [
            ("The Constellation", "Click a star to put it in focus and drag it to move its place; click a cable to choose its phenomenon and drag it to set its strength."),
            ("Gestures and scenes", "OPEN, TEAR, REPAIR, STUTTER, DISPLACE, HOLD and PANIC SAFE act on the star in focus. Four scenes and the PATH follow the form."),
            ("The mixer and the library", "One stem per star, with presence, level, mute and solo, a filter and echo and hall sends; VARIATION lets the sky light and dim stars and POLYPHONY sets how many sound together. The LIBRARY swaps or pins a body's star and opens LEARN, the fiche of each star."),
            ("The Axis of Forces", "Two points move eight macros — autonomy, intervention, density, tension, memory, risk, space, depth. With the web on, a ring shows the energy and the tension of the sky."),
        ],
        "s_rec": "Recording and keeping",
        "rec": [
            "REC records the take: a 24-bit stereo WAV, one WAV per body and a JSON manifest, in one folder per take;",
            "TRACK · DURATION records a piece of 1, 2, 3 or 5 minutes: the sky's form fits the length and the coda arrives at the end;",
            "▶ LISTEN opens the last take in the system player;",
            "MEMORY keeps eight named places; SAVE SESSION and OPEN SESSION write and read .asterismus files — parameters only, never audio.",
        ],
        "output": "Output in the common RASGO standard: fixed gain, limiter with 4× true-peak detection, −1.41 dBTP ceiling, BS.1770 metering and dither on recording. If the sound system stalls or the device disappears, the app reopens it and the piece goes on from where it was.",
        "tutorial": "The app's GUIDE explains everything step by step, and LEARN presents each star — in English, Portuguese, French or Spanish.",
        "s_dl": "Download",
        "dl_pending": "The first version (0.2.0, preview) is in preparation: the Windows, macOS and Linux packages will be built by continuous integration. Until then, the instrument can be built from source.",
        "dl_intro": "Version " + VERSAO + " brings new stars, the sky's cuts and tape, a filter and sends on every star, the DRUMS groove and LEARN (what changed is in the CHANGELOG). The instrument remains open, in development. Choose the package for your system; the steps are in INSTALL.",
        "dl_os": {"Windows": "Windows 10 or 11, 64-bit — installer (.exe)", "macOS": "macOS 10.15 or later, Intel and Apple Silicon", "Linux": ".deb for Ubuntu and Debian; .tar.gz for other distributions"},
        "dl_warn": "On first launch, Windows and macOS show a security warning: it is expected, because the app does not carry the two companies' paid signature. The installation guide says where to click.",
        "dl_install": "Installation guide",
        "dl_all": "All packages and release notes",
        "dl_build": "Build from source",
        "s_prov": "Provenance",
        "prov": "The DSP of the stars comes from the Rasgo Synth Studio, as a pinned copy with source commit and checksums; the output stage and the meter were adapted from Antitotem and Navalha 2. Every take records in its manifest the seed, the stars and the relations it came from.",
        "s_credits": "Credits",
        "credits": "Lúcio de Araújo — concept, artistic direction, music and code. Licence GNU GPLv3 or later; JUCE under its AGPLv3 option. Full credits and notices:",
        "s_state": "Status",
        "state": "In development, open. Built and tested on Linux (39 tests) and played by the author on real hardware; the Windows, macOS and Linux packages are built by continuous integration.",
        "contact": "Contact",
        "footer_credits": "Credits",
        "footer_license": "GPLv3+ licence",
        "dialog_title": "CONTACT",
        "dialog_copy_hint": "Copy the email address:",
        "dialog_copy": "Copy email",
        "dialog_close": "Close contact window",
        "copied": "Email copied.",
        "fallback": "Select and copy the address above.",
    },
    "fr": {
        "title": "Rasgo Synth Asterismus — un ciel d'étoiles qui joue",
        "desc": "Rasgo Synth Asterismus : un instrument de scène dans lequel chaque module de synthèse est une étoile. Les étoiles se relient, bougent selon la physique, comptent leurs cycles et jouent ; une seed propose le ciel, le musicien le joue et enregistre la pièce.",
        "strip": "RASGO — famille d'instruments et de recherche sonore",
        "skip": "Aller au contenu",
        "langnav": "Langue",
        "pill": "Version " + VERSAO + " — en préparation" if not PUBLICADO else "Version " + VERSAO + " — pour Windows, macOS et Linux",
        "lead": "Asterismus est un instrument de scène dans lequel chaque module de synthèse est une étoile. Les étoiles se relient, bougent selon la physique, comptent leurs propres cycles et jouent. Une seed propose le ciel ; le musicien le joue, échange des étoiles, dessine des relations et enregistre la pièce.",
        "s_shot": "L'interface",
        "shot_alt": "Asterismus en jeu : l'Axe des forces à gauche, la Constellation au centre avec les étoiles mues par la physique et les paires en focus, le mixeur des huit étoiles à droite, la forme et le tempo qui respire dans l'en-tête.",
        "shot_cap": "À gauche, l'Axe des forces et les macros ; au centre, la Constellation — les étoiles, les câbles et les paires en focus de la trame ; à droite, le mixeur avec un stem par étoile. Dans l'en-tête, la forme, le tempo qui respire et l'enregistrement.",
        "s_sky": "Le ciel",
        "sky": [
            ("Étoiles et seed", "36 étoiles de ton, de texture, de voix et de pulsation, dont quatre de contrôle, qui ne sonnent pas mais filtrent le jeu des autres. La seed en choisit huit parmi les 28 de son tirage, un centre tonal et un mode, une forme et les cycles : chaque seed est une autre pièce ; les huit autres entrent par la BIBLIOTHÈQUE."),
            ("La trame", "À chaque mesure le ciel choisit des paires en focus et ce que fait chaque paire : réponse, contagion, ombre, hoquet, pédale, friction, cadence. La pièce parcourt ouverture, développement, contraste et retour, et finit par une coda."),
            ("Le temps qui respire", "Un tempo élastique, un swing qui vient d'une orbite, des mesures groupées en 3+3+2, une étoile qui dérive contre la pulsation, des éclipses amenées par des lunes calculées."),
            ("Mouvement physique", "Gravité, ressorts, pendules, billard, magnétisme, effet et mousse. Les chocs forts sonnent, et chaque étoile sonne dans la stéréo là où elle est dans la Constellation."),
            ("Coupes et bande", "Dans certaines parties le ciel coupe — parenthèse, coupe de montage, apocryphe, décimation — et s'use : bande ou disque, tape stop, spin back, varispeed, selon la seed."),
        ],
        "s_play": "Jouer",
        "play": [
            ("La Constellation", "Cliquez sur une étoile pour la mettre en focus et glissez-la pour changer sa place ; cliquez sur un câble pour choisir son phénomène et glissez-le pour son intensité."),
            ("Gestes et scènes", "OUVRIR, DÉCHIRER, RÉPARER, STUTTER, DÉCALER, HOLD et PANIC SAFE agissent sur l'étoile en focus. Quatre scènes et le PARCOURS suivent la forme."),
            ("Le mixeur et la bibliothèque", "Un stem par étoile, avec présence, niveau, mute et solo, un filtre et des envois d'écho et de salle ; la VARIATION laisse le ciel allumer et éteindre des étoiles et la POLYPHONIE règle combien sonnent ensemble. La BIBLIOTHÈQUE change ou épingle l'étoile d'un corps et ouvre le LEARN, la fiche de chaque étoile."),
            ("L'Axe des forces", "Deux points bougent huit macros — autonomie, intervention, densité, tension, mémoire, risque, espace, profondeur. Avec la trame, un anneau montre l'énergie et la tension du ciel."),
        ],
        "s_rec": "Enregistrer et garder",
        "rec": [
            "REC enregistre la prise : un WAV stéréo 24 bits, un WAV par corps et un manifeste JSON, dans un dossier par prise ;",
            "PISTE · DURÉE enregistre une pièce de 1, 2, 3 ou 5 minutes : la forme du ciel s'ajuste à la durée et la coda arrive à la fin ;",
            "▶ ÉCOUTER ouvre la dernière prise dans le lecteur du système ;",
            "la MÉMOIRE garde huit places nommées ; ENREGISTRER LA SESSION et OUVRIR UNE SESSION écrivent et lisent des fichiers .asterismus — seulement des paramètres, jamais d'audio.",
        ],
        "output": "Sortie selon le standard commun RASGO : gain fixe, limiteur avec true peak 4×, plafond à −1,41 dBTP, mesure BS.1770 et dither à l'enregistrement. Si le système audio se bloque ou si le périphérique disparaît, l'app le rouvre et la pièce continue là où elle était.",
        "tutorial": "Le TUTORIEL de l'app explique tout pas à pas, et le LEARN présente chaque étoile — en français, anglais, portugais ou espagnol.",
        "s_dl": "Télécharger",
        "dl_pending": "La première version (0.2.0, aperçu) est en préparation : les paquets Windows, macOS et Linux seront construits par l'intégration continue. D'ici là, l'instrument peut être compilé depuis les sources.",
        "dl_intro": "La version " + VERSAO + " apporte de nouvelles étoiles, les coupes et la bande du ciel, un filtre et des envois sur chaque étoile, le groove de la DRUMS et le LEARN (ce qui a changé est dans le CHANGELOG). L'instrument reste ouvert, en développement. Choisissez le paquet de votre système ; les étapes sont dans INSTALL.",
        "dl_os": {"Windows": "Windows 10 ou 11, 64 bits — installateur (.exe)", "macOS": "macOS 10.15 ou plus récent, Intel et Apple Silicon", "Linux": ".deb pour Ubuntu et Debian ; .tar.gz pour les autres distributions"},
        "dl_warn": "Au premier lancement, Windows et macOS affichent un avertissement de sécurité : c'est attendu, car l'app n'a pas la signature payante des deux entreprises. Le guide d'installation indique où cliquer.",
        "dl_install": "Guide d'installation",
        "dl_all": "Tous les paquets et les notes de version",
        "dl_build": "Compiler depuis les sources",
        "s_prov": "Provenance",
        "prov": "Le DSP des étoiles vient du Rasgo Synth Studio, en copie figée avec le commit d'origine et les sommes de contrôle ; l'étage de sortie et le mesureur ont été adaptés d'Antitotem et de Navalha 2. Chaque prise enregistre dans son manifeste la seed, les étoiles et les relations dont elle vient.",
        "s_credits": "Crédits",
        "credits": "Lúcio de Araújo — conception, direction artistique, musique et code. Licence GNU GPLv3 ou ultérieure ; JUCE sous sa voie AGPLv3. Crédits et avis complets :",
        "s_state": "État",
        "state": "En développement, ouvert. Compilé et testé sous Linux (39 tests) et joué par l'auteur sur du matériel réel ; les paquets Windows, macOS et Linux sont construits par l'intégration continue.",
        "contact": "Contact",
        "footer_credits": "Crédits",
        "footer_license": "Licence GPLv3+",
        "dialog_title": "CONTACT",
        "dialog_copy_hint": "Copiez l'adresse e-mail :",
        "dialog_copy": "Copier l'e-mail",
        "dialog_close": "Fermer la fenêtre de contact",
        "copied": "E-mail copié.",
        "fallback": "Sélectionnez et copiez l'adresse ci-dessus.",
    },
    "es": {
        "title": "Rasgo Synth Asterismus — un cielo de estrellas que toca",
        "desc": "Rasgo Synth Asterismus: un instrumento en vivo en el que cada módulo de síntesis es una estrella. Las estrellas se relacionan, se mueven por física, cuentan sus ciclos y tocan; una seed propone el cielo, el músico lo toca y graba la pieza.",
        "strip": "RASGO — familia de instrumentos e investigación sonora",
        "skip": "Saltar al contenido",
        "langnav": "Idioma",
        "pill": "Versión " + VERSAO + " — en preparación" if not PUBLICADO else "Versión " + VERSAO + " — para Windows, macOS y Linux",
        "lead": "Asterismus es un instrumento en vivo en el que cada módulo de síntesis es una estrella. Las estrellas se relacionan, se mueven por física, cuentan sus propios ciclos y tocan. Una seed propone el cielo; el músico lo toca, cambia estrellas, dibuja relaciones y graba la pieza.",
        "s_shot": "La interfaz",
        "shot_alt": "Asterismus tocando: el Eje de fuerzas a la izquierda, la Constelación en el centro con las estrellas movidas por la física y los pares en foco, el mezclador de las ocho estrellas a la derecha, la forma y el tempo que respira en la cabecera.",
        "shot_cap": "A la izquierda, el Eje de fuerzas y los macros; en el centro, la Constelación — las estrellas, los cables y los pares en foco de la trama; a la derecha, el mezclador con un stem por estrella. En la cabecera, la forma, el tempo que respira y la grabación.",
        "s_sky": "El cielo",
        "sky": [
            ("Estrellas y seed", "36 estrellas de tono, textura, voz y pulso, cuatro de ellas de control, que no suenan pero filtran cómo tocan las otras. La seed elige ocho entre las 28 de su sorteo, un centro tonal y un modo, una forma y los ciclos: cada seed es otra pieza; las otras ocho entran por la BIBLIOTECA."),
            ("La trama", "En cada compás el cielo elige pares en foco y lo que hace cada par: respuesta, contagio, sombra, hoquetus, pedal, fricción, cadencia. La pieza recorre apertura, desarrollo, contraste y retorno, y termina en una coda."),
            ("El tiempo que respira", "Un tempo elástico, un swing que viene de una órbita, compases agrupados en 3+3+2, una estrella que deriva contra el pulso, eclipses traídos por lunas calculadas."),
            ("Movimiento físico", "Gravedad, resortes, péndulos, billar, magnetismo, efecto y espuma. Los choques fuertes suenan, y cada estrella suena en el estéreo donde está en la Constelación."),
            ("Cortes y cinta", "En algunas partes el cielo corta — paréntesis, corte de montaje, apócrifo, decimación — y se desgasta: cinta o disco, tape stop, spin back, varispeed, según la seed."),
        ],
        "s_play": "Tocar",
        "play": [
            ("La Constelación", "Haga clic en una estrella para ponerla en foco y arrástrela para cambiar su lugar; haga clic en un cable para elegir su fenómeno y arrástrelo para su intensidad."),
            ("Gestos y escenas", "ABRIR, RASGAR, REPARAR, STUTTER, DESPLAZAR, HOLD y PANIC SAFE actúan sobre la estrella en foco. Cuatro escenas y el RECORRIDO siguen la forma."),
            ("El mezclador y la biblioteca", "Un stem por estrella, con presencia, nivel, mute y solo, un filtro y envíos de eco y sala; la VARIACIÓN deja que el cielo encienda y apague estrellas y la POLIFONÍA define cuántas suenan juntas. La BIBLIOTECA cambia o fija la estrella de un cuerpo y abre el LEARN, la ficha de cada estrella."),
            ("El Eje de fuerzas", "Dos puntos mueven ocho macros — autonomía, intervención, densidad, tensión, memoria, riesgo, espacio, profundidad. Con la trama, un anillo muestra la energía y la tensión del cielo."),
        ],
        "s_rec": "Grabar y guardar",
        "rec": [
            "REC graba la toma: un WAV estéreo de 24 bits, un WAV por cuerpo y un manifiesto JSON, en una carpeta por toma;",
            "PISTA · DURACIÓN graba una pieza de 1, 2, 3 o 5 minutos: la forma del cielo se ajusta a la duración y la coda llega al final;",
            "▶ ESCUCHAR abre la última toma en el reproductor del sistema;",
            "la MEMORIA guarda ocho lugares con nombre; GUARDAR SESIÓN y ABRIR SESIÓN escriben y leen archivos .asterismus — solo parámetros, nunca audio.",
        ],
        "output": "Salida en el estándar común RASGO: ganancia fija, limitador con true peak 4×, techo de −1,41 dBTP, medición BS.1770 y dither en la grabación. Si el sistema de sonido se bloquea o el dispositivo desaparece, la app lo reabre y la pieza sigue desde donde estaba.",
        "tutorial": "El TUTORIAL de la app lo explica todo paso a paso, y el LEARN presenta cada estrella — en español, inglés, portugués o francés.",
        "s_dl": "Descargar",
        "dl_pending": "La primera versión (0.2.0, vista previa) está en preparación: los paquetes para Windows, macOS y Linux se construirán en la integración continua. Mientras tanto, el instrumento puede compilarse desde el código.",
        "dl_intro": "La versión " + VERSAO + " trae estrellas nuevas, los cortes y la cinta del cielo, un filtro y envíos en cada estrella, el groove de la DRUMS y el LEARN (lo que cambió está en el CHANGELOG). El instrumento sigue abierto, en desarrollo. Elige el paquete de tu sistema; los pasos están en INSTALL.",
        "dl_os": {"Windows": "Windows 10 u 11, 64 bits — instalador (.exe)", "macOS": "macOS 10.15 o más reciente, Intel y Apple Silicon", "Linux": ".deb para Ubuntu y Debian; .tar.gz para las demás distribuciones"},
        "dl_warn": "En la primera apertura, Windows y macOS muestran un aviso de seguridad: es lo esperado, porque la app no lleva la firma de pago de las dos empresas. La guía de instalación dice dónde hacer clic.",
        "dl_install": "Guía de instalación",
        "dl_all": "Todos los paquetes y las notas de la versión",
        "dl_build": "Compilar desde el código",
        "s_prov": "Procedencia",
        "prov": "El DSP de las estrellas viene del Rasgo Synth Studio, como copia fijada con commit de origen y sumas de verificación; la etapa de salida y el medidor se adaptaron de Antitotem y Navalha 2. Cada toma registra en su manifiesto la seed, las estrellas y las relaciones de las que vino.",
        "s_credits": "Créditos",
        "credits": "Lúcio de Araújo — concepción, dirección artística, música y código. Licencia GNU GPLv3 o posterior; JUCE bajo su vía AGPLv3. Créditos y avisos completos:",
        "s_state": "Estado",
        "state": "En desarrollo, abierto. Compilado y probado en Linux (39 pruebas) y tocado por el autor en hardware real; los paquetes de Windows, macOS y Linux se montan por integración continua.",
        "contact": "Contacto",
        "footer_credits": "Créditos",
        "footer_license": "Licencia GPLv3+",
        "dialog_title": "CONTACTO",
        "dialog_copy_hint": "Copia la dirección de correo:",
        "dialog_copy": "Copiar correo",
        "dialog_close": "Cerrar ventana de contacto",
        "copied": "Correo copiado.",
        "fallback": "Selecciona y copia la dirección de arriba.",
    },
}


def pagina(arquivo, lang):
    t = T[lang]
    e = lambda s: html.escape(s, quote=False)
    a = lambda s: html.escape(s, quote=True)
    ceu = "\n".join('    <li><h3>%s</h3><p>%s</p></li>' % (e(h), e(p)) for h, p in t["sky"])
    tocar = "\n".join('    <li><h3>%s</h3><p>%s</p></li>' % (e(h), e(p)) for h, p in t["play"])
    gravar = "\n".join("    <li>%s</li>" % e(f) for f in t["rec"])
    robots = "" if PUBLICADO else '<meta name="robots" content="noindex" />\n'
    if PUBLICADO:
        pacotes = "\n".join(
            '    <li><h3>%s</h3><p>%s</p><p class="dl-links">%s</p></li>' % (
                e(so), e(t["dl_os"][so]), " ".join('<a class="button" href="%s%s">%s</a>' % (DL, a(f), e(rot)) for f, rot in arqs))
            for so, arqs in PACOTES)
        baixar = f"""  <p>{e(t["dl_intro"])}</p>
  <ul class="steps downloads">
{pacotes}
  </ul>
  <p class="example">{e(t["dl_warn"])}</p>
  <div class="contact-row">
    <a class="button button-primary" href="{BLOB}/INSTALL.md">{e(t["dl_install"])}</a>
    <a class="button" href="{RELEASE}">{e(t["dl_all"])}</a>
  </div>"""
    else:
        baixar = f"""  <p>{e(t["dl_pending"])}</p>
  <div class="contact-row">
    <a class="button button-primary" href="{BLOB}/INSTALL.md">{e(t["dl_build"])}</a>
  </div>"""
    return f"""<!doctype html>
<html lang="{lang}">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
{robots}<title>{e(t["title"])}</title>
<meta name="description" content="{a(t["desc"])}" />
{icones()}<link rel="stylesheet" href="styles.css" />
<script src="assets/contact.js" defer></script>
<script src="assets/ceu.js?v=20261106" defer></script>
</head>
<body>
{cabecalho(lang, "sobre")}

<main id="conteudo">
<section class="hero" id="top">
  <div class="hero-text">
    <span class="status-pill">{e(t["pill"])}</span>
    <h1>Rasgo <b>Synth</b> Asterismus</h1>
    <p class="lead">{e(t["lead"])}</p>
  </div>
  <div class="hero-art"><canvas data-ceu aria-hidden="true"></canvas></div>
</section>

<section class="section">
  <h2>{e(t["s_shot"])}</h2>
  <figure class="shot">
    <img src="assets/interface.jpg" alt="{a(t["shot_alt"])}" width="1920" height="1006" loading="lazy" />
    <figcaption>{e(t["shot_cap"])}</figcaption>
  </figure>
</section>

<section class="section sky-section">
  <h2>{e(t["s_sky"])}</h2>
  <ol class="steps">
{ceu}
  </ol>
</section>

<section class="section">
  <h2>{e(t["s_play"])}</h2>
  <ol class="steps">
{tocar}
  </ol>
</section>

<section class="section section-dark">
  <h2>{e(t["s_rec"])}</h2>
  <ul class="facts">
{gravar}
  </ul>
  <p class="example">{e(t["output"])}</p>
  <p>{e(t["tutorial"])}</p>
</section>

<section class="section download" id="baixar">
  <h2>{e(t["s_dl"])}</h2>
{baixar}
</section>

{secao_studio(lang)}

<section class="section section-dark">
  <h2>{e(t["s_prov"])}</h2>
  <p>{e(t["prov"])}</p>
</section>

<section class="section">
  <h2>{e(t["s_credits"])}</h2>
  <p>{e(t["credits"])} <a href="{BLOB}/CREDITS.md">CREDITS.md</a> · <a href="{BLOB}/THIRD_PARTY_NOTICES.md">THIRD_PARTY_NOTICES.md</a>.</p>
</section>

<section class="section section-dark">
  <h2>{e(t["s_state"])}</h2>
  <p>{e(t["state"])}</p>
  <div class="contact-row">
    <button type="button" class="button button-primary" data-contact-trigger>{e(t["contact"])}</button>
    <a class="button" href="{REPO}">GitHub</a>
  </div>
</section>
</main>

{rodape(lang)}"""


# ---- menu, páginas novas, Studio e ícones (6/11, pedido do autor: "como
# fizemos nas páginas de outros instrumentos da família rasgo") ----------
# O menu e as páginas Tutorial e Instalação seguem o site do Aquorbium
# (lucioaraujo/aquorbium, website/): tutorial.html, tutorial-en.html…

TIPOS = {"sobre": "", "tutorial": "tutorial", "learn": "learn", "instalacao": "instalacao"}
SUFIXO = {"pt-BR": "", "en": "-en", "fr": "-fr", "es": "-es"}
INDICE = {"pt-BR": "index.html", "en": "en.html", "fr": "fr.html", "es": "es.html"}


def arquivo_de(tipo, lang):
    if tipo == "sobre":
        return INDICE[lang]
    return TIPOS[tipo] + SUFIXO[lang] + ".html"


MENU = {
    "pt-BR": [("sobre", "Sobre"), ("tutorial", "Tutorial"), ("instalacao", "Instalação"), ("#baixar", "Baixar"), ("#studio", "Studio")],
    "en": [("sobre", "About"), ("tutorial", "Tutorial"), ("instalacao", "Installation"), ("#baixar", "Download"), ("#studio", "Studio")],
    "fr": [("sobre", "À propos"), ("tutorial", "Tutoriel"), ("instalacao", "Installation"), ("#baixar", "Télécharger"), ("#studio", "Studio")],
    "es": [("sobre", "Acerca"), ("tutorial", "Tutorial"), ("instalacao", "Instalación"), ("#baixar", "Descargar"), ("#studio", "Studio")],
}


def menu(lang, com_learn):
    itens = list(MENU[lang])
    if com_learn:
        itens.insert(2, ("learn", "LEARN"))
    return itens


def icones():
    return ('<link rel="icon" href="assets/identity/asterismus-favicon.svg" type="image/svg+xml" />\n'
            '<link rel="icon" href="assets/identity/favicon-32.png" sizes="32x32" type="image/png" />\n'
            '<link rel="apple-touch-icon" href="assets/identity/apple-touch-icon.png" />\n')


def cabecalho(lang, atual, com_learn=None):
    if com_learn is None:
        com_learn = LEARN_PUBLICO
    t = T[lang]
    e = lambda s: html.escape(s, quote=False)
    a = lambda s: html.escape(s, quote=True)
    itens = []
    for alvo, rotulo in menu(lang, com_learn):
        if alvo.startswith("#"):
            href = INDICE[lang] + alvo
            cur = ""
        else:
            href = arquivo_de(alvo, lang)
            cur = ' aria-current="page"' if alvo == atual else ""
        itens.append('      <a href="%s"%s>%s</a>' % (href, cur, e(rotulo)))
    langs = "\n".join('      <a href="%s"%s>%s</a>' % (arquivo_de(atual, l), ' aria-current="true"' if l == lang else "", c)
                       for _, l, c in PAGINAS)
    return f"""<div class="rasgo-strip"><a href="{PORTAL}">{e(t["strip"])}</a></div>
<a class="skip-link" href="#conteudo">{e(t["skip"])}</a>

<div class="site-header">
  <a class="brand" href="{INDICE[lang]}" aria-label="Rasgo Synth Asterismus">
    <img src="assets/identity/rasgo_logo_2026.svg" alt="" height="26" />
    <span>RASGO<b>SYNTH</b> · ASTERISMUS</span>
  </a>
  <div class="header-nav">
    <nav class="page-nav" aria-label="{a(PAGINAS_NAV[lang])}">
{chr(10).join(itens)}
    </nav>
    <nav class="lang-switch" aria-label="{a(t["langnav"])}">
{langs}
    </nav>
  </div>
</div>
"""


PAGINAS_NAV = {"pt-BR": "Páginas", "en": "Pages", "fr": "Pages", "es": "Páginas"}


def rodape(lang):
    t = T[lang]
    e = lambda s: html.escape(s, quote=False)
    a = lambda s: html.escape(s, quote=True)
    return f"""<footer class="site-footer">
  <span>© Lúcio de Araújo · Rasgo Synth Asterismus 2026 · RASGO</span>
  <nav>
    <a href="{REPO}">GitHub</a> ·
    <a href="{STUDIO}">Rasgo Synth Studio</a> ·
    <a href="{BLOB}/CREDITS.md">{e(t["footer_credits"])}</a> ·
    <a href="{BLOB}/LICENSE">{e(t["footer_license"])}</a>
  </nav>
</footer>
<dialog class="contact-dialog" data-contact-dialog aria-labelledby="contact-title">
  <button class="dialog-close" type="button" data-contact-close aria-label="{a(t["dialog_close"])}">×</button>
  <h2 id="contact-title">{e(t["dialog_title"])}</h2>
  <p>{e(t["dialog_copy_hint"])}</p>
  <code data-contact-address></code>
  <button class="button button-primary" type="button" data-contact-copy>{e(t["dialog_copy"])}</button>
  <p class="contact-feedback" data-contact-feedback data-copied="{a(t["copied"])}" data-fallback="{a(t["fallback"])}" aria-live="polite"></p>
</dialog>
</body>
</html>
"""


STUDIO = "https://lucioaraujo.github.io/rasgo-synth-studio/"
TEXTO_STUDIO = {
    "pt-BR": ("Irmão do Rasgo Synth Studio",
              "O Asterismus nasceu do Rasgo Synth Studio, o instrumento generativo da família RASGO que compõe e sintetiza peças e álbuns inteiros. O Studio renderiza a peça; o Asterismus a toca ao vivo. Várias estrelas do céu são vozes do Studio adaptadas para o tempo real, e a ideia da seed, da forma e do modo álbum vem de lá.",
              "Conhecer o Rasgo Synth Studio →"),
    "en": ("Sibling of Rasgo Synth Studio",
           "Asterismus was born from Rasgo Synth Studio, the RASGO family's generative instrument that composes and synthesises whole pieces and albums. The Studio renders the piece; Asterismus plays it live. Many of the sky's stars are Studio voices adapted to real time, and the idea of the seed, the form and the album mode comes from there.",
           "Discover Rasgo Synth Studio →"),
    "fr": ("Frère du Rasgo Synth Studio",
           "L'Asterismus est né du Rasgo Synth Studio, l'instrument génératif de la famille RASGO qui compose et synthétise des pièces et des albums entiers. Le Studio rend la pièce ; l'Asterismus la joue en direct. Plusieurs étoiles du ciel sont des voix du Studio adaptées au temps réel, et l'idée de la seed, de la forme et du mode album vient de là.",
           "Découvrir le Rasgo Synth Studio →"),
    "es": ("Hermano del Rasgo Synth Studio",
           "El Asterismus nació del Rasgo Synth Studio, el instrumento generativo de la familia RASGO que compone y sintetiza piezas y álbumes enteros. El Studio renderiza la pieza; el Asterismus la toca en vivo. Varias estrellas del cielo son voces del Studio adaptadas al tiempo real, y la idea de la seed, de la forma y del modo álbum viene de allí.",
           "Conocer el Rasgo Synth Studio →"),
}


def secao_studio(lang):
    h, p, b = TEXTO_STUDIO[lang]
    e = lambda s: html.escape(s, quote=False)
    return f"""<section class="section studio-section" id="studio">
  <h2>{e(h)}</h2>
  <p>{e(p)}</p>
  <div class="contact-row">
    <a class="button button-primary" href="{STUDIO}">{e(b)}</a>
  </div>
</section>"""


TITULOS = {
    "tutorial": {"pt-BR": "Tutorial", "en": "Tutorial", "fr": "Tutoriel", "es": "Tutorial"},
    "learn": {"pt-BR": "LEARN · as estrelas", "en": "LEARN · the stars", "fr": "LEARN · les étoiles", "es": "LEARN · las estrellas"},
    "instalacao": {"pt-BR": "Instalação", "en": "Installation", "fr": "Installation", "es": "Instalación"},
}
INTRO_TUTORIAL = {
    "pt-BR": "O mesmo guia que abre no botão TUTORIAL do app, em nove partes: da seed à gravação.",
    "en": "The same guide that opens from the app's GUIDE button, in nine parts: from the seed to the recording.",
    "fr": "Le même guide que le bouton TUTORIEL de l'app, en neuf parties : de la seed à l'enregistrement.",
    "es": "La misma guía que abre el botón TUTORIAL de la app, en nueve partes: de la seed a la grabación.",
}
CHAVE_TUTORIAL = {"pt-BR": "pt", "en": "en", "fr": "fr", "es": "es"}


def corpo_tutorial(lang):
    import json, re
    textos = json.loads((AQUI / "tutorial_textos.json").read_text(encoding="utf-8"))
    texto = textos[CHAVE_TUTORIAL[lang]]
    e = lambda s: html.escape(s, quote=False)
    partes = [p for p in re.split(r"\n\n+", texto.strip()) if p.strip()]
    blocos = []
    for parte in partes:
        linhas = parte.split("\n", 1)
        titulo = linhas[0].strip()
        resto = linhas[1].strip() if len(linhas) > 1 else ""
        blocos.append('  <article class="tutorial-part">\n    <h3>%s</h3>\n    %s\n  </article>' % (
            e(titulo), "".join("<p>%s</p>" % e(l) for l in resto.split("\n") if l.strip())))
    return '<p class="lead">%s</p>\n<div class="tutorial">\n%s\n</div>' % (e(INTRO_TUTORIAL[lang]), "\n".join(blocos))


INTRO_LEARN = {
    "pt-BR": "A mesma ficha que abre no app (BIBLIOTECA → LEARN), para cada estrela: o que é, como funciona, os três controles, o que ouvir e de onde vem.",
    "en": "The same fiche the app opens (LIBRARY → LEARN), for every star: what it is, how it works, the three controls, what to listen for and where it comes from.",
    "fr": "La même fiche que l'app ouvre (BIBLIOTHÈQUE → LEARN), pour chaque étoile : ce qu'elle est, comment elle marche, les trois contrôles, quoi écouter et d'où elle vient.",
    "es": "La misma ficha que abre la app (BIBLIOTECA → LEARN), para cada estrella: qué es, cómo funciona, los tres controles, qué escuchar y de dónde viene.",
}
ROTULOS_LEARN = {
    "pt-BR": ("Como funciona", "Os controles", "Para ouvir", "De onde vem"),
    "en": ("How it works", "The controls", "Listen for", "Where it comes from"),
    "fr": ("Comment elle marche", "Les contrôles", "À écouter", "D'où elle vient"),
    "es": ("Cómo funciona", "Los controles", "Para escuchar", "De dónde viene"),
}
ORDEM_FAMILIAS = ["Tone", "Texture", "Voice", "Pulse", "Control"]


# O JSON vem de tools/extrair_learn.py: em website/ só na release 0.3; a
# prévia o lê da própria pasta.
LEARN_JSON = AQUI / "learn_textos.json"


def corpo_learn(lang):
    import json
    dados = json.loads(LEARN_JSON.read_text(encoding="utf-8"))
    k = CHAVE_TUTORIAL[lang]
    e = lambda s: html.escape(s, quote=False)
    rot = ROTULOS_LEARN[lang]
    indice, blocos = [], []
    for familia in ORDEM_FAMILIAS:
        fichas = [f for f in dados["fichas"] if f["familia"] == familia]
        if not fichas:
            continue
        nome_familia = dados["familias"][familia][k]
        indice.append("<p><b>%s</b> · %s</p>" % (e(nome_familia), " · ".join(
            '<a href="#%s">%s</a>' % (f["corpo"].lower(), e(f["nome"])) for f in fichas)))
        for f in fichas:
            controles = "".join("<p><b>%s</b> — %s</p>" % (e(n[k]), e(c[k]))
                                for n, c in zip(f["nomes_controles"], f["controles"]))
            blocos.append(
                '  <article class="tutorial-part" id="%s">\n    <h3>%s · %s</h3>\n    <p>%s</p>\n'
                '    <h3>%s</h3><p>%s</p>\n    <h3>%s</h3>%s\n    <h3>%s</h3><p>%s</p>\n    <h3>%s</h3><p>%s</p>\n  </article>' % (
                    f["corpo"].lower(), e(f["nome"]), e(nome_familia), e(f["rapido"][k]),
                    e(rot[0]), e(f["como"][k]), e(rot[1]), controles,
                    e(rot[2]), e(f["ouvir"][k]), e(rot[3]), e(f["origem"][k])))
    return '<p class="lead">%s</p>\n%s\n<div class="tutorial">\n%s\n</div>' % (
        e(INTRO_LEARN[lang]), "\n".join(indice), "\n".join(blocos))


INSTALACAO = {
    "pt-BR": [
        ("Windows", ["Rode o instalador rasgo-synth-asterismus-0.3.0-win64.exe (ou use o .zip portátil).",
                     "Se aparecer \"O Windows protegeu o computador\" (SmartScreen): Mais informações → Executar assim mesmo.",
                     "O atalho fica no Menu Iniciar. O runtime do Visual C++ vai dentro do programa."]),
        ("macOS (Intel e Apple Silicon, 10.15 ou mais novo)", ["Abra o .dmg e arraste Rasgo Synth Asterismus para Aplicativos.",
                     "Na primeira vez: botão direito no app → Abrir → Abrir (ou Ajustes do Sistema → Privacidade e Segurança → Abrir Mesmo Assim).",
                     "Se ainda recusar, no Terminal: xattr -dr com.apple.quarantine \"/Applications/Rasgo Synth Asterismus.app\""]),
        ("Linux", ["Pacote .deb: sudo apt install ./rasgo-synth-asterismus-0.3.0-Linux.deb — o atalho aparece em Som e Vídeo.",
                   "Arquivo .tar.gz: extraia e rode usr/bin/rasgo-synth-asterismus."]),
        ("Onde ficam as gravações", ["~/Music/Rasgo Synth Performance/recordings/ (uma pasta por tomada: a mistura em WAV de 24 bits, um WAV por estrela e o manifesto), os álbuns em albuns/ e o diagnóstico em diagnostico/."]),
    ],
    "en": [
        ("Windows", ["Run the installer rasgo-synth-asterismus-0.3.0-win64.exe (or use the portable .zip).",
                     "If \"Windows protected your PC\" (SmartScreen) appears: More info → Run anyway.",
                     "The shortcut is in the Start menu. The Visual C++ runtime is inside the program."]),
        ("macOS (Intel and Apple Silicon, 10.15 or newer)", ["Open the .dmg and drag Rasgo Synth Asterismus to Applications.",
                     "The first time: right-click the app → Open → Open (or System Settings → Privacy & Security → Open Anyway).",
                     "If it still refuses, in Terminal: xattr -dr com.apple.quarantine \"/Applications/Rasgo Synth Asterismus.app\""]),
        ("Linux", [".deb package: sudo apt install ./rasgo-synth-asterismus-0.3.0-Linux.deb — the shortcut appears in Sound & Video.",
                   ".tar.gz archive: extract it and run usr/bin/rasgo-synth-asterismus."]),
        ("Where the recordings go", ["~/Music/Rasgo Synth Performance/recordings/ (one folder per take: the mix as a 24-bit WAV, one WAV per star and the manifest), albums in albuns/ and the diagnostics in diagnostico/."]),
    ],
    "fr": [
        ("Windows", ["Lancez l'installeur rasgo-synth-asterismus-0.3.0-win64.exe (ou utilisez le .zip portable).",
                     "Si « Windows a protégé votre ordinateur » (SmartScreen) apparaît : Informations complémentaires → Exécuter quand même.",
                     "Le raccourci est dans le menu Démarrer. Le runtime Visual C++ est dans le programme."]),
        ("macOS (Intel et Apple Silicon, 10.15 ou plus récent)", ["Ouvrez le .dmg et glissez Rasgo Synth Asterismus dans Applications.",
                     "La première fois : clic droit sur l'app → Ouvrir → Ouvrir (ou Réglages Système → Confidentialité et sécurité → Ouvrir quand même).",
                     "S'il refuse encore, dans le Terminal : xattr -dr com.apple.quarantine \"/Applications/Rasgo Synth Asterismus.app\""]),
        ("Linux", ["Paquet .deb : sudo apt install ./rasgo-synth-asterismus-0.3.0-Linux.deb — le raccourci apparaît dans Son et vidéo.",
                   "Archive .tar.gz : extrayez-la et lancez usr/bin/rasgo-synth-asterismus."]),
        ("Où vont les enregistrements", ["~/Music/Rasgo Synth Performance/recordings/ (un dossier par prise : le mixage en WAV 24 bits, un WAV par étoile et le manifeste), les albums dans albuns/ et le diagnostic dans diagnostico/."]),
    ],
    "es": [
        ("Windows", ["Ejecute el instalador rasgo-synth-asterismus-0.3.0-win64.exe (o use el .zip portátil).",
                     "Si aparece «Windows protegió su PC» (SmartScreen): Más información → Ejecutar de todas formas.",
                     "El acceso directo queda en el menú Inicio. El runtime de Visual C++ va dentro del programa."]),
        ("macOS (Intel y Apple Silicon, 10.15 o más nuevo)", ["Abra el .dmg y arrastre Rasgo Synth Asterismus a Aplicaciones.",
                     "La primera vez: clic derecho en la app → Abrir → Abrir (o Ajustes del Sistema → Privacidad y seguridad → Abrir igualmente).",
                     "Si aún se niega, en la Terminal: xattr -dr com.apple.quarantine \"/Applications/Rasgo Synth Asterismus.app\""]),
        ("Linux", ["Paquete .deb: sudo apt install ./rasgo-synth-asterismus-0.3.0-Linux.deb — el acceso aparece en Sonido y vídeo.",
                   "Archivo .tar.gz: extráigalo y ejecute usr/bin/rasgo-synth-asterismus."]),
        ("Dónde quedan las grabaciones", ["~/Music/Rasgo Synth Performance/recordings/ (una carpeta por toma: la mezcla en WAV de 24 bits, un WAV por estrella y el manifiesto), los álbumes en albuns/ y el diagnóstico en diagnostico/."]),
    ],
}
INTRO_INSTALACAO = {
    "pt-BR": "Sem assinatura digital paga (decisão do projeto RASGO): o Windows e o macOS mostram um aviso na primeira abertura — é esperado.",
    "en": "No paid code signing (a RASGO project decision): Windows and macOS show a warning the first time — it is expected.",
    "fr": "Sans signature payante (décision du projet RASGO) : Windows et macOS affichent un avertissement à la première ouverture — c'est attendu.",
    "es": "Sin firma digital de pago (decisión del proyecto RASGO): Windows y macOS muestran un aviso la primera vez — es lo esperado.",
}


def corpo_instalacao(lang):
    e = lambda s: html.escape(s, quote=False)
    blocos = "\n".join('  <li><h3>%s</h3>%s</li>' % (e(h), "".join("<p>%s</p>" % e(p) for p in ps)) for h, ps in INSTALACAO[lang])
    return f"""<p class="lead">{e(INTRO_INSTALACAO[lang])}</p>
<ul class="steps install">
{blocos}
</ul>
<div class="contact-row">
  <a class="button button-primary" href="{RELEASE}">{e(T[lang]["dl_all"])}</a>
  <a class="button" href="{BLOB}/INSTALL.md">INSTALL.md</a>
</div>"""


def pagina_interna(tipo, lang, com_learn=None):
    t = T[lang]
    e = lambda s: html.escape(s, quote=False)
    a = lambda s: html.escape(s, quote=True)
    titulo = TITULOS[tipo][lang]
    corpo = (corpo_tutorial(lang) if tipo == "tutorial" else corpo_learn(lang) if tipo == "learn"
             else corpo_instalacao(lang))
    return f"""<!doctype html>
<html lang="{lang}">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>{e(titulo)} — Rasgo Synth Asterismus</title>
<meta name="description" content="{a(titulo + " — " + t["desc"])}" />
{icones()}<link rel="stylesheet" href="styles.css" />
<script src="assets/contact.js" defer></script>
</head>
<body>
{cabecalho(lang, tipo, com_learn)}
<main id="conteudo">
<section class="section page-section">
  <h1 class="page-title">{e(titulo)}</h1>
{corpo}
</section>
</main>

{rodape(lang)}"""


def previa_learn(destino):
    # A prévia do LEARN, fora de website/: as páginas LEARN (com o menu que
    # terão), mais os estilos e os assets para abrir no navegador.
    import shutil
    global LEARN_JSON
    destino = pathlib.Path(destino)
    LEARN_JSON = destino / "learn_textos.json"
    destino.mkdir(parents=True, exist_ok=True)
    shutil.copy(AQUI / "styles.css", destino / "styles.css")
    shutil.copytree(AQUI / "assets", destino / "assets", dirs_exist_ok=True)
    for _, lang, _ in PAGINAS:
        (destino / arquivo_de("learn", lang)).write_text(pagina_interna("learn", lang, True), encoding="utf-8")
    print("gerar_paginas.py: prévia do LEARN em %s" % destino)


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--previa-learn":
        previa_learn(sys.argv[2])
        return
    for arquivo, lang, _ in PAGINAS:
        (AQUI / arquivo).write_text(pagina(arquivo, lang), encoding="utf-8")
        for tipo in ("tutorial", "learn", "instalacao") if LEARN_PUBLICO else ("tutorial", "instalacao"):
            (AQUI / arquivo_de(tipo, lang)).write_text(pagina_interna(tipo, lang), encoding="utf-8")
    print("gerar_paginas.py: %d páginas%s" % (len(PAGINAS), "" if PUBLICADO else " (em preparação, noindex)"))


if __name__ == "__main__":
    main()
