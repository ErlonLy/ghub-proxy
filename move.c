#include "move.h"
#include "ioctl.h"
#include "device.h"
#include <stdlib.h>

NTSTATUS Move_SendMoveCommand(char dx, char dy)
{
    NTSTATUS status;
    status = Ioctl_SendMove(dx, dy);
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
        return Move_SendMoveCommand((char)dx, (char)dy);
    }

    int steps = (max_dist + MAX_MOVE_STEP - 1) / MAX_MOVE_STEP;
    int curr_x = 0;
    int curr_y = 0;

    for (int i = 1; i <= steps; i++) {
        int target_x = (int)(((long long)dx * i) / steps);
        int target_y = (int)(((long long)dy * i) / steps);
        int step_dx = target_x - curr_x;
        int step_dy = target_y - curr_y;

        NTSTATUS status = Move_SendMoveCommand((char)step_dx, (char)step_dy);
        if (status != 0) {
            return status;
        }

        curr_x += step_dx;
        curr_y += step_dy;
    }

    return 0;
}
