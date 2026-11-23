# MICHEL DRUM LAB

Instrumento VST3 (sampler / caja de ritmos de 16 pads) para **FL Studio 26** · Windows 11 x64 · C++17 · JUCE 8 · CMake.

## Estado: ETAPA 1 (estructura + audio real)
- 16 pads con samples reales integrados dentro del VST3 (45 WAV incluidos, sin carpetas externas).
- MIDI: cada pad tiene su nota (por defecto pad 1 = 36 ... pad 16 = 51), velocity, varios pads a la vez.
- Por pad: Volume, Pitch, Pan, Attack, Decay, Filter, Reverb, Saturation, Mute, Solo, nota MIDI.
- Limitador suave en la salida (no satura) + medidor.
- Kits y presets guardados en `%APPDATA%\MichelDrumLab\` y estado dentro del proyecto de FL Studio.

## Compilar en tu PC
Requisitos: Visual Studio 2022 (carga "Desarrollo de escritorio con C++"), CMake 3.22+, Git.

```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target MichelDrumLab_VST3
```
El `.vst3` queda en `build\MichelDrumLab_artefacts\Release\VST3\MICHEL DRUM LAB.vst3`.
Copialo a `C:\Program Files\Common Files\VST3\` y en FL Studio: *Options > Manage plugins > Find plugins*.

## Compilar en GitHub (sin instalar nada)
Cada `push` a `main` ejecuta `.github/workflows/build.yml` y publica el VST3 como *artifact*
(pestana **Actions** > ultima ejecucion > `MICHEL-DRUM-LAB-VST3`).

## Anadir mas samples
```
python tools/import_samples.py ruta/NuevosSamples.zip
```
Se copian a `Assets/Samples/<CATEGORIA>/<CATEGORIA>-<Nombre>.wav` y se incrustan al recompilar.
Categorias: 808, KICKS, SNARES, CLAPS, HIHATS, OPEN_HATS, ROLLS, PERCUSSIONS, RIMS, VOCALS, FX.

## Estructura
Ver [docs/ARQUITECTURA.md](docs/ARQUITECTURA.md).
