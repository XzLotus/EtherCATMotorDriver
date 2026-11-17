/* Adapted reading excerpt from the project's SCI RX interrupt.
 * Device SDK and board setup are supplied separately.
 * demo_rx_capture must be a bounded, ISR-safe byte capture routine.
 * The foreground owns framing and calls demo_modbus_handle on complete frames.
 * Host builds use direct byte arrays instead of this hardware excerpt. */
#include <stdint.h>
#include "f28x_project.h"

extern void demo_rx_capture(uint8_t byte);

__interrupt void sciaRxIsr(void)
{
    uint8_t byte = (uint8_t)SciaRegs.SCIRXBUF.bit.SAR;
    demo_rx_capture(byte);
    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1U;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}
