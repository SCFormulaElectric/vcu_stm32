#include "motor_control.h"

bool fault_codes_has_error(const fault_codes_t *faults) {
    return  (faults->INV_Post_Fault_Lo != 0) ||
            (faults->INV_Post_Fault_Hi != 0) ||
            (faults->INV_Run_Fault_Lo  != 0) ||
            (faults->INV_Run_Fault_Hi  != 0);
}
