#ifndef _PWM_BASE_HPP_
#define _PWM_BASE_HPP_

class PWM {
    public:
        virtual void setPWM(uint8_t duty);
        virtual uint8_t getPWM(void);
        virtual void setFreq(uint32_t freq);
        virtual uint32_t getFreq(void);

    protected:
        uint32_t pwm_frequency;
        uint8_t pwm_duty;
};


#endif