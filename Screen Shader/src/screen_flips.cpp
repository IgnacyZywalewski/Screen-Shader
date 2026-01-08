#include "screen_flips.h"

void setOrientation(DWORD orientation) {
    DEVMODE dm = {};
    dm.dmSize = sizeof(dm);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        return;

    if ((dm.dmDisplayOrientation + orientation) % 2 == 1)
        std::swap(dm.dmPelsWidth, dm.dmPelsHeight);

    dm.dmDisplayOrientation = orientation;
    dm.dmFields = DM_DISPLAYORIENTATION | DM_PELSWIDTH | DM_PELSHEIGHT;

    ChangeDisplaySettings(&dm, CDS_UPDATEREGISTRY);
}


void rotate90right() {
    setOrientation(DMDO_270);
}

void rotate90left() {
    setOrientation(DMDO_90);
}

void rotate180() {
    setOrientation(DMDO_180);
}

void normalScreen() {
    setOrientation(DMDO_DEFAULT);
}
