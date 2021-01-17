# 32Blit Sprite Editor <!-- omit in toc -->

Sprite Editor is a tool that's very opinionated about making sprites *for* 32blit.

It works in 8x8 tiles but will handle sprites up to 3x3 tiles in side (24x24 pixels).

- [The Editor](#the-editor)
  - [Cursors](#cursors)
    - [Pixel Cursor](#pixel-cursor)
    - [Sprite Cursor](#sprite-cursor)
      - [Locked Sprite Cursor](#locked-sprite-cursor)
  - [Modes](#modes)
    - [Pixel Mode](#pixel-mode)
    - [Sprite Mode](#sprite-mode)
    - [Animation Mode](#animation-mode)
  - [Tools](#tools)
    - [Load/Save/Clear](#loadsaveclear)
    - [Sprite Size](#sprite-size)
    - [Mirror](#mirror)
    - [Roll / Rotate](#roll--rotate)
- [The Palette](#the-palette)

## The Editor

Central to Sprite Editor is the editor window on the left. This window - by default - shows the entire 128x128 pixel spritesheet of 16x16 tiles.

The viewport status line - above the viewport - shows the current pixel and sprite cursors.

If the viewport is active (light, solid white outline) it can be panned with the d-pad and zoomed in/out with X/Y.

The viewport has two cursors:

1. Pixel Cursor
2. Sprite Cursor

And three main edit modes:

1. Pixel Mode
2. Sprite Mode
3. Animation Mode

Plus 6 tasty tools:

1. Load
2. Save
3. Clear
4. Sprite Size
5. Sprite Mirror
6. Sprite Roll

### Cursors

#### Pixel Cursor

The Pixel Cursor is a single pixel highlight that indicates which pixel you're going to paint and the colour you're painting with.

The Pixel Cursor is only visible in "Pixel" mode, indicated by the 3x3 grid icon being red. It moves with your virtual mouse cursor, but you can use the d-pad to nudge the viewport by 1-pixel in any direction and the pixel cursor will follow under the stationary mouse cursor.

Try drawing with both the analogue stick (mouse) and the d-pad and get a feel for the different levels of precision these offer.

#### Sprite Cursor

The Sprite Cursor indicates both the size of your sprite (1x1, 2x1, 1x2 etc) and the current sprite you're targeting.

When zoomed out, the Sprite Cursor is a *white* box targeting a single sprite of your configured size. It's aligned to the 8x8 grid so you can have a mix of sprite sizes in your sheet. Move your mouse cursor to select a sprite to edit, and zoom in to start painting.

Sprite Editor will do its best to keep the sprite you've selected centered as you zoom in. Once the sprite fills the viewport, the Sprite Cursor will turn red. This is the Sprite Cursor *Lock*

##### Locked Sprite Cursor

A locked sprite cursor (indicated red) will no longer move with your mouse cursor, but it will still move if you pan the viewport.

However, a locked Sprite Cursor:

1. Is aligned to a grid at your chosen sprite size, from 8x8 to 24x24,
2. Will only change if you pan the viewport, and:
3. The Pixel Cursor has left the current sprite region.

Sprite Cursor locking is intended to give you a stable target for sprite modifiers such as Roll and Mirror. Because it does not follow your mouse, you can mouse over these icons to modify the currently selected sprite without accidentally changing the Sprite Cursor.

### Modes

Pixel Mode, Sprite Mode and Animation Mode are all mutually exclusive with each other. The active mode is indicated by a red icon.

In all modes X and Y are Zoom In/Out and the d-pad will pan the viewport if it is active.

#### Pixel Mode

Pixel Mode is the bread and butter of Sprite Editor, it's how you target and paint individual pixels. You can pick up a colour right from your edit view with A (Pink) and lay colour down with B (Yellow).

#### Sprite Mode

Sprite Mode allows you to copy and paste entire sprites.

#### Animation Mode

Animation Mode allows you to set the start and end frame of your animation, Sprite Editor will do its best to figure out how to step through the frames.

### Tools

#### Load/Save/Clear

These should be self-explanatory. Load and save files, or clear back to the initial empty viewport and default palette.

#### Sprite Size

Sprite Size or "size" allows you to configure the size of your sprite cursor, from 1x1 to 3x3 tiles (8x8 to 24x24 pixels).

The Sprite Cursor affects how sprite tools such as copy & paste, rotate, roll and mirror work and dictates the size of the preview window and animation preview.

With your mouse over the Sprite Size tool, press the d-pad Down and Right to increase the Height and Width of your sprite respectively and Up and Left to decrease them.

#### Mirror

The Mirror tool will mirror the current sprite Left/Right or Up/Down. Use the d-pad buttons to mirror the currently selected sprite.

The Mirror tool will only be active if you're zoomed in enough to have a locked (red) Sprite Cursor.

#### Roll / Rotate

The Roll/Rotate tool will roll the current sprite Left/Right or Up/Down or rotate it 90 (square) or 180 (rectangular) degrees.

Use the d-pad buttons to roll one pixel at a time. This is useful for shifting art around inside a sprite bounds for animation, or just correcting your drawing position.

Press the A button rotate clockwise.

The Roll / Rotate tool will only be active if you're zoomed in enough to have a locked (red) Sprite Cursor.

## The Palette

Many colour. Such wow. The palette is initialised with a transparent colour (top left), 15 shades of grey to white, a 4-bit colour swatch and a series of 16-colour rainbow swatches with varying brightness and saturation.

You can use the d-pad Up/Down to cycle through the Red, Green, Blue and Alpha channels of each colour and Left/Right to increase/decrease them.

TODO: The palette is very crude and needs some better tools for colour creation.