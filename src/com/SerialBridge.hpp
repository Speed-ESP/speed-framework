#ifndef __SERIALBRIDGE_H__
#define __SERIALBRIDGE_H__

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>
#include <core/property.hpp>
namespace Speed::Communication
{
    using namespace speed::core;
    template <class TASerialStream, class TBSerialStream>
    class SerialBridge
    {
    private:
        TASerialStream *m_pSerialStreamA;
        TBSerialStream *m_pSerialStreamB;

    public:
        SerialBridge(TASerialStream *pSerialStreamA, TBSerialStream *pSerialStreamB)
        {
            m_pSerialStreamA = pSerialStreamA;
            m_pSerialStreamB = pSerialStreamB;
        }

        void Bridge(int bridgeDelay = 0)
        {
            while (true)
            {
                int available = m_pSerialStreamA->available();
                if (available)
                {
                    for (int i = 0; i < available; i++)
                    {
                        m_pSerialStreamB->write(m_pSerialStreamA->read());
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(bridgeDelay));
                int available = m_pSerialStreamB->available();
                if (available)
                {
                    for (int i = 0; i < available; i++)
                    {
                        m_pSerialStreamA->write(m_pSerialStreamB->read());
                    }
                }
            }
        }
    };

    // CrossSerialStream cross serial stream class is used to bridge two serial ports together
    // where the written to the cross serial stream is write to the first serial port and the read from the cross serial stream is read from the second serial port
    template <class TASerialStream, class TBSerialStream>
    class CrossSerialStream
    {
    private:
        TASerialStream *m_pSerialStreamA;
        TBSerialStream *m_pSerialStreamB;

    public:
        CrossSerialStream(TASerialStream *pSerialStreamA, TBSerialStream *pSerialStreamB)
        {
            m_pSerialStreamA = pSerialStreamA;
            m_pSerialStreamB = pSerialStreamB;
        }

        void write(uint8_t data)
        {
            m_pSerialStreamA->write(data);
        }

        uint8_t read()
        {
            return m_pSerialStreamB->read();
        }

        int available()
        {
            return m_pSerialStreamB->available();
        }
    };

    // SoftwareSerialStream class
    // SoftwareSerial class, this class is used to simulate a serial port using an internal buffer that is used to store the data
    // and is extended to the SoftwareSerialStream class that is used to read and write data from the buffer
    class SoftwareSerialStream
    {

    private:
        std::vector<uint8_t> m_out_buffer;
        uint8_t m_out_readOutIndex;
        uint8_t m_out_writeIndex;

    public:
        SoftwareSerialStream(uint8_t bufferSize = 2048) : m_out_buffer(bufferSize), m_out_readOutIndex(0), m_out_writeIndex(0)
        {
        }

        int available()
        {
            return (m_out_writeIndex - m_out_readOutIndex);
        }

        void write(uint8_t data)
        {
            if (m_out_writeIndex < m_out_buffer.size())
            {
                m_out_buffer[m_out_writeIndex++] = data;
            }
            else
            {
                // Buffer overflow, clear the buffer
                m_out_readOutIndex = 0;
                m_out_writeIndex = 0;
            }
        }

        uint8_t read()
        {
            if (m_out_readOutIndex < m_out_writeIndex)
            {
                uint8_t data = m_out_buffer[m_out_readOutIndex++];
                if (m_out_readOutIndex == m_out_writeIndex)
                {
                    m_out_readOutIndex = 0;
                    m_out_writeIndex = 0;
                }
                return data;
            }
            return 0;
        }
    };

    class SoftwareSerial
    {
    private:
        SoftwareSerialStream m_outBuffer;
        SoftwareSerialStream m_inBuffer;

    public:
        SoftwareSerial(uint8_t inBuffer = 2048, int outBuffer) : m_inBuffer(inBuffer), m_outBuffer(outBuffer)
        {
        }

        ReadOnlyProperty<SoftwareSerialStream> InBuffer = ReadOnlyProperty<SoftwareSerialStream>(
            [this]() -> SoftwareSerialStream
            {
                return m_inBuffer;
            });

        ReadOnlyProperty<SoftwareSerialStream> OutBuffer = ReadOnlyProperty<SoftwareSerialStream>(
            [this]() -> SoftwareSerialStream
            {
                return m_outBuffer;
            });
    };

} // namespace Speed::Communication

#endif // __SERIALBRIDGE_H__