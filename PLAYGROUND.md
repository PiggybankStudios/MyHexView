# Playground

This was an old project that I used to test various functionality when I was originally trying to separate the "engine" of PCQ from the game. Ultimately this fork of the code was scrapped and we eventually did a separate pass to create Pig Engine.

## Screenshot

![Screenshot](/release/screenshot2.png)

## Controls

**Left Click**: Add point to polygon (connect a loop to finish polygon and test triangulation)

**Right Click**: Clear polygon points

**T**: Hold to retriangulate polygon 100 times every frame (test perf, use with F11)

**Control + Left/Right Click**: Move control/end point for bezier curve

**Left/Right**: Change background color using predefined colors list

**Ctrl + P**: Test rectangle packing with stb_rectpack

**G**: Test Japanese Glyph Rendering

**A**: Enable some algebra solving related test (it's calculating (a-3at+3bt+3att-6btt+3ctt-attt+3bttt-3cttt+dttt)^2)

**C**: Combine like terms (after pressing A)

**F11**: Show Debug Menu (Timing info)