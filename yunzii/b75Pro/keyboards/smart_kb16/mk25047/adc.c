#include "adc.h"
#include "quantum.h"


#define REG32(addr)                     (*(volatile uint32_t *)(uint32_t)(addr))
#define BITS(start, end)                ((0xFFFFFFFFUL << (start)) & (0xFFFFFFFFUL >> (31U - (uint32_t)(end))))

#define APB2_BUS_BASE                   ((uint32_t)0x40010000U)        /*!< apb2 base address                */
#define ADC_BASE                        (APB2_BUS_BASE + 0x00002400U)  /*!< ADC base address                 */
#define ADC0                            ADC_BASE
#define ADC_OVSAMPCTL(adcx)             REG32((adcx) + 0x80U)            /*!< ADC oversampling control register */
#define ADC_OVSAMPCTL_DRES              BITS(12,13)                      /*!< oversampling shift */


/* ADC resolution */
#define OVSAMPCTL_DRES(regval)           (BITS(12,13) & ((uint32_t)(regval) << 12))  /*!< write value to ADC_OVSAMPCTL_DRES bit field */
#define ADC_RESOLUTION_12B               OVSAMPCTL_DRES(0)                           /*!< 12-bit ADC resolution */
#define ADC_RESOLUTION_10B               OVSAMPCTL_DRES(1)                           /*!< 10-bit ADC resolution */
#define ADC_RESOLUTION_8B                OVSAMPCTL_DRES(2)                           /*!< 8-bit ADC resolution */
#define ADC_RESOLUTION_6B                OVSAMPCTL_DRES(3)                           /*!< 6-bit ADC resolution */

#define N 10

void adc_init(void)
{

    /* ADC时钟来自APB2,即PCLK2,频率为72Mhz, ADC最大时钟一般不要超过14M
    *在72M主频条件下，我们使用8分频，得到PCLK2 / 8 = 9Mhz 的ADC时钟
    */

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // RCC->APB2RSTR |= 3 << 9;
    // RCC->APB2RSTR &= ~(3 << 9);

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;     //开GPIOB时钟
    palSetLineMode(B1, PAL_MODE_INPUT_ANALOG);


    ADC1->CR2 |= (1 << 23);

    RCC->CFGR &= ~(3 << 14);    // ADCPRE[1:0] ADC时钟预分频清零
    RCC->CFGR |= 3 << 14;       /*设置ADC时钟预分频系数为8，即PCLK2 /8=9Mhz*/

    ADC1->CR1 &= ~(0XF << 16);  /*工作模式清零 */
    ADC1->CR1 |= 0 << 16;       /* 独立工作模式 */
    ADC1->CR1 &= ~(1 << 8);     /* 非扫描模式 */
    ADC_OVSAMPCTL(ADC0) &= ~((uint32_t)ADC_OVSAMPCTL_DRES);
    ADC_OVSAMPCTL(ADC0) |= (uint32_t)ADC_RESOLUTION_10B;

    ADC1->CR2 &= ~(1 << 1);     /* 单次转换模式 */
    ADC1->CR2 &= ~(7 << 17);
    ADC1->CR2 |= 7 << 17;       /*软件控制转换*/
    ADC1->CR2 |= 1 << 20;       /* 使用用外部触发(SWSTART)！！！必须使用一个事件来触发*/
    ADC1->CR2 &= ~(1 << 11);    /* 右对齐 */

    ADC1->SQR1 &= ~(0XF << 20); /* L[3:0]清零*/
    ADC1->SQR1 |= 0 << 20;      /* 1个转换在规则序列中也就是只转换规则序列1 */

    ADC1->CR2 |= 1 << 0;        /* 开启AD转换器*/

    ADC1->CR2 |= 1 << 3;        /* 使能复位校准*/

    while (ADC1->CR2 & 1 << 3); /*等待校准结束*/

    /*该位由软件设置并由硬件清除 在校准寄存器被初始化后该位将被清除*/
    ADC1->CR2 |= 1 << 2;        /* 开启AD校准 */

    while (ADC1->CR2 & 1 << 2); /*等待校准结束*/
}

void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime)
{
    if (ch < 10)
    {
        adcx->SMPR2 &= ~(7 << (3 * ch));
        adcx->SMPR2 |= 7 << (3 * ch);
    }
    else
    {
        adcx->SMPR1 &= ~(7 << (3 * (ch - 10)));
        adcx->SMPR1 |= 7 << (3 * (ch - 10));
    }
}
/**
 * @brief   获得ADC转换后的结果
 * @param   ch：通道号，0~17
 * @retval  无
 * */
uint32_t adc_get_result(uint8_t ch)
{
    adc_channel_set(ADC1, ch, 7);   //设置ADCx对应通道采样时间为239.5个时钟周期

    ADC1->SQR3 &= ~(0X1F << 5 * 0); //规则序列1通道清零
    ADC1->SQR3 |= ch << (5 * 0);    // 规则序列1通道=ch
    ADC1->CR2 |= 1 << 22;           //启动规则转换通道

    while (!(ADC1->SR & 1 << 1));   //等待转换结束

    return ADC1->DR;                //返回adc值
}
/**
 *@brief  获取通道ch的转换值,取times次,然后平均
 *@param  ch：通道号，0~17
 *@param  times:获取次数
 *@retval 通道ch的times次转换结果平均值
 */
uint32_t adc_get_result_average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)
    {
        temp_val += adc_get_result(ch); /*获取times次数据*/
        wait_ms(5);
    }

    return temp_val / times;    /*返回平均值*/
}

uint16_t get_adc_value(void)
{
  
    uint16_t value_buf[N];
    uint16_t count,i,j,adc_temp;
    uint32_t sum=0;
    for(count = 0;count < N;count++) //获取采样值
    {
        value_buf[count] = adc_get_result(9);
    }
    for(j = 0;j<(N-1);j++)
    {
        for(i = 0;i<(N-j-1);i++)
        {
            if(value_buf[i]>value_buf[i+1])
            {
                adc_temp = value_buf[i];
                value_buf[i] = value_buf[i+1];
                value_buf[i+1] = adc_temp;
            }
        }
    }
    for(count = 1;count<N-1;count++)
    {
        sum += value_buf[count];
    }
    return sum/(N-2);
}

uint16_t get_adc_vref(void)
{
    uint16_t vref_buf[N];
    uint16_t counter,x,y,adc_1;
    uint32_t total=0;
    for(counter = 0;counter < N;counter++) //获取采样值
    {
        vref_buf[counter] = adc_get_result(17);
    }
    for(y = 0;y<(N-1);y++)
    {
        for(x = 0;x<(N-y-1);x++)
        {
            if(vref_buf[x]>vref_buf[x+1])
            {
                adc_1 = vref_buf[x];
                vref_buf[x] = vref_buf[x+1];
                vref_buf[x+1] = adc_1;
            }
        }
    }
    for(counter = 1;counter<N-1;counter++)
    {
        total += vref_buf[counter];
    }
    return total/(N-2);
}


