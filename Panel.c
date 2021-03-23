/*
htop - Panel.c
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "Panel.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "CRT.h"
#include "ListItem.h"
#include "Macros.h"
#include "ProvideCurses.h"
#include "RichString.h"
#include "XUtils.h"


const PanelClass Panel_class = {
   .super = {
      .extends = Class(Object),
      .delete = Panel_delete
   },
   .eventHandler = Panel_selectByTyping,
};

Panel* Panel_new(int x, int y, unsigned int w, unsigned int h, const ObjectClass* type, bool owner, FunctionBar* fuBar) {
   Panel* this;
   this = xMalloc(sizeof(Panel));
   Object_setClass(this, Class(Panel));
   Panel_init(this, x, y, w, h, type, owner, fuBar);
   return this;
}

void Panel_delete(Object* cast) {
   Panel* this = (Panel*)cast;
   Panel_done(this);
   free(this);
}

void Panel_init(Panel* this, int x, int y, unsigned int w, unsigned int h, const ObjectClass* type, bool owner, FunctionBar* fuBar) {
   this->x = x;
   this->y = y;
   this->w = w;
   this->h = h;
   this->eventHandlerState = NULL;
   this->items = Vector_new(type, owner, DEFAULT_SIZE);
   this->scrollV = 0;
   this->scrollH = 0;
   this->selected = 0;
   this->oldSelected = 0;
   this->selectedLen = 0;
   this->needsRedraw = true;
   this->wasFocus = false;
   RichString_beginAllocated(this->header);
   this->defaultBar = fuBar;
   this->currentBar = fuBar;
   this->selectionColorId = PANEL_SELECTION_FOCUS;
}

void Panel_done(Panel* this) {
   assert (this != NULL);
   free(this->eventHandlerState);
   Vector_delete(this->items);
   FunctionBar_delete(this->defaultBar);
   RichString_delete(&this->header);
}

void Panel_setSelectionColor(Panel* this, ColorElements colorId) {
   this->selectionColorId = colorId;
}

inline void Panel_setHeader(Panel* this, const char* header) {
   RichString_writeWide(&(this->header), CRT_colors[PANEL_HEADER_FOCUS], header);
   this->needsRedraw = true;
}

void Panel_move(Panel* this, int x, int y) {
   assert (this != NULL);

   this->x = x;
   this->y = y;
   this->needsRedraw = true;
}

void Panel_resize(Panel* this, int w, int h) {
   assert (this != NULL);

   this->w = CAST_UNSIGNED(w);
   this->h = CAST_UNSIGNED(h);
   this->needsRedraw = true;
}

void Panel_prune(Panel* this) {
   assert (this != NULL);

   Vector_prune(this->items);
   this->scrollV = 0;
   this->selected = 0;
   this->oldSelected = 0;
   this->needsRedraw = true;
}

void Panel_add(Panel* this, Object* o) {
   assert (this != NULL);

   Vector_add(this->items, o);
   this->needsRedraw = true;
}

void Panel_insert(Panel* this, size_t i, Object* o) {
   assert (this != NULL);

   Vector_insert(this->items, i, o);
   this->needsRedraw = true;
}

void Panel_set(Panel* this, size_t i, Object* o) {
   assert (this != NULL);

   Vector_set(this->items, i, o);
}

Object* Panel_get(Panel* this, size_t i) {
   assert (this != NULL);

   return Vector_get(this->items, i);
}

Object* Panel_remove(Panel* this, size_t i) {
   assert (this != NULL);

   this->needsRedraw = true;
   Object* removed = Vector_remove(this->items, i);
   if (this->selected > 0 && this->selected >= Vector_size(this->items)) {
      this->selected--;
   }

   return removed;
}

Object* Panel_getSelected(Panel* this) {
   assert (this != NULL);
   if (Vector_size(this->items) > 0) {
      return Vector_get(this->items, this->selected);
   } else {
      return NULL;
   }
}

void Panel_moveSelectedUp(Panel* this) {
   assert (this != NULL);

   Vector_moveUp(this->items, this->selected);
   if (this->selected > 0) {
      this->selected--;
   }
}

void Panel_moveSelectedDown(Panel* this) {
   assert (this != NULL);

   Vector_moveDown(this->items, this->selected);
   if (this->selected + 1 < Vector_size(this->items)) {
      this->selected++;
   }
}

size_t Panel_getSelectedIndex(const Panel* this) {
   assert (this != NULL);

   return this->selected;
}

size_t Panel_size(const Panel* this) {
   assert (this != NULL);

   return Vector_size(this->items);
}

void Panel_setSelected(Panel* this, size_t selected) {
   assert (this != NULL);

   size_t size = Vector_size(this->items);
   if (size == 0) {
      selected = 0;
   } else if (selected >= size) {
      selected = size - 1;
   }
   this->selected = selected;
   if (Panel_eventHandlerFn(this)) {
      Panel_eventHandler(this, EVENT_SET_SELECTED);
   }
}

void Panel_splice(Panel* this, Vector* from) {
   assert (this != NULL);
   assert (from != NULL);

   Vector_splice(this->items, from);
   this->needsRedraw = true;
}

void Panel_draw(Panel* this, bool force_redraw, bool focus, bool highlightSelected, bool hideFunctionBar) {
   assert (this != NULL);

   size_t size = Vector_size(this->items);
   size_t scrollH = this->scrollH;
   int y = this->y;
   int x = this->x;
   unsigned int h = this->h;

   if (hideFunctionBar)
      h++;

   const int header_attr = focus
                         ? CRT_colors[PANEL_HEADER_FOCUS]
                         : CRT_colors[PANEL_HEADER_UNFOCUS];
   if (force_redraw) {
      if (Panel_printHeaderFn(this))
         Panel_printHeader(this);
      else
         RichString_setAttr(&this->header, header_attr);
   }
   size_t headerLen = RichString_sizeVal(this->header);
   if (headerLen > 0) {
      attrset(header_attr);
      mvhline(y, x, ' ', CAST_INT(this->w));
      if (scrollH < headerLen) {
         RichString_printoffnVal(this->header, y, x, scrollH,
            CAST_INT(MINIMUM(headerLen - scrollH, this->w)));
      }
      attrset(CRT_colors[RESET_COLOR]);
      y++;
      h--;
   }

   // ensure scroll area is on screen
   if (this->scrollV + h > size) {
      this->scrollV = (size > h) ? (size - h) : 0;
      this->needsRedraw = true;
   }
   // ensure selection is on screen
   if (this->selected < this->scrollV) {
      this->scrollV = this->selected;
      this->needsRedraw = true;
   } else if (this->selected >= this->scrollV + h) {
      this->scrollV = this->selected - h + 1;
      this->needsRedraw = true;
   }

   const size_t first = this->scrollV;
   const size_t upTo = MINIMUM(first + h, size);

   const int selectionColor = focus
                            ? CRT_colors[this->selectionColorId]
                            : CRT_colors[PANEL_SELECTION_UNFOCUS];

   if (this->needsRedraw || force_redraw) {
      size_t line = 0;
      for (size_t i = first; line < h && i < upTo; i++) {
         const Object* itemObj = Vector_get(this->items, i);
         RichString_begin(item);
         Object_display(itemObj, &item);
         size_t itemLen = RichString_sizeVal(item);
         size_t amt = MINIMUM(itemLen - scrollH, this->w);
         if (highlightSelected && i == this->selected) {
            item.highlightAttr = selectionColor;
         }
         if (item.highlightAttr) {
            attrset(item.highlightAttr);
            RichString_setAttr(&item, item.highlightAttr);
            this->selectedLen = itemLen;
         }
         mvhline(y + CAST_INT(line), x, ' ', CAST_INT(this->w));
         if (amt > 0)
            RichString_printoffnVal(item, y + CAST_INT(line), x, scrollH, CAST_INT(amt));
         if (item.highlightAttr)
            attrset(CRT_colors[RESET_COLOR]);
         RichString_delete(&item);
         line++;
      }
      while (line < h) {
         mvhline(y + CAST_INT(line), x, ' ', CAST_INT(this->w));
         line++;
      }

   } else {
      const Object* oldObj = Vector_get(this->items, this->oldSelected);
      RichString_begin(old);
      Object_display(oldObj, &old);
      size_t oldLen = RichString_sizeVal(old);
      const Object* newObj = Vector_get(this->items, this->selected);
      RichString_begin(new);
      Object_display(newObj, &new);
      size_t newLen = RichString_sizeVal(new);
      this->selectedLen = newLen;
      mvhline(y + CAST_INT(this->oldSelected - first), x, ' ', CAST_INT(this->w));
      if (scrollH < oldLen)
         RichString_printoffnVal(old, y + CAST_INT(this->oldSelected - first), x,
            scrollH, CAST_INT(MINIMUM(oldLen - scrollH, this->w)));
      attrset(selectionColor);
      mvhline(y + CAST_INT(this->selected - first), x, ' ', CAST_INT(this->w));
      RichString_setAttr(&new, selectionColor);
      if (scrollH < newLen)
         RichString_printoffnVal(new, y + CAST_INT(this->selected - first), x,
            scrollH, CAST_INT(MINIMUM(newLen - scrollH, this->w)));
      attrset(CRT_colors[RESET_COLOR]);
      RichString_delete(&new);
      RichString_delete(&old);
   }

   if (focus && (this->needsRedraw || force_redraw || !this->wasFocus)) {
      if (Panel_drawFunctionBarFn(this))
         Panel_drawFunctionBar(this, hideFunctionBar);
      else if (!hideFunctionBar)
         FunctionBar_draw(this->currentBar);
   }

   this->oldSelected = this->selected;
   this->wasFocus = focus;
   this->needsRedraw = false;
   move(0, 0);
}

static unsigned int Panel_headerHeight(const Panel* this) {
   return RichString_sizeVal(this->header) > 0 ? 1 : 0;
}

bool Panel_onKey(Panel* this, int key) {
   assert (this != NULL);

   const size_t size = Vector_size(this->items);

   #define PANEL_SCROLLDOWN(amount_)                                                                                 \
   do {                                                                                                              \
      this->selected = this->selected + (amount_);                                                                   \
      this->scrollV = MINIMUM(this->scrollV + (amount_), (size > this->h + Panel_headerHeight(this)) ? (size - this->h - Panel_headerHeight(this)) : 0);  \
      this->needsRedraw = true;                                                                                      \
   } while (0)

   #define PANEL_SCROLLUP(amount_)                                                            \
   do {                                                                                       \
      this->selected = (this->selected > (amount_)) ? (this->selected - (amount_)) : 0;       \
      this->scrollV = MINIMUM((this->scrollV > (amount_)) ? (this->scrollV - (amount_)) : 0,  \
                              (size > this->h + Panel_headerHeight(this)) ? (size - this->h - Panel_headerHeight(this)) : 0);  \
      this->needsRedraw = true;                                                               \
   } while (0)

   switch (key) {
   case KEY_DOWN:
   case KEY_CTRL('N'):
   #ifdef KEY_C_DOWN
   case KEY_C_DOWN:
   #endif
      this->selected++;
      break;

   case KEY_UP:
   case KEY_CTRL('P'):
   #ifdef KEY_C_UP
   case KEY_C_UP:
   #endif
      if (this->selected > 0)
         this->selected--;
      break;

   case KEY_LEFT:
   case KEY_CTRL('B'):
      if (this->scrollH > 0) {
         this->scrollH -= MAXIMUM(CRT_scrollHAmount, 0);
         this->needsRedraw = true;
      }
      break;

   case KEY_RIGHT:
   case KEY_CTRL('F'):
      this->scrollH += CRT_scrollHAmount;
      this->needsRedraw = true;
      break;

   case KEY_PPAGE:
      PANEL_SCROLLUP(this->h - Panel_headerHeight(this));
      break;

   case KEY_NPAGE:
      PANEL_SCROLLDOWN(this->h - Panel_headerHeight(this));
      break;

   case KEY_WHEELUP:
      PANEL_SCROLLUP(CRT_scrollWheelVAmount);
      break;

   case KEY_WHEELDOWN:
      PANEL_SCROLLDOWN(CRT_scrollWheelVAmount);
      break;

   case KEY_HOME:
      this->selected = 0;
      break;

   case KEY_END:
      this->selected = (size > 0) ? (size - 1) : 0;
      break;

   case KEY_CTRL('A'):
   case '^':
      this->scrollH = 0;
      this->needsRedraw = true;
      break;
   case KEY_CTRL('E'):
   case '$':
      this->scrollH = MAXIMUM(this->selectedLen - this->w, 0);
      this->needsRedraw = true;
      break;
   default:
      return false;
   }

   #undef PANEL_SCROLLDOWN
   #undef PANEL_SCROLLUP

   // ensure selection within bounds
   if (size == 0) {
      this->selected = 0;
      this->needsRedraw = true;
   } else if (this->selected >= size) {
      this->selected = size - 1;
      this->needsRedraw = true;
   }

   return true;
}


HandlerResult Panel_selectByTyping(Panel* this, int ch) {
   size_t size = Panel_size(this);

   if (!this->eventHandlerState)
      this->eventHandlerState = xCalloc(100, sizeof(char));
   char* buffer = this->eventHandlerState;

   if (0 < ch && ch < 255 && isgraph((unsigned char)ch)) {
      size_t len = strlen(buffer);
      if (!len) {
         if ('/' == ch) {
            ch = '\001';
         } else if ('q' == ch) {
            return BREAK_LOOP;
         }
      } else if (1 == len && '\001' == buffer[0]) {
         len--;
      }

      if (len < 99) {
         buffer[len] = (char)ch;
         buffer[len+1] = '\0';
      }

      for (int try = 0; try < 2; try++) {
         len = strlen(buffer);
         for (size_t i = 0; i < size; i++) {
            const char* cur = ((ListItem*) Panel_get(this, i))->value;
            while (*cur == ' ') cur++;
            if (strncasecmp(cur, buffer, len) == 0) {
               Panel_setSelected(this, i);
               return HANDLED;
            }
         }

         // if current word did not match,
         // retry considering the character the start of a new word.
         buffer[0] = (char)ch;
         buffer[1] = '\0';
      }

      return HANDLED;
   } else if (ch != ERR) {
      buffer[0] = '\0';
   }

   if (ch == 13) {
      return BREAK_LOOP;
   }

   return IGNORED;
}
