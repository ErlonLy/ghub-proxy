#include "move.h"
#include "ioctl.h"
#include "device.h"
#include <stdlib.h>

NTSTATUS Move_SendMoveCommand(short dx, short dy)
{
    NTSTATUS status = Ioctl_SendMove(dx, dy);
    if (status != 0) {
        Device_Close();
        if (Device_Open()) {
            status = Ioctl_SendMove(dx, dy);
        }
    }
    return status;
}

NTSTATUS Move_MoveMouse(int dx, int dy)
{
    if (dx == 0 && dy == 0) {
        return 0;
    }

    int abs_x = abs(dx);
    int abs_y = abs(dy);
    int max_dist = (abs_x > abs_y) ? abs_x : abs_y;

    if (max_dist <= MAX_MOVE_STEP) {
        return Move_SendMoveCommand((short)dx, (short)dy);
    }

    int steps = (max_dist + MAX_MOVE_STEP - 1) / MAX_MOVE_STEP;
    int curr_x = 0;
    int curr_y = 0;

    for (int i = 1; i <= steps; i++) {
        int target_x = (int)(((long long)dx * i) / steps);
        int target_y = (int)(((long long)dy * i) / steps);
        int step_dx = target_x - curr_x;
        int step_dy = target_y - curr_y;

        NTSTATUS status = Move_SendMoveCommand((short)step_dx, (short)step_dy);
        if (status != 0) {
            return status;
        }

        curr_x += step_dx;
        curr_y += step_dy;
    }

    return 0;
}

NTSTATUS Move_MoveMouseAbsolute(int target_x, int target_y)
{
    POINT pt;
    if (!GetCursorPos(&pt)) {
        // If desktop is inaccessible (e.g. non-interactive service/thread), attach to winsta0\default
        HWINSTA hwinsta = OpenWindowStationW(L"winsta0", FALSE, MAXIMUM_ALLOWED);
        if (hwinsta) {
            SetProcessWindowStation(hwinsta);
            HDESK hdesk = OpenDesktopW(L"default", 0, FALSE, MAXIMUM_ALLOWED);
            if (hdesk) {
                SetThreadDesktop(hdesk);
            }
        }
        if (!GetCursorPos(&pt)) {
            return (NTSTATUS)0xC0000001L; // STATUS_UNSUCCESSFUL
        }
    }

    int dx = target_x - pt.x;
    int dy = target_y - pt.y;
    return Move_MoveMouse(dx, dy);
}
