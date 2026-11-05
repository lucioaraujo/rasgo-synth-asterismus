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

AQUI = pathlib.Path(__file__).parent
PORTAL = "https://rasgoinstruments.arquiviagem.net/"
REPO = "https://github.com/lucioaraujo/rasgo-synth-asterismus"
BLOB = REPO + "/blob/main"
# a versão e os pacotes do release (a CI: .github/workflows/package.yml);
# os nomes são os do CPack (CPACK_PACKAGE_NAME) — conferir na primeira release
VERSAO = "0.2.0"
PUBLICADO = True
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
        "pill": "Versão 0.2.0 — em preparação" if not PUBLICADO else "Versão 0.2.0 — primeira versão pública, para Windows, macOS e Linux",
        "lead": "O Asterismus é um instrumento ao vivo em que cada módulo de síntese é uma estrela. As estrelas se relacionam, se movem por física, contam os seus próprios ciclos e tocam. Uma seed propõe o céu; o músico o toca, troca estrelas, desenha relações e grava a peça.",
        "s_shot": "A interface",
        "shot_alt": "O Asterismus tocando: o Eixo de Forças à esquerda, a Constelação no centro com as estrelas movidas pela física e os pares em foco, o mixer das oito estrelas à direita, a forma e o tempo que respira no cabeçalho.",
        "shot_cap": "À esquerda, o Eixo de Forças e os macros; no centro, a Constelação — as estrelas, os cabos e os pares em foco da trama; à direita, o mixer com um stem por estrela. No cabeçalho, a forma, o tempo que respira e a gravação.",
        "s_sky": "O céu",
        "sky": [
            ("Estrelas e seed", "28 estrelas de tom, textura, voz e pulso, e quatro estrelas de controle, que não soam mas filtram como as outras tocam. A seed escolhe oito delas, um centro tonal e um modo, uma forma e os ciclos: cada seed é outra peça."),
            ("A trama", "A cada compasso o céu escolhe pares em foco e o que cada par faz: resposta, contágio, sombra, hoquetus, pedal, atrito, cadência. A peça percorre abertura, desenvolvimento, contraste e retorno, e termina numa coda."),
            ("O tempo que respira", "Andamento elástico, swing que vem de uma órbita, compassos agrupados em 3+3+2, uma estrela que deriva contra o pulso, eclipses trazidos por luas calculadas."),
            ("Movimento físico", "Gravidade, molas, pêndulos, bilhar, magnetismo, efeito e espuma. Os choques fortes soam, e cada estrela soa no estéreo onde está na Constelação."),
        ],
        "s_play": "Tocar",
        "play": [
            ("A Constelação", "Clique numa estrela para pô-la em foco e arraste-a para mudar o seu lugar; clique num cabo para escolher o fenômeno e arraste-o para a intensidade."),
            ("Gestos e cenas", "ABRIR, RASGAR, REPARAR, STUTTER, DESLOCAR, HOLD e PANIC SAFE agem na estrela em foco. Quatro cenas e o PERCURSO seguem a forma."),
            ("O mixer e a biblioteca", "Um stem por estrela, com presença, nível, mute e solo; a VARIAÇÃO deixa o céu acender e apagar estrelas. A BIBLIOTECA troca ou fixa a estrela de um corpo."),
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
        "tutorial": "O botão TUTORIAL do app explica tudo passo a passo, em português, inglês, francês ou espanhol.",
        "s_dl": "Baixar",
        "dl_pending": "A primeira versão (0.2.0, prévia) está em preparação: os pacotes para Windows, macOS e Linux serão montados pela integração contínua. Até lá, o instrumento pode ser compilado a partir do código.",
        "dl_intro": "A versão 0.2.0 é a primeira versão pública. O instrumento continua aberto, em desenvolvimento. Escolha o pacote do seu sistema; os passos estão no INSTALL.",
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
        "state": "Em desenvolvimento, aberto. Compilado e testado no Linux (37 testes) e tocado pelo autor em hardware real; os pacotes de Windows e macOS ainda não foram montados.",
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
        "pill": "Version 0.2.0 — in preparation" if not PUBLICADO else "Version 0.2.0 — first public version, for Windows, macOS and Linux",
        "lead": "Asterismus is a live instrument in which every synthesis module is a star. The stars relate to each other, move by physics, count their own cycles and play. A seed proposes the sky; the musician plays it, swaps stars, draws relations and records the piece.",
        "s_shot": "The interface",
        "shot_alt": "Asterismus playing: the Axis of Forces on the left, the Constellation in the centre with the stars moved by physics and the pairs in focus, the mixer of the eight stars on the right, the form and the breathing tempo in the header.",
        "shot_cap": "On the left, the Axis of Forces and the macros; in the centre, the Constellation — the stars, the cables and the web's pairs in focus; on the right, the mixer with one stem per star. In the header, the form, the breathing tempo and the recording.",
        "s_sky": "The sky",
        "sky": [
            ("Stars and seed", "28 tone, texture, voice and pulse stars, and four control stars that do not sound but filter how the others play. The seed chooses eight of them, a key and a mode, a form and the cycles: every seed is another piece."),
            ("The web", "Each bar the sky chooses pairs in focus and what each pair does: answer, contagion, shadow, hocket, pedal, friction, cadence. The piece goes through opening, development, contrast and return, and ends with a coda."),
            ("Time that breathes", "An elastic tempo, a swing that comes from an orbit, bars grouped as 3+3+2, a star that drifts against the pulse, eclipses brought by computed moons."),
            ("Physical motion", "Gravity, springs, pendulums, billiards, magnetism, spin and foam. Strong collisions sound, and each star sounds in the stereo field where it is in the Constellation."),
        ],
        "s_play": "Playing",
        "play": [
            ("The Constellation", "Click a star to put it in focus and drag it to move its place; click a cable to choose its phenomenon and drag it to set its strength."),
            ("Gestures and scenes", "OPEN, TEAR, REPAIR, STUTTER, DISPLACE, HOLD and PANIC SAFE act on the star in focus. Four scenes and the PATH follow the form."),
            ("The mixer and the library", "One stem per star, with presence, level, mute and solo; VARIATION lets the sky light and dim stars. The LIBRARY swaps or pins the star of a body."),
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
        "tutorial": "The app's TUTORIAL button explains everything step by step, in English, Portuguese, French or Spanish.",
        "s_dl": "Download",
        "dl_pending": "The first version (0.2.0, preview) is in preparation: the Windows, macOS and Linux packages will be built by continuous integration. Until then, the instrument can be built from source.",
        "dl_intro": "Version 0.2.0 is the first public version. The instrument remains open, in development. Choose the package for your system; the steps are in INSTALL.",
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
        "state": "In development, open. Built and tested on Linux (37 tests) and played by the author on real hardware; the Windows and macOS packages have not been built yet.",
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
        "pill": "Version 0.2.0 — en préparation" if not PUBLICADO else "Version 0.2.0 — première version publique, pour Windows, macOS et Linux",
        "lead": "Asterismus est un instrument de scène dans lequel chaque module de synthèse est une étoile. Les étoiles se relient, bougent selon la physique, comptent leurs propres cycles et jouent. Une seed propose le ciel ; le musicien le joue, échange des étoiles, dessine des relations et enregistre la pièce.",
        "s_shot": "L'interface",
        "shot_alt": "Asterismus en jeu : l'Axe des forces à gauche, la Constellation au centre avec les étoiles mues par la physique et les paires en focus, le mixeur des huit étoiles à droite, la forme et le tempo qui respire dans l'en-tête.",
        "shot_cap": "À gauche, l'Axe des forces et les macros ; au centre, la Constellation — les étoiles, les câbles et les paires en focus de la trame ; à droite, le mixeur avec un stem par étoile. Dans l'en-tête, la forme, le tempo qui respire et l'enregistrement.",
        "s_sky": "Le ciel",
        "sky": [
            ("Étoiles et seed", "28 étoiles de ton, de texture, de voix et de pulsation, et quatre étoiles de contrôle, qui ne sonnent pas mais filtrent la façon dont les autres jouent. La seed en choisit huit, une tonalité et un mode, une forme et les cycles : chaque seed est une autre pièce."),
            ("La trame", "À chaque mesure le ciel choisit des paires en focus et ce que fait chaque paire : réponse, contagion, ombre, hoquet, pédale, friction, cadence. La pièce parcourt ouverture, développement, contraste et retour, et finit par une coda."),
            ("Le temps qui respire", "Un tempo élastique, un swing qui vient d'une orbite, des mesures groupées en 3+3+2, une étoile qui dérive contre la pulsation, des éclipses amenées par des lunes calculées."),
            ("Mouvement physique", "Gravité, ressorts, pendules, billard, magnétisme, effet et mousse. Les chocs forts sonnent, et chaque étoile sonne dans la stéréo là où elle est dans la Constellation."),
        ],
        "s_play": "Jouer",
        "play": [
            ("La Constellation", "Cliquez sur une étoile pour la mettre en focus et glissez-la pour changer sa place ; cliquez sur un câble pour choisir son phénomène et glissez-le pour son intensité."),
            ("Gestes et scènes", "OUVRIR, DÉCHIRER, RÉPARER, STUTTER, DÉCALER, HOLD et PANIC SAFE agissent sur l'étoile en focus. Quatre scènes et le PARCOURS suivent la forme."),
            ("Le mixeur et la bibliothèque", "Un stem par étoile, avec présence, niveau, mute et solo ; la VARIATION laisse le ciel allumer et éteindre des étoiles. La BIBLIOTHÈQUE échange ou épingle l'étoile d'un corps."),
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
        "tutorial": "Le bouton TUTORIEL de l'app explique tout pas à pas, en français, anglais, portugais ou espagnol.",
        "s_dl": "Télécharger",
        "dl_pending": "La première version (0.2.0, aperçu) est en préparation : les paquets Windows, macOS et Linux seront construits par l'intégration continue. D'ici là, l'instrument peut être compilé depuis les sources.",
        "dl_intro": "La version 0.2.0 est la première version publique. L'instrument reste ouvert, en développement. Choisissez le paquet de votre système ; les étapes sont dans INSTALL.",
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
        "state": "En développement, ouvert. Compilé et testé sous Linux (37 tests) et joué par l'auteur sur du vrai matériel ; les paquets Windows et macOS n'ont pas encore été construits.",
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
        "pill": "Versión 0.2.0 — en preparación" if not PUBLICADO else "Versión 0.2.0 — primera versión pública, para Windows, macOS y Linux",
        "lead": "Asterismus es un instrumento en vivo en el que cada módulo de síntesis es una estrella. Las estrellas se relacionan, se mueven por física, cuentan sus propios ciclos y tocan. Una seed propone el cielo; el músico lo toca, cambia estrellas, dibuja relaciones y graba la pieza.",
        "s_shot": "La interfaz",
        "shot_alt": "Asterismus tocando: el Eje de fuerzas a la izquierda, la Constelación en el centro con las estrellas movidas por la física y los pares en foco, el mezclador de las ocho estrellas a la derecha, la forma y el tempo que respira en la cabecera.",
        "shot_cap": "A la izquierda, el Eje de fuerzas y los macros; en el centro, la Constelación — las estrellas, los cables y los pares en foco de la trama; a la derecha, el mezclador con un stem por estrella. En la cabecera, la forma, el tempo que respira y la grabación.",
        "s_sky": "El cielo",
        "sky": [
            ("Estrellas y seed", "28 estrellas de tono, textura, voz y pulso, y cuatro estrellas de control, que no suenan pero filtran cómo tocan las otras. La seed elige ocho de ellas, una tonalidad y un modo, una forma y los ciclos: cada seed es otra pieza."),
            ("La trama", "En cada compás el cielo elige pares en foco y lo que hace cada par: respuesta, contagio, sombra, hoquetus, pedal, fricción, cadencia. La pieza recorre apertura, desarrollo, contraste y retorno, y termina en una coda."),
            ("El tiempo que respira", "Un tempo elástico, un swing que viene de una órbita, compases agrupados en 3+3+2, una estrella que deriva contra el pulso, eclipses traídos por lunas calculadas."),
            ("Movimiento físico", "Gravedad, resortes, péndulos, billar, magnetismo, efecto y espuma. Los choques fuertes suenan, y cada estrella suena en el estéreo donde está en la Constelación."),
        ],
        "s_play": "Tocar",
        "play": [
            ("La Constelación", "Haga clic en una estrella para ponerla en foco y arrástrela para cambiar su lugar; haga clic en un cable para elegir su fenómeno y arrástrelo para su intensidad."),
            ("Gestos y escenas", "ABRIR, RASGAR, REPARAR, STUTTER, DESPLAZAR, HOLD y PANIC SAFE actúan sobre la estrella en foco. Cuatro escenas y el RECORRIDO siguen la forma."),
            ("El mezclador y la biblioteca", "Un stem por estrella, con presencia, nivel, mute y solo; la VARIACIÓN deja que el cielo encienda y apague estrellas. La BIBLIOTECA cambia o fija la estrella de un cuerpo."),
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
        "tutorial": "El botón TUTORIAL de la app lo explica todo paso a paso, en español, inglés, portugués o francés.",
        "s_dl": "Descargar",
        "dl_pending": "La primera versión (0.2.0, vista previa) está en preparación: los paquetes para Windows, macOS y Linux se construirán en la integración continua. Mientras tanto, el instrumento puede compilarse desde el código.",
        "dl_intro": "La versión 0.2.0 es la primera versión pública. El instrumento sigue abierto, en desarrollo. Elige el paquete de tu sistema; los pasos están en INSTALL.",
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
        "state": "En desarrollo, abierto. Compilado y probado en Linux (37 pruebas) y tocado por el autor en hardware real; los paquetes de Windows y macOS todavía no se construyeron.",
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
    langs = "\n".join('      <a href="%s"%s>%s</a>' % (f, ' aria-current="true"' if f == arquivo else "", c) for f, _, c in PAGINAS)
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
<link rel="icon" href="assets/identity/rasgo_logo_2026.svg" type="image/svg+xml" />
<link rel="stylesheet" href="styles.css" />
<script src="assets/contact.js" defer></script>
</head>
<body>
<div class="rasgo-strip"><a href="{PORTAL}">{e(t["strip"])}</a></div>
<a class="skip-link" href="#conteudo">{e(t["skip"])}</a>

<div class="site-header">
  <a class="brand" href="#top" aria-label="Rasgo Synth Asterismus">
    <img src="assets/identity/rasgo_logo_2026.svg" alt="" height="26" />
    <span>RASGO<b>SYNTH</b> · ASTERISMUS</span>
  </a>
  <nav class="lang-switch" aria-label="{a(t["langnav"])}">
{langs}
  </nav>
</div>

<main id="conteudo">
<section class="hero" id="top">
  <span class="status-pill">{e(t["pill"])}</span>
  <h1>Rasgo <b>Synth</b> Asterismus</h1>
  <p class="lead">{e(t["lead"])}</p>
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

<footer class="site-footer">
  <span>© Lúcio de Araújo · Rasgo Synth Asterismus 2026 · RASGO</span>
  <nav>
    <a href="{REPO}">GitHub</a> ·
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


def main():
    for arquivo, lang, _ in PAGINAS:
        (AQUI / arquivo).write_text(pagina(arquivo, lang), encoding="utf-8")
    print("gerar_paginas.py: %d páginas%s" % (len(PAGINAS), "" if PUBLICADO else " (em preparação, noindex)"))


if __name__ == "__main__":
    main()
