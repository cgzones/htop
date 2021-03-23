#ifndef HTOP_HTOP
#define HTOP_HTOP
/*
htop - htop.h
(C) 2020 htop dev team
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/


typedef enum ColorScheme_ {
   COLORSCHEME_DEFAULT = 0,
   COLORSCHEME_MONOCHROME,
   COLORSCHEME_BLACKONWHITE,
   COLORSCHEME_LIGHTTERMINAL,
   COLORSCHEME_MIDNIGHT,
   COLORSCHEME_BLACKNIGHT,
   COLORSCHEME_BROKENGRAY,
   LAST_COLORSCHEME
} ColorScheme;

typedef enum {
   CUSTOM_METERMODE = 0,
   BAR_METERMODE,
   TEXT_METERMODE,
   GRAPH_METERMODE,
   LED_METERMODE,
   LAST_METERMODE
} MeterModeId;

#endif /* HTOP_HTOP */
