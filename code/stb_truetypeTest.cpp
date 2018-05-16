
#if 0
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

unsigned char ttf_buffer[1<<25];

int main(int argc, char **argv)
{
   stbtt_fontinfo font;
   unsigned char *bitmap;
   int w,h,i,j,c = (argc > 1 ? atoi(argv[1]) : 'a'), s = (argc > 2 ? atoi(argv[2]) : 20);

   fread(ttf_buffer, 1, 1<<25, fopen(argc > 3 ? argv[3] : "c:/windows/fonts/arialbd.ttf", "rb"));

   stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
   bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), c, &w, &h, 0,0);

   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i)
         putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
      putchar('\n');
   }
   return 0;
}
#endif

#if 1
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

unsigned char buffer[24<<20];
unsigned char screen[20][79];

int main(int arg, char **argv)
{
   char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness

   fread(buffer, 1, 1000000, fopen("c:/windows/fonts/arialbd.ttf", "rb"));
   
   stbtt_fontinfo font;
   stbtt_InitFont(&font, buffer, 0);

   float scale = stbtt_ScaleForPixelHeight(&font, 15);
   int ascent;
   stbtt_GetFontVMetrics(&font, &ascent,0,0);
   int baseline;
   baseline = (int) (ascent*scale);

   int ch=0;
   float xpos = 2; // leave a little padding in case the character extends left
   while (text[ch]) {
      float x_shift = xpos - (float) floor(xpos);
      
      int advance, lsb;
      stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
      
      int x0,y0,x1,y1;
      stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
      
      stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
      // because this API is really for baking character bitmaps into textures. if you want to render
      // a sequence of characters, you really need to render each bitmap to a temp buffer, then
      // "alpha blend" that into the working buffer
      xpos += (advance * scale);
      if (text[ch+1])
         xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
      ++ch;
   }

   for (int j=0; j < 20; ++j) {
      for (int i=0; i < 78; ++i)
         putchar(" .:ioVM@"[screen[j][i]>>5]);
      putchar('\n');
   }

   return 0;
}
#endif
