#pragma once

// Framebuffer pixel scale factor.  Set once at startup from the saved "Scale"
// preference (default 2).  All primitive drawing methods in RasterDrawMethods
// multiply logical coordinates by this factor before writing to the native
// pixel buffer, so the window is always WINDOWW*PIXEL_SCALE x WINDOWH*PIXEL_SCALE
// pixels with no SDL renderer upscaling involved.  TrueType glyphs are rendered
// at TTF_PIXEL_HEIGHT*PIXEL_SCALE and placed at exact native pixel positions, so
// UI text is always crisp at whatever display size the user chose.
extern int PIXEL_SCALE;
