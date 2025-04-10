#include "driver.h"
#include "grbl/protocol.h"

static bool uart_spindle_on = false;
static spindle_id_t spindle_id = -1;
io_stream_t *spindle_uart_stream;

static bool UARTSpindle_config (spindle_ptrs_t *spindle)
{
      return true;
}

static void UARTSpindle_update_rpm (spindle_ptrs_t *spindle, float rpm)
{
    for(uint8_t i = 0 ; i < 5; i++){
        spindle_uart_stream->write(ftoa(rpm,2));
        spindle_uart_stream->write("\r\n");
    }

    return;
}

static void UARTSpindle_set_state (spindle_ptrs_t *spindle, spindle_state_t state, float rpm){
    uart_spindle_on = state.on;
    UARTSpindle_update_rpm(spindle, rpm);
}

static spindle_state_t UARTSpindle_get_state (spindle_ptrs_t *spindle)
{
    spindle_state_t state = {settings.pwm_spindle.invert.mask};
    state.on = uart_spindle_on;
    state.value ^= settings.pwm_spindle.invert.mask;
    return state;
}

void driver_spindles_init (void)
{
    spindle_uart_stream = stream_open_instance(SPINDLE_SERIAL_PORT, 9600, NULL, "SpindleUART");
    
    static const spindle_ptrs_t spindle = {
        .type = SpindleType_PWM,
        .ref_id = SPINDLE_PWM0_NODIR,
        .config =       UARTSpindle_config,
        .set_state =    UARTSpindle_set_state,
        .get_state =    UARTSpindle_get_state,
        .update_rpm =   UARTSpindle_update_rpm,
        .cap = {
            .gpio_controlled = On,
            .variable = On,
            .laser = On,
            .pwm_invert = On,
        }
    };

    if((spindle_id = spindle_register(&spindle, "UART")) == -1)
        protocol_enqueue_foreground_task(report_warning, "UART spindle failed to initialize!");
}

bool aux_out_claim_explicit (aux_ctrl_out_t *aux_ctrl)
{
    return 0;
}