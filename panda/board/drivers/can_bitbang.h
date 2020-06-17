#include <iostream>

void calculate_bitbang(void) {
    //This will be called in main.c at 1Hz
    //Do the message reconstruction here
    //Write the final message to a variable that we can send over to the EON over USB

};

void bitbang_IRQ_Handler(void) {
if(isReady){
    register_set(&(TIM5->CR), 0x0000, 0xFFFF)
};
bitbangBuff.pushElement(&(TIM5->CR));
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

    //Setup Timer
    register_set(&(TIM5->PSC), (12-1), 0xFFFFU); //Runs TIM5 at 1MHz (12MHz APB1 Timer Clock / 12 Prescalar)
    register_set(&(TIM5->CR1), TIM_CR1_CEN, 0x3FU); //Enable Counter

    bool isReady;

    circbuf bitbangBuff;
};

//class for making circular buffer
class circbuf {
    public:
    explicit circbuf(size_t size = 100):      //constructor init head and tail indexes and buffer specs
    maxbSize(size),
    bufferArray(new size_t[size]),
    head(0),
    tail(0),
    bufferFull(0)
    {}
    ~circbuf()                               //destructor to destroy buffer
    {delete[] bufferArray;}
    size_t popElement(){                     //pops element (FIFO)
        if ((head == tail) && (bufferFull != 1)) {
            std::cout << "Nothing to pop. Buffer is empty \n";
            return 0;
        }else{
            size_t data = bufferArray[tail];
            std::cout << "The value popped from " << tail << " is " << data << "\n";
            tail++;
            if (tail == maxbSize + 1) {
                tail = 0;
            }
            return data;
        }
    };
    void pushElement(size_t data){               //pushes data into buffer (FIFO)
        bufferArray[head] = data;
        std::cout << "The value at index " << head << " is " << (bufferArray[head]) << "\n";
        head++;
        if (head == maxbSize + 1) {
            head = 0;
        }
        if ((head+1 == tail) || (head-100 == tail)){
            bufferFull = 1;
            std::cout << "Warning: You may overwrite data\n";
        }
    };
    private:
    const size_t maxbSize;
    size_t* bufferArray;
    size_t head;
    size_t tail;
    bool bufferFull;
};


