void calculate_bitbang(void) {
    //This will be called in main.c at 1Hz
    //Do the message reconstruction here
    //Write the final message to a variable that we can send over to the EON over USB
};

void bitbang_IRQ_Handler(void) {
//Interrupt Handler Code Here
//Probably just write to the ring buffer?
EXTI->PR = (1U << BITBANG_CAN_PIN);
};


void can_bitbang_init(void) {
    if(car_harness_status == HARNESS_STATUS_NORMAL) {
        REGISTER_INTERRUPT(EXTI15_10_IRQn, bitbang_IRQ_Handler, 600000U, FAULT_INTERRUPT_RATE_BITBANG_CAN);
        register_set(&(SYSCFG->EXTICR[3]), SYSCFG_EXTICR4_EXTI12_PB, 0xF);
        register_set_bits(&(EXTI->IMR), (1U << 12));
        register_set_bits(&(EXTI->RTSR), (1U << 12));
        register_set_bits(&(EXTI->FTSR), (1U << 12));
        NVIC_EnableIRQ(EXTI15_10_IRQn);
        #define BITBANG_CAN_PIN 12
    }
    else if(car_harness_status == HARNESS_STATUS_FLIPPED) {
        REGISTER_INTERRUPT(EXTI9_5_IRQn, bitbang_IRQ_Handler, 600000U, FAULT_INTERRUPT_RATE_BITBANG_CAN);
        register_set(&(SYSCFG->EXTICR[1]), SYSCFG_EXTICR2_EXTI5_PB, 0xF0)
        register_set_bits(&(EXTI->IMR), (1U << 5));
        register_set_bits(&(EXTI->RTSR), (1U << 5));
        register_set_bits(&(EXTI->FTSR), (1U << 5));
        NVIC_EnableIRQ(EXTI9_5_IRQn);
        #define BITBANG_CAN_PIN 5
    }
    else {
        puts("Bitbang CAN init failed!\n")
    }
};

