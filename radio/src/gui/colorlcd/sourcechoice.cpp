/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "libopenui_config.h"
#include "sourcechoice.h"
#include "menutoolbar.h"
#include "menu.h"
#include "dataconstants.h"
#include "lcd.h"
#include "strhelpers.h"
#include "draw_functions.h"
#include "opentx.h"

class SourceChoiceMenuToolbar : public MenuToolbar<SourceChoice>
{
  public:
    SourceChoiceMenuToolbar(SourceChoice * choice, Menu * menu):
      MenuToolbar<SourceChoice>(choice, menu)
    {
      addButton(CHAR_INPUT, MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT);
#if defined(LUA_MODEL_SCRIPTS)
      addButton(CHAR_LUA, MIXSRC_LAST_LUA, MIXSRC_FIRST_LUA);
#endif
      addButton(CHAR_STICK, MIXSRC_FIRST_STICK, MIXSRC_LAST_STICK);
      addButton(CHAR_POT, MIXSRC_FIRST_POT, MIXSRC_LAST_POT);
      addButton(CHAR_FUNCTION, MIXSRC_MAX, MIXSRC_MAX);
#if defined(HELI)
      addButton(CHAR_CYC, MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI);
#endif
      addButton(CHAR_TRIM, MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM);
      addButton(CHAR_SWITCH, MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH);
      addButton(CHAR_TRAINER, MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER);
      addButton(CHAR_CHANNEL, MIXSRC_FIRST_CH, MIXSRC_LAST_CH);
#if defined(GVARS)
      addButton(CHAR_SLIDER, MIXSRC_LAST_GVAR, MIXSRC_FIRST_GVAR);
#endif
      addButton(CHAR_TELEMETRY, MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM);
    }
};

// defined in gui/gui_common.cpp
uint8_t switchToMix(uint8_t source);

SourceChoice::SourceChoice(FormGroup *parent, const rect_t &rect, int16_t vmin,
                           int16_t vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue,
                           WindowFlags windowFlags, LcdFlags textFlags) :
    ChoiceEx(parent, rect, vmin, vmax, getValue, setValue)
{
  setBeforeDisplayMenuHandler([=](Menu *menu) {
    menu->setToolbar(new SourceChoiceMenuToolbar(this, menu));
#if defined(AUTOSOURCE)
    menu->setWaitHandler([=]() {
      int16_t val = getMovedSource(vmin);
      if (val) {
        fillMenu(menu, val);
      }
#if defined(AUTOSWITCH)
      else {
        swsrc_t swtch = abs(getMovedSwitch());
        if (swtch && !IS_SWITCH_MULTIPOS(swtch)) {
          val = switchToMix(swtch);
          if (val && (val >= vmin) && (val <= vmax)) {
            fillMenu(menu, val);
          }
        }
      }
#endif
    });
#endif
  });

  setTextHandler([=](int value) {
    if (isValueAvailable && !isValueAvailable(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSourceString(value));
  });

  setAvailableHandler([](int v){ return isSourceAvailable(v); });
}

void SourceChoice::fillMenu(Menu *menu, int16_t value, const FilterFct& filter)
{
  int count = 0;
  int current = 0;

  menu->removeLines();
  for (int i = vmin; i <= vmax; ++i) {
    if (filter && !filter(i)) continue;
    if (isValueAvailable && !isValueAvailable(i)) continue;
    menu->addLine(getSourceString(i), [=]() { setValue(i); });
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }
}
