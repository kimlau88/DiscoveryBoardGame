// Kim LAU
#include <stm32f3_discovery.h>
#include <stm32f3_discovery_lsm303dlhc.h>
// initializer to set up the board with nassaory variables
int b = 1;    // start with top LED (GPIOE9)
const int max_loop = 7; // max num in a sequence
int current_loop = 3;   // min num in a sequence
volatile unsigned sysTiming;
volatile unsigned sysTicks = 0;
const unsigned sequence[4] = {512, 2048, 8192, 32768}; // fixed sequence for selection
volatile unsigned current_sequence[max_loop];
volatile unsigned break_sequence[max_loop];

// set up for time ticking
void SysTick_Handler(void) {
    sysTicks++;
    if (sysTiming > 0) --sysTiming; 
}
void sysDelayMs(unsigned dly) {
    sysTiming = dly;
    while (sysTiming > 0) __wfi();
}
inline unsigned millis(void){
    return sysTicks;
}

// step 1, initialise the program for the device
void initial_status(void);

// step 2, once blue button pressed, generate a random sequence
// random value implemented by time as when button pressed, time may vary from each time
void randomSequence(void);
void breakSequence(void);

// step 3, display the new sequence from 3 LEDs
// if detection of correct sequence, return to step 2 and increase number of LEDs (current_loop)
// if incorrect LED detected, return to initial status
void display_sequence(void);
void detectSequence(void);

// set-up accelerometer for detection
void AccelerometerConfig(void);
void ReadAccelerometer(int16_t *);

// final step once a sequence of 7 LEDs completed, flash all LEDs by 200ms unless the blue button is pressed
void Congrats(void);

int main(void) {
    SysTick_Config((SystemCoreClock / 1000));
    RCC->AHBENR |= RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOA; // enable clocks for GPIOA and GPIOE
    GPIOE->MODER = (GPIOE->MODER & 0xffff) | 0x55550000;    // output mode for PE8-15 (LEDs 0-7)
    GPIOA->MODER = (GPIOA->MODER & 0xfffffffc)  ;           // input mode for PA0 (User Button)
    GPIOE->ODR &= 0x00ff; // switch all LEDs off by clearing bits 8..15
    
    
    initial_status();
}

// function implementaions
void randomSequence(void){
    if(current_loop == max_loop + 1) Congrats();
    b = 0x00ff;
    GPIOE->ODR&=b;
    for(int i = 0; i < current_loop; i ++){
        breakSequence();
        unsigned tmp = (((millis()*b^b))*(millis()/b*(millis())))/167;
        current_sequence[i] = break_sequence[tmp%8];
        sysDelayMs((millis()*i+1)%59);
    }
    display_sequence();
}

// to reallocate the sequence in a new set of collection
void breakSequence(void){
        for(int i = 0; i < 8; i ++){
            unsigned tmp = ((((millis()*b^b))*(millis()/b*(millis())))/101);
            break_sequence[i] = sequence[tmp%4];
            sysDelayMs((millis()*i+1)%53);
        }
}
void initial_status(void){
    for(int i = 0; i < 8; i++){
        current_sequence[i] = 0;
        break_sequence[i] = 0;
    }
    breakSequence();
    current_loop = 3;
    while(1) {
        GPIOE->ODR |= 1<<(b+8);     // switch on LED b by setting bit 8+b
        sysDelayMs(80);
        GPIOE->ODR&=0x00ff;         // switch all LEDs off
        b=(b+1)%8;                  // move on to next LED - %8 to make sure its range not exceed 8 (0 - 7)
        while(GPIOA->IDR & 1){      // wait while button is pressed
            randomSequence();
        }
    }
}

void display_sequence(void){
    for(int display_i = 0; display_i < current_loop; display_i ++){
        //printf("%d ", current_sequence[display_i]);
        GPIOE->ODR&=0x00ff;         // switch all LEDs off
        sysDelayMs(500);
        GPIOE->ODR|=current_sequence[display_i];
        sysDelayMs(500);
    }
    GPIOE->ODR&=0x00ff;
    detectSequence();
}


void AccelerometerConfig(void){
    LSM303DLHCMag_InitTypeDef LSM303DLHC_InitStructure;
    LSM303DLHCAcc_InitTypeDef LSM303DLHCAcc_InitStructure;
    LSM303DLHCAcc_FilterConfigTypeDef LSM303DLHCFilter_InitStructure;

    /* Configure MEMS magnetometer main parameters: temp, working mode, full Scale and Data rate */
    LSM303DLHC_InitStructure.Temperature_Sensor = LSM303DLHC_TEMPSENSOR_DISABLE;
    LSM303DLHC_InitStructure.MagOutput_DataRate =LSM303DLHC_ODR_30_HZ ;
    LSM303DLHC_InitStructure.MagFull_Scale = LSM303DLHC_FS_8_1_GA;
    LSM303DLHC_InitStructure.Working_Mode = LSM303DLHC_CONTINUOS_CONVERSION;
    LSM303DLHC_MagInit(&LSM303DLHC_InitStructure);

    /* Fill the accelerometer structure */
    LSM303DLHCAcc_InitStructure.Power_Mode = LSM303DLHC_NORMAL_MODE;
    LSM303DLHCAcc_InitStructure.AccOutput_DataRate = LSM303DLHC_ODR_50_HZ;
    LSM303DLHCAcc_InitStructure.Axes_Enable= LSM303DLHC_AXES_ENABLE;
    LSM303DLHCAcc_InitStructure.AccFull_Scale = LSM303DLHC_FULLSCALE_2G;
    LSM303DLHCAcc_InitStructure.BlockData_Update = LSM303DLHC_BlockUpdate_Continous;
    LSM303DLHCAcc_InitStructure.Endianness=LSM303DLHC_BLE_LSB;
    LSM303DLHCAcc_InitStructure.High_Resolution=LSM303DLHC_HR_ENABLE;
    /* Configure the accelerometer main parameters */
    LSM303DLHC_AccInit(&LSM303DLHCAcc_InitStructure);

    /* Fill the accelerometer LPF structure */
    LSM303DLHCFilter_InitStructure.HighPassFilter_Mode_Selection =LSM303DLHC_HPM_NORMAL_MODE;
    LSM303DLHCFilter_InitStructure.HighPassFilter_CutOff_Frequency = LSM303DLHC_HPFCF_16;
    LSM303DLHCFilter_InitStructure.HighPassFilter_AOI1 = LSM303DLHC_HPF_AOI1_DISABLE;
    LSM303DLHCFilter_InitStructure.HighPassFilter_AOI2 = LSM303DLHC_HPF_AOI2_DISABLE;

    /* Configure the accelerometer LPF main parameters */
    LSM303DLHC_AccFilterConfig(&LSM303DLHCFilter_InitStructure);
}
void ReadAccelerometer(int16_t * data) {  
  LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_L_A, (uint8_t *)data, 6);
}
void detectSequence(void){
    /* output for testing
    printf("Total: %d, ");
    for(int i = 0; i < current_loop; i++){
        printf("%d: %d  ->", i+1, current_sequence[i]);
    }
    */
    AccelerometerConfig();
    int16_t acc[2];
    int bb;
    int XY=200*16;
    for(int i = 0; i < current_loop; i ++){
        bb=0x00ff;
        while(1) {
            GPIOE->ODR&=bb;
            // delay as for waiting detection sensitivity
            sysDelayMs(50);
            ReadAccelerometer(acc);
            // tip towards North (LD3)
            if((acc[0]>XY) && (acc[1]<XY) && (acc[1]>-XY)) bb=sequence[0];
            // tip towards East (LD7)
            if((acc[0]>-XY) && (acc[0]<XY) && (acc[1]<-XY)) bb=sequence[1];
            // tip towards South (LD10)
            if((acc[0]<-XY) && (acc[1]<XY) && (acc[1]<XY)) bb=sequence[2];
            // tip towards West (LD6)
            if((acc[0]>-XY) && (acc[0]<XY) && (acc[1]>XY)) bb=sequence[3];
            GPIOE->ODR|=bb;
            sysDelayMs(500);
            GPIOE->ODR&=0x00ff;
            if(bb != 255){
                if((bb == current_sequence[i])){
                    bb = 0x00ff;
                    // output for testing
                    // printf("hello %d vs %d\n", bb, current_sequence[i]);
                    if(i == (current_loop -1)){
                        // when a sequence has been completed correctly, increase number of LEDs in the next sequence
                        current_loop += 1;
                        randomSequence();
                    } else {
                        bb = 0x00ff;
                        break;
                    }
                } else {
                    // printf("WRONG: %d vs %d\n", bb, current_sequence[i]);
                    GPIOE->ODR|=current_sequence[i];
                    sysDelayMs(500);
                    GPIOE->ODR&=0x00ff;
                    bb = 0x00ff;
                    printf("]  You DID NOT MADE IT!  [\n!TYR AGAIN!\n\n");
                    initial_status();
                    break;
                }
            }
        }
    }
}
void Congrats(void){
    GPIOE->ODR &= 0x00ff;
    printf("\n****** YOU MADE IT! ******\n");
    while(1){
        /*
        // using multiple codes to increase sensitivity to triger initial_status()
        while(GPIOA->IDR & 1){
            initial_status();
        }
        */
        GPIOE->ODR |= 0xffff;
        while(GPIOA->IDR & 1){
            initial_status();
        }
        sysDelayMs(200);
        while(GPIOA->IDR & 1){
            initial_status();
        }
        GPIOE->ODR &= 0x00ff;
        while(GPIOA->IDR & 1){
            initial_status();
        }
        sysDelayMs(200);
        while(GPIOA->IDR & 1){
            initial_status();
        }
    }
}
