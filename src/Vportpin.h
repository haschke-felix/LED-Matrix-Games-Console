#pragma once
#include "avr.h"

class PortPin
{
public:
    enum Port
    {
        B,
        C,
        D,
    };

    PortPin()
    {
    }
    PortPin(Port port, byte pin)
    {
        pin_ = pin;
        port_ = port;
    }

    void setPins(Port port, byte pin)
    {
        pin_ = pin;
        port_ = port;
    }

    static PortPin create(Port port, byte pin)
    {
        PortPin port_pin(port, pin);
        return port_pin;
    }

    void clear()
    {
        bitClear(*getPort(), pin_);
    }
    void set()
    {
        bitSet(*getPort(), pin_);
    }
    void write(bool value)
    {
        bitWrite(*getPort(), pin_, value);
    }
    bool read()
    {
        return bitRead(*getPIN(), pin_);
    }

    void output()
    {
        bitSet(*getDDR(), pin_);
    }
    void input()
    {
        bitClear(*getDDR(), pin_);
    }

private:
    Port port_;
    byte pin_;
    volatile byte *getPIN()
    {
        if (port_ == Port::B)
        {
            return &PINB;
        }
        else if (port_ == Port::C)
        {
            return &PINC;
        }
        else if (port_ == Port::D)
        {
            return &PIND;
        }
    }
    volatile byte *getPort()
    {
        if (port_ == Port::B)
        {
            return &PORTB;
        }
        else if (port_ == Port::C)
        {
            return &PORTC;
        }
        else if (port_ == Port::D)
        {
            return &PORTD;
        }
    }
    volatile byte *getDDR()
    {
        if (port_ == Port::B)
        {
            return &DDRB;
        }
        else if (port_ == Port::C)
        {
            return &DDRC;
        }
        else if (port_ == Port::D)
        {
            return &DDRD;
        }
    }
};
