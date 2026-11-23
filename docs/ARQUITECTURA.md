# Arquitectura

```
MichelDrumLab/
├─ CMakeLists.txt            JUCE (FetchContent 8.0.6) + BinaryData + plugin VST3
├─ Assets/Samples/<CAT>/     WAV -> se incrustan en el binario (BinaryData)
├─ tools/import_samples.py   organiza ZIP/carpetas de samples
├─ .github/workflows/        compilacion automatica en Windows
└─ Source/
   ├─ PluginProcessor  (AudioProcessor)  parametros, MIDI sample-accurate, estado
   ├─ PluginEditor     (AudioEditor)     interfaz negro/amarillo, 16 pads, knobs
   ├─ SampleLibrary    catalogo de samples integrados (categoria/nombre/id estable)
   ├─ SampleManager    decodifica los WAV a memoria al cargar (inmutable luego)
   ├─ PadEngine        4 voces por pad, pitch (interp. lineal), attack, decay
   ├─ MIDIManager      nota MIDI -> pads (mascara de bits)
   ├─ MixerEngine      filtro, saturacion, volumen, pan, reverb send, limitador
   ├─ KitManager       kits (16 samples + params) en %APPDATA%\MichelDrumLab\Kits
   └─ PresetManager    guardado/lectura XML de kits y presets
```

## Reglas de tiempo real
- Sin `new`/`malloc`/locks en `processBlock`: buffers preasignados en `prepareToPlay`.
- Samples decodificados una vez y nunca modificados -> lectura sin bloqueos desde audio.
- UI -> audio mediante atomics (`uiTrigger`, parametros APVTS).

## Biblioteca grande de samples
Etapa 1: todo decodificado en RAM (45 WAV ~ 19 MB). Para bibliotecas de cientos de MB: convertir a 16 bit /
comprimir (FLAC) en el import, y cargar bajo demanda en un hilo aparte con doble buffer.

## Hoja de ruta
2. Choke groups (hi-hat cerrado corta al abierto), modo mono para 808, modo loop.
3. Biblioteca con busqueda/preescucha y samples de usuario.
4. Kits de fabrica extra (COLOMBIAN DRILL, REGGAETON, AFROBEAT) cuando tengamos los sonidos.
5. Salidas multiples por pad, mejoras visuales.
