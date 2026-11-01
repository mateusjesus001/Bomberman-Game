# Bomberman 8-bit em C

Jogo simples inspirado no clássico **Bomberman** do Nintendo 8-bits, escrito inteiramente em **C puro** usando a biblioteca **SDL2** para janela, renderização 2D, áudio e entrada de teclado.

---

## 📋 Visão Geral

O jogador navega por um mapa baseado em grade, solta bombas para destruir blocos quebráveis e inimigos, coleta power-ups e tenta alcançar a porta de saída para avançar de fase. O jogo termina quando o jogador perde todas as vidas (Game Over) ou quando todas as fases são completadas (Vitória).

### Recursos principais

- **3 estágios** com layouts diferentes de paredes fixas e blocos quebráveis
- **Inimigos com IA diferente**: *Balloon* (lento, vaga aleatoriamente), *Onion* (persegue o jogador quando próximo), *Doll* (rápido e errático)
- **Sistema de bombas** com temporizador, explosão em cruz, reação em cadeia (bomba explode bomba) e detonação remota (power-up Detonator)
- **7 power-ups** coletíveis ao destruir blocos: Extra Bomb, Fire Up, Speed Up, Wall Pass, Bomb Pass, Flame Pass, Detonator
- **HUD na tela** com vidas, bombas, alcance do fogo, pontuação, tempo restante e estágio atual
- **Tela de menu**, vitória e game over
- **Efeitos sonoros processuais** gerados via código (sem dependência de arquivos de áudio externos para SFX)
- **Música de fundo** em WAV (loop infinito)
- **Sprites BMP** para jogador, bomba e inimigos (com transparência por chave de cor magenta)

---


## 🛠️ Tecnologias Usadas

| Componente | Tecnologia | Finalidade |
|------------|-----------|------------|
| **Linguagem** | C (C11) | Lógica do jogo, física, IA, renderização |
| **Biblioteca multimídia** | SDL2 (Simple DirectMedia Layer) | Janela, renderização 2D acelerada, áudio, input de teclado |
| **Compilador** | MinGW GCC | Compilação no Windows |
| **Build** | Script `.bat` (Windows) | Automatiza a compilação e cópia da `SDL2.dll` |
| **Assets** | BMP (sprites), WAV (música) | Imagens do jogador, bomba, inimigo e trilha sonora |
| **Ferramenta auxiliar** | Python 3 (`convert_png_to_bmp.py`) | Converte PNGs para BMP 32-bit com alfa, usado pelo engine |
| **Editor/IDE** | VS Code | Configuração do IntelliSense para C/C++ (`c_cpp_properties.json`) |

### Por que SDL2?

SDL2 é uma camada de abstração multiplataforma que oferece:

- Criação de janela e contexto gráfico acelerado por hardware (`SDL_Renderer`)
- Tratamento de eventos de teclado (`SDL_Event`, `SDL_GetKeyboardState`)
- Mixagem de áudio de baixa latência via callback (`SDL_AudioDeviceID`)
- Carregamento de imagens BMP nativas (`SDL_LoadBMP`)

---

## 📁 Estrutura do Projeto

```
Primeiro Game/
├── bomberman.exe          # Executável pré-compilado (Windows)
├── SDL2.dll               # Biblioteca SDL2 necessária para rodar
├── build.bat              # Script de compilação Windows (MinGW)
├── README.md              # Este arquivo
├── main.c                 # Ponto de entrada e loop principal do jogo
├── assets/                # Recursos do jogo
│   ├── player.bmp         # Sprite do jogador
│   ├── bomb.bmp           # Sprite da bomba
│   ├── enemy.bmp          # Sprite dos inimigos
│   └── music.wav          # Música de fundo
├── src/                   # Código-fonte organizado por módulos
│   ├── bomber.h           # Constantes globais, tipos e helpers de grade
│   ├── map.c / map.h      # Layout dos estágios, blocos, colisão com cenário
│   ├── player.c / player.h# Jogador, movimento, power-ups, estatísticas
│   ├── bomb.c / bomb.h    # Sistema de bombas (pool), temporizador, detonação
│   ├── explosion.c / explosion.h # Explosões em cruz, reação em cadeia
│   ├── enemy.c / enemy.h  # Inimigos, IA (wander/chase/erratic)
│   ├── item.c / item.h    # Power-ups spawn, coleta, blink visual
│   ├── hud.c / hud.h      # Interface na tela, fonte bitmap 5x7
│   ├── render.c / render.h# Renderização de mapa, entidades, partículas, porta
│   └── audio.c / audio.h  # Áudio procedural (SFX) + música WAV
└── tools/
    └── convert_png_to_bmp.py  # Conversor de PNG para BMP 32-bit
```

### Organização dos Módulos

Cada módulo segue o padrão **`.c` + `.h`**, com responsabilidade única:

- `bomber.h` — Cabeçalho central com constantes (`WINDOW_W`, `TILE_SIZE`, `MAX_ENEMIES`, etc.), enum de itens, direções e struct `Entity` compartilhada por todos.
- `map.c` — Define os layouts fixos dos estágios, espalha blocos quebráveis aleatoriamente e gerencia colisão com paredes/blocos.
- `player.c` — Movimento do jogador com snapping em grade, sistema de vidas e aplicação de 7 tipos de power-ups.
- `bomb.c` — Pool de bombas com temporizador (2 segundos estilo NES), detonação remota e passagem através de bombas recém-colocadas.
- `explosion.c` — Explosão em cruz com raio configurável, destrói blocos e propaga reação em cadeia a outras bombas.
- `enemy.c` — Três tipos de inimigos com comportamentos diferentes (vagar, perseguir, errático) e animação de morte.
- `item.c` — Spawn aleatório ponderado de power-ups ao destruir blocos.
- `hud.c` — Desenho da interface com fonte bitmap 5x7 feita à mão.
- `render.c` — Renderização de todos os elementos (mapa, jogador, bombas, explosões, itens, porta) com sprites ou fallback por cores.
- `audio.c` — Efeitos sonoros gerados proceduralmente (senóides) e música de fundo WAV com loop.

---

## 🚀 Como Rodar

### Opção 1: Executável já compilado (Windows)

Se você não quer compilar, basta usar o executável já incluso:

1. Certifique-se de que o arquivo **`SDL2.dll`** está na mesma pasta que o `bomberman.exe` (já está incluído no projeto).
2. Dê um duplo clique em **`bomberman.exe`**.
3. O jogo abrirá em janela centralizada (640 × 568 px).

> ⚠️ Se o jogo não abrir e reclamar de `SDL2.dll`, verifique se a DLL está ao lado do executável.

### Opção 2: Compilar do zero

#### No Windows (MinGW + SDL2)

Pré-requisitos:
- [MinGW-w64](https://www.mingw-w64.org/) com `gcc` no PATH
- [SDL2 development libraries](https://www.libsdl.org/download-2.0.php) para Windows

O projeto já inclui um script de build:

```bat
build.bat
```

O que o script faz:
1. Compila todos os arquivos `.c` de `src/` junto com `main.c`
2. Vincula contra `SDL2main` e `SDL2`
3. Gera `bomberman.exe`
4. Copia `SDL2.dll` da pasta do MinGW para ao lado do executável (se ainda não estiver lá)

Caminhos esperados pelo `build.bat` (ajuste se o seu MinGW estiver em outro lugar):

```
C:\msys64\mingw64\include\SDL2
C:\msys64\mingw64\lib
```

Compilação manual (exemplo):

```cmd
gcc -o bomberman.exe main.c src\map.c src\player.c src\bomb.c src\explosion.c ^
    src\item.c src\enemy.c src\hud.c src\audio.c src\render.c src\game.c ^
    -I"C:\msys64\mingw64\include\SDL2" -I"C:\msys64\mingw64\include" ^
    -L"C:\msys64\mingw64\lib" -lmingw32 -lSDL2main -lSDL2
```

#### No Linux

Pré-requisitos:

```bash
sudo apt install libsdl2-dev gcc make
```

Compilação:

```bash
gcc -o bomberman main.c src/map.c src/player.c src/bomb.c src/explosion.c \
    src/item.c src/enemy.c src/hud.c src/audio.c src/render.c src/game.c \
    $(sdl2-config --cflags --libs)
```

Execução:

```bash
./bomberman
```

> No Linux, certifique-se de que o `assets/` está acessível a partir do diretório onde você roda o executável (sprites BMP e música WAV são carregados com caminhos relativos).

---

## 🎮 Como Jogar

| Tecla | Ação |
|-------|------|
| `Enter` | Iniciar o jogo no menu / Reiniciar após Vitória ou Game Over |
| `Setas direcionais` ou `WASD` | Mover o jogador |
| `Espaço` / `Z` / `X` / `C` / `Ctrl` | Soltar bomba |
| `Enter` / `E` | Detonar bombas remotamente (requer power-up Detonator) |
| `Esc` | Sair do jogo |
| `Clique do mouse` | Clicar no botão START no menu |

### Objetivo

- Destrua **todos os inimigos** da fase para liberar a porta de saída.
- Entre na porta para avançar para o próximo estágio.
- Colete **power-ups** ao destruir blocos quebráveis para ficar mais forte.
- Cuidado: se uma explosão atingir o jogador, você perde uma vida. Se acabar as vidas, é Game Over.

---

## 🔊 Áudio

- **SFX (efeitos sonoros):** Gerados processualmente via síntese de senóide (senóide com decay de amplitude). Não requerem arquivos externos.
  - Movimento, colocação de bomba, explosão, item, morte, vitória, game over, menu.
- **Música de fundo:** Carregada de `assets/music.wav` e reproduzida em loop. Caso o arquivo não seja encontrado, o jogo continua sem música (sem travar).

## 🛠️ Ferramenta de Conversão de Sprites

O engine carrega sprites em **BMP 32-bit com alfa**. O script em `tools/convert_png_to_bmp.py` converte arquivos PNG (com transparência) para esse formato usando apenas a biblioteca padrão do Python:

```bash
python tools/convert_png_to_bmp.py
```

Mapeamento atual:
- `protagonist.png` → `assets/player.bmp`
- `bomb.png` → `assets/bomb.bmp`
- `enemy.png` → `assets/enemy.bmp`

## 🎨 Estilo Visual

O jogo usa uma paleta inspirada no NES com cores definidas diretamente no código (`render.c`) como fallback quando os sprites BMP não carregam. Os elementos são desenhados em uma grade de 15 × 13 tiles de 40 × 40 px, com uma HUD de 48 px no topo.

## 📜 Licença / Uso

Este é um projeto educacional de aprendizado de desenvolvimento de jogos em C. Sinta-se livre para estudar, modificar e expandir.

## 🔧 Possíveis Melhorias Futuras

- Adicionar mais estágios e layouts
- Suporte a controle de gamepad (SDL GameController)
- Partículas de explosão mais detalhadas
- Sons em WAV/OGG para SFX em vez de síntese procedural
- Tela de seleção de fase e placar de recordes (high score)
- Suporte a telas cheia e redimensionável
