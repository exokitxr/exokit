<h1 align="center">Exokit</h1>
<p align="center"><img width="300" height="300" alt="Exokit" src="assets/icon.png"/></p>
<p align="center"><b>:dark_sunglasses: Native VR and AR engine for JavaScript 🦖</b></p>

<p align="center">
  <a href="https://github.com/exokitxr/exokit/releases"><img src="https://img.shields.io/github/downloads/exokitxr/exokit/total.svg"></a>
  <a href="https://www.npmjs.com/package/exokit"><img src="https://img.shields.io/npm/v/exokit.svg"></a>
  <a href="https://travis-ci.org/modulesio/exokit-windows"><img src="https://travis-ci.org/modulesio/exokit-windows.svg?branch=master"></a>
  <a href="https://ci.appveyor.com/project/modulesio/exokit-windows"><img src="https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true"></a>  
  <a href="https://twitter.com/exokitxr"><img src="https://img.shields.io/twitter/follow/exokitxr.svg?style=social"></a>
</p>

<div align="center">
  <a href="https://discord.gg/9M8awV8">Discord</a>
  &mdash;
  <a href="https://twitter.com/exokitxr">Twitter</a>
  &mdash;
  <a href="http://eepurl.com/dFiLMz">Email List</a>
</div>

## Examples

<a href="https://youtu.be/cd_DEwCDF6U"><img alt="Hands Reality Tab" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507781-0e463300-561e-11e9-9b1a-f43b8259d041.gif" height="190" width="32%"></a>
<a href="https://youtu.be/b-UKSg0QCRE"><img alt="Live Reload Magic Leap" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507118-a216ff80-561c-11e9-829e-74d8244571c3.gif" height="190" width="32%"></a>
<a href="https://youtu.be/O1xA1r5SZUM"><img alt="Tutorial Reality Tab" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507125-a3e0c300-561c-11e9-835f-3a26a9e879b5.gif" height="190" width="32%"></a>

<a href="https://www.youtube.com/watch?v=m_QntqZmd_Q"><img alt="Reality Projection with HTC Vive and Magic Leap" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507271-e60a0480-561c-11e9-87ad-7dc736ba0760.gif" height="190" width="32%"></a>
<a href="https://youtu.be/i0MXRCNkdB4"><img alt="Emukit" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507623-a8f24200-561d-11e9-97a3-194b6b4a1d8b.gif" height="190" width="32%"></a>
<img alt="Various Exokit Apps" target="_blank" src="https://user-images.githubusercontent.com/29695350/55506701-ba3a4f00-561b-11e9-9e19-ba808bed7c5a.gif" height="190" width="32%">

*Find more examples [here](https://github.com/exokitxr/exokit/tree/master/examples) and on [YouTube](https://www.youtube.com/channel/UC87Q7_5ooY8FSLwOec52ZPQ).*


## Overview

This project **enables developers to build XR experiences using the same code that runs on the web**. Exokit engine is written on top of Node and emulates a web browser, providing native hooks for WebGL, WebXR, WebAudio, and other APIs used in immersive experiences.

:eyeglasses: **Exokit currently targets the following platforms**:
* OpenVR Desktop VR (Steam compatible)
* Oculus Desktop (Oculus Rift/Rift S)
* Oculus Mobile (Oculus Quest/Go, GearVR)
* Magic Leap
* iOS ARKit *
* Android ARCore *
* Google VR (Daydream / Cardboard / Mirage Solo) *
* any XR device, start a [pull request](https://github.com/exokitxr/exokit/compare) with a native binding if it isn't listed here! *

\* not supported yet

:electric_plug: **Since Exokit supports anything that runs on the web, it powers experiences built with**:
* Three.js
* Unity
* Pixi.js
* Babylon.js
* A-Frame
* Custom WebGL frameworks
* WebAssembly, TypeScript, and any language that transpiles to JavaScript

:thumbsup: **Why Exokit?**

- You want your WebGL/WebXR to run fast.
- You want the hot new web APIs.
- You want to add your own integrations -- including native -- into a browser environment.
- You want a lightweight browser as a hackable node module.
- You want to combine the web with a 3D engine like Unity.

:thumbsdown: **Why not Exokit?**

- You're looking for a traditional "web browser".
- You don't care about 3D or mixed reality.
- You're looking for strict and/or legacy standards support.

:book: **Manifesto**

- The future is immersive. The web is the best application platform. Javascript is the best ecosystem.
- Content should be hardware agnostic. Tomorrow will have different hardware. VR and AR should be compatible.
- It's not possible to do both 2D and 3D well. We don't do 2D. We can use an external 2D browser.
- Use your favorite game engine. Exokit is not a game engine.
- Legacy browser design choices don't make sense in XR.
- Exokit empowers and connect apps, even (especially) if they aren't designed to cooperate.
- Apps should run in "reality tabs", layers of reality that blend together.



## Quickstart

### Desktop
<h4><a href="https://unavailable">Download for current OS</a></h4>

#### Run a WebXR site (desktop)

```sh
exokit https://aframe.io/a-painter/ # start A-Painter in Exokit
```

### Magic Leap

<h4><a href="https://unavailable/magicleap">Download for Magic Leap</a></h4>

#### Run (Magic Leap device)

```sh
mldb connect 192.168.0.10:1131 # mldb needs to be connected; see MLDB documentation
mldb install -u exokit.mpk # downloaded or built package
mldb launch com.webmr.exokit -v "ARGS=node . file:///package/examples/hello_ml.html" # or URL to load
mldb log exokit:*
```

## Architecture

Exokit is a Javascript [Node.js](https://nodejs.org) module.

C++ bindings hook into WebGL, WebXR, Magic Leap, Leap Motion, and various other device APIs.

`const {window} = require('exokit')()` to get an immersive browser in another project.

Exokit runs on Windows, macOS, Linux (x64), Linux (ARM64), and Magic Leap (ARM64).

## Web API support

- HTTP/S
- HTML5
- `<script>`
- DOM
- WebGL
- WebXR
- WebVR
- WebRTC
- Canvas2D
- WebSocket
- Web Workers
- `<img>`, `<audio>`, `<video>`
- WebAudio
- Keyboard/Mouse events
- Gamepad API
- `<iframe>`
- ~~HTML layout~~
- ~~HTML rendering~~
- ~~CSS~~
- ~~Legacy APIs~~

## Hardware bindings

- OpenGL
- OpenVR (Steam VR)
- Oculus 
- Oculus Mobile 
- Magic Leap
- Leap Motion

## Local Development
See full building instructions in [BUILDING.md](https://github.com/exokitxr/exokit/blob/master/BUILDING.md).

```sh
git clone https://github.com/exokitxr/exokit.git
cd exokit
npm install
node . <url> # or node . -h for home
```

## Debugging

Uses [ndb](https://github.com/GoogleChromeLabs/ndb).

```sh
npm run debug
```

Then in the console, input:

```js
let window = await require('./src/').load(yourUrl);
```

Now you have a handle on the window object as you test your application, and
you can set `debugger` breakpoints, inspect memory, profile CPU, etc.

## Flags

- `--xr webvr` Makes exokit WebVR mode regardless of the webpage. If exokit is always opening in pancake mode you can use this to try to fix that.

## Stay in Touch

- [Join our Discord](https://discord.gg/Apk6cZN) for hanging out.
- [Follow @exokitxr on Twitter](https://twitter.com/exokitxr) for updates.


## Contributing

Get involved! Check out the [Contributing Guide](CONTRIBUTING.md) for how to get started.

## License

This program is free software and is distributed under an [MIT License](LICENSE.md).


## 🌐 Web Resources & Interactive Index
- [SOLVE THE CUBE WOODEN BLOCKS 2D](https://themindzone.pages.dev/solve-the-cube-wooden-blocks-2d.html)
- [CATEGORY RACING DRIVING 2](https://quizverses-9d2f2.web.app/category-racing-driving-2.html)
- [CATEGORY JIGSAW](https://studyplayings.pages.dev/category-jigsaw.html)
- [NUTS BOLTS WOOD PUZZLE GAME](https://studyplayings.web.app/nuts-bolts-wood-puzzle-game.html)
- [ROYAL JEWELS MATCH](https://studyquests.pages.dev/royal-jewels-match.html)
- [OBBY YARD SALE](https://quizverses.github.io/obby-yard-sale.html)
- [CRAZY BUNNIES](https://quizverses.github.io/crazy-bunnies.html)
- [SNIPER VS SNIPER](https://studyplaying.github.io/sniper-vs-sniper.html)
- [NAUTILUS SPACESHIP ESCAPE](https://studyplaying.github.io/nautilus-spaceship-escape.html)
- [BOOM LAND LITE](https://quizverses.pages.dev/boom-land-lite.html)
- [CATEGORY GROW GAMES](https://learnquester.github.io/category-grow-games.html)
- [HALLOWEEN FRUIT SLICE](https://studyplaying.github.io/halloween-fruit-slice.html)
- [INDEX22](https://studyplaying.github.io/index22.html)
- [INDEX19](https://quizverses.pages.dev/index19.html)
- [CATEGORY BATTLE ROYALE25](https://studyquesthub.web.app/category-battle-royale25.html)
- [PEG SOLITAIRE](https://studyplaying.github.io/peg-solitaire.html)
- [STICKMAN ARCHERO FIGHT STICK SHADOW FIGHT WAR](https://thelearnquester.web.app/stickman-archero-fight-stick-shadow-fight-war.html)
- [AQUA SORT WATER COLOR PUZZLE](https://studyplaying.github.io/aqua-sort-water-color-puzzle.html)
- [BANG BANG MAHJONG](https://thelearnquester.web.app/bang-bang-mahjong.html)
- [FRUIT CATCHER](https://studyquests.github.io/fruit-catcher.html)
- [RIDDLEMATH](https://studyplaying.github.io/riddlemath.html)
- [CATEGORY GOGUARDIAN](https://quizverses.pages.dev/category-goguardian.html)
- [AQUA SORT WATER COLOR PUZZLE](https://quizverses-9d2f2.web.app/aqua-sort-water-color-puzzle.html)
- [ASMR PET TREATMENT](https://studyplaying.github.io/asmr-pet-treatment.html)
- [TREASURE CHAMPION CHEST CAPTURE](https://learnquester.github.io/treasure-champion-chest-capture.html)
- [OBBY POGO PARKOUR](https://learnquester.github.io/obby-pogo-parkour.html)
- [CATEGORY OBBY56](https://quizverses.pages.dev/category-obby56.html)
- [SLIPPERY DRIFT RACING](https://quizverses-9d2f2.web.app/slippery-drift-racing.html)
- [PET DOCTOR BUSINESS TYCOON PET CARE GAME](https://thelearnquester.web.app/pet-doctor-business-tycoon-pet-care-game.html)
- [ARROW HIT](https://studyquests.github.io/arrow-hit.html)
- [INDEX18](https://quizverses.pages.dev/index18.html)
- [GUIVOIO](https://quizverses-9d2f2.web.app/guivoio.html)
- [COLOR SORT IMPOSTOR EDITION](https://studyplaying.github.io/color-sort-impostor-edition.html)
- [CATEGORY MANAGEMENT209](https://quizverses.pages.dev/category-management209.html)
- [CATEGORY CASUAL 5](https://learnquester.github.io/category-casual-5.html)
- [MATCHING PUZZLE](https://studyplaying.github.io/matching-puzzle.html)
- [ROYAL JEWELS MATCH](https://studyplaying.github.io/royal-jewels-match.html)
- [CATEGORY STICKMAN 2](https://quizverses.pages.dev/category-stickman-2.html)
- [V AND N PIZZA COOKING GAME](https://studyquests.github.io/v-and-n-pizza-cooking-game.html)
- [ZOMBIE SURVIVAL](https://studyplayings.pages.dev/zombie-survival.html)
- [CATEGORY COLLECT565](https://learnquester.github.io/category-collect565.html)
- [SPOOKY HALLOWEEN HIDDEN PUMPKIN](https://studyquests.github.io/spooky-halloween-hidden-pumpkin.html)
- [CATEGORY SCHOOL UNBLOCKER](https://quizverses.pages.dev/category-school-unblocker.html)
- [CATEGORY TITANIUMNETWORK PROXIES](https://quizverses.pages.dev/category-titaniumnetwork-proxies.html)
- [SITEMAP](https://studyplayings.web.app/sitemap.html)
- [INDEX32](https://studyplaying.github.io/index32.html)
- [CATEGORY ART](https://quizverses.pages.dev/category-art.html)
- [ANGRY PLANTS FLOWER](https://studyplaying.github.io/angry-plants-flower.html)
- [GRADUATION MAKEUP TRENDS](https://quizverses-9d2f2.web.app/graduation-makeup-trends.html)
- [SHAPE TRANSFORM BLOB RACING](https://studyquests.github.io/shape-transform-blob-racing.html)
- [HIGHSCHOOL MEAN GIRLS 3](https://studyquests.github.io/highschool-mean-girls-3.html)
- [MINI GOLF BATTLE](https://studyplaying.github.io/mini-golf-battle.html)
- [STICK KILL 3D](https://quizverses-9d2f2.web.app/stick-kill-3d.html)
- [DRIVERZ ED](https://studyplaying.github.io/driverz-ed.html)
- [CATEGORY PROXIES](https://learnquester.github.io/category-proxies.html)
- [MAGIC TRI PEAKS SOLITAIRE](https://studyplaying.github.io/magic-tri-peaks-solitaire.html)
- [TRALALA LA LA LA ITALIANO](https://thelearnquester.web.app/tralala-la-la-la-italiano.html)
- [ZOMBIE OUTBREAK SURVIVE](https://studyplaying.github.io/zombie-outbreak-survive.html)
- [SISYPHUS SIMULATOR](https://studyquests.github.io/sisyphus-simulator.html)
- [CATEGORY GAMES](https://learnquester.github.io/category-games.html)
- [ULTIMATE SPORTS CAR DRIFT](https://studyquests.github.io/ultimate-sports-car-drift.html)
- [GIRL COLORING DRESS UP GAMES](https://quizverses-9d2f2.web.app/girl-coloring-dress-up-games.html)
- [WORDS FROM WORDS](https://quizverses-9d2f2.web.app/words-from-words.html)
- [MAHJONG PET QUEST](https://quizverses.pages.dev/mahjong-pet-quest.html)
- [AGENT HUNT SPY SHOOTER GAME](https://studyquests.github.io/agent-hunt-spy-shooter-game.html)
- [SNIPER FOR BRAINROT](https://quizverses.pages.dev/sniper-for-brainrot.html)
- [CATEGORY FLASH 2](https://quizverses.pages.dev/category-flash-2.html)
- [ROAD OF FURY 4](https://studyquests.pages.dev/road-of-fury-4.html)
- [DIG OUT OF PRISON](https://thelearnquester.web.app/dig-out-of-prison.html)
- [CAT FOOTBALL](https://studyplaying.github.io/cat-football.html)
- [CLEAN THE OCEAN](https://quizverses-9d2f2.web.app/clean-the-ocean.html)
- [BEAUTY WORLD AND FASHION STYLIST](https://quizverses.pages.dev/beauty-world-and-fashion-stylist.html)
- [NUBIK CREATE YOUR PLACE](https://studyplaying.github.io/nubik-create-your-place.html)
- [CATEGORY SPACE](https://quizverses.pages.dev/category-space.html)
- [GUIVOIO](https://studyquests.github.io/guivoio.html)
- [FPS TOY REALISM](https://quizverses-9d2f2.web.app/fps-toy-realism.html)
- [BANANA FARM](https://studyquests.github.io/banana-farm.html)
- [LINE ON HOLE](https://thelearnquester.web.app/line-on-hole.html)
- [FIRE BALL AND WATER BALL PARKOUR LOVE BALLS](https://studyplaying.github.io/fire-ball-and-water-ball-parkour-love-balls.html)
- [SKYDOM](https://studyquests.github.io/skydom.html)
- [MOJO EMOJI](https://thelearnquester.web.app/mojo-emoji.html)
- [BREAK BEAT](https://thelearnquester.web.app/break-beat.html)
- [GRADUATION MAKEUP TRENDS](https://studyplaying.github.io/graduation-makeup-trends.html)
- [SQUID GAME MEMORY CARD MATCH](https://quizverses-9d2f2.web.app/squid-game-memory-card-match.html)
- [GLOVES GROW RUSH](https://studyquests.github.io/gloves-grow-rush.html)
- [CUBE TO HOLE PUZZLE](https://quizverses-9d2f2.web.app/cube-to-hole-puzzle.html)
- [HEXA TILE MASTER](https://studyplaying.github.io/hexa-tile-master.html)
- [QUIZ 10 SECONDS MATH](https://studyquests.github.io/quiz-10-seconds-math.html)
- [KICK LOSER](https://quizverses-9d2f2.web.app/kick-loser.html)
- [FRUIT MATCH JUICY PUZZLE](https://quizverses.pages.dev/fruit-match-juicy-puzzle.html)
- [HUNGRY SNAKE IO](https://studyplaying.github.io/hungry-snake-io.html)
- [MAGIC BOTTLES](https://quizverses-9d2f2.web.app/magic-bottles.html)
- [CATEGORY BALL173](https://studyplayings.web.app/category-ball173.html)
- [INDEX20](https://quizverses.pages.dev/index20.html)
- [THE PRISM CITY DETECTIVES](https://thelearnquester.web.app/the-prism-city-detectives.html)
- [IDOL LIVESTREAM DOLL DRESS UP](https://learnquester.github.io/idol-livestream-doll-dress-up.html)
- [FIRESIDE SOLITAIRE](https://studyplaying.github.io/fireside-solitaire.html)
- [BLOCKS AND THATS IT](https://studyplayings.web.app/blocks-and-thats-it.html)
- [CATEGORY MERGE](https://studyplayings.web.app/category-merge.html)
- [BIRD SORT CHALLENGES](https://quizverses-9d2f2.web.app/bird-sort-challenges.html)
- [INDEX20](https://learnquester.github.io/index20.html)
- [MINI GRAND THEFT CITY](https://studyplayings.web.app/mini-grand-theft-city.html)
- [CAKE LINK MASTER](https://studyquests.github.io/cake-link-master.html)
- [DEVIL DASH](https://thelearnquester.web.app/devil-dash.html)
- [REACH 2048](https://studyplayings.pages.dev/reach-2048.html)
- [CATEGORY UNBLOCKED](https://quizverses.pages.dev/category-unblocked.html)
- [CATEGORY FASHION105](https://studyplaying.github.io/category-fashion105.html)
- [CATEGORY BATTLE](https://studyplayings.web.app/category-battle.html)
- [CATEGORY BIKE](https://learnquester.github.io/category-bike.html)
- [WORM APPLE QUEST](https://studyplaying.github.io/worm-apple-quest.html)
- [SOKOBAN PUZZLE GAME](https://studyplayings.web.app/sokoban-puzzle-game.html)
- [FORMULA RACING GAMES CAR GAME](https://quizverses.pages.dev/formula-racing-games-car-game.html)
- [CANDY MAKER DESSERT GAMES](https://studyplaying.github.io/candy-maker-dessert-games.html)
- [CATEGORY MATCH 3](https://learnquester.github.io/category-match-3.html)
- [UNPUZZLE MASTER](https://studyplayings.web.app/unpuzzle-master.html)
- [K POP HUNTER FASHION](https://thelearnquester.web.app/k-pop-hunter-fashion.html)
- [CHRISTMAS CANDY ESCAPE 3D](https://studyquests.github.io/christmas-candy-escape-3d.html)
- [CATEGORY BATTLE523](https://learnquester.github.io/category-battle523.html)
- [CATEGORY DEEP IMMERSIVE24](https://quizverses.pages.dev/category-deep-immersive24.html)
- [CATEGORY DRESS UP97](https://thelearnquester.web.app/category-dress-up97.html)
- [SPACE CRAFT SHIP WAR](https://studyplaying.github.io/space-craft-ship-war.html)
- [FRUITE SWIPE](https://studyplaying.github.io/fruite-swipe.html)
- [AYLA WORLD PRINCESS LIFE](https://studyplaying.github.io/ayla-world-princess-life.html)
- [BLOCK CRAFT 3D](https://quizverses-9d2f2.web.app/block-craft-3d.html)
- [SINGLE STROKE ENERGY LINE PUZZLE](https://thelearnquester.web.app/single-stroke-energy-line-puzzle.html)
- [SNIPER ASSASSIN GOVERNMENT AGENT](https://studyplayings.web.app/sniper-assassin-government-agent.html)
- [ORGANIZE IT](https://studyquests.github.io/organize-it.html)
- [FOREST MATCH 4](https://studyplayings.web.app/forest-match-4.html)
- [CATEGORY SNAKE40](https://studyquests.github.io/category-snake40.html)
- [FOREST TILE MATCH](https://studyquests.pages.dev/forest-tile-match.html)
