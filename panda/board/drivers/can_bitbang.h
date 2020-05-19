void calculate_bitbang(void) {
    //This will be called in main.c at 1Hz
    //Do the message reconstruction here
    //Write the final message to a variable that we can send over to the EON over USB
};

void bitbang_IRQ_Handler(void) {
//Interrupt Handler Code Here
//Probably just write to the ring buffer?
};


void can_bitbang_init(void) {
    if(car_harness_status == HARNESS_STATUS_NORMAL) {
        REGISTER_INTERRUPT(EXTI15_10_IRQn, bitbang_IRQ_Handler, 600000U, FAULT_INTERRUPT_RATE_BITBANG_CAN);
        register_set(&(SYSCFG->EXTICR[3]), SYSCFG_EXTICR4_EXTI12_PB, /*CALCULATE MASK*/);
        register_set_bits(&(EXTI->IMR), (1U << 12));
        register_set_bits(&(EXTI->RTSR), (1U << 12));
        register_set_bits(&(EXTI->FTSR), (1U << 12));
        NVIC_EnableIRQ(EXTI15_10_IRQn);
    }
    else if(car_harness_status == HARNESS_STATUS_FLIPPED) {
        REGISTER_INTERRUPT(EXTI9_5_IRQn, bitbang_IRQ_Handler, 600000U, FAULT_INTERRUPT_RATE_BITBANG_CAN);
        register_set(&(SYSCFG->EXTICR[1]), SYSCFG_EXTICR2_EXTI5_PB, /*CALCULATE MASK*/)
        register_set_bits(&(EXTI->IMR), (1U << 5));
        register_set_bits(&(EXTI->RTSR), (1U << 5));
        register_set_bits(&(EXTI->FTSR), (1U << 5));
        NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
    else {
        puts("Bitbang CAN init failed!\n")
    }
};
/*
This is the circular buffer.
Struct is established, then buffer is initialized with preset size, then head and tail
are intialized.
*/
//initial structure
typedef struct circ_t {
    uint8_t *bitbangbuffer;
    bool bufferFull;
    bool bufferEmpty;
    uint16_t bufferHead;
    uint16_t bufferTail; 
    uint16_t maxBufferSize;
} circ;

circ circbuffer_init (circ_t *circ, int16_t maxBufferSize) {
    circ->bufferFull = 0;
    circ->bufferEmpty = 1;
    circ->bufferHead = 0;
    circ->bufferTail = 0;
    circ->maxBufferSize = maxBufferSize;
    circ->bitbangbuffer = (circ *) malloc(sizeof(circ)*maxBufferSize);
}
void pushBuffer(circ_t *circ,uint8_t *data) {
    if (bufferFull == 1) {
        std::cout << ("Buffer is Full\n");
    }
    else{
        circ->bitbangbuffer[circ->bufferHead] = *data;
        circ->bufferHead++;
        circ->bufferEmpty = 0;
        if ((circ->bufferHead-1) == circ->maxBufferSize) {
            circ->bufferFull = 1;
        }
    }
}
uint8_t  popBuffer(circ_t *circ) {
    uint8_t data;
    if (circ->bufferEmpty) {
        std::cout << ("Buffer is Empty\n");
        return 0;
    }
    else{
        data = circ->bitbangbuffer[circ->bufferTail];
        circ->bufferTail++;
        circ->bufferFull = 0;
        if ((circ->bufferHead == circ->bufferTail) && (circ->bufferFull !=1)) 
            {circ->bufferEmpty = 1;}
        else if (circ->bufferHead == (circ->bufferTail-1)) {
            circ->bufferHead = 0;
            circ->bufferTail = 0;
        }
        return data;
    }
}


