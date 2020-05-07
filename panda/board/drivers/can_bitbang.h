void set_bitbang_can_rx_pin(void) {
    if (/*Need to get orientation of OBD-C connector and change pin accordingly*/)
    {
        #define OBD_CAN_RX_PIN 12
    }
    else
    {
        #define OBD_CAN_RX_PIN 13
    }
}

void calculate_bitbang(void) {
    //This will be called in main.c at 1Hz
    //Do the message reconstruction here
    //Write the final message to a variable that we can send over to the EON over USB
};

void EXTI15_10_IRQ_Handler(void) {
//Interrupt Handler Code Here
//Probably just write to the ring buffer?
};


void can_bitbang_init(void) {
    REGISTER_INTERRUPT(EXTI15_10_IRQn, EXTI15_10_IRQ_Handler, 600000U, FAULT_INTERRUPT_RATE_BITBANG_CAN);
    if(OBD_CAN_RX_PIN == 12) {
        register_set(&(SYSCFG->EXTICR[3]), SYSCFG_EXTICR4_EXTI12_PB, /*CALCULATE MASK*/)
    }
    else {
        register_set(&(SYSCFG->EXTICR[3]), SYSCFG_EXTICR4_EXTI13_PB, /*CALCULATE MASK*/)
    }

    register_set_bits(&(EXTI->IMR), (1U << OBD_CAN_RX_PIN));
    register_set_bits(&(EXTI->RTSR), (1U << OBD_CAN_RX_PIN));
    register_set_bits(&(EXTI->FTSR), (1U << OBD_CAN_RX_PIN));

    NVIC_EnableIRQ(EXTI15_10_IRQn);
};

