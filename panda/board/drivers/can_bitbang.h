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
    REGISTER_INTERRUPT(EXTI15_10_IRQn, EXTI15_10_IRQ_Handler, //Figure out Call Rate/Max Call Rate, //Figure out Fault Rate);
    //register_set_bits() probably needed to init the interrupts?
    //Figure out what EXTI->IMR, EXTI->RTSR, EXTI->FTSR, and SYSCFG->EXTICR[0] are

    NVIC_EnableIRQ(EXTI15_10_IRQn);
};

