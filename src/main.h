#ifndef PRESS_F_QT_MAIN_H
#define PRESS_F_QT_MAIN_H

#include <QString>

extern "C"
{
  #include "libpressf/src/hw/system.h"
}

extern QString g_BiosPath;
extern f8_system_t g_ChannelF;

#endif
